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

#include <cstdio>
#include <vector>
#include "types.h"

namespace PlayStation
{
    /// @brief Defines the interconnect bewteen the CPU and devices.
    class SystemBus final
    {
    public:
        /// @brief Initializes the system bus.
        SystemBus() noexcept;

        /// @brief Resets the system bus to the startup state.
        auto reset() noexcept -> void;

        /// @brief Sets the BIOS data.
        /// @param data The data to use. Be advised that this function does not
        /// check whether or not the data is valid.
        auto set_bios_data(const std::vector<Byte>& data) noexcept -> void;

        /// @brief Returns data from memory.
        /// @tparam T The type of data to read.
        /// @param vaddr The address to read from. This is automatically
        /// converted to a physical address.
        /// @return The data from memory.
        template<typename T>
        auto memory_access(const Word vaddr) noexcept -> T
        {
            // XXX: This technically isn't accurate as it clobbers the Cache
            // Control register (0xFFFE0130), but for now it works.
            const Word paddr{ vaddr & 0x1FFFFFFF };

            T result = 0;

            switch ((paddr & 0xFFFF0000) >> 16)
            {
                // [0x00000000 - 0x001FFFFF]: Main RAM
                case 0x0000 ... 0x001F:
                    std::memcpy(&result, &ram.data()[paddr], sizeof(T));
                    return result;

                // [0x1FC00000 - 0x1FC7FFFF]: BIOS ROM (512 KB)
                case 0x1FC0 ... 0x1FC7:
                    std::memcpy(&result,
                                &bios.data()[paddr & 0x000FFFFF],
                                sizeof(T));
                    return result;

                default:
                    printf("Unknown memory read: 0x%08X, returning 0\n",
                           paddr);
                    return result;
            }
        }

        /// @brief Writes data into memory.
        /// @tparam T The data type of the data.
        /// @param vaddr The address to write to. This is automatically
        /// converted to a physical address.
        /// @param data The data to store.
        template<typename T>
        auto memory_access(const Word vaddr, const T data) noexcept -> void
        {
            // XXX: This technically isn't accurate as it clobbers the Cache
            // Control register (0xFFFE0130), but for now it works.
            const Word paddr{ vaddr & 0x1FFFFFFF };

            switch ((paddr & 0xFFFF0000) >> 16)
            {
                // [0x00000000 - 0x001FFFFF]: Main RAM
                case 0x0000 ... 0x001F:
                    std::memcpy(&ram.data()[paddr], &data, sizeof(T));
                    return;

                default:
                    printf("Unknown memory write: 0x%08X <- 0x%x\n", paddr,
                                                                     data);
                    return;
            }
        }

        /// @brief [0x00000000 - 0x001FFFFF]: Main RAM
        std::vector<Byte> ram;

    private:
        /// @brief Number of bytes that composes the main RAM area.
        const unsigned int RAM_SIZE{ 2097152 };

        /// @brief [0x1FC00000 - 0x1FC7FFFF]: BIOS ROM (512 KB)
        std::vector<Byte> bios;
    };
}
