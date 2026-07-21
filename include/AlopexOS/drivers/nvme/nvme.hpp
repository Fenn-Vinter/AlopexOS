#if !defined (ALOPEXOS_DRIVERS_NVME_NVME_HPP)
#define ALOPEXOS_DRIVERS_NVME_NVME_HPP

#include <primetives.h>
#include "nvme_command.hpp"

class NvmeDriver {
public:
    explicit NvmeDriver(uptr bar0_mmio_address);
    ~NvmeDriver() = default;

    bool initialize();
    bool read_blocks(u64 lba, u32 count, void* buffer);
    bool write_blocks(u64 lba, u32 count, const void* buffer);

private:
    uptr mmio_base;
    
    // Admin queue state
    u16 admin_sq_tail;
    u16 admin_cq_head;
    bool admin_cq_phase;

    // I/O queue state
    u16 io_sq_tail;
    u16 io_cq_head;
    bool io_cq_phase;
    u32 doorbell_stride;

    volatile u32* get_register(u32 offset);
    void ring_admin_doorbell(u16 tail);
    void ring_io_sq_doorbell(u16 tail);
    void ring_io_cq_doorbell(u16 head);
    bool send_admin_command(const NvmeCommand& cmd);
    bool send_io_command(const NvmeCommand& cmd);
};

#endif