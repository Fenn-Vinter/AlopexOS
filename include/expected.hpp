#if !defined (EXPECTED_HPP)
#define EXPECTED_HPP

// AlopexOS Core Expected Primitive
// Brand: Fenn Vinter / AurenFox.Studio

#include "memory.hpp"

#ifndef FN
#define fn auto
#endif

namespace AurenFox::core {

template <typename Error>
class unexpected {
private:
    Error error_;

public:
    constexpr explicit unexpected(const Error& err) : error_(err) {}
    constexpr explicit unexpected(Error&& err) noexcept : error_(AurenFox::memory::move(err)) {}

    [[nodiscard]] constexpr fn error() & noexcept -> Error& {
        return error_;
    }

    [[nodiscard]] constexpr fn error() const& noexcept -> const Error& {
        return error_;
    }
};

template <typename Error>
unexpected(Error) -> unexpected<Error>;

template <typename Type, typename Error>
class Expected {
private:
    union Storage {
        Type value;
        Error error;

        constexpr Storage() noexcept {}
        constexpr ~Storage() {}
    } storage_;

    bool has_value_;

public:
    constexpr explicit Expected(const Type& val) : has_value_(true) {
        AurenFox::memory::construct_at(&storage_.value, val);
    }

    constexpr explicit Expected(Type&& val) noexcept : has_value_(true) {
        AurenFox::memory::construct_at(&storage_.value, AurenFox::memory::move(val));
    }

    constexpr explicit Expected(const unexpected<Error>& err) : has_value_(false) {
        AurenFox::memory::construct_at(&storage_.error, err.error());
    }

    constexpr explicit Expected(unexpected<Error>&& err) noexcept : has_value_(false) {
        AurenFox::memory::construct_at(&storage_.error, AurenFox::memory::move(err).error());
    }

    constexpr ~Expected() {
        if (has_value_) {
            AurenFox::memory::destroy_at(&storage_.value);
        } else {
            AurenFox::memory::destroy_at(&storage_.error);
        }
    }

    [[nodiscard]] constexpr fn has_value() const noexcept -> bool {
        return has_value_;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return has_value_;
    }

    [[nodiscard]] constexpr fn value() & -> Type& {
        return storage_.value;
    }

    [[nodiscard]] constexpr fn value() const& -> const Type& {
        return storage_.value;
    }

    [[nodiscard]] constexpr fn error() & -> Error& {
        return storage_.error;
    }

    [[nodiscard]] constexpr fn error() const& -> const Error& {
        return storage_.error;
    }

    constexpr Expected(const Error& err) : has_value_(false) {
        AurenFox::memory::construct_at(&storage_.error, err);
    }

    constexpr Expected(Error&& err) noexcept : has_value_(false) {
        AurenFox::memory::construct_at(&storage_.error, AurenFox::memory::move(err));
    }
};

} // namespace AurenFox::core

#endif