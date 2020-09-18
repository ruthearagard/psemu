// Copyright 2020 Michael Rodriguez
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
// OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
// CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace PlayStation
{
    using Word     = uint32_t;
    using Halfword = uint16_t;
    using Byte     = uint8_t;

    using SignedWord     = int32_t;
    using SignedHalfword = int16_t;
    using SignedByte     = int8_t;

    constexpr auto operator""
    _as_word(const unsigned long long int lvalue) noexcept -> Word
    {
        return lvalue;
    }

    constexpr auto operator""
    _as_halfword(const unsigned long long int lvalue) noexcept -> Halfword
    {
        return lvalue;
    }

    constexpr auto operator""
    _as_byte(const unsigned long long int lvalue) noexcept -> Byte
    {
        return lvalue;
    }

    constexpr auto operator""
    _as_signed_word(const unsigned long long int lvalue) noexcept -> SignedWord
    {
        return lvalue;
    }

    constexpr auto operator""
    _as_signed_halfword(const unsigned long long int lvalue) noexcept -> SignedHalfword
    {
        return lvalue;
    }

    constexpr auto operator""
    _as_signed_byte(const unsigned long long int lvalue) noexcept -> SignedByte
    {
        return lvalue;
    }

    template<typename T>
    constexpr auto sign_extend_word(const T t) noexcept
    {
        return Word(SignedWord(typename std::make_signed<T>::type(t)));
    };

    template<typename T>
    constexpr auto sign_extend_halfword(const T t) noexcept
    {
        return Word(SignedHalfword(typename std::make_signed<T>::type(t)));
    };

    template<typename T>
    constexpr auto zero_extend(T t) noexcept
    { };
}
