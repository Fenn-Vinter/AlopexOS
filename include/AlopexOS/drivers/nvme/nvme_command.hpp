#if !defined (NVME_COMMAND_HPP)
#define NVME_COMMAND_HPP

#include <primetives.h>

#if defined(_MSC_VER)
    #define PACKED __pragma(pack(push, 1))
    #define PACKED_END ; __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
    #define PACKED
    #define PACKED_END __attribute__((packed));
#endif

PACKED
struct NvmeCommand {
    u8 opcode;
    u8 flags;
    u16 cid;
    u32 nsid;
    u64 rsvd2;
    u64 mptr;
    u64 prp1;
    u64 prp2;
    u32 cdw10;
    u32 cdw11;
    u32 cdw12;
    u32 cdw13;
    u32 cdw14;
    u32 cdw15;
}PACKED_END;

#undef PACKED
#undef PACKED_END

#endif