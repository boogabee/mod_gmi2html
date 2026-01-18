#include "gemini_parser.h"
#include <string.h>
#include <ctype.h>

/* Built-in CSS stylesheet */
static const char *BUILTIN_STYLESHEET = 
    "    body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif; line-height: 1.6; max-width: 900px; margin: 0 auto; padding: 20px; color: #333; }\n"
    "    h1 { font-size: 2em; margin: 0.5em 0; }\n"
    "    h2 { font-size: 1.5em; margin: 0.67em 0; }\n"
    "    h3 { font-size: 1.2em; margin: 0.83em 0; }\n"
    "    a { color: #0066cc; text-decoration: none; }\n"
    "    a:hover { text-decoration: underline; }\n"
    "    pre { background: #f4f4f4; padding: 15px; overflow-x: auto; border-radius: 4px; font-family: 'Courier New', monospace; }\n"
    "    blockquote { border-left: 4px solid #ddd; margin: 0; padding-left: 15px; color: #666; }\n"
    "    ul { margin: 1em 0; padding-left: 2em; }\n"
    "    li { margin: 0.5em 0; }\n"
    "    code { background: #f4f4f4; padding: 2px 6px; border-radius: 3px; font-family: 'Courier New', monospace; }\n"
    "    .gemini-link { display: block; margin: 0.5em 0; padding: 0.5em; background: #f9f9f9; border-left: 3px solid #0066cc; padding-left: 12px; }\n"
    "    .gemini-link a { font-weight: bold; }\n";

/* Forward declarations */
static char *convert_link_path(const char *url);
static char *process_inline_code(const char *text);
static char *process_inline_bold(const char *text);

/* Helper function to duplicate a string */
static char *strndup_safe(const char *src, size_t n) {
    char *dst = malloc(n + 1);
    if (dst) {
        memcpy(dst, src, n);
        dst[n] = '\0';
    }
    return dst;
}

/* Helper function to skip whitespace */
static const char *skip_whitespace(const char *p, const char *end) {
    while (p < end && (*p == ' ' || *p == '\t')) {
        p++;
    }
    return p;
}

/* Helper function to find end of line */
static const char *find_line_end(const char *p, const char *end) {
    while (p < end && *p != '\n' && *p != '\r') {
        p++;
    }
    return p;
}

/* Skip CRLF or LF */
static const char *skip_newline(const char *p, const char *end) {
    if (p < end && *p == '\r') p++;
    if (p < end && *p == '\n') p++;
    return p;
}

/* Parse a link line (=> URL [label]) */
static GeminiLink parse_link_line(const char *line, size_t len) {
    GeminiLink link = {0};
    const char *p = line;
    const char *end = line + len;
    
    /* Skip => */
    if (len >= 2 && line[0] == '=' && line[1] == '>') {
        p = line + 2;
    }
    
    /* Skip whitespace */
    p = skip_whitespace(p, end);
    
    /* Extract URL */
    const char *url_start = p;
    while (p < end && *p != ' ' && *p != '\t') {
        p++;
    }
    
    if (p > url_start) {
        char *raw_url = strndup_safe(url_start, p - url_start);
        link.url = convert_link_path(raw_url);
        free(raw_url);
    }
    
    /* Skip whitespace after URL */
    p = skip_whitespace(p, end);
    
    /* Extract label (rest of line) */
    if (p < end) {
        link.label = strndup_safe(p, end - p);
    }
    
    return link;
}

