#ifndef GEMINI_PARSER_H
#define GEMINI_PARSER_H

#include <stdlib.h>
#include <stdio.h>

/**
 * Gemini to HTML Parser
 * 
 * Converts Gemini format (.gmi) documents to HTML
 * Based on Gemini specification v0.16.0
 */

typedef enum {
    LINE_TYPE_TEXT,
    LINE_TYPE_LINK,
    LINE_TYPE_PREFORMAT_TOGGLE,
    LINE_TYPE_PREFORMATTED,
    LINE_TYPE_HEADING,
    LINE_TYPE_LIST_ITEM,
    LINE_TYPE_QUOTE,
    LINE_TYPE_BLANK,
    LINE_TYPE_HORIZONTAL_RULE
} GeminiLineType;

typedef struct {
    char *url;
    char *label;
} GeminiLink;

typedef struct {
    GeminiLineType type;
    char *content;
    int heading_level;  /* 1-3 for headings */
    GeminiLink link;    /* for link lines */
    char *alt_text;     /* for preformat toggle */
} GeminiLine;

typedef struct {
    GeminiLine *lines;
    size_t line_count;
    size_t capacity;
    char *page_title;  /* Extracted from first # heading */
} GeminiDocument;

/**
 * Parse a Gemini document from file content
 * @param content: The raw Gemini file content as a string
 * @param length: Length of the content
 * @return: Parsed GeminiDocument structure
 */
GeminiDocument *gemini_parse(const char *content, size_t length);

/**
 * Convert parsed Gemini document to HTML
 * @param doc: Parsed Gemini document
 * @param title: Optional title for the HTML document
 * @return: HTML string (must be freed by caller)
 */
char *gemini_to_html(GeminiDocument *doc, const char *title);

/**
 * Convert parsed Gemini document to HTML with custom stylesheet
 * @param doc: Parsed Gemini document
 * @param title: Optional title for the HTML document
 * @param stylesheet: Custom CSS stylesheet (NULL to use built-in)
 * @return: HTML string (must be freed by caller)
 */
char *gemini_to_html_with_stylesheet(GeminiDocument *doc, const char *title, const char *stylesheet);

/**
 * Free a parsed Gemini document
 * @param doc: Document to free
 */
void gemini_document_free(GeminiDocument *doc);

/**
 * Free HTML string
 * @param html: HTML string to free
 */
void gemini_html_free(char *html);

#endif
