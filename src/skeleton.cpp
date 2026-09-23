#include <skeleton.hpp>
#include <limine/limine.h>


__attribute__((section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = nullptr
};

__attribute__((section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = nullptr
};

__attribute__((section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
    .response = nullptr
};

__attribute__((section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID, 
    .revision = 0,
    .response = nullptr
};

namespace io {

    void Skeleton::init() {
        if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0)
            m_framebuffer = framebuffer_request.response->framebuffers[0];
        
        if (memmap_request.response)
            m_memmap = memmap_request.response;
        
        if (rsdp_request.response)
            m_rsdp = rsdp_request.response->address;
        
        if (hhdm_request.response)
            m_hhdm_offset = hhdm_request.response->offset;

        m_initialized = true;
    }

}