#include "primitives.h"
#if !defined(NVME_HPP)
#define NVME_HPP

#include <AlopexOS/PCI/nvme/nvme_regs.hpp>
#include <AlopexOS/AlopexOS.hpp>

namespace AlopexOS::NVMe {

class Controller {
public:
    static Controller& get_instance() {
        static Controller instance;
        return instance;
    }

    auto init(uptr bar0_physical_address, uptr hhdm_offset) -> bool;
    auto enable() -> bool;
    auto identify() -> bool;
    auto create_io_queues() -> bool;
    auto identify_namespace() -> bool;
    auto format_namespace() -> bool;
    auto namespace_attach() -> bool;
    auto reset() -> bool;
    auto check_status(u16 status_field) -> bool;
    auto shutdown() -> bool;
    auto poll_io_completion(u16 expected_cid) -> bool;

    auto read(PhysicalAddress dest_phys, u64 lba, u32 count) -> errorCode;
    auto write(PhysicalAddress src_phys, u64 lba, u32 count) -> errorCode;

    const IdentifyControllerData& get_identify_data() const { return m_identify_data; }

    // Made public so it can be accessed from kmain() or external modules for testing
    auto virt_to_phys(const void* virt_ptr) const -> uptr;

private:
    Controller() = default;

    ControllerRegs* m_regs{nullptr};
    uptr m_hhdm_offset{0};
    u32 m_db_stride{0};
    u16 m_max_queue_entries{0};

    alignas(4096) SubmissionQueueEntry m_admin_sq[64]{};
    alignas(4096) CompletionQueueEntry m_admin_cq[64]{};
    alignas(4096) SubmissionQueueEntry m_io_sq[64]{};
    alignas(4096) CompletionQueueEntry m_io_cq[64]{};
    alignas(4096) IdentifyControllerData m_identify_data{};
    alignas(4096) IdentifyNamespaceData m_namespace_data{};

    u16 m_sq_tail{0};
    u16 m_cq_head{0};
    u8 m_cq_phase{1};

    u16 m_io_sq_tail{0};
    u16 m_io_cq_head{0};
    u8 m_io_cq_phase{1};

    auto ring_sq_doorbell(u32 qid, u16 value) -> void;
    auto ring_cq_doorbell(u32 qid, u16 value) -> void;
};

}

#endif