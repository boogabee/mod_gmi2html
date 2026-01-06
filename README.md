# mod_gmi2html - Apache Module for Gemini to HTML Conversion

A lightweight Apache webserver module written in C that converts Gemini format (.gmi) documents to HTML on-the-fly. This module is inspired by `mod_transform` but specifically designed for serving Gemini-formatted files as HTML.

## Features

- **Gemini Format Support**: Full support for Gemini text format (v0.16.0 specification)
- **On-the-fly Conversion**: Automatically converts .gmi files to HTML without needing pre-conversion
- **Clean HTML Output**: Generates semantic, responsive HTML with built-in CSS styling
- **Line Type Support**:
  - Text paragraphs
  - Headings (H1, H2, H3)
  - Links (styled as distinct blocks)
  - Preformatted text (with syntax highlighting potential)
  - Lists (unordered)
  - Block quotes
  - Code blocks
  - **Horizontal rules** (new)
- **Enhancement Features** (v1.1+):
  - **Horizontal Rule Support**: Render `---` as `<hr>` tags for visual separation
  - **Page Title Extraction**: Auto-extract first heading as HTML page title
  - **Link Path Conversion**: Auto-convert .gmi links to .html for web serving
  - **Inline Code**: Space-delimited backticks ` code ` render as `<code>`
  - **Inline Bold**: Space-delimited asterisks **text** render as `<strong>`

## Installation

### Prerequisites

- Apache 2.4 or later
- Apache development headers (`apache2-dev` or equivalent)
- APR development headers (`libapr1-dev` or equivalent)
- GCC compiler
- Make or CMake (optional)

### Ubuntu/Debian

```bash
sudo apt-get install apache2 apache2-dev libapr1-dev build-essential
```

### Building from Source

#### Method 1: Using Make

```bash
cd /path/to/mod_gmi2html
make
sudo make install
```

#### Method 2: Using apxs (Apache Extension Tool)

```bash
cd /path/to/mod_gmi2html
apxs2 -c -i src/mod_gmi2html.c src/gemini_parser.c
```

#### Method 3: Using CMake

```bash
cd /path/to/mod_gmi2html
mkdir build
cd build
cmake ..
make
sudo make install
```

### Enable the Module

After installation, enable the module in Apache:

```bash
sudo a2enmod gmi2html
sudo systemctl restart apache2
```

Or manually add to Apache configuration:

```apache
LoadModule gmi2html_module /usr/lib/apache2/modules/mod_gmi2html.so
```

## Configuration

### Basic Setup

1. Create or edit your Apache configuration file (e.g., `/etc/apache2/sites-available/your-site.conf`):

```apache
<VirtualHost *:80>
    ServerName example.com
    DocumentRoot /var/www/gemini
    
    <Directory /var/www/gemini>
        Gmi2HtmlEnabled on
        AddHandler gmi2html .gmi
        AddType text/gemini .gmi
        Options +Indexes FollowSymLinks
    </Directory>
</VirtualHost>
```

2. Restart Apache:

```bash
sudo systemctl restart apache2
```

### Configuration Directives

#### `Gmi2HtmlEnabled on|off`

Enables or disables the Gemini to HTML conversion for the current directory scope.

- **Syntax**: `Gmi2HtmlEnabled on|off`
- **Context**: Directory, .htaccess
- **Default**: `off`

**Example**:
```apache
<Directory /var/www/html/gemini>
    Gmi2HtmlEnabled on
</Directory>
```

### Apache Handler Assignment

Use the `AddHandler` directive to map the `gmi2html` handler to `.gmi` files:

```apache
AddHandler gmi2html .gmi
```

## Usage

Once configured, simply place Gemini files in your web directory:

```bash
mkdir -p /var/www/gemini
cat > /var/www/gemini/example.gmi << 'EOF'
# Welcome to Gemini

This is a test Gemini document.

## Subsection

Some content here.

=> https://example.com Example Link
=> /local/file.gmi Local Link

* List item 1
* List item 2

> This is a quote

\`\`\`
code block
example
\`\`\`

EOF
```

Access the file via your browser:
```
http://example.com/gemini/example.gmi
```

The module will automatically convert it to HTML and display it.

## Gemini Format Reference

### Headings

```
# Main Heading (H1)
## Sub Heading (H2)
### Sub-sub Heading (H3)
```

### Links

```
=> https://example.com/ Example Link
=> /local/file.gmi Local File
=> gemini://gemini.space/ Gemini Protocol Link
```

### Text Formatting

```
Regular text - wraps to viewport width

* Unordered list item
* Another item

> Block quote
> Can span multiple lines

\`\`\`alt text
Preformatted text
  preserves spacing
    and formatting
\`\`\`
```

### Complete Example

```
# Welcome to My Gemini Site

This is a simple Gemini document converted to HTML.

## Navigation

=> / Home
=> /blog/ Blog
=> /about.gmi About Me

## Features

* Easy to write
* Minimal markup
* Focus on content

> "The best documents are simple ones."

\`\`\`python
def hello():
    print("Hello, Gemini!")
\`\`\`
```

## Enhanced Features (v1.1+)

