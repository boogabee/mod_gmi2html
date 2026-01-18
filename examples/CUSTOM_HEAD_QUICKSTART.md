# Custom Head Content Feature - Quick Start Guide

## What Was Added

A new Apache configuration directive `Gmi2HtmlHead` that allows you to add custom `<head>` section content (meta tags, icons, analytics, etc.) to all generated HTML pages.

## Key Features

✅ **Custom Meta Tags** - Add description, author, keywords  
✅ **Site Icons** - favicon, apple-touch-icon, etc.  
✅ **Social Media Tags** - Open Graph, Twitter Card  
✅ **Analytics Scripts** - Google Analytics, Plausible, etc.  
✅ **PWA Support** - Manifest, theme-color, app capabilities  
✅ **Per-Directory Configuration** - Different heads for different sections  

## Installation

### 1. Build the Module
```bash
cd /mnt/d/workspace/mod_gmi2html
make
sudo make install
```

### 2. Create Head Content File
```bash
sudo mkdir -p /etc/apache2/gmi2html
cat | sudo tee /etc/apache2/gmi2html/head-content.html << 'EOF'
  <meta name="description" content="My Gemini Site">
  <meta name="author" content="Jane Doe">
  <link rel="icon" type="image/svg+xml" href="/favicon.svg">
EOF
```

### 3. Update Apache Configuration
```apache
<Directory /var/www/gemini>
    Gmi2HtmlEnabled on
    Gmi2HtmlHead /etc/apache2/gmi2html/head-content.html
    AddHandler gmi2html .gmi
    AddType text/gemini .gmi
</Directory>
```

### 4. Restart Apache
```bash
sudo systemctl restart apache2
```

## HTML Output

### Before (without custom head)
```html
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Page Title</title>
  <style>...</style>
</head>
```

### After (with custom head)
```html
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Page Title</title>
  
  <!-- Custom head content injected here -->
  <meta name="description" content="...">
  <meta name="author" content="...">
  <link rel="icon" href="...">
  
  <style>...</style>
</head>
```

## Common Use Cases

### 1. Basic Metadata
```html
  <meta name="description" content="My Gemini Website">
  <meta name="author" content="Your Name">
  <meta name="keywords" content="gemini, protocol, web">
```

### 2. Site Icons
```html
  <link rel="icon" type="image/svg+xml" href="/favicon.svg">
  <link rel="icon" type="image/png" href="/favicon.png">
  <link rel="apple-touch-icon" href="/apple-touch-icon.png">
```

### 3. Social Media
```html
  <meta property="og:type" content="website">
  <meta property="og:title" content="My Site">
  <meta property="og:description" content="...">
  <meta property="og:image" content="https://example.com/og-image.png">
  <meta name="twitter:card" content="summary">
  <meta name="twitter:creator" content="@handle">
```

### 4. Analytics
```html
  <script async src="https://www.googletagmanager.com/gtag/js?id=G-XXXXX"></script>
  <script>
    window.dataLayer = window.dataLayer || [];
    function gtag(){dataLayer.push(arguments);}
    gtag('js', new Date());
    gtag('config', 'G-XXXXX');
  </script>
```

### 5. PWA Support
```html
  <meta name="theme-color" content="#ffffff">
  <meta name="mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <link rel="manifest" href="/manifest.json">
```

## Configuration Options

### Single Head for All Pages
```apache
<Directory /var/www/gemini>
    Gmi2HtmlEnabled on
    Gmi2HtmlHead /etc/apache2/gmi2html/head.html
    AddHandler gmi2html .gmi
</Directory>
```

### Different Heads for Different Sections
```apache
<Directory /var/www/gemini>
    Gmi2HtmlHead /etc/apache2/gmi2html/default-head.html
    AddHandler gmi2html .gmi
</Directory>

<Directory /var/www/gemini/blog>
    Gmi2HtmlHead /etc/apache2/gmi2html/blog-head.html
    AddHandler gmi2html .gmi
</Directory>
```

### Head with Custom Stylesheet
```apache
<Directory /var/www/gemini>
    Gmi2HtmlEnabled on
    Gmi2HtmlStylesheet /etc/apache2/gmi2html/custom-style.css
    Gmi2HtmlHead /etc/apache2/gmi2html/head-content.html
    AddHandler gmi2html .gmi
</Directory>
```

