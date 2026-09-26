#if !defined (NVME_COMPLETION_HPP)
#define NVME_COMPLETION_HPP

#include <primetives.h>

#if defined(_MSC_VER)
    #define PACKED __pragma(pack(push, 1))
    #define PACKED_END ; __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
    #define PACKED
    #define PACKED_END __attribute__((packed));
#endif

PACKED
struct NvmeCompletion {
    u32 dw0;
    u32 rsvd;
    u16 sqhd;
    u16 sqid;
    u16 cid;
    u16 status;
}PACKED_END;

#undef PACKED
#undef PACKED_END

#endif