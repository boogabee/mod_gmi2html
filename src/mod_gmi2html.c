/*
 * mod_gmi2html - Apache module for serving Gemini files as HTML
 * 
 * Converts Gemini format (.gmi) documents to HTML on-the-fly
 * Based on Gemini specification v0.16.0
 */

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_request.h"
#include "ap_config.h"
#include "apr_strings.h"
#include "apr_file_io.h"
#include "apr_fnmatch.h"
#include <string.h>
#include <sys/stat.h>

#include "gemini_parser.h"

/* Forward declarations */
module AP_MODULE_DECLARE_DATA gmi2html_module;

/* Module-specific configuration */
typedef struct {
    int enabled;
    const char *gemini_type;
    const char *stylesheet_path;  /* Path to custom stylesheet file */
    const char *head_file_path;    /* Path to custom head content file */
} gmi2html_config;

/* Get module configuration */
static gmi2html_config *get_config(request_rec *r) {
    return (gmi2html_config *)ap_get_module_config(r->per_dir_config, 
                                                    &gmi2html_module);
}

/* Create per-directory configuration */
static void *create_dir_config(apr_pool_t *p, char *dir) {
    (void)dir;  /* Unused */
    gmi2html_config *cfg = apr_pcalloc(p, sizeof(gmi2html_config));
    cfg->enabled = 0;  /* Disabled by default */
    cfg->gemini_type = "text/gemini";
    cfg->stylesheet_path = NULL;  /* No custom stylesheet by default */
    cfg->head_file_path = NULL;    /* No custom head content by default */
    return cfg;
}

/* Merge per-directory configuration */
static void *merge_dir_config(apr_pool_t *p, void *base_conf, void *new_conf) {
    gmi2html_config *base = (gmi2html_config *)base_conf;
    gmi2html_config *new = (gmi2html_config *)new_conf;
    gmi2html_config *merged = apr_pcalloc(p, sizeof(gmi2html_config));
    
    merged->enabled = new->enabled ? new->enabled : base->enabled;
    merged->gemini_type = new->gemini_type ? new->gemini_type : base->gemini_type;
    merged->stylesheet_path = new->stylesheet_path ? new->stylesheet_path : base->stylesheet_path;
    merged->head_file_path = new->head_file_path ? new->head_file_path : base->head_file_path;
    
    return merged;
}

/* Configuration directive: Gmi2HtmlEnabled on|off */
static const char *set_gmi2html_enabled(cmd_parms *cmd, void *config, 
                                        const char *arg) {
    (void)cmd;  /* Unused */
    gmi2html_config *cfg = (gmi2html_config *)config;
    
    if (!strcasecmp(arg, "on")) {
        cfg->enabled = 1;
    } else if (!strcasecmp(arg, "off")) {
        cfg->enabled = 0;
    } else {
        return "Gmi2HtmlEnabled must be 'on' or 'off'";
    }
    
    return NULL;
}

/* Configuration directive: Gmi2HtmlStylesheet <path> */
static const char *set_gmi2html_stylesheet(cmd_parms *cmd, void *config, 
                                          const char *arg) {
    (void)cmd;  /* Unused */
    gmi2html_config *cfg = (gmi2html_config *)config;
    cfg->stylesheet_path = arg;
    return NULL;
}

/* Configuration directive: Gmi2HtmlHead <path> */
static const char *set_gmi2html_head(cmd_parms *cmd, void *config, 
                                    const char *arg) {
    (void)cmd;  /* Unused */
    gmi2html_config *cfg = (gmi2html_config *)config;
    cfg->head_file_path = arg;
    return NULL;
}

/* Configuration directives */
static const command_rec gmi2html_directives[] = {
    AP_INIT_TAKE1("Gmi2HtmlEnabled", 
                  set_gmi2html_enabled, 
                  NULL, 
                  OR_OPTIONS,
                  "Enable or disable Gemini to HTML conversion (on|off)"),
    AP_INIT_TAKE1("Gmi2HtmlStylesheet",
                  set_gmi2html_stylesheet,
                  NULL,
                  OR_OPTIONS,
                  "Path to custom CSS stylesheet file (optional)"),
    AP_INIT_TAKE1("Gmi2HtmlHead",
                  set_gmi2html_head,
                  NULL,
                  OR_OPTIONS,
                  "Path to custom <head> content file with meta tags, icons, etc. (optional)"),
    { NULL }
};

