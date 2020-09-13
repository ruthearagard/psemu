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

#include "bus.h"

using namespace PlayStation;

/// @brief Initializes the system bus.
SystemBus::SystemBus() noexcept
{
    ram.resize(RAM_SIZE);
    scratchpad.resize(SCRATCHPAD_SIZE);
}

/// @brief Resets the system bus to the startup state.
auto SystemBus::reset() noexcept -> void
{
    ram.clear();
    scratchpad.clear();
}

/// @brief Sets the BIOS data.
/// @param data The data to use. Be advised that this function does not check
/// whether or not the data is valid.
auto SystemBus::set_bios_data(const std::vector<Byte>& data) noexcept -> void
{
    bios = data;
}
