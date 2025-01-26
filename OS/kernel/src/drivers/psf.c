#include <kernel/drivers/psf.h>

#define PSF1_FONT_MAGIC 0x0436
#define PSF_FONT_MAGIC 0x864ab572

typedef struct {
    uint16_t magic;         // Magic for file identification
    uint8_t fontMode;       // PSF font mode
    uint8_t characterSize;  // PSF character size
} PSF1_Header;

typedef struct {
    uint32_t magic;
    uint32_t version;       // Zero
    uint32_t headerSize;    // Offset of bitmaps
    uint32_t flags;         // 0 if no unicode table
    uint32_t numGlyphs;     // Number of glyphs
    uint32_t bytesPerGlyph; // Size of each glyph
    uint32_t height;        // Height in pixels
    uint32_t width;         // Width in pixels
} PSF_Font;
