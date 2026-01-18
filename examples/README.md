# Custom Stylesheets and Head Content Examples for mod_gmi2html

This directory contains:
- Example stylesheets and head content files for customizing mod_gmi2html output
- Detailed documentation about features and configuration
- Ready-to-use templates for common use cases

For basic module operation and directive overview, see the main [README.md](../README.md).

## Documentation

### Quick Start
- **[README.md](README.md)** (this file) - Examples and templates

### Detailed Reference
- **[CUSTOM_HEAD_DOCUMENTATION_INDEX.md](CUSTOM_HEAD_DOCUMENTATION_INDEX.md)** - Navigation guide to all head content resources
- **[CUSTOM_HEAD_FEATURE.md](CUSTOM_HEAD_FEATURE.md)** - Complete custom head content feature documentation
- **[CUSTOM_HEAD_IMPLEMENTATION.md](CUSTOM_HEAD_IMPLEMENTATION.md)** - Implementation details and internals
- **[HEAD_CONFIGURATION_EXAMPLES.md](HEAD_CONFIGURATION_EXAMPLES.md)** - Advanced Apache configuration patterns

## Stylesheets

### Usage

To use a custom stylesheet with mod_gmi2html, add the `Gmi2HtmlStylesheet` directive to your Apache configuration:

```apache
<Directory /var/www/html/gemini>
    Gmi2HtmlEnabled on
    Gmi2HtmlStylesheet /path/to/your/stylesheet.css
</Directory>
```

The stylesheet path can be:
- An absolute path (e.g., `/var/www/stylesheets/custom.css`)
- A relative path (relative to the server's working directory)

If the stylesheet file cannot be found or read, the module will fall back to the built-in stylesheet.

## Included Examples

### default-stylesheet.css

This is the built-in stylesheet used by mod_gmi2html. It provides:
- Clean, readable typography with system fonts
- Proper spacing and margins
- Link styling with hover effects
- Code block and inline code formatting
- Special styling for Gemini-specific elements (links, blockquotes)
- Responsive design that works on mobile devices

### dark-mode-stylesheet.css

A dark mode stylesheet example featuring:
- Dark background with light text for reduced eye strain
- Colorful headings (blue, green, orange)
- Green syntax highlighting for code blocks
- Enhanced contrast for better readability
- Similar structure to the default stylesheet

## Creating Your Own Stylesheet

You can create your own custom stylesheet by copying one of the included examples and modifying it. Important elements to style:

### Required Elements

- `body` - Main document container
- `h1, h2, h3` - Heading levels (1-3)
- `a` - Links (including `a:hover` for hover state)
- `p` - Paragraphs (implied by text elements)
- `pre` - Preformatted code blocks
- `code` - Inline code
- `blockquote` - Block quotes
- `ul, li` - Unordered lists
- `hr` - Horizontal rules

### Gemini-Specific Elements

- `.gemini-link` - Gemini link blocks (styled as `<div class="gemini-link">`)
- `.gemini-link a` - Links within Gemini link blocks

## Tips for Custom Stylesheets

1. **Valid CSS**: Ensure your CSS is valid. The module includes the entire stylesheet as-is, so any syntax errors will appear in the HTML.

2. **Responsive Design**: Consider using responsive design techniques for mobile devices:
   ```css
   @media (max-width: 600px) {
     body { padding: 10px; font-size: 14px; }
     h1 { font-size: 1.5em; }
   }
   ```

3. **Font Families**: Use web-safe fonts or system font stacks:
   ```css
   body {
     font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
   }
   ```

4. **Accessibility**: Maintain good contrast ratios for readability:
   - Text: at least 4.5:1 contrast ratio
   - Large text: at least 3:1 contrast ratio

5. **Testing**: Always test your stylesheet with actual Gemini content to ensure all elements render correctly.

## File Size Considerations

The stylesheet is embedded directly in the HTML output for each request. Keep stylesheets reasonably sized (under 50KB) to avoid excessive HTML output. If you need a very large stylesheet, consider serving it as a separate file with a `<link>` tag instead.

However, note that mod_gmi2html currently embeds stylesheets directly. For separate stylesheet files, you would need to modify the HTML generation to use `<link rel="stylesheet" href="...">` instead.

## Example Apache Configuration

```apache
<Directory /var/www/html/gemini>
    Gmi2HtmlEnabled on
    Gmi2HtmlStylesheet /etc/apache2/mod_gmi2html/stylesheets/custom.css
</Directory>

<Directory /var/www/html/blog>
    Gmi2HtmlEnabled on
    Gmi2HtmlStylesheet /etc/apache2/mod_gmi2html/stylesheets/blog.css
</Directory>
```

This allows different directories to use different stylesheets.
