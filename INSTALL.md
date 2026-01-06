# Installation Guide for mod_gmi2html

## Quick Start

```bash
cd /path/to/mod_gmi2html
make
sudo make install
sudo systemctl restart apache2
```

## Detailed Installation Steps

### 1. Install Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y apache2 apache2-dev libapr1-dev build-essential
```

#### CentOS/RHEL
```bash
sudo yum install -y httpd httpd-devel apr-devel gcc make
```

#### Alpine Linux
```bash
apk add --no-cache apache2 apache2-dev apr-dev gcc musl-dev make
```

### 2. Build the Module

#### Using GNU Make (Recommended)

```bash
cd /path/to/mod_gmi2html
make clean
make
```

Troubleshooting:
- If you get "apxs2: command not found", you may need to locate apxs:
  ```bash
  find /usr -name "apxs*" 2>/dev/null
  ```
  Then edit the Makefile and set the correct APXS path.

#### Using Apache's apxs Tool Directly

```bash
cd /path/to/mod_gmi2html
apxs2 -c -i -n gmi2html src/mod_gmi2html.c src/gemini_parser.c
```

#### Using CMake

```bash
cd /path/to/mod_gmi2html
mkdir -p build
cd build
cmake ..
make
sudo make install
```

### 3. Verify Installation

Check that the module file exists:
```bash
ls -la /usr/lib/apache2/modules/mod_gmi2html.so
```

Or for CentOS/RHEL:
```bash
ls -la /usr/lib64/httpd/modules/mod_gmi2html.so
```

### 4. Enable the Module

#### Method A: Using a2enmod (Debian/Ubuntu)
```bash
sudo a2enmod gmi2html
```

#### Method B: Manual Configuration
Edit `/etc/apache2/mods-enabled/gmi2html.load` and add:
```apache
LoadModule gmi2html_module /usr/lib/apache2/modules/mod_gmi2html.so
```

For CentOS/RHEL, edit `/etc/httpd/conf.modules.d/10-gmi2html.conf`:
```apache
LoadModule gmi2html_module /usr/lib64/httpd/modules/mod_gmi2html.so
```

### 5. Configure Apache Sites

Create a configuration file `/etc/apache2/sites-available/gemini.conf`:

```apache
<VirtualHost *:80>
    ServerName gemini.example.com
    DocumentRoot /var/www/gemini
    ErrorLog ${APACHE_LOG_DIR}/gemini-error.log
    CustomLog ${APACHE_LOG_DIR}/gemini-access.log combined
    
    <Directory /var/www/gemini>
        Gmi2HtmlEnabled on
        AddHandler gmi2html .gmi
        AddType text/gemini .gmi
        Options -Indexes FollowSymLinks
        AllowOverride All
        Require all granted
    </Directory>
</VirtualHost>
```

### 6. Enable the Site

#### Debian/Ubuntu
```bash
sudo a2ensite gemini
sudo a2enmod rewrite   # Optional, for URL rewriting
```

#### CentOS/RHEL
The site configuration goes in `/etc/httpd/conf.d/gemini.conf`

### 7. Test Configuration and Restart

```bash
# Test Apache configuration syntax
sudo apache2ctl configtest
# or for CentOS/RHEL:
sudo httpd -t

# Restart Apache
sudo systemctl restart apache2
# or for CentOS/RHEL:
sudo systemctl restart httpd
```

### 8. Create Test Content

```bash
sudo mkdir -p /var/www/gemini
sudo chown www-data:www-data /var/www/gemini
sudo chmod 755 /var/www/gemini

sudo tee /var/www/gemini/test.gmi > /dev/null << 'EOF'
# Test Page

This is a test of mod_gmi2html

=> https://example.com Example Link

\`\`\`
code block
\`\`\`
EOF

sudo chown www-data:www-data /var/www/gemini/test.gmi
```

### 9. Verify Installation

Open your browser and navigate to:
```
http://gemini.example.com/test.gmi
```

You should see the Gemini content rendered as HTML.

## Troubleshooting Installation

### Module Not Loading

Check for errors:
```bash
sudo apache2ctl -t
# Look for errors related to gmi2html

# Check loaded modules
sudo apache2ctl -M | grep gmi2html
```

View detailed error log:
```bash
sudo tail -f /var/log/apache2/error.log
```

### Permission Issues

Ensure Apache can read the module:
```bash
sudo ls -la /usr/lib/apache2/modules/mod_gmi2html.so
sudo chmod 644 /usr/lib/apache2/modules/mod_gmi2html.so
```

Ensure Apache can read the content directory:
```bash
sudo chown -R www-data:www-data /var/www/gemini
sudo chmod -R 755 /var/www/gemini
sudo chmod 644 /var/www/gemini/*.gmi
```

### Compilation Errors

If you get compilation errors:

1. Verify Apache dev headers are installed:
   ```bash
   apt-file search apache2.h | grep -E "^apache2-dev"
   ```

2. Locate the Apache include directory:
   ```bash
   apxs2 -q INCLUDEDIR
   ```

3. Update the Makefile APACHE_INCLUDES if needed

### Files Not Converting

Verify configuration:
```bash
# Check if the directive is in config
sudo grep -r "Gmi2HtmlEnabled" /etc/apache2/

# Verify handler is set
sudo grep -r "AddHandler gmi2html" /etc/apache2/
```

### Performance Issues

For high-traffic sites, enable caching:

Edit the site configuration and add:
```apache
<Directory /var/www/gemini>
    # ... existing config ...
    
    # Enable output caching
    CacheEnable disk /
    CacheDefaultExpire 3600
    CacheMaxExpire 604800
    CacheLock on
    CacheLockPath /tmp/mod_cache_lock
</Directory>
```

Then enable the cache module:
```bash
sudo a2enmod cache
sudo a2enmod cache_disk
sudo systemctl restart apache2
```

## Uninstallation

To remove the module:

```bash
# Disable the module
sudo a2dismod gmi2html

# Remove the module file
sudo rm /usr/lib/apache2/modules/mod_gmi2html.so

# Restart Apache
sudo systemctl restart apache2
```

## Next Steps

1. Read the README.md for detailed usage
2. Check apache-config.conf for more configuration examples
3. Review example.gmi for Gemini syntax examples
4. Configure custom CSS styling if needed

## Getting Help

1. Check Apache error logs: `/var/log/apache2/error.log`
2. Verify module compilation: `make test`
3. Test configuration: `apachectl configtest`
4. Review gemini_specification.txt for format validation
