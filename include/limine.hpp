#pragma once
#include <limine/limine.h>
#include <fennlib/types>

namespace fennlib::boot {
    __attribute__((used, section(".requests")))
    inline volatile fennlib::u64 limine_base_revision[3] = LIMINE_BASE_REVISION(2);

    __attribute__((used, section(".requests")))
    inline volatile struct limine_framebuffer_request framebuffer_request = {
        .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
        .revision = 0,
        .response = nullptr
    };
    
    __attribute__((used, section(".requests")))
    inline volatile struct limine_memmap_request memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST_ID,
        .revision = 0,
        .response = nullptr
    };
    
    __attribute__((used, section(".requests")))
    inline volatile struct limine_rsdp_request rsdp_request = {
        .id = LIMINE_RSDP_REQUEST_ID,
        .revision = 0,
        .response = nullptr
    };

    __attribute__((used, section(".requests")))
    inline volatile struct limine_smbios_request smbios_request = {
        .id = LIMINE_SMBIOS_REQUEST_ID,
        .revision = 0,
        .response = nullptr
    };

    inline void halt() {
        for (;;) {
            asm volatile ("hlt");
        }
    }

    inline struct limine_framebuffer* get_primary_framebuffer() {
        if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) {
            halt();
        }

        if (framebuffer_request.response == nullptr || 
            framebuffer_request.response->framebuffer_count < 1) {
            halt();
        }

        return framebuffer_request.response->framebuffers[0];
    }
    
    inline struct limine_memmap_response* get_memmap() {
        if (memmap_request.response == nullptr) {
            halt();
        }
        return memmap_request.response;
    }

    inline struct limine_rsdp_response* get_rsdp() {
        if (rsdp_request.response == nullptr) {
            halt();
        }
        return rsdp_request.response;
    }

    inline struct limine_smbios_response* get_smbios() {
        if (smbios_request.response == nullptr) {
            return nullptr;
        }
        return smbios_request.response;
    }
}