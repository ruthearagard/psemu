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

#include <array>
#include <functional>
#include <vector>
#include "types.h"

namespace PlayStation
{
    /// @brief Defines a graphics processing unit (GPU).
    class GPU final
    {
    public:
        /// @brief Resets the GPU to the startup state.
        auto reset() noexcept -> void;

        /// @brief Process a GP0 command packet for rendering and VRAM access.
        /// @param packet The GP0 command packet.
        auto gp0(const Word packet) noexcept -> void;

        /// @brief Process a GP1 command packet for display control.
        /// @param packet The GP1 command packet to process.
        auto gp1(const Word packet) noexcept -> void;

        /// @brief I/O register map
        enum Registers
        {
            /// @brief 0x1F801810 - Send GP0 Commands/Packets
            /// (Rendering and VRAM Access) (W)
            GP0 = 0x810,

            /// @brief 0x1F801814 - Send GP1 Commands (Display/DMA Control) (W)
            GP1 = 0x814,

            /// @brief 0x1F801814 - GPU Status Register (R)
            GPUSTAT = 0x814
        };

        // A1B5G5R5
        VRAM vram;

        /// @brief 0x1F801810 - Receive responses to GP0(0xC0) and GP1(0x10)
        /// commands (R)
        Word gpuread;

    private:
        /// @brief GP0 port state.
        ///
        /// XXX: With proper GPUSTAT implementation, this may not be necessary.
        enum class GP0State
        {
            /// @brief The GP0 port is awaiting a command to process. This is
            /// the normal operation.
            AwaitingCommand,

            /// @brief The GP0 port has received a command and is processing
            /// parameters to the command.
            ReceivingParameters,

            /// @brief The GP0 port is receiving raw data for the command to
            /// use.
            ReceivingData,

            /// @brief The GP0 port is transferring data to GPUREAD.
            TransferringData
        };

        /// @brief Current GP0 command data.
        struct
        {
            /// @brief Parameters to the command.
            std::vector<Word> params;

            /// @brief The function to call when all of the parameters to the
            /// command have been received.
            std::function<void(const Word)> func;

            /// @brief The number of parameters required by the command.
            unsigned int remaining_words;
        } cmd;

        struct Vertex
        {
            /// @brief -1024..+1023
            SignedHalfword x;

            /// @brief -1024..+1023
            SignedHalfword y;

            /// @brief 15-bit color
            Word color;
        };

        /// @brief Resets the GP0 port to accept commands.
        auto reset_gp0() noexcept -> void;

        /// @brief Draws a rectangle.
        /// @param v0 The first and only vertex data to use.
        auto draw_rect(const Vertex& v0) noexcept -> void;

        /// @brief Converts rectangle command parameters to vertex data, and
        /// draws a rectangle.
        auto draw_rect_helper() noexcept -> void;

        /// @brief Current GP0 port state.
        GP0State gp0_state;
    };
}
