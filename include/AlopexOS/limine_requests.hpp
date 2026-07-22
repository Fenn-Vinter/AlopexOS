#if !defined(LIMINE_REQUESTS_HPP)
#define LIMINE_REQUESTS_HPP

#include <limine.h>

extern volatile limine_framebuffer_request framebuffer_request;
extern volatile limine_rsdp_request rsdp_request;
extern volatile limine_hhdm_request hhdm_request;
extern volatile limine_executable_address_request exec_addr_request;

#endif