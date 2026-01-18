# Apache Configuration Examples for mod_gmi2html with Custom Head Content

## Basic Configuration with Custom Head

```apache
<VirtualHost *:80>
    ServerName example.com
    DocumentRoot /var/www/gemini
    
    <Directory /var/www/gemini>
        Gmi2HtmlEnabled on
        Gmi2HtmlHead /etc/apache2/gmi2html/head-content.html
        AddHandler gmi2html .gmi
        AddType text/gemini .gmi
    </Directory>
</VirtualHost>
```

## Full Configuration with Custom Stylesheet and Head

```apache
<VirtualHost *:80>
    ServerName gemini.example.com
    DocumentRoot /var/www/gemini
    
    <Directory /var/www/gemini>
        Gmi2HtmlEnabled on
        # Custom CSS stylesheet
        Gmi2HtmlStylesheet /etc/apache2/gmi2html/custom-style.css
        # Custom head content (meta tags, icons, etc.)
        Gmi2HtmlHead /etc/apache2/gmi2html/head-content.html
        AddHandler gmi2html .gmi
        AddType text/gemini .gmi
    </Directory>
</VirtualHost>
```

## Multiple Directories with Different Heads

```apache
<VirtualHost *:80>
    ServerName example.com
    DocumentRoot /var/www/gemini
    
    # Main site content
    <Directory /var/www/gemini>
        Gmi2HtmlEnabled on
        Gmi2HtmlHead /etc/apache2/gmi2html/main-head.html
        AddHandler gmi2html .gmi
    </Directory>
    
    # Blog section with different head content
    <Directory /var/www/gemini/blog>
        Gmi2HtmlEnabled on
        Gmi2HtmlHead /etc/apache2/gmi2html/blog-head.html
        AddHandler gmi2html .gmi
    </Directory>
</VirtualHost>
```

## .htaccess Configuration

```apache
Gmi2HtmlEnabled on
Gmi2HtmlStylesheet /var/www/gemini/styles/custom.css
Gmi2HtmlHead /var/www/gemini/includes/head-tags.html
AddHandler gmi2html .gmi
AddType text/gemini .gmi
```

## Head Content File Examples

### Standard SEO and Branding

**File:** `/etc/apache2/gmi2html/head-content.html`

```html
  <meta name="description" content="My Personal Gemini Web Space">
  <meta name="author" content="Jane Doe">
  <meta name="keywords" content="gemini, personal, web">
  <link rel="icon" type="image/svg+xml" href="/favicon.svg">
  <link rel="alternate" type="application/gemini" href="gemini://example.com/">
```

### With Social Media Meta Tags

**File:** `/etc/apache2/gmi2html/head-content.html`

```html
  <meta name="description" content="Gemini-based blog and portfolio">
  <meta name="author" content="Author Name">
  
  <!-- Open Graph for social sharing -->
  <meta property="og:type" content="website">
  <meta property="og:site_name" content="My Gemini Site">
  <meta property="og:image" content="https://example.com/og-image.png">
  
  <!-- Twitter Card -->
  <meta name="twitter:card" content="summary">
  <meta name="twitter:creator" content="@myhandle">
  
  <!-- Icons -->
  <link rel="icon" type="image/svg+xml" href="/favicon.svg">
  <link rel="apple-touch-icon" href="/apple-icon.png">
```

### Minimal Mobile-Optimized

**File:** `/etc/apache2/gmi2html/head-content.html`

```html
  <meta name="description" content="Lightweight Gemini Content">
  <link rel="icon" href="/favicon.ico">
  <link rel="manifest" href="/site.webmanifest">
```

### With Analytics

**File:** `/etc/apache2/gmi2html/head-content.html`

```html
  <meta name="description" content="My Gemini Site">
  
  <!-- Google Analytics -->
  <script async src="https://www.googletagmanager.com/gtag/js?id=G-XXXXXXXXXX"></script>
  <script>
    window.dataLayer = window.dataLayer || [];
    function gtag(){dataLayer.push(arguments);}
    gtag('js', new Date());
    gtag('config', 'G-XXXXXXXXXX');
  </script>
```

### With PWA Support

**File:** `/etc/apache2/gmi2html/head-content.html`

```html
  <meta name="description" content="Progressive Gemini Site">
  <meta name="theme-color" content="#ffffff">
  <meta name="mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
  <link rel="manifest" href="/manifest.json">
  <link rel="icon" type="image/svg+xml" href="/favicon.svg">
  <link rel="apple-touch-icon" href="/apple-touch-icon.png">
```

## Setup Instructions

1. **Create head content file:**
   ```bash
   sudo mkdir -p /etc/apache2/gmi2html
   sudo tee /etc/apache2/gmi2html/head-content.html << 'EOF'
   <meta name="description" content="My Gemini Site">
   <meta name="author" content="Author Name">
   <link rel="icon" type="image/svg+xml" href="/favicon.svg">
   EOF
   ```

2. **Update Apache configuration:**
   ```bash
   sudo nano /etc/apache2/sites-available/default.conf
   ```
   Add: `Gmi2HtmlHead /etc/apache2/gmi2html/head-content.html`

3. **Restart Apache:**
   ```bash
   sudo systemctl restart apache2
   ```

## Notes

- Custom head content is inserted after standard meta tags (charset, viewport) and before the stylesheet
- Head content files must be readable by Apache user (www-data on most systems)
- Use relative paths for CSS/images to ensure they work in all contexts
- External resources must be available (analytics, fonts, etc.)
- Keep head content minimal to avoid bloating HTML output

## Security Considerations

- Ensure only trusted users can edit head content files
- Avoid inline JavaScript from untrusted sources
- Validate external CDN URLs for HTTPS
- Keep analytics/tracking code privacy-compliant
