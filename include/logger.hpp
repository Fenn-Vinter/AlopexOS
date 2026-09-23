#pragma once
#include <fennlib/types>
#include <fennlib/string>
#include <fennlib/sequence>

namespace io {

    enum class LogLevel {
        Info,
        Warning,
        Error,
        Critical
    };

    struct LogEntry {
        LogLevel level;
        char message[128];
    };

    class Logger {
    private:
        fennlib::sequence<LogEntry> m_entries{};
        bool m_initialized{false};
        
        static void copy_string(char* dest, const char* src, fennlib::usize max_len) {
            fennlib::usize i = 0;
            while (src[i] != '\0' && i < max_len - 1) {
                dest[i] = src[i];
                i++;
            }
            dest[i] = '\0';
        }

    public:
        constexpr Logger() = default;

        void init() {
            m_entries.clear();
            m_initialized = true;
        }

        void log(LogLevel level, const char* msg) {
            LogEntry entry;
            entry.level = level;
            copy_string(entry.message, msg, sizeof(entry.message));
            m_entries.push_back(entry);
        }

        [[nodiscard]] const fennlib::sequence<LogEntry>& get_entries() const noexcept {
            return m_entries;
        }

        [[nodiscard]] bool is_initialized() const noexcept {
            return m_initialized;
        }
    };   
}