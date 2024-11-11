# Flipper Zero ViewPort API Reference

## Core ViewPort Functions

### Creation & Destruction

```c
ViewPort* view_port_alloc(void);
void view_port_free(ViewPort* view_port);
```

### Size Control

```c
void view_port_set_width(ViewPort* view_port, uint8_t width);
uint8_t view_port_get_width(const ViewPort* view_port);
void view_port_set_height(ViewPort* view_port, uint8_t height);
uint8_t view_port_get_height(const ViewPort* view_port);
```

### Enable/Disable

```c
void view_port_enabled_set(ViewPort* view_port, bool enabled);
bool view_port_is_enabled(const ViewPort* view_port);
```

### Callbacks

```c
void view_port_draw_callback_set(ViewPort* view_port, ViewPortDrawCallback callback, void* context);
void view_port_input_callback_set(ViewPort* view_port, ViewPortInputCallback callback, void* context);
```

### Updates & Orientation

```c
void view_port_update(ViewPort* view_port);
void view_port_set_orientation(ViewPort* view_port, ViewPortOrientation orientation);
ViewPortOrientation view_port_get_orientation(const ViewPort* view_port);
```

## ViewPort Orientation Types

```c
typedef enum {
    ViewPortOrientationHorizontal,
    ViewPortOrientationHorizontalFlip,
    ViewPortOrientationVertical,
    ViewPortOrientationVerticalFlip,
} ViewPortOrientation;
```

## GUI Layer Types

```c
typedef enum {
    GuiLayerDesktop,     // Desktop layer with status bar
    GuiLayerWindow,      // Window layer with status bar
    GuiLayerStatusBarLeft,  // Status bar left auto-layout
    GuiLayerStatusBarRight, // Status bar right auto-layout
    GuiLayerFullscreen,  // Fullscreen layer no status bar
} GuiLayer;
```

## GUI Integration Functions

```c
void gui_add_view_port(Gui* gui, ViewPort* view_port, GuiLayer layer);
void gui_remove_view_port(Gui* gui, ViewPort* view_port);
void gui_view_port_send_to_front(Gui* gui, ViewPort* view_port);
void gui_view_port_send_to_back(Gui* gui, ViewPort* view_port);
```

## Canvas Drawing Functions

### Text

```c
void canvas_draw_str(Canvas* canvas, int32_t x, int32_t y, const char* str);
void canvas_draw_str_aligned(Canvas* canvas, int32_t x, int32_t y, Align horizontal, Align vertical, const char* str);
uint16_t canvas_string_width(Canvas* canvas, const char* str);
size_t canvas_glyph_width(Canvas* canvas, uint16_t symbol);
```

### Basic Drawing

```c
void canvas_draw_dot(Canvas* canvas, int32_t x, int32_t y);
void canvas_draw_box(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);
void canvas_draw_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);
void canvas_draw_line(Canvas* canvas, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void canvas_draw_circle(Canvas* canvas, int32_t x, int32_t y, size_t radius);
void canvas_draw_disc(Canvas* canvas, int32_t x, int32_t y, size_t radius);
void canvas_draw_triangle(Canvas* canvas, int32_t x, int32_t y, size_t base, size_t height, CanvasDirection dir);
```

### Rounded Elements

```c
void canvas_draw_rframe(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height, size_t radius);
void canvas_draw_rbox(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height, size_t radius);
```

### Icons & Images

```c
void canvas_draw_icon(Canvas* canvas, int32_t x, int32_t y, const Icon* icon);
void canvas_draw_icon_ex(Canvas* canvas, int32_t x, int32_t y, const Icon* icon, IconRotation rotation);
void canvas_draw_icon_animation(Canvas* canvas, int32_t x, int32_t y, IconAnimation* icon_animation);
void canvas_draw_xbm(Canvas* canvas, int32_t x, int32_t y, size_t w, size_t h, const uint8_t* bitmap);
```

### Canvas Settings

```c
void canvas_clear(Canvas* canvas);
void canvas_set_color(Canvas* canvas, Color color);
void canvas_set_font(Canvas* canvas, Font font);
void canvas_set_font_direction(Canvas* canvas, CanvasDirection dir);
void canvas_invert_color(Canvas* canvas);
void canvas_set_custom_u8g2_font(Canvas* canvas, const uint8_t* font);
void canvas_set_bitmap_mode(Canvas* canvas, bool alpha);
```

## Canvas Types

### Colors

```c
typedef enum {
    ColorWhite = 0x00,
    ColorBlack = 0x01,
    ColorXOR = 0x02,
} Color;
```

### Fonts

```c
typedef enum {
    FontPrimary,
    FontSecondary,
    FontKeyboard,
    FontBigNumbers,
} Font;
```

### Alignment

```c
typedef enum {
    AlignLeft,
    AlignRight,
    AlignTop,
    AlignBottom,
    AlignCenter,
} Align;
```

### Direction

```c
typedef enum {
    CanvasDirectionLeftToRight,
    CanvasDirectionTopToBottom,
    CanvasDirectionRightToLeft,
    CanvasDirectionBottomToTop,
} CanvasDirection;
```

### Input Event Types

```c
typedef enum {
    InputTypePress,
    InputTypeRelease,
    InputTypeShort,
    InputTypeLong,
    InputTypeRepeat,
} InputType;

typedef enum {
    InputKeyUp,
    InputKeyDown,
    InputKeyRight,
    InputKeyLeft,
    InputKeyOk,
    InputKeyBack,
} InputKey;
```
