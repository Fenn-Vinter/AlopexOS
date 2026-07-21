#include <AlopexOS/PCI/nvme/nvme.hpp>

namespace AlopexOS::NVMe {

bool Controller::init(uptr bar0_address) {
    if (!bar0_address) {
        return false;
    }

    m_regs = reinterpret_cast<ControllerRegs*>(bar0_address);

    u64 cap = m_regs->cap;
    m_max_queue_entries = (cap & 0xFFFF) + 1;
    m_db_stride = 1 << ((cap >> 32) & 0xF);

    if (m_regs->cc & 0x1) {
        m_regs->cc &= ~0x1u;
        while (m_regs->csts & 0x1) {
            asm volatile("pause");
        }
    }

    u32 aqa = (63 << 16) | 63;
    m_regs->aqa = aqa;

    m_regs->asq = reinterpret_cast<u64>(&m_admin_sq[0]);
    m_regs->acq = reinterpret_cast<u64>(&m_admin_cq[0]);

    return enable();
}

bool Controller::enable() {
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
            return false;
        }
        asm volatile("pause");
    }

    return true;
}

void Controller::ring_sq_doorbell(u32 qid, u16 value) {
    u32 offset = (2 * qid) * (4 << m_db_stride);
    m_regs->doorbells[offset / sizeof(u32)] = value;
}

void Controller::ring_cq_doorbell(u32 qid, u16 value) {
    u32 offset = ((2 * qid) + 1) * (4 << m_db_stride);
    m_regs->doorbells[offset / sizeof(u32)] = value;
}

}