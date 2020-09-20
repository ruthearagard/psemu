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

#include "gpu.h"

using namespace PlayStation;

/// @brief Resets the GPU to the startup state.
auto GPU::reset() noexcept -> void
{
    reset_gp0();
    vram.fill(0x0000);
}

/// @brief Resets the GP0 port to accept commands.
auto GPU::reset_gp0() noexcept -> void
{
    gp0_state = GP0State::AwaitingCommand;
    cmd = { };
}

/// @brief Draws a rectangle.
/// @param v0 The first and only vertex data to use.
auto GPU::draw_rect(const Vertex& v0) noexcept -> void
{
    const unsigned int pixel_r = (v0.color & 0x000000FF) / 8;
    const unsigned int pixel_g = ((v0.color >> 8) & 0xFF) / 8;
    const unsigned int pixel_b = ((v0.color >> 16) & 0xFF) / 8;

    vram[v0.x + (VRAM_WIDTH * v0.y)] =
    (pixel_g << 5) | (pixel_b << 10) | pixel_r;
}

/// @brief Converts rectangle command parameters to vertex data, and draws a
/// rectangle.
auto GPU::draw_rect_helper() noexcept -> void
{
    Vertex v0;

    v0.color = cmd.params[0];
    v0.y     = static_cast<SignedHalfword>(cmd.params[1] >> 16);
    v0.x     = static_cast<SignedHalfword>(cmd.params[1] & 0x0000FFFF);

    draw_rect(v0);
    reset_gp0();
}

/// @brief Process a GP0 command packet for rendering and VRAM access.
/// @param packet The GP0 command packet.
auto GPU::gp0(const Word packet) noexcept -> void
{
    switch (gp0_state)
    {
        case GP0State::AwaitingCommand:
            switch (packet >> 24)
            {
                // GP0(0x68) - Monochrome Rectangle(1x1) (Dot) (opaque)
                case 0x68:
                    cmd.params.push_back(packet & 0x00FFFFFF);
                    cmd.remaining_words = 1;

                    cmd.func = [this](const Word) { draw_rect_helper(); };

                    gp0_state = GP0State::ReceivingParameters;
                    break;

                // GP0(0xA0) - Copy Rectangle (CPU to VRAM)
                case 0xA0:
                    cmd.remaining_words = 2;
                    gp0_state = GP0State::ReceivingParameters;

                    cmd.func = [=](const Word data) noexcept -> void
                    {
                        // Current X position
                        static auto vram_x_pos{ 0 };

                        // Current Y position
                        static auto vram_y_pos{ 0 };

                        // Maximum length of a line (should be Xxxx+Xsiz)
                        static auto vram_x_pos_max{ 0 };

                        switch (gp0_state)
                        {
                            case GP0State::ReceivingParameters:
                            {
                                const Halfword width =
                                (((cmd.params[1] & 0x0000FFFF) - 1) & 0x000003FF) + 1;

                                const Halfword height =
                                (((cmd.params[1] >> 16) - 1) & 0x000001FF) + 1;

                                vram_x_pos =
                                ((cmd.params[0] & 0x0000FFFF) & 0x000003FF);

                                vram_y_pos =
                                ((cmd.params[0] >> 16) & 0x000001FF);

                                vram_x_pos_max = vram_x_pos + width;

                                cmd.remaining_words = (width * height) / 2;

                                // Lock the GP0 state to this function.
                                gp0_state = GP0State::ReceivingData;

                                // We don't want to do anything until we
                                // receive at least one data word.
                                return;
                            }

                            case GP0State::ReceivingData:
                            {
                                if (cmd.remaining_words != 0)
                                {
                                    vram[vram_x_pos++ + (VRAM_WIDTH * vram_y_pos)] =
                                    data & 0x0000FFFF;

                                    if (vram_x_pos >= vram_x_pos_max)
                                    {
                                        vram_y_pos++;
                                        vram_x_pos = ((cmd.params[0] & 0x0000FFFF) & 0x000003FF);
                                    }

                                    vram[vram_x_pos++ + (VRAM_WIDTH * vram_y_pos)] =
                                    data >> 16;

                                    if (vram_x_pos >= vram_x_pos_max)
                                    {
                                        vram_y_pos++;
                                        vram_x_pos = ((cmd.params[0] & 0x0000FFFF) & 0x000003FF);
                                    }
                                    cmd.remaining_words--;
                                }

                                if (cmd.remaining_words == 0)
                                {
                                    // All of the expected data has been sent. Return to normal
                                    // operation.
                                    reset_gp0();
                                }
                                return;
                            }
                        }
                    };
                    break;

                // GP0(0xC0) - Copy Rectangle(VRAM to CPU)
                case 0xC0:
                    cmd.remaining_words = 2;
                    gp0_state = GP0State::ReceivingParameters;

                    cmd.func = [=](const Word data) noexcept -> void
                    {
                        // Current X position
                        static auto vram_x_pos{ 0 };

                        // Current Y position
                        static auto vram_y_pos{ 0 };

                        // Maximum length of a line (should be Xxxx+Xsiz)
                        static auto vram_x_pos_max{ 0 };

                        switch (gp0_state)
                        {
                            case GP0State::ReceivingParameters:
                            {
                                const Halfword width =
                                (((cmd.params[1] & 0x0000FFFF) - 1) & 0x000003FF) + 1;

                                const Halfword height =
                                (((cmd.params[1] >> 16) - 1) & 0x000001FF) + 1;

                                vram_x_pos =
                                ((cmd.params[0] & 0x0000FFFF) & 0x000003FF);

                                vram_y_pos =
                                ((cmd.params[0] >> 16) & 0x000001FF);

                                vram_x_pos_max = vram_x_pos + width;

                                cmd.remaining_words = (width * height) / 2;

                                // Lock the GP0 state to this function.
                                gp0_state = GP0State::TransferringData;

                                // We don't want to do anything until we
                                // receive at least one data word.
                                return;
                            }

                            case GP0State::TransferringData:
                            {
                                if (cmd.remaining_words != 0)
                                {
                                    const uint16_t pixel0 =
                                    vram[vram_x_pos++ + (VRAM_WIDTH * vram_y_pos)];

                                    if (vram_x_pos >= vram_x_pos_max)
                                    {
                                        vram_y_pos++;
                                        vram_x_pos = ((cmd.params[0] & 0x0000FFFF) & 0x000003FF);
                                    }
                                         
                                    const Halfword pixel1 =
                                    vram[vram_x_pos++ + (VRAM_WIDTH * vram_y_pos)];

                                    if (vram_x_pos >= vram_x_pos_max)
                                    {
                                        vram_y_pos++;
                                        vram_x_pos = ((cmd.params[0] & 0x0000FFFF) & 0x000003FF);
                                    }

                                    gpuread = ((pixel1 << 16) | pixel0);
                                    cmd.remaining_words--;
                                }

                                if (cmd.remaining_words == 0)
                                {
                                    // All of the expected data has been sent.
                                    // Return to normal operation.
                                    reset_gp0();
                                }
                                return;
                            }
                        }
                    };
                    break;

                default:
                    break;
            }
            break;

        case GP0State::ReceivingParameters:
            cmd.params.push_back(packet);
            cmd.remaining_words--;

            if (cmd.remaining_words == 0)
            {
                cmd.func(0);
            }
            break;

        case GP0State::ReceivingData:
        case GP0State::TransferringData:
            cmd.func(packet);
            break;
    }
}

/// @brief Process a GP1 command packet for display control.
/// @param packet The GP1 command packet to process.
auto GPU::gp1(const Word packet) noexcept -> void
{

}
