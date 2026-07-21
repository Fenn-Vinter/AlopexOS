#include <limine/limine.h>
#include <AlopexOS/limine_requests.hpp>
#include <primitives.h>
#include <AlopexOS/display.hpp>
#include <arr.hpp>

static volatile struct {
    uint64_t id[3];
} base_revision
__attribute__((used, section(".limine_requests"))) =
{
    LIMINE_BASE_REVISION(0)
};

static inline void serial_out(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_out(0x3F8, str[i]);
    }
}

extern "C" void kmain()
{
    serial_print("[KMAIN] Starting AlopexOS kernel initialization...\n");

    serial_print("[TEST] Allocating test dynarr...\n");
    dynarr<int> test_arr;
    serial_print("[TEST] Calling resize(5)...\n");
    test_arr.resize(5);
    serial_print("[TEST] Resize succeeded!\n");

    auto& disp = AlopexOS::displays::initialize();
    serial_print("[KMAIN] Display manager initialized.\n");

    auto* mainScreen = disp.get_screen(0);
    if (!mainScreen) {
        serial_print("[KMAIN] ERROR: Failed to retrieve main screen!\n");
        while (true) { asm volatile("hlt"); }
    }
    serial_print("[KMAIN] Main screen pointer acquired successfully.\n");

    uvector2D size = mainScreen->get_screen_size();
    serial_print("[KMAIN] Screen dimensions obtained.\n");

    serial_print("[KMAIN] Beginning fade-in loop...\n");

    uint32_t target_r = 0xFE;
    uint32_t target_g = 0x90;
    uint32_t target_b = 0x02;

    int steps = 60;
    for (int step = 0; step <= steps; step++)
    {
        uint32_t r = (target_r * step) / steps;
        uint32_t g = (target_g * step) / steps;
        uint32_t b = (target_b * step) / steps;
        uint32_t current_color = (r << 16) | (g << 8) | b;

        for (uint y = 0; y < size.y; y++)
        {
            for (uint x = 0; x < size.x; x++)
            {
                mainScreen->set_pixel({x, y}, current_color);
            }
        }

        for (volatile int i = 0; i < 2000000; i++) {
            asm volatile("nop");
        }
    }

    serial_print("[KMAIN] Fade-in complete. Entering infinite halt loop.\n");



    while (true)
    {
        asm volatile("hlt");
    }
}