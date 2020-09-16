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

/// @brief Traps an exception.
/// @param exc The exception to trap.
/// @param bad_vaddr The bad virtual address, if any.
auto CPU::trap(const Exception exc, const Word bad_vaddr) noexcept -> void
{
    // So on an exception, the CPU:

    // 1) sets up EPC to point to the restart location.
    cop0[COP0Register::EPC] = pc - 4;

    // 2) The pre-existing user-mode and interrupt-enable flags in SR are saved
    //    by pushing the 3-entry stack inside SR, and changing to kernel mode
    //    with interrupts disabled.
    cop0[COP0Register::SR] = (cop0[COP0Register::SR] & 0xFFFFFFC0) |
                            ((cop0[COP0Register::SR] & 0x0000000F) << 2);

    // 3a) Cause is setup so that software can see the reason for the
    //     exception.
    cop0[COP0Register::Cause] = (cop0[COP0Register::Cause] & ~0xFFFF00FF) |
                                (exc << 2);

    // 3b) On address exceptions BadVaddr is also set.
    if (exc == Exception::AdEL)
    {
        cop0[COP0Register::BadA] = bad_vaddr;
    }

    // 4) Transfers control to the exception entry point.
    pc      = 0x80000080;
    next_pc = pc + 4;
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
    if ((pc & 0x00000003) != 0)
    {
        trap(Exception::AdEL);
    }

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

                case SPECIALInstruction::SLLV:
                    gpr[instruction.rd] =
                    gpr[instruction.rt] <<
                   (gpr[instruction.rs] & 0x0000001F);

                    break;

                case SPECIALInstruction::SRLV:
                    gpr[instruction.rd] =
                    gpr[instruction.rt] >>
                   (gpr[instruction.rs] & 0x0000001F);

                    break;

                case SPECIALInstruction::SRAV:
                    gpr[instruction.rd] =
                    static_cast<SignedWord>(gpr[instruction.rt]) >>
                    (gpr[instruction.rs] & 0x0000001F);

                    break;

                case SPECIALInstruction::JR:
                    next_pc = gpr[instruction.rs];
                    break;

                case SPECIALInstruction::JALR:
                    gpr[instruction.rd] = next_pc;
                    next_pc = gpr[instruction.rs];

                    break;

                case SPECIALInstruction::SYSCALL:
                    trap(Exception::Sys);
                    break;

                case SPECIALInstruction::BREAK:
                    trap(Exception::Bp);
                    break;

                case SPECIALInstruction::MFHI:
                    gpr[instruction.rd] = hi;
                    break;

                case SPECIALInstruction::MTHI:
                    hi = gpr[instruction.rs];
                    break;

                case SPECIALInstruction::MFLO:
                    gpr[instruction.rd] = lo;
                    break;

                case SPECIALInstruction::MTLO:
                    lo = gpr[instruction.rs];
                    break;

                case SPECIALInstruction::MULT:
                {
                    const uint64_t product =
                    static_cast<int64_t>(static_cast<SignedWord>(gpr[instruction.rs])) *
                    static_cast<int64_t>(static_cast<SignedWord>(gpr[instruction.rt]));

                    lo = product & 0x00000000FFFFFFFF;
                    hi = product >> 32;

                    break;
                }

                case SPECIALInstruction::MULTU:
                {
                    const uint64_t product = static_cast<uint64_t>(gpr[instruction.rs]) *
                                             static_cast<uint64_t>(gpr[instruction.rt]);

                    lo = product & 0x00000000FFFFFFFF;
                    hi = product >> 32;

                    break;
                }

                case SPECIALInstruction::DIV:
                {
                    // The result of a division by zero is consistent with the
                    // result of a simple radix-2 ("one bit at a time")
                    // implementation.
                    const SignedWord rt
                    {
                        static_cast<SignedWord>(gpr[instruction.rt])
                    };

                    const SignedWord rs
                    {
                        static_cast<SignedWord>(gpr[instruction.rs])
                    };

                    // Divisor is zero
                    if (rt == 0)
                    {
                        // If the dividend is negative, the quotient is 1
                        // (0x00000001), and if the dividend is positive or
                        // zero, the quotient is -1 (0xFFFFFFFF).
                        lo = (rs < 0) ? 0x00000001 : 0xFFFFFFFF;

                        // In both cases the remainder equals the dividend.
                        hi = static_cast<Word>(rs);
                    }
                    // Will trigger an arithmetic exception when dividing
                    // 0x80000000 by 0xFFFFFFFF. The result of the division is
                    // a quotient of 0x80000000 and a remainder of 0x00000000.
                    else if (static_cast<Word>(rs) == 0x80000000 &&
                             static_cast<Word>(rt) == 0xFFFFFFFF)
                    {
                        lo = static_cast<Word>(rs);
                        hi = 0x00000000;
                    }
                    else
                    {
                        lo = rs / rt;
                        hi = rs % rt;
                    }
                    break;
                }

                case SPECIALInstruction::DIVU:
                {
                    const Word rt{ gpr[instruction.rt] };
                    const Word rs{ gpr[instruction.rs] };

                    // In the case of unsigned division, the dividend can't be
                    // negative and thus the quotient is always -1 (0xFFFFFFFF)
                    // and the remainder equals the dividend.
                    if (rt == 0)
                    {
                        lo = 0xFFFFFFFF;
                        hi = rs;
                    }
                    else
                    {
                        lo = rs / rt;
                        hi = rs % rt;
                    }
                    break;
                }

                case SPECIALInstruction::ADD:
                {
                    const Word rs{ gpr[instruction.rs] };
                    const Word rt{ gpr[instruction.rt] };

                    const Word result = rs + rt;

                    if (!((rs ^ rt) & 0x80000000) &&
                        ((result ^ rs) & 0x80000000))
                    {
                        trap(Exception::Ovf);
                        break;
                    }

                    gpr[instruction.rd] = result;
                    break;
                }

                case SPECIALInstruction::ADDU:
                    gpr[instruction.rd] =
                    gpr[instruction.rs] +
                    gpr[instruction.rt];

                    break;

                case SPECIALInstruction::SUB:
                {
                    const Word rs{ gpr[instruction.rs] };
                    const Word rt{ gpr[instruction.rt] };

                    const uint32_t result = rs - rt;

                    if (((rs ^ rt) & 0x80000000) &&
                       ((result ^ rs) & 0x80000000))
                    {
                        trap(Exception::Ovf);
                        break;
                    }

                    gpr[instruction.rd] = result;
                    break;
                }

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

                case SPECIALInstruction::XOR:
                    gpr[instruction.rd] =
                    gpr[instruction.rs] ^
                    gpr[instruction.rt];

                    break;

                case SPECIALInstruction::NOR:
                    gpr[instruction.rd] =
                  ~(gpr[instruction.rs] |
                    gpr[instruction.rt]);

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
            branch_if(static_cast<int32_t>(gpr[instruction.rs] ^
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
            branch_if(static_cast<SignedWord>(gpr[instruction.rs]) > 0);
            break;

        case Instruction::ADDI:
        {
            const Word imm
            {
                static_cast<Word>(static_cast<SignedHalfword>(immediate()))
            };

            const Word rs{ gpr[instruction.rs] };

            const uint32_t result = imm + rs;

            if (!((rs ^ imm) & 0x80000000) && ((result ^ rs) & 0x80000000))
            {
                trap(Exception::Ovf);
                break;
            }

            gpr[instruction.rt] = result;
            break;
        }

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
            static_cast<Word>(static_cast<SignedHalfword>(immediate()));

            break;

        case Instruction::ANDI:
            gpr[instruction.rt] = gpr[instruction.rs] & immediate();
            break;

        case Instruction::ORI:
            gpr[instruction.rt] = gpr[instruction.rs] | immediate();
            break;

        case Instruction::XORI:
            gpr[instruction.rt] = gpr[instruction.rs] ^ immediate();
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
                    switch (instruction.funct)
                    {
                        case COP0Instruction::RFE:
                            cop0[COP0Register::SR] =
                           (cop0[COP0Register::SR] & 0xFFFFFFF0) |
                          ((cop0[COP0Register::SR] & 0x0000003C) >> 2);

                            break;

                        default:
                            __debugbreak();
                            break;
                    }
                    break;
            }
            break;

        case Instruction::LB:
            gpr[instruction.rt] = bus.memory_access<SignedByte>(vaddr());
            break;

        case Instruction::LH:
        {
            const Word m_vaddr{ vaddr() };

            if ((m_vaddr & 1) != 0)
            {
                trap(Exception::AdEL, m_vaddr);
                break;
            }
            gpr[instruction.rt] = bus.memory_access<SignedHalfword>(m_vaddr);
            break;
        }

        case Instruction::LWL:
        {
            const Word m_vaddr{ vaddr() };
            const Word data{ bus.memory_access<Word>(m_vaddr & 0xFFFFFFFC) };

            switch (m_vaddr & 3)
            {
                case 0:
                    gpr[instruction.rt] =
                    (gpr[instruction.rt] & 0x00FFFFFF) | (data << 24);

                    break;

                case 1:
                    gpr[instruction.rt] =
                    (gpr[instruction.rt] & 0x0000FFFF) | (data << 16);

                    break;

                case 2:
                    gpr[instruction.rt] =
                    (gpr[instruction.rt] & 0x000000FF) | (data << 8);

                    break;

                case 3:
                    gpr[instruction.rt] =
                    (gpr[instruction.rt] & 0x00000000) | (data << 0);

                    break;
            }
            break;
        }

        case Instruction::LW:
        {
            const Word m_vaddr{ vaddr() };

            if ((m_vaddr & 0x00000003) != 0)
            {
                trap(Exception::AdEL, m_vaddr);
                break;
            }
            gpr[instruction.rt] = bus.memory_access<Word>(vaddr());
            break;
        }

        case Instruction::LBU:
            gpr[instruction.rt] = bus.memory_access<Byte>(vaddr());
            break;

        case Instruction::LHU:
        {
            const Word m_vaddr{ vaddr() };

            if ((m_vaddr & 1) != 0)
            {
                trap(Exception::AdEL, m_vaddr);
                break;
            }
            gpr[instruction.rt] = bus.memory_access<Halfword>(m_vaddr);
            break;
        }

        case Instruction::LWR:
        {
            const Word m_vaddr{ vaddr() };
            const Word data{ bus.memory_access<Word>(m_vaddr & 0xFFFFFFFC) };

            switch (m_vaddr & 3)
            {
                case 0:
                    gpr[instruction.rt] = data;
                    break;

                case 1:
                    gpr[instruction.rt] =
                    (gpr[instruction.rt] & 0xFF000000) | (data >> 8);

                    break;

                case 2:
                    gpr[instruction.rt] =
                    (gpr[instruction.rt] & 0xFFFF0000) | (data >> 16);

                    break;

                case 3:
                    gpr[instruction.rt] =
                    (gpr[instruction.rt] & 0xFFFFFF00) | (data >> 24);

                    break;
            }
            break;
        }

        case Instruction::SB:
            bus.memory_access<Byte>(vaddr(),
                                    gpr[instruction.rt] & 0x000000FF);
            break;

        case Instruction::SH:
        {
            const Word m_vaddr{ vaddr() };

            if ((m_vaddr & 1) != 0)
            {
                trap(Exception::AdES, m_vaddr);
                break;
            }
            bus.memory_access<Halfword>(m_vaddr,
                                        gpr[instruction.rt] & 0x0000FFFF);
            break;
        }

        case Instruction::SWL:
        {
            const Word m_vaddr{ vaddr() };
            Word data{ bus.memory_access<Word>(m_vaddr & 0xFFFFFFFC) };

            switch (m_vaddr & 3)
            {
                case 0:
                    data = (data & 0xFFFFFF00) | (gpr[instruction.rt] >> 24);
                    break;

                case 1:
                    data = (data & 0xFFFF0000) | (gpr[instruction.rt] >> 16);
                    break;

                case 2:
                    data = (data & 0xFF000000) | (gpr[instruction.rt] >> 8);
                    break;

                case 3:
                    data = (data & 0x00000000) | (gpr[instruction.rt] >> 0);
                    break;
            }

            bus.memory_access<Word>(m_vaddr & 0xFFFFFFFC, data);
            break;
        }

        case Instruction::SW:
            if (!(cop0[COP0Register::SR] & SRBits::IsC))
            {
                const Word m_vaddr{ vaddr() };

                if ((m_vaddr & 0x00000003) != 0)
                {
                    trap(Exception::AdES);
                    break;
                }
                bus.memory_access<Word>(vaddr(), gpr[instruction.rt]);
            }
            break;

        case Instruction::SWR:
        {
            const Word m_vaddr{ vaddr() };
            Word data{ bus.memory_access<Word>(m_vaddr & 0xFFFFFFFC) };

            switch (m_vaddr & 3)
            {
                case 0:
                    data = (data & 0x00000000) | (gpr[instruction.rt] << 0);
                    break;

                case 1:
                    data = (data & 0x000000FF) | (gpr[instruction.rt] << 8);
                    break;

                case 2:
                    data = (data & 0x0000FFFF) | (gpr[instruction.rt] << 16);
                    break;

                case 3:
                    data = (data & 0x00FFFFFF) | (gpr[instruction.rt] << 24);
                    break;
            }

            bus.memory_access<Word>(m_vaddr & 0xFFFFFFFC, data);
            break;
        }

        default:
            __debugbreak();
            break;
    }

    instruction.word = bus.memory_access<Word>(pc);
    gpr[0] = 0x00000000;
}
