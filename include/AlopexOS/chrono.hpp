#include <fennlib/types>
#include <fennlib/sys>

namespace AlopexOS::chrono {
    using CCT = fennlib::u64;
    using RTT = fennlib::u64;

    inline fennlib::u64 g_cpu_frequency_hz = 0;

    inline auto read_tsc() -> fennlib::u64 {
        if constexpr (fennlib::sys::bitsize::is_64) {
            fennlib::u32 lo, hi;
            __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
            return (static_cast<fennlib::u64>(hi) << 32) | lo;
        }
        return 0;
    }

    inline auto spin_cycles(CCT cycles) -> void {
        CCT const start = read_tsc();
        while ((read_tsc() - start) < cycles) __asm__ __volatile__ ("pause" ::: "memory");
    }

    inline auto cycles_to_ns(CCT cycles) -> fennlib::u64 {
        if (g_cpu_frequency_hz == 0) return 0;
        return (cycles * 1000000ULL) / (g_cpu_frequency_hz / 1000ULL);
    }

    inline auto cycles_to_us(CCT cycles) -> fennlib::u64 {
        if (g_cpu_frequency_hz == 0) return 0;
        return (cycles * 1000ULL) / (g_cpu_frequency_hz / 1000ULL);
    }

    inline auto cycles_to_ms(CCT cycles) -> fennlib::u64 {
        if (g_cpu_frequency_hz == 0) return 0;
        return cycles / (g_cpu_frequency_hz / 1000ULL);
    }

    class ScopedTimer {
        CCT  m_start{0};
        CCT* m_out_target{nullptr};    
    public:
        explicit ScopedTimer(CCT* out_target)
            : m_start(read_tsc()), m_out_target(out_target) {}

        ~ScopedTimer() {
            if (m_out_target) *m_out_target = read_tsc() - m_start;
        }

        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;
        ScopedTimer(ScopedTimer&&) = delete;
        ScopedTimer& operator=(ScopedTimer&&) = delete;
    };
}