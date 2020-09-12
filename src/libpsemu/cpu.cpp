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
#include "cpu.h"

using namespace PlayStation;

/// @brief Initializes the CPU.
/// @param b The system bus instance.
CPU::CPU(SystemBus& b) noexcept : bus(b)
{ }

/// @brief Resets the CPU to the startup state. Officially, this is
/// considered a reset exception.
auto CPU::reset() noexcept -> void
{
    gpr  = { };
    cop0 = { };
    hi   = { };
    lo   = { };

    pc      = RESET_VECTOR;
    next_pc = pc + 4;

    // We load the next instruction here to give debuggers a chance to access
    // it.
    instruction.word = bus.memory_access<Word>(pc);
}

/// @brief Returns the 26-bit target address.
auto CPU::target() const noexcept -> Word
{
    return instruction.word & 0x03FFFFFF;
}

/// @brief Returns the lower 16-bits of the instruction.
auto CPU::immediate() const noexcept -> Halfword
{
    return instruction.word & 0x0000FFFF;
}

/// @brief Same as the `immediate()` method, merely an alias as defined by MIPS
/// conventions.
auto CPU::offset() const noexcept -> Halfword
{
    return immediate();
}

/// @brief Same as the `rs` instruction field, merely an alias as defined by
/// MIPS conventions.
auto CPU::base() const noexcept -> Word
{
    return instruction.rs;
}

/// @brief Gets the current virtual address.
/// @return The current virtual address.
auto CPU::vaddr() const noexcept -> Word
{
    return static_cast<SignedHalfword>(offset()) + gpr[base()];
}

/// @brief Branches to target address if the condition is met.
/// @param condition_met The result of an expression.
auto CPU::branch_if(const bool condition_met) noexcept -> void
{
    if (condition_met)
    {
        next_pc = static_cast<SignedHalfword>(offset() << 2) + pc;
    }
}

