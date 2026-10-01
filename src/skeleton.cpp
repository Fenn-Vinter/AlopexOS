#include <skeleton.hpp>
#include <limine/limine.h>

__attribute__((used, section(".limine_requests"))) 
volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(0);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = nullptr
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = nullptr
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
    .response = nullptr
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID, 
    .revision = 0,
    .response = nullptr
};

namespace io {

    void Skeleton::init() {

        // 1. Grab HHDM offset first so we can map physical addresses correctly
        if (hhdm_request.response)
            m_hhdm_offset = hhdm_request.response->offset;

        if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0)
            m_framebuffer = framebuffer_request.response->framebuffers[0];
        
        if (memmap_request.response)
            m_memmap = memmap_request.response;
        
        // 2. Convert physical RSDP address to higher-half virtual using HHDM offset
        if (rsdp_request.response && rsdp_request.response->address) {
            m_rsdp = reinterpret_cast<const void*>(
                reinterpret_cast<u64>(rsdp_request.response->address) + m_hhdm_offset
            );
        }

        m_initialized = true;
    }

}