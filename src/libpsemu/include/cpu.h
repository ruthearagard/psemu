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

        /// @brief System control co-processor registers (COP0)
        std::array<Word, 32> cop0;

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

        /// @brief Returns the 26-bit target address.
        auto target() const noexcept -> Word;

        /// @brief Returns the lower 16-bits of the instruction.
        auto immediate() const noexcept -> Halfword;

        /// @brief Same as the `immediate()` method, merely an alias as defined
        /// by MIPS conventions.
        auto offset() const noexcept -> Halfword;

        /// @brief Same as the `rs` instruction field, merely an alias as
        /// defined by MIPS conventions.
        auto base() const noexcept -> Word;

        /// @brief Gets the current virtual address.
        /// @return The current virtual address.
        auto vaddr() const noexcept -> Word;

        /// @brief Inherent co-processor instructions
        enum CoprocessorInstruction
        {
            MF = 0x00,
            MT = 0x04
        };

        /// @brief System control co-processor (COP0) instructions
        enum COP0Instruction
        {
            RFE = 0x10
        };

        /// @brief System control co-processor (COP0) registers
        enum COP0Register
        {
            BadA  = 8,
            SR    = 12,
            Cause = 13,
            EPC   = 14
        };

        /// @brief Status register (SR) bits
        enum SRBits
        {
            IsC = 1 << 16
        };

        /// @brief Exception codes
        enum Exception
        {
            AdEL = 0x4,
            AdES = 0x5,
            Sys  = 0x8,
            Bp   = 0x9,
            Ovf  = 0xC
        };

private:
        /// @brief Instruction groups
        enum InstructionGroup
        {
            SPECIAL = 0x00,
            BCOND   = 0x01,
            COP0    = 0x10
        };

        /// @brief Instructions located in bits [31:26] of the current opcode.
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
            XORI  = 0x0E,
            LUI   = 0x0F,
            LB    = 0x20,
            LH    = 0x21,
            LWL   = 0x22,
            LW    = 0x23,
            LBU   = 0x24,
            LHU   = 0x25,
            LWR   = 0x26,
            SB    = 0x28,
            SH    = 0x29,
            SWL   = 0x2A,
            SW    = 0x2B,
            SWR   = 0x2E
        };

        /// @brief Instructions located in bits [6:0] of the current opcode.
        enum SPECIALInstruction
        {
            SLL     = 0x00,
            SRL     = 0x02,
            SRA     = 0x03,
            SLLV    = 0x04,
            SRLV    = 0x06,
            SRAV    = 0x07,
            JR      = 0x08,
            JALR    = 0x09,
            SYSCALL = 0x0C,
            BREAK   = 0x0D,
            MFHI    = 0x10,
            MTHI    = 0x11,
            MFLO    = 0x12,
            MTLO    = 0x13,
            MULT    = 0x18,
            MULTU   = 0x19,
            DIV     = 0x1A,
            DIVU    = 0x1B,
            ADD     = 0x20,
            ADDU    = 0x21,
            SUB     = 0x22,
            SUBU    = 0x23,
            AND     = 0x24,
            OR      = 0x25,
            XOR     = 0x26,
            NOR     = 0x27,
            SLT     = 0x2A,
            SLTU    = 0x2B
        };

        /// @brief Gets the current jump address.
        /// @return The current jump address.
        auto jump() noexcept -> void;

        /// @brief Traps an exception.
        /// @param exc The exception to trap.
        /// @param bad_vaddr The bad virtual address, if any.
        auto trap(const Exception exc,
                  const Word bad_vaddr = 0x00000000) noexcept -> void;

        /// @brief Branches to target address if the condition is met.
        /// @param condition_met The result of an expression.
        auto branch_if(const bool condition_met) noexcept -> void;

        /// @brief The program counter to use when a reset exception is
        /// triggered.
        const Word RESET_VECTOR{ 0xBFC00000 };

        /// @brief System bus instance
        SystemBus& bus;
    };
}