This version includes 5 major enhancements for better web content support:

### 1. Horizontal Rules

Create visual separators using three or more hyphens:

```
Text before

---

Text after
```

Renders as `<hr>` tag in HTML.

### 2. Page Title Extraction

The first `#` (level 1) heading is automatically extracted and used as:
- HTML page `<title>`
- Main `<h1>` heading

```
# My Document Title

This heading becomes the page title automatically.
```

### 3. Link Path Conversion

Relative `.gmi` links are automatically converted to `.html`:

```
=> /index.gmi Home          →  <a href="/index.html">Home</a>
=> /docs/guide.gmi Guide    →  <a href="/docs/guide.html">Guide</a>
=> https://example.com Site →  <a href="https://example.com">Site</a>
```

External URLs (with `://`) remain unchanged.

### 4. Inline Code

Backtick-delimited code is rendered as `<code>`:

```
Use ` printf("hello") ` in your programs.
```

**Note:** Backticks require space before/after to be recognized (or start/end of line).

### 5. Inline Bold

Asterisk-delimited bold text is rendered as `<strong>`:

```
This is **very important** information.
Multiple sections can be **bold one** and **bold two**.
```

**Note:** `**` requires space before (or start), and space/punctuation after (or end).

### Combined Example

```
# My Enhanced Document

Here's text with **bold** and ` inline code ` together.

---

=> /other.gmi Related Page
=> https://example.com External
```

## Project Structure

```
mod_gmi2html/
├── src/
│   ├── mod_gmi2html.c       # Apache module implementation
│   ├── gemini_parser.c      # Gemini parser and HTML converter
│   └── gemini_parser.h      # Gemini parser header
├── Makefile                 # Build configuration (Make)
├── CMakeLists.txt          # Build configuration (CMake)
├── apache-config.conf      # Example Apache configuration
├── gemini_specification.txt # Gemini format specification
├── FEATURES_IMPLEMENTED.md # Detailed feature documentation
└── README.md               # This file
```

## Implementation Details

### Module Architecture

The module consists of two main components:

1. **Gemini Parser** (`gemini_parser.c/.h`)
   - Parses Gemini format documents line-by-line
   - Identifies line types (headings, links, preformatted, etc.)
   - Handles Gemini-specific syntax

2. **Apache Module** (`mod_gmi2html.c`)
   - Apache hook integration
   - HTTP request handling
   - File I/O and response generation
   - Configuration directive processing

### Conversion Flow

1. Request received for `.gmi` file
2. Module checks if enabled in directory config
3. File is read from disk
4. Gemini parser processes file content
5. HTML is generated with semantic markup
6. HTTP response is sent with `text/html` content-type

## Styling

The generated HTML includes embedded CSS for:

- Responsive typography
- Proper heading hierarchy
- Link styling with visual distinction
- Code block formatting (monospace with background)
- Block quote styling with left border
- List formatting

Styling can be customized by modifying the `gemini_to_html()` function in `gemini_parser.c`.

## Performance Considerations

- Files are parsed and converted on each request (no caching)
- For high-traffic sites, consider using Apache's caching modules
- Use `mod_cache` or `mod_cache_disk` to cache HTML output

Example caching configuration:

```apache
<Directory /var/www/gemini>
    Gmi2HtmlEnabled on
    
    # Cache converted HTML for 1 hour
    CacheEnable disk /
    CacheDefaultExpire 3600
    CacheMaxExpire 86400
</Directory>
```

## Troubleshooting

### Module not loading

```bash
sudo apache2ctl -t  # Check configuration syntax
sudo apache2ctl -M  # List loaded modules
```

### Module not converting files

1. Verify module is loaded: `apache2ctl -M | grep gmi2html`
2. Check Apache error log: `/var/log/apache2/error.log`
3. Ensure `Gmi2HtmlEnabled on` is set in the directory config
4. Verify file permissions allow Apache to read the file

### Invalid HTML output

- Check that the .gmi file is valid UTF-8
- Verify Gemini syntax is correct
- Check error.log for parsing errors

## Limitations

- No client-side caching headers are set automatically (use Apache directives)
- Alt text in preformatted blocks is currently ignored in HTML output
- Maximum file size limited by available memory

## Future Enhancements

- Caching support for frequently accessed files
- Custom CSS stylesheet support
- Metadata extraction from comments
- Support for Gemini response status codes
- MIME type detection
- Directory index generation
- Support for .htaccess overrides

## License

This project is provided as-is for use with Apache 2.4+

## References

- [Gemini Specification](https://geminiprotocol.net/)
- [Apache Module Development](https://httpd.apache.org/docs/2.4/developer/)
- [Apache APR Library](https://apr.apache.org/)

## Contributing

Contributions are welcome! Areas for improvement:

- Additional HTML styling/CSS
- Performance optimizations
- Enhanced error handling
- Configuration options
- Unit tests

## Support

For issues or questions:

1. Check the Gemini specification for format validation
2. Review Apache logs at `/var/log/apache2/error.log`
3. Verify module compilation with `make test`
4. Ensure .gmi files are UTF-8 encoded
