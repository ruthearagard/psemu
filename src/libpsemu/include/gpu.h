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
        auto reset() noexcept -> void;

        /// @brief Process a GP0 command packet for rendering and VRAM access.
        /// @param packet The GP0 command packet.
        auto gp0(const Word packet) noexcept -> void;

        /// @brief Process a GP1 command packet for display control.
        /// @param packet The GP1 command packet to process.
        auto gp1(const Word packet) noexcept -> void;

        enum Registers
        {
            GP0     = 0x810,
            GP1     = 0x814,
            GPUSTAT = 0x814
        };

        static constexpr auto VRAM_WIDTH{ 1024 };
        static constexpr auto VRAM_HEIGHT{ 512 };

        using VRAMData = std::array<Halfword, VRAM_WIDTH* VRAM_HEIGHT>;

        // A1B5G5R5
        VRAMData vram;

        Word gpuread;

    private:
        enum GP0State
        {
            AwaitingCommand,
            ReceivingParameters,
            ReceivingData,
            TransferringData
        };

        struct
        {
            std::vector<Word> params;
            std::function<void(const Word)> func;
            unsigned int remaining_words;
        } cmd;

        struct Vertex
        {
            // -1024..+1023
            SignedHalfword x;
            SignedHalfword y;
            Word color;
        };

        auto draw_rect(const Vertex& v0) noexcept -> void;

        auto draw_rect_helper() noexcept -> void;
        GP0State gp0_state;
    };
}