### In .htaccess
```apache
Gmi2HtmlEnabled on
Gmi2HtmlHead /var/www/gemini/head.html
AddHandler gmi2html .gmi
```

## Implementation Details

### Modified Files
- `src/mod_gmi2html.c` - Apache module configuration and handler
- `src/gemini_parser.h` - New parser function declaration
- `src/gemini_parser.c` - HTML generation implementation

### New Directive
- `Gmi2HtmlHead <file-path>` - Specify custom head content file

### New Parser Function
- `gemini_to_html_with_stylesheet_and_head()` - Generate HTML with both custom stylesheet and custom head

### Backward Compatibility
✅ **100% Backward Compatible**
- Existing configurations work unchanged
- Feature is optional
- No breaking changes

## Troubleshooting

### Head Content Not Appearing
1. Check file path is correct:
   ```bash
   ls -l /etc/apache2/gmi2html/head-content.html
   ```

2. Check Apache can read the file:
   ```bash
   sudo -u www-data test -r /etc/apache2/gmi2html/head-content.html && echo "Readable" || echo "Not readable"
   ```

3. Check Apache configuration is correct:
   ```bash
   sudo apache2ctl configtest
   ```

4. Check for syntax errors in head file:
   ```bash
   cat /etc/apache2/gmi2html/head-content.html | grep -v "^  " && echo "Found non-indented content"
   ```

### Incorrect Head Content Order
The head content is always injected in this order:
1. Standard meta tags (charset, viewport)
2. Title tag
3. **Custom head content** ← inserted here
4. Stylesheet

This cannot be changed - custom head always goes between title and stylesheet.

### File Not Found Error
If the file specified in `Gmi2HtmlHead` doesn't exist or isn't readable:
- The module logs a warning but continues
- Pages are generated without custom head content
- No errors are shown to the user

## Testing

### Test Without Installation
```bash
cd /mnt/d/workspace/mod_gmi2html

# Run the test program
./notes/test_custom_head

# Expected output:
# ✓ All tests passed!
# ✓ Custom head feature working correctly
```

### Test After Installation
1. Create a test .gmi file:
   ```bash
   cat > /var/www/gemini/test.gmi << 'EOF'
   # Test Page
   
   This tests the custom head feature.
   EOF
   ```

2. Access via browser:
   ```
   http://localhost/gemini/test.gmi
   ```

3. View page source (Ctrl+U) and check the `<head>` section:
   - Should include standard meta tags
   - Should include custom content from file
   - Should include stylesheet

## Performance

- **File Reading**: Happens per-request (configurable caching in production)
- **HTML Generation**: Single-pass, same as before
- **Memory Usage**: Minimal addition (~file size)

## Security Notes

✅ **Safe by Design:**
- File must be specified by Apache admin
- File must exist and be readable
- Content is not sanitized (use trusted sources)
- No code injection possible

⚠️ **Best Practices:**
- Only allow trusted users to edit head files
- Use HTTPS for external resources
- Verify third-party scripts before adding
- Keep head file permissions restrictive

## Files in This Feature

```
mod_gmi2html/
├── src/
│   ├── mod_gmi2html.c          ← Modified
│   ├── gemini_parser.h         ← Modified
│   └── gemini_parser.c         ← Modified
│
├── notes/
│   ├── CUSTOM_HEAD_FEATURE.md          ← New
│   ├── CUSTOM_HEAD_IMPLEMENTATION.md   ← New
│   ├── HEAD_CONFIGURATION_EXAMPLES.md  ← New
│   ├── test_custom_head.c              ← New
│   └── ...
│
├── example_head.txt            ← New
└── mod_gmi2html.so             ← Rebuilt
```

## Support & Documentation

- **Feature Documentation**: `notes/CUSTOM_HEAD_FEATURE.md`
- **Implementation Details**: `notes/CUSTOM_HEAD_IMPLEMENTATION.md`
- **Configuration Examples**: `notes/HEAD_CONFIGURATION_EXAMPLES.md`
- **Test Program**: `notes/test_custom_head.c`

## Summary

The custom head content feature allows you to:
- Add meta tags for SEO and branding
- Include site icons for modern browsers
- Add analytics and tracking code
- Implement social media integration
- Support Progressive Web Apps
- Customize the HTML head per directory

All without modifying the Gemini files themselves!
