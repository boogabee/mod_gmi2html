# Custom Head Content Feature

## Overview

The custom head content feature (v1.2) allows users to add custom `<head>` section contents to generated HTML pages. This enables adding:

- Meta tags (description, author, keywords, etc.)
- Site icons (favicon, apple-touch-icon)
- Open Graph tags for social media
- Twitter Card tags
- Custom link tags
- Analytics scripts
- Any other `<head>` section elements

## Implementation

### Configuration Directive

```apache
Gmi2HtmlHead <file_path>
```

Specifies a file containing custom `<head>` content to include in generated HTML pages.

**Context:** Directory, .htaccess  
**Default:** None (custom head content is optional)

### Example Configuration

```apache
<Directory /var/www/gemini>
    Gmi2HtmlEnabled on
    Gmi2HtmlStylesheet /etc/apache2/gmi2html/custom-style.css
    Gmi2HtmlHead /etc/apache2/gmi2html/head-content.html
    AddHandler gmi2html .gmi
    AddType text/gemini .gmi
</Directory>
```

### Example Head Content File

**File: `/etc/apache2/gmi2html/head-content.html`**

```html
  <meta name="description" content="My Gemini Site">
  <meta name="author" content="Author Name">
  <meta name="keywords" content="gemini, protocol, web">
  <link rel="icon" type="image/svg+xml" href="/favicon.svg">
  <link rel="apple-touch-icon" href="/apple-icon.png">
  <meta property="og:type" content="website">
  <meta property="og:site_name" content="My Gemini Site">
```

## How It Works

### Parser Functions

The feature adds a new function to the parser:

```c
char *gemini_to_html_with_stylesheet_and_head(
    GeminiDocument *doc,
    const char *title,
    const char *stylesheet,
    const char *custom_head
);
```

The existing functions are simplified to call this new function:

```c
char *gemini_to_html(GeminiDocument *doc, const char *title)
    → calls gemini_to_html_with_stylesheet_and_head(..., NULL, NULL)

char *gemini_to_html_with_stylesheet(...)
    → calls gemini_to_html_with_stylesheet_and_head(..., NULL)
```

### HTML Structure

Generated HTML structure when custom head is provided:

```html
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Page Title</title>
  <!-- Custom head content inserted here -->
  <meta name="description" content="...">
  <link rel="icon" href="...">
  <!-- ... more custom content ... -->
  
  <!-- Built-in or custom stylesheet -->
  <style>
  ...
  </style>
</head>
<body>
  <!-- Gemini content converted to HTML -->
</body>
</html>
```

## Module Implementation

### Configuration Structure

Added `head_file_path` field to `gmi2html_config`:

```c
typedef struct {
    int enabled;
    const char *gemini_type;
    const char *stylesheet_path;   /* Path to custom stylesheet file */
    const char *head_file_path;     /* Path to custom head content file */
} gmi2html_config;
```

### Handler Process

1. Check if `Gmi2HtmlHead` directive is configured
2. If file path is configured:
   - Check if file exists and is readable
   - Open and read the entire file into memory
   - Pass content to HTML generation function
3. If file cannot be read, skip custom head (graceful degradation)
4. HTML generation injects custom head after viewport meta tag, before stylesheet

## Features

✅ **Optional** - Custom head content is completely optional  
✅ **Flexible** - Support any valid HTML `<head>` content  
✅ **Safe** - File must exist and be readable (no runtime errors)  
✅ **Per-Directory** - Can be configured per directory or virtual host  
✅ **Consistent** - Works alongside custom stylesheets  

## Use Cases

### 1. Site Branding
```html
<meta name="description" content="My Personal Gemini Site">
<meta name="author" content="Jane Doe">
```

### 2. Social Media Integration
```html
<meta property="og:title" content="My Site">
<meta property="og:description" content="Personal web content">
<meta property="og:image" content="https://example.com/og-image.jpg">
<meta name="twitter:card" content="summary">
<meta name="twitter:creator" content="@myhandle">
```

### 3. Icons and Theming
```html
<link rel="icon" type="image/svg+xml" href="/favicon.svg">
<link rel="apple-touch-icon" href="/apple-icon.png">
<meta name="theme-color" content="#ffffff">
```

### 4. SEO Optimization
```html
<meta name="robots" content="index, follow">
<link rel="canonical" href="https://example.com/page">
```

## Testing

### Test File: example_head.txt

```html
  <meta name="description" content="Example Gemini document with custom metadata">
  <meta name="author" content="My Name">
  <meta name="keywords" content="gemini, html, conversion">
  <link rel="icon" type="image/svg+xml" href="/favicon.svg">
  <link rel="apple-touch-icon" href="/apple-icon.png">
```

### Configuration for Testing

```apache
<Directory /var/www/test>
    Gmi2HtmlEnabled on
    Gmi2HtmlHead /var/www/test/example_head.txt
    AddHandler gmi2html .gmi
</Directory>
```

### Expected Output

```html
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Example Document</title>
  <meta name="description" content="Example Gemini document with custom metadata">
  <meta name="author" content="My Name">
  <meta name="keywords" content="gemini, html, conversion">
  <link rel="icon" type="image/svg+xml" href="/favicon.svg">
  <link rel="apple-touch-icon" href="/apple-icon.png">
  <style>
  ...built-in or custom CSS...
  </style>
</head>
<body>
  <!-- Converted Gemini content -->
</body>
</html>
```

## Backward Compatibility

✅ Fully backward compatible - the feature is optional  
✅ Existing configurations work unchanged  
✅ If custom head file is not specified, behavior is identical to v1.1  

## Implementation Details

### Code Changes

**mod_gmi2html.c:**
- Added `head_file_path` to config struct
- Added `set_gmi2html_head()` directive handler
- Added head file reading logic in handler (mirrors stylesheet reading)
- Pass custom head to new HTML generation function

**gemini_parser.h:**
- Added function declaration: `gemini_to_html_with_stylesheet_and_head()`

**gemini_parser.c:**
- Implemented `gemini_to_html_with_stylesheet_and_head()`
- Modified `gemini_to_html()` and `gemini_to_html_with_stylesheet()` to use new function
- Custom head content is injected after viewport meta tag, before stylesheet block

### File Statistics

- **Files Modified:** 3 (mod_gmi2html.c, gemini_parser.h, gemini_parser.c)
- **Lines Added:** ~80 (across all files)
- **New Functions:** 1 (`gemini_to_html_with_stylesheet_and_head`)
- **New Struct Fields:** 1 (`head_file_path`)
- **Compilation:** ✅ Clean (no warnings or errors)
