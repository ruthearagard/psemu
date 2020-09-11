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
#include <cstdint>
#include "types.h"

namespace PlayStation
{
    class SystemBus;

    /// @brief Defines an LSI LR33300 interpreter.
    class CPU final
    {
    public:
        /// @brief Initializes the CPU.
        /// @param b The system bus instance.
        explicit CPU(SystemBus& b) noexcept;

        /// @brief Resets the CPU to the startup state. Officially, this is
        /// considered a reset exception.
        auto reset() noexcept -> void;

        /// @brief Executes the next instruction.
        auto step() noexcept -> void;

        /// @brief General purpose registers.
        std::array<Word, 32> gpr;

        /// @brief Program counter.
        Word pc;

        /// @brief Next program counter, used for emulating branch delay slots.
        Word next_pc;

        /// @brief The remainder of a division operation, or the high 32 bits
        /// of a multiplication operation.
        Word hi;

        /// @brief The quotient of a division operation, or the lower 32 bits
        /// of a multiplication operation.
        Word lo;

        /// @brief Current instruction.
        union
        {
            struct
            {
                /// @brief Function field
                unsigned int funct : 6;

                /// @brief Shift amount
                unsigned int shamt : 5;

                /// @brief Destination register specifier
                unsigned int rd : 5;

                /// @brief Target (source/destination) register
                unsigned int rt : 5;

                /// @brief Source register specifier
                unsigned int rs : 5;

                /// @brief Operation code
                unsigned int op : 6;
            };
            Word word;
        } instruction;

        /// @brief System control co-processor instructions (COP0)
        std::array<Word, 32> cop0;

        /// @brief Returns the 26-bit target address.
        auto target() const noexcept -> unsigned int;

        /// @brief Returns the lower 16-bits of the instruction.
        auto immediate() const noexcept -> unsigned int;

        /// @brief Same as the `immediate()` method, merely an alias as defined
        /// by MIPS conventions.
        auto offset() const noexcept -> unsigned int;

        /// @brief Same as referencing the `rs` field of the current
        /// instruction, merely an alias as defined by MIPS conventions.
        auto base() const noexcept -> unsigned int;

        /// @brief Returns the current virtual address.
        auto vaddr() const noexcept -> unsigned int;

        /// @brief Instruction
        enum COP0Register
        {
            BPC   = 3,
            BDA   = 5,
            TAR   = 6,
            DCIC  = 7,
            BDAM  = 9,
            BPCM  = 11,
            SR    = 12,
            Cause = 13
        };

        /// @brief Instruction groups
        enum InstructionGroup
        {
            SPECIAL = 0x00,
            BCOND   = 0x01,
            COP0    = 0x10
        };

        /// @brief Inherent co-processor instructions
        enum CoprocessorInstruction
        {
            MF = 0x00,
            MT = 0x04
        };

        /// @brief Instructions referenced in opcode bits [31:26].
        enum Instruction
        {
            J     = 0x02,
            JAL   = 0x03,
            BEQ   = 0x04,
            BNE   = 0x05,
            BLEZ  = 0x06,
            BGTZ  = 0x07,
            ADDI  = 0x08,
            ADDIU = 0x09,
            SLTI  = 0x0A,
            SLTIU = 0x0B,
            ANDI  = 0x0C,
            ORI   = 0x0D,
            LUI   = 0x0F,
            LB    = 0x20,
            LW    = 0x23,
            LBU   = 0x24,
            SB    = 0x28,
            SH    = 0x29,
            SW    = 0x2B
        };

        /// @brief Instructions referenced in opcode bits [5:0].
        enum SPECIALInstruction
        {
            SLL  = 0x00,
            SRL  = 0x02,
            SRA  = 0x03,
            JR   = 0x08,
            JALR = 0x09,
            MFLO = 0x12,
            DIV  = 0x1A,
            ADD  = 0x20,
            ADDU = 0x21,
            SUBU = 0x23,
            AND  = 0x24,
            OR   = 0x25,
            SLTU = 0x2B
        };

private:
        /// @brief Bits of the status register
        enum SRBits
        {
            IsC = 1 << 16
        };

        /// @brief Branches to the target address if the contents of general
        /// purpose register `rs` and general purpose register `rt` yield
        /// `true` with respect to the operator.
        /// @param op The operator to use.
        auto branch_if(const bool condition_met) noexcept -> void;

        /// @brief The program counter to use when a reset exception is
        /// triggered.
        const Word RESET_VECTOR{ 0xBFC00000 };

        /// @brief System bus instance
        SystemBus& bus;
    };
}