/* Escape HTML special characters */
static char *html_escape(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    size_t new_len = 0;
    
    /* Calculate required size */
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '<': new_len += 4; break;  /* &lt; */
            case '>': new_len += 4; break;  /* &gt; */
            case '&': new_len += 5; break;  /* &amp; */
            case '"': new_len += 6; break;  /* &quot; */
            case '\'': new_len += 5; break; /* &#39; */
            default: new_len++; break;
        }
    }
    
    char *escaped = malloc(new_len + 1);
    if (!escaped) return NULL;
    
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '<': memcpy(&escaped[pos], "&lt;", 4); pos += 4; break;
            case '>': memcpy(&escaped[pos], "&gt;", 4); pos += 4; break;
            case '&': memcpy(&escaped[pos], "&amp;", 5); pos += 5; break;
            case '"': memcpy(&escaped[pos], "&quot;", 6); pos += 6; break;
            case '\'': memcpy(&escaped[pos], "&#39;", 5); pos += 5; break;
            default: escaped[pos++] = str[i]; break;
        }
    }
    escaped[pos] = '\0';
    
    return escaped;
}

/* Return link path as-is (module serves .gmi files directly) */
static char *convert_link_path(const char *url) {
    if (!url) return NULL;
    
    size_t len = strlen(url);
    
    /* Return URL unchanged - the mod_gmi2html module serves .gmi files directly,
       so relative links to .gmi files will be processed by the module */
    return strndup_safe(url, len);
}

/* Process inline code (backtick) within text */
static char *process_inline_code(const char *text) {
    if (!text) return NULL;
    
    size_t len = strlen(text);
    size_t new_size = len * 2 + 100;
    char *result = malloc(new_size);
    if (!result) return NULL;
    
    size_t pos = 0;
    int in_code = 0;
    
    for (size_t i = 0; i < len; i++) {
        /* Check for ` */
        if (text[i] == '`') {
            if (!in_code) {
                /* Check opening ` needs space before (or start) */
                int space_before = (i == 0 || text[i-1] == ' ');
                if (space_before) {
                    memcpy(&result[pos], "<code>", 6);
                    pos += 6;
                    in_code = 1;
                    continue;
                }
            } else {
                /* Check closing ` needs space after (or end) */
                int space_after = (i+1 >= len || text[i+1] == ' ' ||
                                  text[i+1] == '.' || text[i+1] == ',' ||
                                  text[i+1] == '!' || text[i+1] == '?' ||
                                  text[i+1] == ';' || text[i+1] == ':');
                if (space_after) {
                    memcpy(&result[pos], "</code>", 7);
                    pos += 7;
                    in_code = 0;
                    continue;
                }
            }
        }
        
        result[pos++] = text[i];
        
        /* Expand buffer if needed */
        if (pos >= new_size - 20) {
            new_size *= 2;
            char *new_result = realloc(result, new_size);
            if (!new_result) {
                free(result);
                return NULL;
            }
            result = new_result;
        }
    }
    
    result[pos] = '\0';
    return result;
}

/* Process inline bold (**text**) within text */
static char *process_inline_bold(const char *text) {
    if (!text) return NULL;
    
    size_t len = strlen(text);
    size_t new_size = len * 2 + 100;
    char *result = malloc(new_size);
    if (!result) return NULL;
    
    size_t pos = 0;
    int in_bold = 0;
    
    for (size_t i = 0; i < len; i++) {
        /* Check for ** */
        if (i + 1 < len && text[i] == '*' && text[i+1] == '*') {
            if (!in_bold) {
                /* Check opening ** needs space before (or start) */
                int space_before = (i == 0 || text[i-1] == ' ');
                if (space_before) {
                    memcpy(&result[pos], "<strong>", 8);
                    pos += 8;
                    in_bold = 1;
                    i += 1;
                    continue;
                }
            } else {
                /* Check closing ** needs space after (or end) */
                int space_after = (i+2 >= len || text[i+2] == ' ' || 
                                  text[i+2] == '.' || text[i+2] == ',' ||
                                  text[i+2] == '!' || text[i+2] == '?' ||
                                  text[i+2] == ';' || text[i+2] == ':');
                if (space_after) {
                    memcpy(&result[pos], "</strong>", 9);
                    pos += 9;
                    in_bold = 0;
                    i += 1;
                    continue;
                }
            }
        }
        
        result[pos++] = text[i];
        
        /* Expand buffer if needed */
        if (pos >= new_size - 20) {
            new_size *= 2;
            char *new_result = realloc(result, new_size);
            if (!new_result) {
                free(result);
                return NULL;
            }
            result = new_result;
        }
    }
    
    result[pos] = '\0';
    return result;
}

