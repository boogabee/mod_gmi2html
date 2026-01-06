# Project Structure

```
mod_gmi2html/
├── src/
│   ├── mod_gmi2html.c          # Main Apache module (~180 lines)
│   │   ├── Configuration hooks
│   │   ├── Request handler
│   │   ├── File I/O
│   │   └── Module registration
│   │
│   ├── gemini_parser.c         # Gemini parser implementation (~450 lines)
│   │   ├── Line type parsing
│   │   ├── HTML generation
│   │   ├── Character escaping
│   │   └── Memory management
│   │
│   └── gemini_parser.h         # Parser API header (~50 lines)
│       ├── Data structures
│       ├── Function declarations
│       └── Type definitions
│
├── Build Files
│   ├── Makefile                # GNU Make build configuration
│   └── CMakeLists.txt          # CMake build configuration
│
├── Documentation
│   ├── README.md               # Main documentation
│   ├── INSTALL.md              # Installation guide
│   ├── apache-config.conf      # Apache configuration examples
│   └── ARCHITECTURE.md         # This file
│
├── Examples
│   ├── example.gmi             # Example Gemini document
│   └── test.sh                 # Build verification script
│
└── Reference
    └── gemini_specification.txt # Gemini format specification (v0.16.0)
```

## Code Statistics

- **Total Lines of Code**: ~680
- **Main Module**: ~180 lines
- **Parser Library**: ~450 lines
- **Headers**: ~50 lines
- **Documentation**: ~2000 lines
- **Languages**: C (99%), Shell scripts

## Module Dependencies

### Apache Modules
- `httpd` - Apache web server
- `apache2-dev` - Apache development headers

### System Libraries
- `libapr1` - Apache Portable Runtime
- `libapr1-dev` - APR development headers
- `glibc` - C standard library
- `libm` - Math library (if needed)

### Build Tools
- `gcc` - C compiler
- `make` - Build automation
- `apxs2` - Apache extension tool

## Architecture Overview

### Request Flow

```
HTTP Request for .gmi file
         ↓
Apache Handler Hook
         ↓
Module Checks Configuration (Gmi2HtmlEnabled)
         ↓
File Read from Disk
         ↓
Gemini Parser Parses Content
         ↓
Line Type Identification
         ↓
HTML Generation with CSS
         ↓
HTTP Response (text/html)
         ↓
Client Browser
```

### Data Flow in Parser

```
Raw Gemini Content
         ↓
Line Tokenization
         ↓
Line Type Detection
         ├── Heading (# ## ###)
         ├── Link (=>)
         ├── Preformat (```)
         ├── List (* )
         ├── Quote (>)
         └── Text (default)
         ↓
GeminiDocument Structure (Array of GeminiLine)
         ↓
HTML Conversion with Escaping
         ↓
Complete HTML Page
```

## Key Features

### Parser Features
- Single-pass parsing (optimal performance)
- Preformat mode state tracking
- HTML entity escaping
- UTF-8 support
- Line-oriented processing
- Type-safe data structures

### Apache Integration
- Per-directory configuration
- Configuration merging support
- Standard Apache logging
- Proper error handling
- HTTP header management
- Content-type setting

### HTML Output
- Responsive design
- Semantic HTML5
- Embedded CSS styling
- Accessible markup
- SEO-friendly structure

## Configuration Directives

```
Gmi2HtmlEnabled on|off
    Enables/disables conversion in a directory scope
    Context: Directory, .htaccess
    Default: off
```

## Performance Characteristics

- **Parse Time**: O(n) where n = file size
- **Memory Usage**: O(n) for parsed document
- **HTML Generation**: O(n) 
- **Total Request Time**: Depends on file I/O and Apache overhead

### Optimization Opportunities
- Cache parsed documents in memory
- Implement lazy HTML generation
- Use Apache's output filtering for streaming
- Add gzip compression

## Error Handling

### Parse Errors
- Invalid UTF-8 → Default to latin-1 fallback
- Malformed links → Escape and continue
- Memory allocation failure → Return NULL

### HTTP Errors
- File not found → 404 Not Found
- Permission denied → 403 Forbidden
- Memory errors → 500 Internal Server Error
- Invalid config → Configuration error message

## Security Considerations

- All user content (from .gmi files) is HTML-escaped
- No arbitrary code execution
- No file system access outside document root
- Apache authentication/authorization respected
- No CSRF or XSS vulnerabilities introduced

## Gemini Features Supported

### Core Features (Required)
- ✓ Text lines
- ✓ Link lines
- ✓ Preformatted text blocks

### Advanced Features (Optional)
- ✓ Heading levels (#, ##, ###)
- ✓ Unordered lists (* )
- ✓ Block quotes (>)

### Not Applicable in HTML Context
- ✗ Status codes (HTTP used instead)
- ✗ Charset parameter (UTF-8 used)
- ✗ Gemini-specific MIME types

## Testing Coverage

Run the test script to verify:
```bash
./test.sh
```

This checks:
- Compiler availability
- Apache headers present
- APR headers present
- Source files exist
- Module compiles
- Symbols exported

## Future Enhancement Ideas

1. **Caching Layer**
   - In-memory document cache
   - Cache invalidation on file changes
   - Cache statistics

2. **Styling Options**
   - Custom CSS file support
   - Theme selection
   - Dark mode support

3. **Advanced Parsing**
   - Gemini metadata extraction
   - Automatic TOC generation
   - Link validation

4. **Performance**
   - Streaming output
   - Lazy evaluation
   - Memory pooling

5. **Features**
   - Directory index generation
   - Breadcrumb navigation
   - Search functionality
   - Analytics integration

6. **Compatibility**
   - Apache 2.2 support
   - Windows compatibility
   - Alternative build systems

## Building and Testing

### Quick Build
```bash
make clean && make && ./test.sh
```

### Debug Build
```bash
make clean && \
  CFLAGS="-Wall -Wextra -g -O0 -fPIC" make
```

### Static Analysis
```bash
cppcheck src/
splint src/*.c
```

## References

- [Apache Module Development Guide](https://httpd.apache.org/docs/2.4/developer/)
- [Apache APR Library](https://apr.apache.org/)
- [Gemini Specification](https://geminiprotocol.net/)
- [HTTP/1.1 Specification](https://tools.ietf.org/html/rfc7231)