/* Handler for .gmi files */
static int gmi2html_handler(request_rec *r) {
    gmi2html_config *cfg = get_config(r);
    
    /* Only handle if enabled */
    if (!cfg->enabled) {
        return DECLINED;
    }
    
    /* Only handle .gmi files */
    if (strcmp(r->handler, "gmi2html") != 0 && 
        apr_fnmatch("*.gmi", r->filename, 0) != APR_SUCCESS) {
        return DECLINED;
    }
    
    /* Check if file exists and is readable */
    apr_finfo_t finfo;
    if (apr_stat(&finfo, r->filename, APR_FINFO_SIZE, r->pool) != APR_SUCCESS) {
        return HTTP_NOT_FOUND;
    }
    
    if (finfo.filetype != APR_REG) {
        return HTTP_NOT_FOUND;
    }
    
    /* Read the file */
    apr_file_t *file;
    apr_status_t status = apr_file_open(&file, r->filename, APR_READ, 
                                        APR_OS_DEFAULT, r->pool);
    if (status != APR_SUCCESS) {
        return HTTP_FORBIDDEN;
    }
    
    /* Allocate buffer for file content */
    char *content = apr_palloc(r->pool, finfo.size + 1);
    if (!content) {
        apr_file_close(file);
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    
    /* Read file content */
    apr_size_t bytes_read;
    status = apr_file_read_full(file, content, finfo.size, &bytes_read);
    apr_file_close(file);
    
    if (status != APR_SUCCESS || bytes_read != (apr_size_t)finfo.size) {
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    
    content[finfo.size] = '\0';
    
    /* Parse Gemini document */
    GeminiDocument *doc = gemini_parse(content, finfo.size);
    if (!doc) {
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    
    /* Extract title from filename */
    const char *filename = apr_pstrdup(r->pool, r->filename);
    char *title = apr_pstrdup(r->pool, filename);
    
    /* Get basename */
    char *slash = strrchr(title, '/');
    if (slash) {
        title = slash + 1;
    }
    
    /* Remove .gmi extension */
    char *dot = strrchr(title, '.');
    if (dot && !strcmp(dot, ".gmi")) {
        *dot = '\0';
    }
    
    /* Load custom stylesheet if configured */
    char *custom_stylesheet = NULL;
    if (cfg->stylesheet_path) {
        apr_file_t *style_file;
        apr_finfo_t style_finfo;
        
        /* Check if stylesheet file exists and is readable */
        if (apr_stat(&style_finfo, cfg->stylesheet_path, APR_FINFO_SIZE, r->pool) == APR_SUCCESS &&
            style_finfo.filetype == APR_REG) {
            
            /* Try to open and read the stylesheet */
            if (apr_file_open(&style_file, cfg->stylesheet_path, APR_READ, 
                            APR_OS_DEFAULT, r->pool) == APR_SUCCESS) {
                
                custom_stylesheet = apr_palloc(r->pool, style_finfo.size + 1);
                apr_size_t style_bytes_read;
                
                if (custom_stylesheet &&
                    apr_file_read_full(style_file, custom_stylesheet, style_finfo.size, 
                                     &style_bytes_read) == APR_SUCCESS &&
                    style_bytes_read == (apr_size_t)style_finfo.size) {
                    custom_stylesheet[style_finfo.size] = '\0';
                } else {
                    custom_stylesheet = NULL;  /* Failed to read, use default */
                }
                
                apr_file_close(style_file);
            }
        }
    }
    
    /* Load custom head content if configured */
    char *custom_head = NULL;
    if (cfg->head_file_path) {
        apr_file_t *head_file;
        apr_finfo_t head_finfo;
        
        /* Check if head file exists and is readable */
        if (apr_stat(&head_finfo, cfg->head_file_path, APR_FINFO_SIZE, r->pool) == APR_SUCCESS &&
            head_finfo.filetype == APR_REG) {
            
            /* Try to open and read the head file */
            if (apr_file_open(&head_file, cfg->head_file_path, APR_READ, 
                            APR_OS_DEFAULT, r->pool) == APR_SUCCESS) {
                
                custom_head = apr_palloc(r->pool, head_finfo.size + 1);
                apr_size_t head_bytes_read;
                
                if (custom_head &&
                    apr_file_read_full(head_file, custom_head, head_finfo.size, 
                                     &head_bytes_read) == APR_SUCCESS &&
                    head_bytes_read == (apr_size_t)head_finfo.size) {
                    custom_head[head_finfo.size] = '\0';
                } else {
                    custom_head = NULL;  /* Failed to read, skip custom head */
                }
                
                apr_file_close(head_file);
            }
        }
    }
    
    /* Convert to HTML with optional custom stylesheet and custom head content */
    char *html = gemini_to_html_with_stylesheet_and_head(doc, title, custom_stylesheet, custom_head);
    if (!html) {
        gemini_document_free(doc);
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    
    /* Set response headers */
    r->content_type = "text/html; charset=utf-8";
    ap_set_content_length(r, strlen(html));
    
    /* Send the HTML response */
    /* Note: ap_send_http_header is deprecated in Apache 2.4+
       Headers are sent automatically, just write the body */
    if (!r->header_only) {
        ap_rputs(html, r);
    }
    
    /* Cleanup */
    gemini_html_free(html);
    gemini_document_free(doc);
    
    return OK;
}

/* Register hooks */
static void gmi2html_register_hooks(apr_pool_t *p) {
    (void)p;  /* Unused */
    ap_hook_handler(gmi2html_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/* Module definition */
module AP_MODULE_DECLARE_DATA gmi2html_module = {
    STANDARD20_MODULE_STUFF,
    create_dir_config,    /* Per-directory config creator */
    merge_dir_config,     /* Per-directory config merger */
    NULL,                 /* Per-server config creator */
    NULL,                 /* Per-server config merger */
    gmi2html_directives,  /* Command table */
    gmi2html_register_hooks, /* Register hooks */
    0                     /* flags */
};
