#if !defined(NVME_HPP)
#define NVME_HPP

#include <AlopexOS/PCI/nvme/nvme_regs.hpp>

namespace AlopexOS::NVMe {

class Controller {
public:
    static Controller& get_instance() {
        static Controller instance;
        return instance;
    }

    bool init(uptr bar0_address);
    bool enable();
    
private:
    Controller() = default;

    ControllerRegs* m_regs{nullptr};
    u32 m_db_stride{0};
    u16 m_max_queue_entries{0};

    alignas(4096) SubmissionQueueEntry m_admin_sq[64]{};
    alignas(4096) CompletionQueueEntry m_admin_cq[64]{};

    u16 m_sq_tail{0};
    u16 m_cq_head{0};
    u8 m_cq_phase{1};

    void ring_sq_doorbell(u32 qid, u16 value);
    void ring_cq_doorbell(u32 qid, u16 value);
};

}

#endif