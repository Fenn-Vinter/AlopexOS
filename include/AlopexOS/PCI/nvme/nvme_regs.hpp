#if !defined(NVME_REGS_HPP)
#define NVME_REGS_HPP

#include <primitives.h>

namespace AlopexOS::NVMe {

struct ControllerRegs {
    volatile u64 cap;
    volatile u32 vs;
    volatile u32 intms;
    volatile u32 intmc;
    volatile u32 cc;
    volatile u32 reserved0;
    volatile u32 csts;
    volatile u32 nssr;
    volatile u32 aqa;
    volatile u64 asq;
    volatile u64 acq;
    volatile u32 cmbloc;
    volatile u32 cmbsz;
    volatile u32 bpinfo;
    volatile u32 bprsel;
    volatile u64 bpmbl;
    volatile u64 cmbmsc;
    volatile u64 cmbsts;
    volatile u32 reserved1[848];
    volatile u32 doorbells[];
} __attribute__((packed));

struct SubmissionQueueEntry {
    u32 cdw0;
    u32 nsid;
    u64 reserved0;
    u64 mptr;
    u64 dptr[2];
    u32 cdw10;
    u32 cdw11;
    u32 cdw12;
    u32 cdw13;
    u32 cdw14;
    u32 cdw15;
} __attribute__((packed));

struct CompletionQueueEntry {
    u32 dw0;
    u32 reserved;
    u16 sq_head;
    u16 sq_id;
    u16 command_id;
    u16 status;
} __attribute__((packed));

}

#endif