#include <AlopexOS/drivers/nvme/nvme.hpp>
#include "nvme_completion.hpp"

[[gnu::unused]] constexpr u32 NVMe_REG_CAP  = 0x00;
[[gnu::unused]] constexpr u32 NVMe_REG_VS   = 0x08;
constexpr u32 NVMe_REG_CC   = 0x14;
constexpr u32 NVMe_REG_CSTS = 0x1C;
constexpr u32 NVMe_REG_AQA  = 0x24;
constexpr u32 NVMe_REG_ASQ  = 0x28;
constexpr u32 NVMe_REG_ACQ  = 0x30;

NvmeDriver::NvmeDriver(uptr bar0_mmio_address) 
    : mmio_base(bar0_mmio_address), admin_sq_tail(0), admin_cq_head(0), admin_cq_phase(true) {}

volatile u32* NvmeDriver::get_register(u32 offset) {
    return reinterpret_cast<volatile u32*>(mmio_base + offset);
}

void NvmeDriver::ring_admin_doorbell(u16 tail) {
    volatile u32* doorbell_base = reinterpret_cast<volatile u32*>(mmio_base + 0x1000);
    doorbell_base[0] = tail;
}

bool NvmeDriver::send_admin_command(const NvmeCommand& cmd) {
    auto* sq_entries = reinterpret_cast<NvmeCommand*>(0x100000);
    
    sq_entries[admin_sq_tail] = cmd;
    admin_sq_tail = (admin_sq_tail + 1) % 64;
    ring_admin_doorbell(admin_sq_tail);

    auto* cq_entries = reinterpret_cast<NvmeCompletion*>(0x101000);

    while (true) {
        volatile NvmeCompletion& cqe = cq_entries[admin_cq_head];
        
        bool cqe_phase = (cqe.status & 0x1) != 0;
        if (cqe_phase == admin_cq_phase) {
            admin_cq_head++;
            if (admin_cq_head >= 64) {
                admin_cq_head = 0;
                admin_cq_phase = !admin_cq_phase;
            }

            volatile u32* doorbell_base = reinterpret_cast<volatile u32*>(mmio_base + 0x1000);
            doorbell_base[1] = admin_cq_head;

            u16 status_code = (cqe.status >> 1) & 0xFF;
            return status_code == 0;
        }
    }
}

bool NvmeDriver::initialize() {
    volatile u32* cc = get_register(NVMe_REG_CC);
    volatile u32* csts = get_register(NVMe_REG_CSTS);
    volatile u32* aqa = get_register(NVMe_REG_AQA);
    volatile u32* asq = get_register(NVMe_REG_ASQ);
    volatile u32* acq = get_register(NVMe_REG_ACQ);

    *cc &= ~(1 << 0);
    while ((*csts & (1 << 0)) != 0) {}

    u32 aqa_val = ((64 - 1) << 16) | (64 - 1);
    *aqa = aqa_val;

    *asq = 0x100000;
    *acq = 0x101000;

    u32 cc_val = *cc;
    cc_val &= ~(0x3FU << 16);
    cc_val &= ~(0x7U << 4);
    cc_val |= (6 << 16); 
    cc_val |= (4 << 4);  
    cc_val |= (1 << 0);  
    *cc = cc_val;

    while ((*csts & (1 << 0)) == 0) {}

    NvmeCommand identify_cmd = {};
    identify_cmd.opcode = 0x06; 
    identify_cmd.cid = 1;
    identify_cmd.cdw10 = 1;     
    identify_cmd.prp1 = 0x102000; 

    if (!send_admin_command(identify_cmd)) {
        return false;
    }

    NvmeCommand create_io_cq = {};
    create_io_cq.opcode = 0x05; 
    create_io_cq.cid = 2;
    create_io_cq.prp1 = 0x103000; 
    create_io_cq.cdw10 = ((64 - 1) << 16) | 1; 
    create_io_cq.cdw11 = 1; 

    if (!send_admin_command(create_io_cq)) {
        return false;
    }

    NvmeCommand create_io_sq = {};
    create_io_sq.opcode = 0x01; 
    create_io_sq.cid = 3;
    create_io_sq.prp1 = 0x104000; 
    create_io_sq.cdw10 = ((64 - 1) << 16) | 1; 
    create_io_sq.cdw11 = (1 << 16) | 1; 

    if (!send_admin_command(create_io_sq)) {
        return false;
    }

    return true;
}

void NvmeDriver::ring_io_sq_doorbell(u16 tail) {
    // Queue 1 Submission Doorbell is at offset 0x1000 + (2 * doorbell_stride * 4)
    // Assuming stride factor of 0 (4 bytes per doorbell register)
    volatile u32* doorbell_base = reinterpret_cast<volatile u32*>(mmio_base + 0x1000);
    doorbell_base[2] = tail; // Index 2 is SQ1 Tail
}

void NvmeDriver::ring_io_cq_doorbell(u16 head) {
    volatile u32* doorbell_base = reinterpret_cast<volatile u32*>(mmio_base + 0x1000);
    doorbell_base[3] = head; // Index 3 is CQ1 Head
}

bool NvmeDriver::send_io_command(const NvmeCommand& cmd) {
    auto* sq_entries = reinterpret_cast<NvmeCommand*>(0x104000);
    
    sq_entries[io_sq_tail] = cmd;
    io_sq_tail = (io_sq_tail + 1) % 64;
    ring_io_sq_doorbell(io_sq_tail);

    auto* cq_entries = reinterpret_cast<NvmeCompletion*>(0x103000);

    while (true) {
        volatile NvmeCompletion& cqe = cq_entries[io_cq_head];
        
        bool cqe_phase = (cqe.status & 0x1) != 0;
        if (cqe_phase == io_cq_phase) {
            io_cq_head++;
            if (io_cq_head >= 64) {
                io_cq_head = 0;
                io_cq_phase = !io_cq_phase;
            }

            ring_io_cq_doorbell(io_cq_head);

            u16 status_code = (cqe.status >> 1) & 0xFF;
            return status_code == 0;
        }
    }
}

bool NvmeDriver::read_blocks(u64 lba, u32 count, void* buffer) {
    NvmeCommand read_cmd = {};
    read_cmd.opcode = 0x02; // Read command
    read_cmd.cid = 4;
    read_cmd.nsid = 1;      // Namespace 1
    read_cmd.prp1 = reinterpret_cast<uptr>(buffer);
    read_cmd.cdw10 = static_cast<u32>(lba & 0xFFFFFFFF);
    read_cmd.cdw11 = static_cast<u32>(lba >> 32);
    read_cmd.cdw12 = (count - 1) & 0xFFFF; // Number of logical blocks (0-based)

    return send_io_command(read_cmd);
}

bool NvmeDriver::write_blocks(u64 lba, u32 count, const void* buffer) {
    NvmeCommand write_cmd = {};
    write_cmd.opcode = 0x01; // Write command
    write_cmd.cid = 5;
    write_cmd.nsid = 1;      // Namespace 1
    write_cmd.prp1 = reinterpret_cast<uptr>(buffer);
    write_cmd.cdw10 = static_cast<u32>(lba & 0xFFFFFFFF);
    write_cmd.cdw11 = static_cast<u32>(lba >> 32);
    write_cmd.cdw12 = (count - 1) & 0xFFFF; // Number of logical blocks (0-based)

    return send_io_command(write_cmd);
}