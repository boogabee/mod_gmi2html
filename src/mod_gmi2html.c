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
    return cfg;
}

/* Merge per-directory configuration */
static void *merge_dir_config(apr_pool_t *p, void *base_conf, void *new_conf) {
    gmi2html_config *base = (gmi2html_config *)base_conf;
    gmi2html_config *new = (gmi2html_config *)new_conf;
    gmi2html_config *merged = apr_pcalloc(p, sizeof(gmi2html_config));
    
    merged->enabled = new->enabled ? new->enabled : base->enabled;
    merged->gemini_type = new->gemini_type ? new->gemini_type : base->gemini_type;
    
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

/* Configuration directives */
static const command_rec gmi2html_directives[] = {
    AP_INIT_TAKE1("Gmi2HtmlEnabled", 
                  set_gmi2html_enabled, 
                  NULL, 
                  OR_OPTIONS,
                  "Enable or disable Gemini to HTML conversion (on|off)"),
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
    
    /* Convert to HTML */
    char *html = gemini_to_html(doc, title);
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
