#include <limine/limine.h>
#include <AlopexOS/limine_requests.hpp>
#include <primitives.h>
#include <AlopexOS/display.hpp>
#include <arr.hpp>
#include <AlopexOS/PCI/pcie.hpp>
#include <AlopexOS/ACPI/acpi.hpp>

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

namespace AlopexOS::ACPI {
    struct SDTHeader {
        char signature[4];
        u32 length;
        u8 revision;
        u8 checksum;
        char oem_id[6];
        char oem_table_id[8];
        u32 oem_revision;
        u32 creator_id;
        u32 creator_revision;
    } __attribute__((packed));

    struct RSDPDescriptor {
        char signature[8];
        u8 checksum;
        char oem_id[6];
        u8 revision;
        u32 rsdt_address;
    } __attribute__((packed));
}

namespace {
    bool is_mcfg_signature(const char* sig) {
        return sig[0] == 'M' && sig[1] == 'C' && sig[2] == 'F' && sig[3] == 'G';
    }
}

const AlopexOS::ACPI::MCFGHeader* locate_acpi_table_mcfg(const AlopexOS::ACPI::SDTHeader* rsdt) {
    if (!rsdt) {
        return nullptr;
    }

    u32 entries = (rsdt->length - sizeof(AlopexOS::ACPI::SDTHeader)) / sizeof(u32);
    const u32* table_pointers = reinterpret_cast<const u32*>(
        reinterpret_cast<uptr>(rsdt) + sizeof(AlopexOS::ACPI::SDTHeader)
    );

    for (u32 i = 0; i < entries; ++i) {
        auto* header = reinterpret_cast<const AlopexOS::ACPI::SDTHeader*>(
            static_cast<uptr>(table_pointers[i])
        );

        if (header && is_mcfg_signature(header->signature)) {
            return reinterpret_cast<const AlopexOS::ACPI::MCFGHeader*>(header);
        }
    }

    return nullptr;
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

        for (int i = 0; i < 2000000; i++) {
            asm volatile("" ::: "memory");
        }
    }

    serial_print("[KMAIN] Fade-in complete.\n");

    const AlopexOS::ACPI::MCFGHeader* mcfg = nullptr;

    if (rsdp_request.response != nullptr) {
        serial_print("[ACPI] Limine RSDP response found.\n");
        auto* rsdp = reinterpret_cast<const AlopexOS::ACPI::RSDPDescriptor*>(rsdp_request.response->address);
        if (rsdp) {
            serial_print("[ACPI] Extracting RSDT base address...\n");
            auto* rsdt = reinterpret_cast<const AlopexOS::ACPI::SDTHeader*>(static_cast<uptr>(rsdp->rsdt_address));
            mcfg = locate_acpi_table_mcfg(rsdt);
            if (mcfg) {
                serial_print("[ACPI] MCFG table successfully located in RSDT!\n");
            } else {
                serial_print("[ACPI] MCFG table not found in RSDT. Defaulting to Port I/O fallback.\n");
            }
        }
    } else {
        serial_print("[ACPI] Limine RSDP response is null! Defaulting to Port I/O fallback.\n");
    }

    serial_print("[KMAIN] Initializing PCIe Subsystem...\n");
    auto& pcie = AlopexOS::PCIe::get_instance();
    pcie.initialize(mcfg);

    if (pcie.is_active()) {
        serial_print("[KMAIN] PCIe Subsystem active! Searching for NVMe BAR0...\n");
        uptr nvme_bar0 = pcie.find_nvme_bar0();
        if (nvme_bar0) {
            serial_print("[KMAIN] NVMe Controller successfully leached! BAR0 mapped.\n");
        } else {
            serial_print("[KMAIN] PCIe Subsystem initialized, but no NVMe BAR0 found.\n");
        }
    } else {
        serial_print("[KMAIN] PCIe Subsystem failed to activate.\n");
    }

    mainScreen->clear(0x00FF00FF);

    while (true)
    {
        asm volatile("hlt");
    }
}