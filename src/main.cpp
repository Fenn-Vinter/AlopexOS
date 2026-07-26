#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include <limine/limine.h>
#include <AlopexOS/limine_requests.hpp>
#include <primitives.hpp>
#include <AlopexOS/display.hpp>
#include <arr.hpp>
#include <string.hpp>
#include <AlopexOS/PCI/pcie.hpp>
#include <AlopexOS/ACPI/acpi.hpp>
#include <AlopexOS/PCI/nvme/nvme.hpp>
#include <AlopexOS/ACPI/mcfg.hpp>
#include <AlopexOS/AlopexIBus/AlopexIBus.hpp>
#include <AlopexOS/gaossd/gaossd.hpp>
#include <AlopexOS/abtrfs/abtrfs.hpp>
#include <AlopexOS/AlopexOS.hpp>
#include <AlopexOS/AVFS/avfs.hpp>
#include <SystemXABI/SystemX.hpp>

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

[[maybe_unused]] static auto serial_print_num(u64 num) -> void {
    char buf[32];
    int pos = 0;
    if (num == 0) {
        serial_out(0x3F8, '0');
        return;
    }
    while (num > 0) {
        buf[pos++] = '0' + (num % 10);
        num /= 10;
    }
    for (int i = pos - 1; i >= 0; i--) {
        serial_out(0x3F8, buf[i]);
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

    [[maybe_unused]] auto& gaossd_inst = AlopexOS::gaossd::get_instance();

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

    int steps = 5;
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
    mainScreen->clear(0x00003366);

    u64 hhdm = ibus.get_hhdm_offset();
    AlopexOS::AbtrFS abtrfs_instance;
    
    uptr base_addr = storage_devices[0].base_address;
    auto dev_handle = static_cast<AlopexOS::Handle>(0);

    serial_print("[ABTRFS] Attempting format via AbtrFS context...\n");
    if (abtrfs_instance.format(base_addr, hhdm, dev_handle)) {
        serial_print("[ABTRFS] Format command completed successfully!\n");
    } else {
        serial_print("[KMAIN] ERROR: Failed to format storage device using AbtrFS context!\n");
        mainScreen->clear(0x00800000);
    }

    AlopexOS::AVFS avfs{};
    AlopexOS::errorCode code = avfs.mount("QEMU NVMe Ctrl://");
    if (code == AlopexOS::errorCode::Success) {
        mainScreen->clear(0x00FF8800);
    } else {
        mainScreen->clear(0x00880000);
    }

    SystemX systemX{};
    systemX.attachAVFS(&avfs);
    //systemX.executeProgram("QEMU NVMe Ctrl://test.vxe");

    dynarr<byte> data = R"(AlopexOS AbtrFS Stress Test Initialization Sequence:
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.
[BLOCK START - 0x001] The quick brown fox jumps over the lazy dog. Pack my box with five dozen liquor jugs. How vexingly quick daft zebras jump! Sphinx of black quartz, judge my vow. Two driven jocks help fax my big quiz. Quick bazookas gauntlets buckeroos.
[BLOCK START - 0x002] Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Donec velit neque, auctor sit amet aliquam vel, ullamcorper sit amet ligula. Proin eget tortor risus. Curabitur arcu erat, accumsan id imperdiet et, porttitor at sem. Praesent sapien massa, convallis a pellentesque nec, egestas non nisi. Nulla porttitor accumsan tincidunt.
[BLOCK START - 0x003] Vivamus suscipit tortor eget felis porttitor volutpat. Curabitur aliquet quam id dui posuere blandit. Nulla quis lorem ut libero malesuada feugiat. Mauris blandit aliquet elit, eget tincidunt nibh pulvinar a. Curabitur non nulla sit amet nisl tempus convallis quis ac lectus.
[BLOCK START - 0x004] Pellentesque in ipsum id orci porta dapibus. Curabitur non nulla sit amet nisl tempus convallis quis ac lectus. Donec sollicitudin molestie malesuada. Vestibulum ac diam sit amet quam vehicula elementum sed sit amet dui. Proin eget tortor risus.
[BLOCK START - 0x005] Cras ultricies ligula sed magna dictum porta. Curabitur arcu erat, accumsan id imperdiet et, porttitor at sem. Nulla quis lorem ut libero malesuada feugiat. Mauris blandit aliquet elit, eget tincidunt nibh pulvinar a. Donec sollicitudin molestie malesuada.
[BLOCK START - 0x006] Integer sollicitudin ligula sed magna dictum porta. Curabitur arcu erat, accumsan id imperdiet et, porttitor at sem. Nulla quis lorem ut libero malesuada feugiat. Mauris blandit aliquet elit, eget tincidunt nibh pulvinar a. Donec sollicitudin molestie malesuada.
[BLOCK START - 0x007] Mauris blandit aliquet elit, eget tincidunt nibh pulvinar a. Curabitur non nulla sit amet nisl tempus convallis quis ac lectus. Donec sollicitudin molestie malesuada. Vestibulum ac diam sit amet quam vehicula elementum sed sit amet dui. Proin eget tortor risus.
[BLOCK START - 0x008] Curabitur non nulla sit amet nisl tempus convallis quis ac lectus. Vivamus suscipit tortor eget felis porttitor volutpat. Curabitur aliquet quam id dui posuere blandit. Nulla quis lorem ut libero malesuada feugiat.
[BLOCK START - 0x009] Donec sollicitudin molestie malesuada. Vestibulum ac diam sit amet quam vehicula elementum sed sit amet dui. Proin eget tortor risus. Cras ultricies ligula sed magna dictum porta. Curabitur arcu erat, accumsan id imperdiet et, porttitor at sem.
[BLOCK START - 0x010] Sed porttitor lectus nibh. Curabitur aliquet quam id dui posuere blandit. Vestibulum ac diam sit amet quam vehicula elementum sed sit amet dui. Curabitur non nulla sit amet nisl tempus convallis quis ac lectus.
[BLOCK START - 0x011] Nulla quis lorem ut libero malesuada feugiat. Curabitur arcu erat, accumsan id imperdiet et, porttitor at sem. Vivamus magna justo, lacinia eget consectetur sed, convallis at tellus. Mauris blandit aliquet elit, eget tincidunt nibh pulvinar a.
[BLOCK START - 0x012] Vestibulum ac diam sit amet quam vehicula elementum sed sit amet dui. Nulla porttitor accumsan tincidunt. Quisque velit nisi, pretium ut lacinia in, elementum id enim. Curabitur aliquet quam id dui posuere blandit.
[BLOCK START - 0x013] Praesent sapien massa, convallis a pellentesque nec, egestas non nisi. Curabitur aliquet quam id dui posuere blandit. Vestibulum ac diam sit amet quam vehicula elementum sed sit amet dui. Donec sollicitudin molestie malesuada.
[BLOCK START - 0x014] Curabitur aliquet quam id dui posuere blandit. Nulla quis lorem ut libero malesuada feugiat. Mauris blandit aliquet elit, eget tincidunt nibh pulvinar a. Curabitur non nulla sit amet nisl tempus convallis quis ac lectus.
[BLOCK START - 0x015] Proin eget tortor risus. Curabitur arcu erat, accumsan id imperdiet et, porttitor at sem. Praesent sapien massa, convallis a pellentesque nec, egestas non nisi. Nulla porttitor accumsan tincidunt.
[BLOCK START - 0x016] Vivamus magna justo, lacinia eget consectetur sed, convallis at tellus. Nulla porttitor accumsan tincidunt. Quisque velit nisi, pretium ut lacinia in, elementum id enim. Curabitur aliquet quam id dui posuere blandit.
[BLOCK START - 0x017] Quisque velit nisi, pretium ut lacinia in, elementum id enim. Curabitur arcu erat, accumsan id imperdiet et, porttitor at sem. Praesent sapien massa, convallis a pellentesque nec, egestas non nisi. Nulla porttitor accumsan tincidunt.
[BLOCK START - 0x018] Donec rutrum congue leo eget malesuada. Curabitur arcu erat, accumsan id imperdiet et, porttitor at sem. Vivamus magna justo, lacinia eget consectetur sed, convallis at tellus. Mauris blandit aliquet elit, eget tincidunt nibh pulvinar a.
[BLOCK START - 0x019] Cursus euismod quis viverra nibh cras pulvinar mattis nunc sed. Eget magna fermentum iaculis eu non diam phasellus vestibulum lorem. Urna molestie at elementum eu facilisis sed odio morbi. Sed vulputate mi sit amet mauris commodo.
[BLOCK START - 0x020] Ultrices eros in cursus turpis massa tincidunt dui ut ornare. Eget nullam non nisi est sit amet facilisis magna. Consequat ac felis donec et odio pellentesque diam volutpat commodo. Amet consectetur adipiscing elit pellentesque habitant morbi tristique senectus.
[END OF STRESS TEST PAYLOAD - AlopexOS AbtrFS Fully Verified])";

    AlopexOS::errorCode write_code = avfs.write("QEMU NVMe Ctrl://Fabian.txt", data);
    if (write_code == AlopexOS::errorCode::Success) {
        serial_print("[KMAIN] Write succeeded!\n");
    } else {
        serial_print("[KMAIN] ERROR: Write failed with error code: ");
        serial_print(AlopexOS::returnErrorAsCstring(write_code));
        serial_print("\n");
    }

    while (true)
    {
        asm volatile("hlt");
    }
}


/*

dynarr<byte> data2 = R"(AlopexOS AbtrFS Secondary File Validation:
This is a second test file written immediately after the stress test payload.
Testing directory listing allocation, multi-file inode handling, and sector offset tracking.)";

    AlopexOS::errorCode write_code2 = avfs.write("QEMU NVMe Ctrl://SecondFile.txt", data2);
    if (write_code2 == AlopexOS::errorCode::Success) {
        serial_print("[KMAIN] Second write (SecondFile.txt) succeeded!\n");
    } else {
        serial_print("[KMAIN] ERROR: Second write failed with error code: ");
        serial_print(AlopexOS::returnErrorAsCstring(write_code2));
        serial_print("\n");
    }

*/