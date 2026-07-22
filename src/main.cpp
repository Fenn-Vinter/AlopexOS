#include <limine/limine.h>
#include <AlopexOS/limine_requests.hpp>
#include <primitives.h>
#include <AlopexOS/display.hpp>
#include <arr.hpp>
#include <AlopexOS/PCI/pcie.hpp>
#include <AlopexOS/ACPI/acpi.hpp>
#include <AlopexOS/PCI/nvme/nvme.hpp>
#include <AlopexOS/ACPI/mcfg.hpp>
#include <AlopexOS/AlopexIBus/AlopexIBus.hpp>
#include <AlopexOS/gaossd/gaossd.hpp>
#include <AlopexOS/abtrfs/abtrfs.hpp>
#include <AlopexOS/AlopexOS.hpp>

static volatile struct {
    uint64_t id[3];
} base_revision
__attribute__((used, section(".limine_requests"))) =
{
    LIMINE_BASE_REVISION(0)
};

static inline auto serial_out(uint16_t port, uint8_t val) -> void {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static auto serial_print(const char* str) -> void {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_out(0x3F8, str[i]);
    }
}

extern "C" auto kmain() -> void {
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

    auto& ibus = AlopexOS::AlopexIBus::get_instance();
    ibus.scan_all_buses();

    const auto& storage_devices = ibus.get_storage_devices();
    if (storage_devices.size() == 0) {
        serial_print("[KMAIN] ERROR: No storage devices detected on bus!\n");
    } else {
        for (size_t i = 0; i < storage_devices.size(); i++) {
            serial_print("[KMAIN] Found storage device index\n");
        }
    }

    serial_print("[KMAIN] Beginning fade-in loop...\n");

    uint32_t target_r = 0x02;
    uint32_t target_g = 0x66;
    uint32_t target_b = 0x99;

    int steps = 15;
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

        for (int i = 0; i < 2000000; i++) {
            asm volatile("" ::: "memory");
        }
    }

    serial_print("[KMAIN] Fade-in complete.\n");

    mainScreen->clear(0x001E1E24);

    [[maybe_unused]] auto& gaossd_inst = AlopexOS::gaossd::get_instance();

    mainScreen->clear(0x00003366);

    u64 hhdm = ibus.get_hhdm_offset();
    AlopexOS::AbtrFS abtrfs_instance;
    
    uptr base_addr = 0;
    auto dev_handle = static_cast<AlopexOS::Handle>(0);

    serial_print("[ABTRFS] Attempting format via AbtrFS context...\n");
    if (abtrfs_instance.format(base_addr, hhdm, dev_handle)) {
        serial_print("[ABTRFS] Format command completed successfully!\n");
        
        if (abtrfs_instance.mount(base_addr, hhdm, dev_handle)) {
            serial_print("[KMAIN] AbtrFS formatted and mounted successfully!\n");
            mainScreen->clear(0x00008040);

            // --- ABTRFS DATA ROUND-TRIP TEST ---
            serial_print("[TEST] Starting AbtrFS block read/write round-trip test...\n");
            
            u8 write_buffer[512];
            u8 read_buffer[512];

            // Fill write buffer with a recognizable pattern
            for (int i = 0; i < 512; i++) {
                write_buffer[i] = static_cast<u8>(i & 0xFF);
            }
            __builtin_memset(read_buffer, 0, sizeof(read_buffer));

            // Use data block 1 (Block 0 holds the partition header)
            u64 test_data_block = 1;

            serial_print("[TEST] Writing test payload to block 1...\n");
            if (abtrfs_instance.write_block(test_data_block, write_buffer)) {
                serial_print("[TEST] Write block 1: SUCCESS\n");

                serial_print("[TEST] Reading back test payload from block 1...\n");
                if (abtrfs_instance.read_block(test_data_block, read_buffer)) {
                    serial_print("[TEST] Read block 1: SUCCESS\n");

                    // Verify data integrity
                    bool data_match = true;
                    for (int i = 0; i < 512; i++) {
                        if (write_buffer[i] != read_buffer[i]) {
                            data_match = false;
                            break;
                        }
                    }

                    if (data_match) {
                        serial_print("[TEST] DATA INTEGRITY VERIFICATION: PASSED!\n");
                        mainScreen->clear(0x0000FF00); // Flash Green on complete success
                    } else {
                        serial_print("[TEST] ERROR: Data mismatch detected between read and write buffers!\n");
                        mainScreen->clear(0x00FF0000);
                    }
                } else {
                    serial_print("[TEST] ERROR: Failed to read back block 1!\n");
                }
            } else {
                serial_print("[TEST] ERROR: Failed to write block 1!\n");
            }
            // -----------------------------------

            serial_print("[KMAIN] System initialization completed successfully. Entering idle loop.\n");
        } else {
            serial_print("[KMAIN] ERROR: Mount failed immediately after successful format!\n");
            mainScreen->clear(0x00800000);
        }
    } else {
        serial_print("[KMAIN] ERROR: Failed to format storage device using AbtrFS context!\n");
        mainScreen->clear(0x00800000);
    }

    while (true)
    {
        asm volatile("hlt");
    }
}