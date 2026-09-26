#include <AlopexOS/display.hpp>
#include <AlopexOS/limine_requests.hpp>
#include <new.hpp>

namespace AlopexOS {

static inline void serial_out_dbg(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_print_dbg(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_out_dbg(0x3F8, str[i]);
    }
}

screen::screen() : _frameBuffer(nullptr) {}

screen::screen(limine_framebuffer* buffer) : _frameBuffer(buffer) {}

screen::~screen() = default;

auto screen::getPixels() -> u32* { 
    return static_cast<u32*>(_frameBuffer->address); 
}

auto screen::getPitch() -> u64 { 
    return _frameBuffer->pitch >> 2; 
}

auto screen::get_screen_size() -> uvector2D {
    return uvector2D(_frameBuffer->width, _frameBuffer->height);
}

auto screen::set_pixel(vector2D<u32> coord, u32 color) -> void {
    if (!_frameBuffer || coord.x >= _frameBuffer->width || coord.y >= _frameBuffer->height) return;
    u64 pitch_pixels = _frameBuffer->pitch >> 2;
    u32* fb_pixels = static_cast<u32*>(_frameBuffer->address);
    if (!fb_pixels) return;
    fb_pixels[coord.y * pitch_pixels + coord.x] = color;
}

auto screen::clear(u32 color) -> void {
    if (!_frameBuffer) return;
    u64 pitch_pixels = _frameBuffer->pitch >> 2;
    u32* fb_pixels = static_cast<u32*>(_frameBuffer->address);
    if (!fb_pixels) return;
    u64 width = _frameBuffer->width;
    u64 height = _frameBuffer->height;

    for (u64 y = 0; y < height; y++) {
        u32* row_start = fb_pixels + (y * pitch_pixels);
        for (u64 x = 0; x < width; x++) {
            row_start[x] = color;
        }
    }
}

auto screen::draw_line(vector2D<u32> p0, vector2D<u32> p1, u32 color) -> void {
    int dx = __builtin_abs(static_cast<int>(p1.x) - static_cast<int>(p0.x));
    int sx = p0.x < p1.x ? 1 : -1;
    int dy = -__builtin_abs(static_cast<int>(p1.y) - static_cast<int>(p0.y));
    int sy = p0.y < p1.y ? 1 : -1;
    int err = dx + dy;
    
    int x = p0.x;
    int y = p0.y;

    while (true) {
        set_pixel({static_cast<u32>(x), static_cast<u32>(y)}, color);
        if (x == static_cast<int>(p1.x) && y == static_cast<int>(p1.y)) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

auto screen::draw_rect(vector2D<u32> pos, vector2D<u32> size, u32 color) -> void {
    if (!_frameBuffer) return;
    u32 x1 = pos.x;
    u32 y1 = pos.y;
    u32 x2 = pos.x + size.x;
    u32 y2 = pos.y + size.y;

    u32 max_w = _frameBuffer->width;
    u32 max_h = _frameBuffer->height;
    if (x1 >= max_w || y1 >= max_h) return;
    if (x2 > max_w) x2 = max_w;
    if (y2 > max_h) y2 = max_h;

    for (u32 x = x1; x < x2; x++) {
        set_pixel({x, y1}, color);
        if (y2 - 1 > y1) {
            set_pixel({x, y2 - 1}, color);
        }
    }

    for (u32 y = y1 + 1; y < y2 - 1; y++) {
        set_pixel({x1, y}, color);
        if (x2 - 1 > x1) {
            set_pixel({x2 - 1, y}, color);
        }
    }
}

auto screen::fill_rect(vector2D<u32> pos, vector2D<u32> size, u32 color) -> void {
    if (!_frameBuffer) return;
    u32 x1 = pos.x;
    u32 y1 = pos.y;
    u32 x2 = pos.x + size.x;
    u32 y2 = pos.y + size.y;

    u32 max_w = _frameBuffer->width;
    u32 max_h = _frameBuffer->height;
    if (x1 >= max_w || y1 >= max_h) return;
    if (x2 > max_w) x2 = max_w;
    if (y2 > max_h) y2 = max_h;

    u64 pitch_pixels = _frameBuffer->pitch >> 2;
    u32* fb_pixels = static_cast<u32*>(_frameBuffer->address);
    if (!fb_pixels) return;

    for (u32 y = y1; y < y2; y++) {
        u32* row_start = fb_pixels + (y * pitch_pixels + x1);
        u32 width = x2 - x1;
        
        for (u32 x = 0; x < width; x++) {
            row_start[x] = color;
        }
    }
}

displays::displays() {
    serial_print_dbg("[DISPLAYS] Waiting for Limine framebuffer response...\n");
    
    int counter = 0;
    while(framebuffer_request.response == nullptr) {
        counter++;
        if (counter == 100000000) {
            serial_print_dbg("[DISPLAYS] STUCK: Framebuffer response is still null!\n");
            counter = 0;
        }
        asm volatile("hlt");
    }
    
    serial_print_dbg("[DISPLAYS] Framebuffer response received successfully!\n");
    response = framebuffer_request.response;

    if (response->framebuffer_count == 0 || response->framebuffers == nullptr) {
        serial_print_dbg("[DISPLAYS] ERROR: Framebuffer count is 0 or framebuffers pointer is null!\n");
        return;
    }

    serial_print_dbg("[DISPLAYS] Resizing dynamic screen buffer...\n");
    _screen_buffer.resize(response->framebuffer_count);

    serial_print_dbg("[DISPLAYS] Iterating framebuffers...\n");
    for (u64 i = 0; i < _screen_buffer.size(); i++) {
        if (response->framebuffers[i] == nullptr) {
            serial_print_dbg("[DISPLAYS] ERROR: Encountered null limine_framebuffer pointer!\n");
            continue;
        }
        serial_print_dbg("[DISPLAYS] Direct screen placement initialization...\n");
        new (&_screen_buffer[i]) screen(response->framebuffers[i]);
    }
    serial_print_dbg("[DISPLAYS] Constructor fully completed.\n");
}

auto displays::initialize() -> displays& {
    static u8 storage[sizeof(displays)];
    static bool initialized = false;
    if (!initialized) {
        instance = new (storage) displays();
        initialized = true;
    }
    return *instance;
}

auto displays::get_screen_count() -> u8 { 
    return static_cast<u8>(_screen_buffer.size()); 
}

auto displays::get_screen(u8 id) -> screen* { 
    if (id >= _screen_buffer.size()) return nullptr;
    return &_screen_buffer[id]; 
}

auto displays::get_screens() -> dynarr<screen>* { 
    return &_screen_buffer; 
}

auto displays::get_instance() -> displays& {
    return *instance;
}

}