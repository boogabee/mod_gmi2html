# Quick Reference Guide - mod_gmi2html

## Installation (30 seconds)

```bash
cd mod_gmi2html
make && sudo make install
sudo systemctl restart apache2
```

## Configuration (1 minute)

Add to Apache config:
```apache
<Directory /var/www/gemini>
    Gmi2HtmlEnabled on
    AddHandler gmi2html .gmi
    AddType text/gemini .gmi
</Directory>
```

## Usage

1. Create a `.gmi` file:
```bash
cat > /var/www/gemini/test.gmi << 'EOF'
# Hello Gemini

Visit => https://example.com My Site

\`\`\`
code
\`\`\`
EOF
```

2. Access via browser:
```
http://localhost/gemini/test.gmi
```

## Gemini Syntax Cheatsheet

| Element | Syntax | HTML Output |
|---------|--------|-------------|
| Heading 1 | `# Title` | `<h1>` |
| Heading 2 | `## Subtitle` | `<h2>` |
| Heading 3 | `### Section` | `<h3>` |
| Link | `=> URL [Label]` | `<a href>` (styled block) |
| List Item | `* Item` | `<li>` in `<ul>` |
| Quote | `> Quote text` | `<blockquote>` |
| Code Block | ` ``` ... ``` ` | `<pre>` |
| Text | Regular line | `<p>` |
| Blank Line | Empty line | `<br>` |

## File Structure

```
mod_gmi2html/
├── src/
│   ├── mod_gmi2html.c       # Apache module
│   ├── gemini_parser.c      # Gemini parser
│   └── gemini_parser.h      # Parser API
├── Makefile                 # Build (make / make install)
├── README.md                # Full documentation
├── INSTALL.md               # Installation guide
├── ARCHITECTURE.md          # Technical details
└── example.gmi              # Example file
```

## Build Options

```bash
# Using Make
make                    # Build module
make install            # Install module
make clean              # Clean build files

# Using apxs directly
apxs2 -c -i -n gmi2html src/mod_gmi2html.c src/gemini_parser.c

# Using CMake
mkdir build && cd build
cmake .. && make && sudo make install
```

## Configuration Directives

### `Gmi2HtmlEnabled on|off`

Enable conversion for a directory:

```apache
# Enable for one directory
<Directory /var/www/gemini>
    Gmi2HtmlEnabled on
</Directory>

# Disable for subdirectory
<Directory /var/www/gemini/raw>
    Gmi2HtmlEnabled off
</Directory>

# Enable globally
Gmi2HtmlEnabled on
```

## Testing

```bash
# Run build tests
./test.sh

# Verify module loaded
apache2ctl -M | grep gmi2html

# Check configuration
apache2ctl configtest

# View errors
tail -f /var/log/apache2/error.log
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Module won't load | Run `apache2ctl -M` to check, verify `/usr/lib/apache2/modules/mod_gmi2html.so` exists |
| Files not converting | Ensure `Gmi2HtmlEnabled on` is set in config |
| Permission denied | Check file permissions: `chmod 644 *.gmi` |
| Compilation fails | Install apache2-dev: `apt-get install apache2-dev` |
| Apache won't restart | Run `apache2ctl configtest` to find syntax errors |

## Performance Tips

1. **Enable Caching**
   ```apache
   CacheEnable disk /gemini
   CacheDefaultExpire 3600
   ```

2. **Use gzip**
   ```apache
   AddOutputFilterByType DEFLATE text/html
   ```

3. **Set Cache Headers**
   ```apache
   <FilesMatch "\.gmi$">
       Header set Cache-Control "public, max-age=3600"
   </FilesMatch>
   ```

## Common Apache Configuration

Full virtual host example:

```apache
<VirtualHost *:80>
    ServerName gemini.example.com
    DocumentRoot /var/www/gemini
    
    <Directory /var/www/gemini>
        Gmi2HtmlEnabled on
        AddHandler gmi2html .gmi
        AddType text/gemini .gmi
        Options FollowSymLinks
        AllowOverride All
        Require all granted
    </Directory>
    
    # Enable caching for 1 hour
    <FilesMatch "\.gmi$">
        Header set Cache-Control "public, max-age=3600"
    </FilesMatch>
    
    # Enable compression
    AddOutputFilterByType DEFLATE text/html
    
    ErrorLog ${APACHE_LOG_DIR}/gemini-error.log
    CustomLog ${APACHE_LOG_DIR}/gemini-access.log combined
</VirtualHost>
```

## Directory Structure

Create directories for organized content:

```bash
/var/www/gemini/
├── index.gmi              # Home page
├── blog/
│   ├── post1.gmi
│   └── post2.gmi
├── docs/
│   ├── guide.gmi
│   └── faq.gmi
└── raw/                   # Disable conversion here
    └── original.gmi
```

Config with mixed settings:

```apache
<Directory /var/www/gemini>
    Gmi2HtmlEnabled on
</Directory>

<Directory /var/www/gemini/raw>
    Gmi2HtmlEnabled off
    AddType text/gemini .gmi
</Directory>
```

## Output Format

The module generates clean HTML:
- UTF-8 encoding
- Responsive design
- Semantic HTML5
- Embedded CSS styling
- Accessible markup

All special characters are HTML-escaped for security.

## Advanced Topics

### Custom Styling

Edit `gemini_to_html()` function in `src/gemini_parser.c` to customize:
- Colors and fonts
- Spacing and layout
- Link appearance
- Code block styling

### Caching

Use Apache's caching modules:
```bash
sudo a2enmod cache
sudo a2enmod cache_disk
```

Then configure in VirtualHost.

### URL Rewriting

Remove .gmi extension with mod_rewrite:
```apache
<Directory /var/www/gemini>
    RewriteEngine On
    RewriteCond %{REQUEST_FILENAME} !-f
    RewriteRule ^(.+)$ $1.gmi [L]
</Directory>
```

## Support

1. Check error logs: `/var/log/apache2/error.log`
2. Verify config: `apache2ctl configtest`
3. Review gemini_specification.txt for format validation
4. Run: `./test.sh` for build verification
5. Check README.md for detailed documentation

## Links

- **Official Gemini**: https://geminiprotocol.net/
- **Specification**: https://geminiprotocol.net/docs/spec-spec.html
- **Apache Modules**: https://httpd.apache.org/docs/2.4/mod/
- **APR Documentation**: https://apr.apache.org/

---

**Version**: 1.2  
**Last Updated**: January 2026  
**License**: Open Source