/* Parse Gemini document */
GeminiDocument *gemini_parse(const char *content, size_t length) {
    GeminiDocument *doc = malloc(sizeof(GeminiDocument));
    if (!doc) return NULL;
    
    doc->lines = malloc(sizeof(GeminiLine) * 100);
    doc->line_count = 0;
    doc->capacity = 100;
    doc->page_title = NULL;
    
    if (!doc->lines) {
        free(doc);
        return NULL;
    }
    
    const char *p = content;
    const char *end = content + length;
    int in_preformat = 0;
    
    while (p < end) {
        const char *line_start = p;
        const char *line_end = find_line_end(p, end);
        size_t line_len = line_end - line_start;
        
        GeminiLine parsed_line = {0};
        
        /* Remove trailing whitespace */
        while (line_len > 0 && (line_start[line_len - 1] == '\r' || 
                               line_start[line_len - 1] == '\n')) {
            line_len--;
        }
        
        /* Check if line is blank */
        int is_blank = 1;
        for (size_t i = 0; i < line_len; i++) {
            if (!isspace((unsigned char)line_start[i])) {
                is_blank = 0;
                break;
            }
        }
        
        if (is_blank) {
            parsed_line.type = LINE_TYPE_BLANK;
            parsed_line.content = strndup_safe("", 0);
        } else if (in_preformat) {
            /* Check if this is a preformat toggle */
            if (line_len >= 3 && strncmp(line_start, "```", 3) == 0) {
                parsed_line.type = LINE_TYPE_PREFORMAT_TOGGLE;
                parsed_line.content = strndup_safe("", 0);
                if (line_len > 3) {
                    parsed_line.alt_text = strndup_safe(line_start + 3, line_len - 3);
                }
                in_preformat = 0;
            } else {
                parsed_line.type = LINE_TYPE_PREFORMATTED;
                parsed_line.content = strndup_safe(line_start, line_len);
            }
        } else {
            /* Not in preformat mode */
            if (line_len >= 3 && strncmp(line_start, "```", 3) == 0) {
                parsed_line.type = LINE_TYPE_PREFORMAT_TOGGLE;
                parsed_line.content = strndup_safe("", 0);
                if (line_len > 3) {
                    parsed_line.alt_text = strndup_safe(line_start + 3, line_len - 3);
                }
                in_preformat = 1;
            } else if (line_len == 3 && strncmp(line_start, "---", 3) == 0) {
                parsed_line.type = LINE_TYPE_HORIZONTAL_RULE;
                parsed_line.content = strndup_safe("", 0);
            } else if (line_len >= 2 && strncmp(line_start, "=>", 2) == 0) {
                parsed_line.type = LINE_TYPE_LINK;
                parsed_line.content = strndup_safe(line_start, line_len);
                parsed_line.link = parse_link_line(line_start, line_len);
            } else if (line_len >= 1 && line_start[0] == '#') {
                parsed_line.type = LINE_TYPE_HEADING;
                parsed_line.heading_level = 1;
                size_t offset = 1;
                
                if (offset < line_len && line_start[offset] == '#') {
                    parsed_line.heading_level = 2;
                    offset++;
                }
                if (offset < line_len && line_start[offset] == '#') {
                    parsed_line.heading_level = 3;
                    offset++;
                }
                
                /* Skip whitespace after # */
                while (offset < line_len && isspace((unsigned char)line_start[offset])) {
                    offset++;
                }
                
                parsed_line.content = strndup_safe(line_start + offset, line_len - offset);
                
                /* Extract page title from first # heading */
                if (parsed_line.heading_level == 1 && !doc->page_title && parsed_line.content) {
                    doc->page_title = strndup_safe(parsed_line.content, strlen(parsed_line.content));
                }
            } else if (line_len >= 2 && line_start[0] == '*' && line_start[1] == ' ') {
                parsed_line.type = LINE_TYPE_LIST_ITEM;
                parsed_line.content = strndup_safe(line_start + 2, line_len - 2);
            } else if (line_len >= 1 && line_start[0] == '>') {
                parsed_line.type = LINE_TYPE_QUOTE;
                size_t offset = 1;
                while (offset < line_len && isspace((unsigned char)line_start[offset])) {
                    offset++;
                }
                parsed_line.content = strndup_safe(line_start + offset, line_len - offset);
            } else {
                parsed_line.type = LINE_TYPE_TEXT;
                parsed_line.content = strndup_safe(line_start, line_len);
            }
        }
        
        /* Resize if needed */
        if (doc->line_count >= doc->capacity) {
            doc->capacity *= 2;
            GeminiLine *new_lines = realloc(doc->lines, sizeof(GeminiLine) * doc->capacity);
            if (!new_lines) {
                gemini_document_free(doc);
                return NULL;
            }
            doc->lines = new_lines;
        }
        
        doc->lines[doc->line_count++] = parsed_line;
        p = skip_newline(line_end, end);
    }
    
    return doc;
}

