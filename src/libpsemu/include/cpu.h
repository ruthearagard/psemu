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

        /// @brief System control co-processor registers (COP0)
        struct
        {
            /// @brief The Bad Address (BadA) Register is a read-only register
            /// that saves the address associated with an illegal access. This
            /// register saves only addressing errors (AdEL or AdES), not bus
            /// errors.
            Word BadA;

            /// @brief The Status Register contains all major status bits for
            /// exception conditions. All bits in the Status Register, with the
            /// exception of the TS (TLB Shutdown) bit, are readable and
            /// writable; the TS bit is read-only.
            union
            {
                struct
                {
                    /// @brief Current interrupt enable setting
                    /// (0 means disabled, 1 means enabled).
                    unsigned int IEc : 1;

                    /// @brief Current Kernel-User mode setting
                    /// (0 means kernel, 1 means user).
                    unsigned int KUc : 1;

                    /// @brief Previous interrupt enable setting
                    /// (0 means disabled, 1 means enabled).
                    unsigned int IEp : 1;

                    /// @brief Previous Kernel-User mode setting
                    /// (0 means kernel, 1 means user).
                    unsigned int KUp : 1;

                    /// @brief Old interrupt enable setting
                    /// (0 means disabled, 1 means enabled).
                    unsigned int IEo : 1;

                    /// @brief Old Kernel-User mode setting
                    /// (0 means kernel, 1 means user).
                    unsigned int KUo : 1;

                    /// @brief Reserved, read only and always read as 0.
                    unsigned int : 2;

                    /// @brief Software sets this bit to one to enable the SW0
                    /// software interrupt.
                    unsigned int SW0 : 1;

                    /// @brief Software sets this bit to one to enable the SW1
                    /// software interrupt.
                    unsigned int SW1 : 1;

                    /// @brief Software sets this bit to enable the INT0
                    /// hardware interrupt.
                    unsigned int INT0 : 1;

                    /// @brief Software sets this bit to enable the INT1
                    /// hardware interrupt.
                    unsigned int INT1 : 1;

                    /// @brief Software sets this bit to enable the INT2
                    /// hardware interrupt.
                    unsigned int INT2 : 1;

                    /// @brief Software sets this bit to enable the INT3
                    /// hardware interrupt.
                    unsigned int INT3 : 1;

                    /// @brief Software sets this bit to enable the INT4
                    /// hardware interrupt.
                    unsigned int INT4 : 1;

                    /// @brief Software sets this bit to enable the INT5
                    /// hardware interrupt.
                    unsigned int INT5 : 1;

                    /// @brief When this bit is set to one, store operations do
                    /// not propagate through LR333x0 to the external memory
                    /// system. This bit must be set to one for cache testing.
                    unsigned int IsC : 1;

                    /// @brief Reserved, read only and always read as 0.
                    unsigned int : 1;

                    /// @brief Software sets PZ to one to force the LR333x0 to
                    /// generate zeros on DP[3:0] during store transactions.
                    ///
                    /// XXX: I have no idea what this means.
                    unsigned int PZ : 1;

                    /// @brief Reserved, read only and always read as 0.
                    unsigned int : 1;

                    /// @brief The LR333x0 sets the PE bit to one if it detects
                    /// an external memory parity error. Software may use the
                    /// PE bit to log external memory parity errors. The parity
                    /// error condition is reset by writing a one into PE;
                    /// writing a zero into this bit does not affect its value.
                    unsigned int PE : 1;

                    /// @brief In the LR333x0, this bit is permanently set to
                    /// zero and is read only.
                    unsigned int TS : 1;

                    /// @brief The BEV bit controls the location of general
                    /// exception vectors during bootstrap (immediately
                    /// following reset). When this bit is set to zero, the
                    /// normal exception vectors are used. When this bit is set
                    /// to one, bootstrap vector locations are used.
                    unsigned int BEV : 1;

                    /// @brief Reserved, read only and always read as 0.
                    unsigned int : 5;

                    /// @brief Software sets this bit to one to indicate that
                    /// the system control co-processor (COP0) is usable.
                    unsigned int CU0 : 1;

                    /// @brief Software sets this bit to one to indicate that
                    /// the COP1 coprocessor is usable.
                    unsigned int CU1 : 1;

                    /// @brief Software sets this bit to one to indicate that
                    /// the Geometry Transformation Engine (GTE, COP2) is
                    /// usable.
                    unsigned int CU2 : 1;

                    /// @brief Software sets this bit to one to indicate that
                    /// the COP3 coprocessor is usable.
                    unsigned int CU3 : 1;
                };
                Word word;
            } SR;

            union
            {
                struct
                {
                    /// @brief Reserved, read only and always read as 0.
                    unsigned int : 2;

                    /// @brief The LR333x0 sets this field to indicate the type
                    /// of event that caused the last general exception.
                    unsigned int ExcCode : 4;

                    /// @brief Reserved, read only and always read as 0.
                    unsigned int : 2;

                    /// @brief By setting this bit to one, software causes the
                    /// LR333x0 to transfer control to the general exception
                    /// routine. The exception routine must reset the SW0 bit
                    /// before returning control to the interrupting software.
                    unsigned int SW0 : 1;

                    /// @brief By setting this bit to one, software causes the
                    /// LR333x0 to transfer control to the general exception
                    /// routine. The exception routine must reset the SW0 bit
                    /// before returning control to the interrupting software.
                    unsigned int SW1 : 1;

                    /// @brief The LR333x0 sets this bit to one to indicate
                    /// that an external interrupt is pending on INT0.
                    unsigned int IP0 : 1;

                    /// @brief The LR333x0 sets this bit to one to indicate
                    /// that an external interrupt is pending on INT1.
                    unsigned int IP1 : 1;

                    /// @brief The LR333x0 sets this bit to one to indicate
                    /// that an external interrupt is pending on INT2.
                    unsigned int IP2 : 1;

                    /// @brief The LR333x0 sets this bit to one to indicate
                    /// that an external interrupt is pending on INT3.
                    unsigned int IP3 : 1;

                    /// @brief The LR333x0 sets this bit to one to indicate
                    /// that an external interrupt is pending on INT4.
                    unsigned int IP4 : 1;

                    /// @brief The LR333x0 sets this bit to one to indicate
                    /// that an external interrupt is pending on INT5.
                    unsigned int IP5 : 1;

                    /// @brief Reserved, read only and always read as 0.
                    unsigned int : 11;

                    /// @brief When taking a Coprocessor Unusable exception,
                    /// the LR333x0 writes the referenced coprocessor number in
                    /// this field. This field is otherwise undefined.
                    unsigned int CE : 2;

                    /// @brief When the BD bit is set, the BT bit determines
                    /// whether or not the branch is taken. A value of one in
                    /// BT indicates that the branch is taken. The Target
                    /// Address Register holds the return address.
                    unsigned int BT : 1;

                    /// @brief The LR333x0 sets this bit to one to indicate
                    /// that the last exception was taken while executing in a
                    /// branch delay slot.
                    unsigned int BD : 1;
                };
                Word word;
            } Cause;

            /// @brief The 32-bit Exception Program Counter (EPC) Register
            /// contains the address where processing resumes after an
            /// exception is serviced. In most cases, the EPC Register contains
            /// the address of the instruction that caused the exception.
            /// However, when the exception instruction resides in a branch
            /// delay slot, the Cause Register's BD bit is set to one to
            /// indicate that the EPC Register contains the address of the
            /// immediately preceding branch or jump instruction.
            Word EPC;

            /// @brief Unused value which is returned when the subscript
            /// operator encounters an unknown index.
            Word unused;

            /// @brief We overload the subscript operator to allow for indexed
            /// based access to the registers. This method is more intuitive vs
            /// a separate function call for it.
            auto operator[](const int index) noexcept -> Word&
            {
                switch (index)
                {
                    case COP0Register::BadA:  return BadA;
                    case COP0Register::SR:    return SR.word;
                    case COP0Register::Cause: return Cause.word;
                    case COP0Register::EPC:   return EPC;
                    default:                  return unused;
                }
            }
        } cop0;

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

        /// @brief System control co-processor (COP0) registers
        enum COP0Register
        {
            BadA  = 8,
            SR    = 12,
            Cause = 13,
            EPC   = 14
        };

        struct
        {
            Word* reg;
            Word value;
            unsigned int instrs;
            bool pending;
        } delay_slot;

        /// @brief Handles a load delay slot.
        /// @param reg The register to load the value into.
        /// @param value The value to store.
        auto load(Word* const reg, const Word value) noexcept -> void;

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