/// @brief Executes the next instruction.
auto CPU::step() noexcept -> void
{
    instruction.word = bus.memory_access<Word>(pc);

    pc = next_pc;
    next_pc += 4;

    switch (instruction.op)
    {
        case InstructionGroup::SPECIAL:
            switch (instruction.funct)
            {
                case SPECIALInstruction::SLL:
                    gpr[instruction.rd] =
                    gpr[instruction.rt] <<
                    instruction.shamt;

                    break;

                case SPECIALInstruction::SRL:
                    gpr[instruction.rd] =
                    gpr[instruction.rt] >>
                    instruction.shamt;

                    break;

                case SPECIALInstruction::SRA:
                    gpr[instruction.rd] =
                    static_cast<SignedWord>(gpr[instruction.rt]) >>
                    instruction.shamt;

                    break;

                case SPECIALInstruction::JR:
                    next_pc = gpr[instruction.rs];
                    break;

                case SPECIALInstruction::JALR:
                    gpr[instruction.rd] = next_pc;
                    next_pc = gpr[instruction.rs];

                    break;

                case SPECIALInstruction::MFHI:
                    gpr[instruction.rd] = hi;
                    break;

                case SPECIALInstruction::MFLO:
                    gpr[instruction.rd] = lo;
                    break;

                case SPECIALInstruction::DIV:
                    lo = static_cast<SignedWord>(gpr[instruction.rs]) /
                         static_cast<SignedWord>(gpr[instruction.rt]);

                    hi = static_cast<SignedWord>(gpr[instruction.rs]) %
                         static_cast<SignedWord>(gpr[instruction.rt]);

                    break;

                case SPECIALInstruction::DIVU:
                    lo = gpr[instruction.rs] / gpr[instruction.rt];
                    hi = gpr[instruction.rs] % gpr[instruction.rt];

                    break;

                case SPECIALInstruction::ADD:
                case SPECIALInstruction::ADDU:
                    gpr[instruction.rd] =
                    gpr[instruction.rs] +
                    gpr[instruction.rt];

                    break;

                case SPECIALInstruction::SUBU:
                    gpr[instruction.rd] =
                    gpr[instruction.rs] -
                    gpr[instruction.rt];

                    break;

                case SPECIALInstruction::AND:
                    gpr[instruction.rd] =
                    gpr[instruction.rs] &
                    gpr[instruction.rt];

                    break;

                case SPECIALInstruction::OR:
                    gpr[instruction.rd] =
                    gpr[instruction.rs] |
                    gpr[instruction.rt];

                    break;

                case SPECIALInstruction::SLT:
                    gpr[instruction.rd] =
                    static_cast<SignedWord>(gpr[instruction.rs]) <
                    static_cast<SignedWord>(gpr[instruction.rt]);

                    break;

                case SPECIALInstruction::SLTU:
                    gpr[instruction.rd] =
                    gpr[instruction.rs] <
                    gpr[instruction.rt];

                    break;

                default:
                    __debugbreak();
                    break;
            }
            break;

        // XXX: The BCOND instruction group on the LSI LR33300, at least on
        // the PlayStation does not operate in accordance with any of the
        // MIPS-I manuals that I can find. So if you're coming from MIPS-I and
        // are wondering why we're not specifically checking for instructions
        // in this group, the comments document this anomaly.
        //
        // *Any* value passed to the BCOND instruction group is valid. I really
        // do not understand why this is the case, but psxtest_cpu.exe confirms
        // this behavior is accurate.
        case InstructionGroup::BCOND:
            // Linking occurs regardless of whether or not the branch will be
            // taken. The request to link is determined by inspecting the most
            // significant bit of the `rt` instruction field. A value of 1
            // signifies that the instruction will unconditionally place the
            // address of the instruction after the delay slot in the link
            // register (31). A value of 0 does not.
            if (instruction.rt & 0x10)
            {
                gpr[31] = next_pc;
            }
            branch_if(static_cast<int32_t>((gpr[instruction.rs]) ^
                                           (instruction.rt << 31)) < 0);
            break;

        case Instruction::J:
            next_pc = (target() << 2) | (pc & 0xF0000000);
            break;

        case Instruction::JAL:
            gpr[31] = next_pc;
            next_pc = (target() << 2) | (pc & 0xF0000000);

            break;

        case Instruction::BEQ:
            branch_if(gpr[instruction.rs] == gpr[instruction.rt]);
            break;

        case Instruction::BNE:
            branch_if(gpr[instruction.rs] != gpr[instruction.rt]);
            break;

        case Instruction::BLEZ:
            branch_if(static_cast<SignedWord>(gpr[instruction.rs]) <= 0);
            break;

        case Instruction::BGTZ:
            branch_if(gpr[instruction.rs] > 0);
            break;

        case Instruction::ADDI:
        case Instruction::ADDIU:
            gpr[instruction.rt] =
            gpr[instruction.rs] +
            static_cast<SignedHalfword>(immediate());

            break;

        case Instruction::SLTI:
            gpr[instruction.rt] =
            static_cast<SignedWord>(gpr[instruction.rs]) <
            static_cast<SignedHalfword>(immediate());

            break;

        case Instruction::SLTIU:
            gpr[instruction.rt] =
            gpr[instruction.rs] <
            static_cast<Word>(static_cast<Halfword>(immediate()));

            break;

        case Instruction::ANDI:
            gpr[instruction.rt] = gpr[instruction.rs] & immediate();
            break;

        case Instruction::ORI:
            gpr[instruction.rt] = gpr[instruction.rs] | immediate();
            break;

        case Instruction::LUI:
            gpr[instruction.rt] = immediate() << 16;
            break;

        case InstructionGroup::COP0:
            switch (instruction.rs)
            {
                case CoprocessorInstruction::MF:
                    gpr[instruction.rt] = cop0[instruction.rd];
                    break;

                case CoprocessorInstruction::MT:
                    cop0[instruction.rd] = gpr[instruction.rt];
                    break;

                default:
                    __debugbreak();
                    break;
            }
            break;

        case Instruction::LB:
            gpr[instruction.rt] = bus.memory_access<SignedByte>(vaddr());
            break;

        case Instruction::LW:
            gpr[instruction.rt] = bus.memory_access<Word>(vaddr());
            break;

        case Instruction::LBU:
            gpr[instruction.rt] = bus.memory_access<Byte>(vaddr());
            break;

        case Instruction::SB:
            bus.memory_access<Byte>(vaddr(),
                                    gpr[instruction.rt] & 0x000000FF);
            break;

        case Instruction::SH:
            bus.memory_access<Halfword>(vaddr(),
                                        gpr[instruction.rt] & 0x0000FFFF);
            break;

        case Instruction::SW:
            if (!(cop0[COP0Register::SR] & SRBits::IsC))
            {
                bus.memory_access<Word>(vaddr(), gpr[instruction.rt]);
            }
            break;

        default:
            __debugbreak();
            break;
    }

    instruction.word = bus.memory_access<Word>(pc);
}