/* Convert Gemini document to HTML */
char *gemini_to_html(GeminiDocument *doc, const char *title) {
    return gemini_to_html_with_stylesheet(doc, title, NULL);
}

/* Convert Gemini document to HTML with custom stylesheet */
char *gemini_to_html_with_stylesheet(GeminiDocument *doc, const char *title, const char *stylesheet) {
    return gemini_to_html_with_stylesheet_and_head(doc, title, stylesheet, NULL);
}

/* Convert Gemini document to HTML with custom stylesheet and custom head content */
char *gemini_to_html_with_stylesheet_and_head(GeminiDocument *doc, const char *title, const char *stylesheet, const char *custom_head) {
    if (!doc) return NULL;
    
    /* Build HTML string - start with larger buffer to reduce reallocs */
    size_t buffer_size = 65536;  /* Start with 64KB instead of 8KB */
    char *html = malloc(buffer_size);
    if (!html) return NULL;
    
    size_t pos = 0;
    
    /* Use custom stylesheet or built-in */
    const char *css = stylesheet ? stylesheet : BUILTIN_STYLESHEET;
    
    /* HTML header with standard meta tags and stylesheet */
    pos += snprintf(html + pos, buffer_size - pos,
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "  <meta charset=\"UTF-8\">\n"
        "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "  <title>%s</title>\n",
        doc->page_title ? doc->page_title : (title ? title : "Gemini Document"));
    
    /* Add custom head content if provided */
    if (custom_head) {
        pos += snprintf(html + pos, buffer_size - pos, "%s\n", custom_head);
    }
    
    /* Add stylesheet */
    pos += snprintf(html + pos, buffer_size - pos,
        "  <style>\n"
        "%s"
        "  </style>\n"
        "</head>\n"
        "<body>\n",
        css);
    
    int in_list = 0;
    int in_preformat = 0;
    int in_blockquote = 0;
    
    for (size_t i = 0; i < doc->line_count; i++) {
        GeminiLine *line = &doc->lines[i];
        
        /* Expand buffer if getting full (be conservative - expand at 75% capacity) */
        if (pos > buffer_size * 3 / 4) {
            buffer_size *= 2;
            char *new_html = realloc(html, buffer_size);
            if (!new_html) {
                free(html);
                return NULL;
            }
            html = new_html;
        }
        
        /* Close open tags if needed */
        if (in_list && line->type != LINE_TYPE_LIST_ITEM) {
            pos += snprintf(html + pos, buffer_size - pos, "</ul>\n");
            in_list = 0;
        }
        
        if (in_blockquote && line->type != LINE_TYPE_QUOTE) {
            pos += snprintf(html + pos, buffer_size - pos, "</blockquote>\n");
            in_blockquote = 0;
        }
        
        switch (line->type) {
            case LINE_TYPE_TEXT: {
                char *escaped = html_escape(line->content);
                char *with_bold = process_inline_bold(escaped);
                char *with_code = process_inline_code(with_bold ? with_bold : escaped);
                pos += snprintf(html + pos, buffer_size - pos, "<p>%s</p>\n", with_code ? with_code : "");
                free(escaped);
                free(with_bold);
                free(with_code);
                break;
            }
            
            case LINE_TYPE_BLANK:
                pos += snprintf(html + pos, buffer_size - pos, "<br>\n");
                break;
            
            case LINE_TYPE_HEADING: {
                char *escaped = html_escape(line->content);
                int level = line->heading_level;
                pos += snprintf(html + pos, buffer_size - pos,
                    "<h%d>%s</h%d>\n",
                    level, escaped ? escaped : "", level);
                free(escaped);
                break;
            }
            
            case LINE_TYPE_LIST_ITEM: {
                if (!in_list) {
                    pos += snprintf(html + pos, buffer_size - pos, "<ul>\n");
                    in_list = 1;
                }
                char *escaped = html_escape(line->content);
                pos += snprintf(html + pos, buffer_size - pos, "  <li>%s</li>\n", escaped ? escaped : "");
                free(escaped);
                break;
            }
            
            case LINE_TYPE_QUOTE: {
                if (!in_blockquote) {
                    pos += snprintf(html + pos, buffer_size - pos, "<blockquote>\n");
                    in_blockquote = 1;
                }
                char *escaped = html_escape(line->content);
                pos += snprintf(html + pos, buffer_size - pos, "<p>%s</p>\n", escaped ? escaped : "");
                free(escaped);
                break;
            }
            
            case LINE_TYPE_PREFORMAT_TOGGLE:
                if (in_preformat) {
                    pos += snprintf(html + pos, buffer_size - pos, "</pre>\n");
                    in_preformat = 0;
                } else {
                    pos += snprintf(html + pos, buffer_size - pos,
                        "<pre%s>\n",
                        line->alt_text ? "" : "");
                    in_preformat = 1;
                }
                break;
            
            case LINE_TYPE_PREFORMATTED: {
                char *escaped = html_escape(line->content);
                pos += snprintf(html + pos, buffer_size - pos, "%s\n", escaped ? escaped : "");
                free(escaped);
                break;
            }
            
            case LINE_TYPE_HORIZONTAL_RULE:
                pos += snprintf(html + pos, buffer_size - pos, "<hr>\n");
                break;
            
            case LINE_TYPE_LINK: {
                if (line->link.url) {
                    char *url_escaped = html_escape(line->link.url);
                    char *label_escaped = line->link.label ? 
                        html_escape(line->link.label) : 
                        html_escape(line->link.url);
                    
                    pos += snprintf(html + pos, buffer_size - pos,
                        "<div class=\"gemini-link\"><a href=\"%s\">%s</a></div>\n",
                        url_escaped ? url_escaped : "",
                        label_escaped ? label_escaped : "");
                    
                    free(url_escaped);
                    free(label_escaped);
                }
                break;
            }
        }
    }
    
    /* Close any remaining open tags */
    if (in_list) pos += snprintf(html + pos, buffer_size - pos, "</ul>\n");
    if (in_blockquote) pos += snprintf(html + pos, buffer_size - pos, "</blockquote>\n");
    if (in_preformat) pos += snprintf(html + pos, buffer_size - pos, "</pre>\n");
    
    /* HTML footer */
    pos += snprintf(html + pos, buffer_size - pos,
        "</body>\n"
        "</html>\n");
    
    return html;
}

/* Free Gemini document */
void gemini_document_free(GeminiDocument *doc) {
    if (!doc) return;
    
    for (size_t i = 0; i < doc->line_count; i++) {
        free(doc->lines[i].content);
        free(doc->lines[i].link.url);
        free(doc->lines[i].link.label);
        free(doc->lines[i].alt_text);
    }
    
    free(doc->lines);
    free(doc);
}

/* Free HTML string */
void gemini_html_free(char *html) {
    free(html);
}
