#if !defined(NVME_REGS_HPP)
#define NVME_REGS_HPP

#include <primitives.hpp>

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

struct IdentifyControllerData {
    u16 vid;
    u16 ssvid;
    char sn[20];
    char mn[40];
    char fr[8];
    u8 rab;
    u8 ieee[3];
    u8 cmic;
    u8 mdts;
    u16 cntlid;
    u32 ver;
    u32 rtd3r;
    u32 rtd3e;
    u32 oaes;
    u32 ctratt;
    u8 reserved0[140];
    u8 reserved1[256];
    u8 sqes;
    u8 cqes;
    u16 maxcmd;
    u32 nn;
    u16 oncs;
    u16 fuses;
    u8 fna;
    u8 vwc;
    u16 awun;
    u16 awupf;
    u8 nvscc;
    u8 nwpc;
    u16 acwu;
    u8 reserved2[2];
    u32 sgls;
    u8 reserved3[3572];
} __attribute__((packed));

struct IdentifyNamespaceData {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;
    u8  mc;
    u8  dpc;
    u8  dps;
    u8  nmic;
    u8  rescap;
    u8  fpi;
    u8  dlfeat;
    u16 nawun;
    u16 nawupf;
    u16 nacwu;
    u16 nabsn;
    u16 nabo;
    u16 nabspf;
    u16 noiob;
    u8  nvmcap[16];
    u8  resv1[40];
    u8  nguid[16];
    u8  eui64[8];
    u8  lbaf[64];
    u8  vs[3904];
} __attribute__((packed));

static_assert(sizeof(IdentifyNamespaceData) == 4096, "IdentifyNamespaceData must be 4096 bytes");

static_assert(sizeof(IdentifyControllerData) == 4096, "IdentifyControllerData must be 4096 bytes");

}

#endif