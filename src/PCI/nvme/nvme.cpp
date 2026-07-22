#include <AlopexOS/PCI/nvme/nvme.hpp>
#include <AlopexOS/limine_requests.hpp>

static inline auto serial_out(u16 port, u8 val) -> void {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static auto serial_print_nvme(const char* str) -> void {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_out(0x3F8, str[i]);
    }
}

static auto serial_print_hex(u16 val) -> void {
    char buf[5];
    for (int i = 3; i >= 0; i--) {
        u8 nibble = (val >> (i * 4)) & 0xF;
        buf[3 - i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
    }
    buf[4] = '\0';
    serial_print_nvme(buf);
}

static auto decode_nvme_status(u16 status_field, const char* context_msg) -> void {
    u8 sct = (status_field >> 9) & 0x7;
    u8 sc = (status_field >> 1) & 0xFF;
    bool dnr = (status_field & (1 << 11)) != 0;
    bool more = (status_field & (1 << 12)) != 0;

    serial_print_nvme("[NVME] --- STATUS DECODE FOR: ");
    serial_print_nvme(context_msg);
    serial_print_nvme(" ---\n");

    serial_print_nvme("[NVME] Raw Status Field: 0x");
    serial_print_hex(status_field);
    serial_print_nvme("\n");

    serial_print_nvme("[NVME]   - Status Code Type (SCT): ");
    switch (sct) {
        case 0: serial_print_nvme("Generic Command Status\n"); break;
        case 1: serial_print_nvme("Command Specific Status\n"); break;
        case 2: serial_print_nvme("Media and Data Integrity Error\n"); break;
        case 3: serial_print_nvme("Path Related Status\n"); break;
        default: serial_print_nvme("Vendor Specific / Reserved\n"); break;
    }

    serial_print_nvme("[NVME]   - Status Code (SC): 0x");
    serial_print_hex(sc);
    serial_print_nvme(" -> ");

    if (sct == 0) {
        switch (sc) {
            case 0x00: serial_print_nvme("Successful Completion\n"); break;
            case 0x01: serial_print_nvme("Invalid Command Opcode\n"); break;
            case 0x02: serial_print_nvme("Invalid Field in Command\n"); break;
            case 0x03: serial_print_nvme("Command ID Conflict\n"); break;
            case 0x04: serial_print_nvme("Data Transfer Error\n"); break;
            case 0x05: serial_print_nvme("Commands Aborted due to Power Loss Notification\n"); break;
            case 0x06: serial_print_nvme("Internal Error\n"); break;
            case 0x07: serial_print_nvme("Command Abort Requested\n"); break;
            case 0x08: serial_print_nvme("Command Aborted due to SQ Deletion\n"); break;
            case 0x09: serial_print_nvme("Command Aborted due to Failed Fused Command\n"); break;
            case 0x0A: serial_print_nvme("Command Aborted due to Missing Fused Command\n"); break;
            case 0x0B: serial_print_nvme("Invalid Namespace or Format (NSID is invalid/inactive)\n"); break;
            case 0x0C: serial_print_nvme("Command Sequence Error\n"); break;
            default: serial_print_nvme("Unknown Generic Status Code\n"); break;
        }
    } else {
        serial_print_nvme("Non-generic status type range\n");
    }

    serial_print_nvme("[NVME]   - Do Not Retry (DNR): ");
    serial_print_nvme(dnr ? "YES\n" : "NO\n");
    serial_print_nvme("[NVME]   - More (MORE): ");
    serial_print_nvme(more ? "YES\n" : "NO\n");
    serial_print_nvme("[NVME] -----------------------------------------\n");
}

auto AlopexOS::NVMe::Controller::init(uptr bar0_physical_address, uptr hhdm_offset) -> bool {
    if (!bar0_physical_address) {
        serial_print_nvme("[NVME] ERROR: bar0_physical_address is zero!\n");
        return false;
    }

    m_hhdm_offset = hhdm_offset;
    m_regs = reinterpret_cast<ControllerRegs*>(bar0_physical_address + m_hhdm_offset);

    u64 cap = m_regs->cap;
    m_max_queue_entries = (cap & 0xFFFF) + 1;
    m_db_stride = (cap >> 32) & 0xF;

    if (m_regs->cc & 0x1) {
        serial_print_nvme("[NVME] Controller was already enabled, resetting...\n");
        m_regs->cc &= ~0x1u;
        while (m_regs->csts & 0x1) {
            asm volatile("pause");
        }
    }

    u32 aqa = (63 << 16) | 63;
    m_regs->aqa = aqa;

    m_regs->asq = static_cast<u64>(virt_to_phys(&m_admin_sq[0]));
    m_regs->acq = static_cast<u64>(virt_to_phys(&m_admin_cq[0]));

    if (!enable()) {
        serial_print_nvme("[NVME] ERROR: Failed to enable controller during init!\n");
        return false;
    }

    if (!identify()) {
        serial_print_nvme("[NVME] ERROR: Failed identify command during init!\n");
        return false;
    }

    if (!identify_namespace()) {
        serial_print_nvme("[NVME] ERROR: Failed identify namespace during init!\n");
        return false;
    }

    if (!format_namespace()) {
        serial_print_nvme("[NVME] ERROR: Failed to format namespace during init!\n");
        return false;
    }

    if (!namespace_attach()) {
        serial_print_nvme("[NVME] ERROR: Failed namespace attach during init!\n");
        return false;
    }

    if (!create_io_queues()) {
        serial_print_nvme("[NVME] ERROR: Failed to create I/O queues during init!\n");
        return false;
    }

    serial_print_nvme("[NVME] Controller init completed successfully.\n");
    return true;
}

auto AlopexOS::NVMe::Controller::enable() -> bool {
    u32 cc = m_regs->cc;
    cc &= ~(0x7u << 4);
    cc &= ~(0xFu << 16);
    cc &= ~(0xFu << 20);
    
    cc |= (0u << 4);
    cc |= (6u << 16);
    cc |= (4u << 20);
    cc |= 0x1u;

    m_regs->cc = cc;

    while (!(m_regs->csts & 0x1)) {
        if (m_regs->csts & 0x2) {
            serial_print_nvme("[NVME] ERROR: Controller fatal status (CFS) set during enable!\n");
            return false;
        }
        asm volatile("pause");
    }

    return true;
}

auto AlopexOS::NVMe::Controller::identify_namespace() -> bool {
    u16 cid = m_sq_tail;
    SubmissionQueueEntry& cmd = m_admin_sq[m_sq_tail];
    __builtin_memset(&cmd, 0, sizeof(SubmissionQueueEntry));

    cmd.cdw0 = 0x06 | (static_cast<u32>(cid) << 16);
    cmd.nsid = 1; 
    cmd.dptr[0] = static_cast<u64>(virt_to_phys(&m_namespace_data));
    cmd.cdw10 = 0x00;

    serial_print_nvme("[NVME] Submitting Identify Namespace (NSID=1)...\n");
    m_sq_tail = (m_sq_tail + 1) % 64;
    ring_sq_doorbell(0, m_sq_tail);

    while (true) {
        volatile CompletionQueueEntry& cqe = m_admin_cq[m_cq_head];
        u8 phase = (cqe.status & 0x1);

        if (phase == m_cq_phase) {
            u16 status_field = cqe.status;
            
            m_cq_head = (m_cq_head + 1) % 64;
            if (m_cq_head == 0) {
                m_cq_phase ^= 1;
            }

            ring_cq_doorbell(0, m_cq_head);
            if (!check_status(status_field)) {
                decode_nvme_status(status_field, "Identify Namespace");
                return false;
            } else {
                serial_print_nvme("[NVME] Identify Namespace completed successfully.\n");
                
                // --- PRINT NAMESPACE METRICS ---
                serial_print_nvme("[NVME INFO] Namespace Size (NSZE): ");
                serial_print_hex(static_cast<u16>(m_namespace_data.nsze >> 48));
                serial_print_hex(static_cast<u16>(m_namespace_data.nsze >> 32));
                serial_print_hex(static_cast<u16>(m_namespace_data.nsze >> 16));
                serial_print_hex(static_cast<u16>(m_namespace_data.nsze & 0xFFFF));
                serial_print_nvme("\n");
            }
            return true;
        }

        asm volatile("pause");
    }
}

auto AlopexOS::NVMe::Controller::format_namespace() -> bool {
    u16 cid = m_sq_tail;

    SubmissionQueueEntry& cmd = m_admin_sq[m_sq_tail];
    __builtin_memset(&cmd, 0, sizeof(SubmissionQueueEntry));

    cmd.cdw0 = 0x80 | (static_cast<u32>(cid) << 16);
    cmd.nsid = 1;
    cmd.cdw10 = 0x00;

    serial_print_nvme("[NVME] Submitting Format NVM (NSID=1, LBAF=0)...\n");
    m_sq_tail = (m_sq_tail + 1) % 64;
    ring_sq_doorbell(0, m_sq_tail);

    while (true) {
        volatile CompletionQueueEntry& cqe = m_admin_cq[m_cq_head];
        u8 phase = (cqe.status & 0x1);

        if (phase == m_cq_phase) {
            u16 status_field = cqe.status;
            
            m_cq_head = (m_cq_head + 1) % 64;
            if (m_cq_head == 0) {
                m_cq_phase ^= 1;
            }

            ring_cq_doorbell(0, m_cq_head);
            if (!check_status(status_field)) {
                decode_nvme_status(status_field, "Format NVM Command");
                return false;
            } else {
                serial_print_nvme("[NVME] Format NVM completed successfully.\n");
            }
            return true;
        }

        asm volatile("pause");
    }
}

auto AlopexOS::NVMe::Controller::namespace_attach() -> bool {
    u16 cid = m_sq_tail;

    SubmissionQueueEntry& cmd = m_admin_sq[m_sq_tail];
    __builtin_memset(&cmd, 0, sizeof(SubmissionQueueEntry));

    // Namespace Management / Attach Opcode for Admin command is 0x15 (Namespace Attach)
    // cdw10 = 0x00 (Attach Namespace, select controller)
    cmd.cdw0 = 0x15 | (static_cast<u32>(cid) << 16);
    cmd.nsid = 1;

    // We can allocate a small buffer containing the controller ID list (usually just controller ID 0)
    struct alignas(4096) AlignedBuffer {
        u8 data[4096];
    };
    static AlignedBuffer attach_buffer;
    __builtin_memset(attach_buffer.data, 0, sizeof(attach_buffer.data));
    // Number of controllers = 1, Controller ID = 0 (or whatever controller ID is reported, typically 0)
    *reinterpret_cast<u16*>(attach_buffer.data) = 1; 
    *reinterpret_cast<u16*>(attach_buffer.data + 2) = 0; // First controller entry = 0

    cmd.dptr[0] = static_cast<u64>(virt_to_phys(attach_buffer.data));
    cmd.cdw10 = 0x00; // Select Namespace Attach (Sel = 0)

    serial_print_nvme("[NVME] Submitting Namespace Attach (NSID=1)...\n");
    m_sq_tail = (m_sq_tail + 1) % 64;
    ring_sq_doorbell(0, m_sq_tail);

    while (true) {
        volatile CompletionQueueEntry& cqe = m_admin_cq[m_cq_head];
        u8 phase = (cqe.status & 0x1);

        if (phase == m_cq_phase) {
            u16 status_field = cqe.status;
            
            m_cq_head = (m_cq_head + 1) % 64;
            if (m_cq_head == 0) {
                m_cq_phase ^= 1;
            }

            ring_cq_doorbell(0, m_cq_head);
            
            if (!check_status(status_field)) {
                // Extract SCT and SC to check for 0x18 (Namespace Already Attached)
                u8 sct = (status_field >> 9) & 0x7;
                u8 sc = (status_field >> 1) & 0xFF;

                if (sct == 1 && sc == 0x18) {
                    serial_print_nvme("[NVME] Namespace is already attached (ignoring status 0x18).\n");
                } else {
                    decode_nvme_status(status_field, "Namespace Attach Command");
                    return false;
                }
            } else {
                serial_print_nvme("[NVME] Namespace Attach completed successfully.\n");
            }
            return true;
        }

        asm volatile("pause");
    }
}

auto AlopexOS::NVMe::Controller::reset() -> bool {
    serial_print_nvme("[NVME] Executing Controller Reset...\n");
    m_regs->cc &= ~0x1u;
    while (m_regs->csts & 0x1) {
        asm volatile("pause");
    }
    return enable();
}

auto AlopexOS::NVMe::Controller::check_status(u16 status_field) -> bool {
    u16 status = (status_field >> 1) & 0x7FFF;
    return status == 0;
}

auto AlopexOS::NVMe::Controller::shutdown() -> bool {
    serial_print_nvme("[NVME] Initiating Controller Shutdown...\n");
    u32 cc = m_regs->cc;
    cc |= (0x2u << 14); // Set SHN to normal shutdown
    m_regs->cc = cc;

    while (!(m_regs->csts & (0x2u << 4))) {
        asm volatile("pause");
    }
    serial_print_nvme("[NVME] Controller Shutdown complete.\n");
    return true;
}

auto AlopexOS::NVMe::Controller::poll_io_completion(u16 expected_cid) -> bool {
    serial_print_nvme("[NVME] Polling for I/O completion with CID...\n");
    (void)expected_cid;
    return true;
}

auto AlopexOS::NVMe::Controller::identify() -> bool {
    u16 cid = m_sq_tail;

    SubmissionQueueEntry& cmd = m_admin_sq[m_sq_tail];
    __builtin_memset(&cmd, 0, sizeof(SubmissionQueueEntry));

    cmd.cdw0 = 0x06 | (static_cast<u32>(cid) << 16);
    cmd.nsid = 0xFFFFFFFF; 
    cmd.dptr[0] = static_cast<u64>(virt_to_phys(&m_identify_data));
    cmd.cdw10 = 0x01; 

    serial_print_nvme("[NVME] Submitting Identify Controller (NSID=FFFFFFFF)...\n");
    m_sq_tail = (m_sq_tail + 1) % 64;
    ring_sq_doorbell(0, m_sq_tail);

    while (true) {
        volatile CompletionQueueEntry& cqe = m_admin_cq[m_cq_head];
        u8 phase = (cqe.status & 0x1);

        if (phase == m_cq_phase) {
            u16 status_field = cqe.status;
            
            m_cq_head = (m_cq_head + 1) % 64;
            if (m_cq_head == 0) {
                m_cq_phase ^= 1;
            }

            ring_cq_doorbell(0, m_cq_head);
            if (!check_status(status_field)) {
                decode_nvme_status(status_field, "Identify Controller");
                return false;
            } else {
                serial_print_nvme("[NVME] Identify Controller completed successfully.\n");
            }
            return true;
        }

        asm volatile("pause");
    }
}

auto AlopexOS::NVMe::Controller::create_io_queues() -> bool {
    {
        u16 cid = m_sq_tail;
        SubmissionQueueEntry& cmd = m_admin_sq[m_sq_tail];
        __builtin_memset(&cmd, 0, sizeof(SubmissionQueueEntry));

        cmd.cdw0 = 0x05 | (static_cast<u32>(cid) << 16);
        cmd.dptr[0] = static_cast<u64>(virt_to_phys(&m_io_cq[0]));
        cmd.cdw10 = (63 << 16) | 1;
        cmd.cdw11 = 0x1;

        serial_print_nvme("[NVME] Submitting Create I/O Completion Queue...\n");
        m_sq_tail = (m_sq_tail + 1) % 64;
        ring_sq_doorbell(0, m_sq_tail);

        while (true) {
            volatile CompletionQueueEntry& cqe = m_admin_cq[m_cq_head];
            u8 phase = (cqe.status & 0x1);
            if (phase == m_cq_phase) {
                u16 status_field = cqe.status;
                m_cq_head = (m_cq_head + 1) % 64;
                if (m_cq_head == 0) m_cq_phase ^= 1;
                ring_cq_doorbell(0, m_cq_head);
                if (!check_status(status_field)) {
                    decode_nvme_status(status_field, "Create I/O Completion Queue");
                    return false;
                }
                break;
            }
            asm volatile("pause");
        }
    }

    {
        u16 cid = m_sq_tail;
        SubmissionQueueEntry& cmd = m_admin_sq[m_sq_tail];
        __builtin_memset(&cmd, 0, sizeof(SubmissionQueueEntry));

        cmd.cdw0 = 0x01 | (static_cast<u32>(cid) << 16);
        cmd.dptr[0] = static_cast<u64>(virt_to_phys(&m_io_sq[0]));
        cmd.cdw10 = (63 << 16) | 1;
        cmd.cdw11 = (1 << 16) | 1;

        serial_print_nvme("[NVME] Submitting Create I/O Submission Queue...\n");
        m_sq_tail = (m_sq_tail + 1) % 64;
        ring_sq_doorbell(0, m_sq_tail);

        while (true) {
            volatile CompletionQueueEntry& cqe = m_admin_cq[m_cq_head];
            u8 phase = (cqe.status & 0x1);
            if (phase == m_cq_phase) {
                u16 status_field = cqe.status;
                m_cq_head = (m_cq_head + 1) % 64;
                if (m_cq_head == 0) m_cq_phase ^= 1;
                ring_cq_doorbell(0, m_cq_head);
                if (!check_status(status_field)) {
                    decode_nvme_status(status_field, "Create I/O Submission Queue");
                    return false;
                }
                break;
            }
            asm volatile("pause");
        }
    }

    serial_print_nvme("[NVME] I/O Queues created successfully.\n");
    return true;
}

auto AlopexOS::NVMe::Controller::read(PhysicalAddress dest_phys, u64 lba, u32 count) -> errorCode {
    u16 cid = m_io_sq_tail;
    SubmissionQueueEntry& cmd = m_io_sq[m_io_sq_tail];
    __builtin_memset(&cmd, 0, sizeof(SubmissionQueueEntry));

    // CDW0: Opcode 0x02 in bits [7:0], Command ID in bits [31:16]
    cmd.cdw0 = 0x02 | (static_cast<u32>(cid) << 16);
    cmd.nsid = 1;
    
    // Ensure address is strictly physical
    u64 phys_target = dest_phys;
    if (phys_target > m_hhdm_offset) {
        phys_target = virt_to_phys(reinterpret_cast<void*>(phys_target));
    }
    cmd.dptr[0] = phys_target;

    // CDW10/11: Starting LBA (64-bit)
    cmd.cdw10 = static_cast<u32>(lba & 0xFFFFFFFF);
    cmd.cdw11 = static_cast<u32>((lba >> 32) & 0xFFFFFFFF);
    
    // CDW12: Number of Logical Blocks (0-based, so count - 1)
    cmd.cdw12 = (count - 1) & 0xFFFF;

    serial_print_nvme("[NVME] Submitting I/O Read Command...\n");
    m_io_sq_tail = (m_io_sq_tail + 1) % 64;
    ring_sq_doorbell(1, m_io_sq_tail);

    while (true) {
        volatile CompletionQueueEntry& cqe = m_io_cq[m_io_cq_head];
        u8 phase = (cqe.status & 0x1);

        if (phase == m_io_cq_phase) {
            u16 status_field = cqe.status;
            
            if (!check_status(status_field)) {
                decode_nvme_status(status_field, "I/O Read Command");
            }

            m_io_cq_head = (m_io_cq_head + 1) % 64;
            if (m_io_cq_head == 0) {
                m_io_cq_phase ^= 1;
            }

            ring_cq_doorbell(1, m_io_cq_head);
            return check_status(status_field) ? errorCode::Success : errorCode::Generic_Error;
        }

        asm volatile("pause");
    }
}

auto AlopexOS::NVMe::Controller::write(PhysicalAddress src_phys, u64 lba, u32 count) -> errorCode {
    u16 cid = m_io_sq_tail;
    SubmissionQueueEntry& cmd = m_io_sq[m_io_sq_tail];
    __builtin_memset(&cmd, 0, sizeof(SubmissionQueueEntry));

    // CDW0: Opcode 0x01 in bits [7:0], Command ID in bits [31:16]
    cmd.cdw0 = 0x01 | (static_cast<u32>(cid) << 16);
    cmd.nsid = 1;

    // Ensure address is strictly physical
    u64 phys_source = src_phys;
    if (phys_source > m_hhdm_offset) {
        phys_source = virt_to_phys(reinterpret_cast<void*>(phys_source));
    }
    cmd.dptr[0] = phys_source;

    // CDW10/11: Starting LBA (64-bit)
    cmd.cdw10 = static_cast<u32>(lba & 0xFFFFFFFF);
    cmd.cdw11 = static_cast<u32>((lba >> 32) & 0xFFFFFFFF);
    
    // CDW12: Number of Logical Blocks (0-based)
    cmd.cdw12 = (count - 1) & 0xFFFF;

    serial_print_nvme("[NVME] Submitting I/O Write Command...\n");
    m_io_sq_tail = (m_io_sq_tail + 1) % 64;
    ring_sq_doorbell(1, m_io_sq_tail);

    while (true) {
        volatile CompletionQueueEntry& cqe = m_io_cq[m_io_cq_head];
        u8 phase = (cqe.status & 0x1);

        if (phase == m_io_cq_phase) {
            u16 status_field = cqe.status;
            
            if (!check_status(status_field)) {
                decode_nvme_status(status_field, "I/O Write Command");
            }

            m_io_cq_head = (m_io_cq_head + 1) % 64;
            if (m_io_cq_head == 0) {
                m_io_cq_phase ^= 1;
            }

            ring_cq_doorbell(1, m_io_cq_head);
            return check_status(status_field) ? errorCode::Success : errorCode::Generic_Error;
        }

        asm volatile("pause");
    }
}

auto AlopexOS::NVMe::Controller::ring_sq_doorbell(u32 qid, u16 value) -> void {
    uptr base = reinterpret_cast<uptr>(m_regs) + 0x1000;
    u32 stride_bytes = 4 << m_db_stride;
    u32 offset = (2 * qid) * stride_bytes;
    *reinterpret_cast<volatile u32*>(base + offset) = value;
}

auto AlopexOS::NVMe::Controller::ring_cq_doorbell(u32 qid, u16 value) -> void {
    uptr base = reinterpret_cast<uptr>(m_regs) + 0x1000;
    u32 offset = ((2 * qid) + 1) * (4 << m_db_stride);
    *reinterpret_cast<volatile u32*>(base + offset) = value;
}

auto AlopexOS::NVMe::Controller::virt_to_phys(const void* virt_ptr) const -> uptr {
    uptr virt = reinterpret_cast<uptr>(virt_ptr);

    if (exec_addr_request.response != nullptr) {
        uptr k_virt = exec_addr_request.response->virtual_base;
        uptr k_phys = exec_addr_request.response->physical_base;

        if (virt >= k_virt) {
            return (virt - k_virt) + k_phys;
        }
    }

    if (virt >= m_hhdm_offset) {
        return virt - m_hhdm_offset;
    }

    return virt;
}