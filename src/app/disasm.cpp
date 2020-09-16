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

#include <cstdlib>
#include "disasm.h"

Disassembler::Disassembler(const PlayStation::CPU& c,
                           PlayStation::SystemBus& b) noexcept : cpu(c), bus(b)
{ }

/// @brief Disassembles the current instruction before it is executed.
auto Disassembler::before() noexcept -> void
{
    const auto pc_str
    {
        QString::number(cpu.pc, 16).rightJustified(8, '0').toUpper()
    };

    const auto instruction_str
    {
        QString::number(cpu.instruction.word, 16)
        .rightJustified(8, '0')
        .toUpper()
    };

    result = QString("0x%1\t%2\t").arg(pc_str).arg(instruction_str);

    auto instruction{ instructions[cpu.instruction.op] };

    if (instruction == "GROUP_SPECIAL")
    {
        instruction = special_instructions[cpu.instruction.funct];
    }
    else if (instruction == "GROUP_BCOND")
    {
        instruction = cpu.instruction.rt & 1 ? "bgez" : "bltz";
        instruction += cpu.instruction.rt & 0x10 ? "al" : "";

        instruction += " $branch_single";
    }
    else if (instruction == "GROUP_COP0")
    {
        switch (cpu.instruction.rs)
        {
            case PlayStation::CPU::CoprocessorInstruction::MF:
                instruction = "mfc0 $rt_cop0";
                break;

            case PlayStation::CPU::CoprocessorInstruction::MT:
                instruction = "mtc0 $rd_cop0";
                break;

            default:
                switch (cpu.instruction.funct)
                {
                    case PlayStation::CPU::COP0Instruction::RFE:
                        instruction = "rfe";
                        break;

                    default:
                        instruction = "illegal";
                        break;
                }
                break;
        }
    }

    // Go through each character, one at a time.
    for (auto index{ 0 }; index < instruction.length();)
    {
        // If the character is not our interpolation token...
        if (instruction[index] != '$')
        {
            // ...then it can be safely added to our result.
            result += instruction[index++];
            continue;
        }

        if (instruction.indexOf("$shift_sa", index) > 0)
        {
            result += QString("%1, %2, %3")
                      .arg(gpr[cpu.instruction.rd])
                      .arg(gpr[cpu.instruction.rt])
                      .arg(QString::number(cpu.instruction.shamt));

            post_regs.push_back(PostExecutionData{ cpu.gpr[cpu.instruction.rd],
                                                   gpr[cpu.instruction.rd] });
            index += 9;
        }

        if (instruction.indexOf("$shift_gpr", index) > 0)
        {
            result += QString("%1, %2, %3")
                      .arg(gpr[cpu.instruction.rd])
                      .arg(gpr[cpu.instruction.rt])
                      .arg(gpr[cpu.instruction.rs]);

            post_regs.push_back(PostExecutionData{ cpu.gpr[cpu.instruction.rd],
                                                   gpr[cpu.instruction.rd] });
            index += 10;
        }

        if (instruction.indexOf("$absolute_jump", index) > 0)
        {
            result += QString("%1").arg(gpr[cpu.instruction.rs]);
            index  += 14;
        }

        if (instruction.indexOf("$jump_with_link", index) > 0)
        {
            result += QString("%1, %2").arg(gpr[cpu.instruction.rd])
                                       .arg(gpr[cpu.instruction.rs]);
            index += 15;
        }

        if (instruction.indexOf("$hi_lo", index) > 0)
        {
            result += QString("%1, %2").arg(gpr[cpu.instruction.rs])
                                       .arg(gpr[cpu.instruction.rt]);

            post_regs.push_back(PostExecutionData{ cpu.hi, "HI" });
            post_regs.push_back(PostExecutionData{ cpu.lo, "LO" });

            index += 6;
        }

        if (instruction.indexOf("$alu_reg", index) > 0)
        {
            result += QString("%1, %2, %3")
                      .arg(gpr[cpu.instruction.rd])
                      .arg(gpr[cpu.instruction.rs])
                      .arg(gpr[cpu.instruction.rt]);

            post_regs.push_back(PostExecutionData{ cpu.gpr[cpu.instruction.rd],
                                                   gpr[cpu.instruction.rd] });
            index += 8;
        }

        if (instruction.indexOf("$branch_address", index) > 0)
        {
            const auto address
            {
                (cpu.pc & 0xF0000000) + (cpu.target() << 2)
            };

            const auto address_str
            {
                QString::number(address, 16).rightJustified(8, '0').toUpper()
            };

            result += QString("0x%1").arg(address_str);
            index += 15;
        }

        if (instruction.indexOf("$branch_double", index) > 0)
        {
            const auto address
            {
                (static_cast<int16_t>(cpu.offset()) << 2) + cpu.pc + 4
            };

            const auto address_str
            {
                QString::number(address, 16).rightJustified(8, '0').toUpper()
            };

            result += QString("%1, %2, 0x%3")
                      .arg(gpr[cpu.instruction.rs])
                      .arg(gpr[cpu.instruction.rt])
                      .arg(address_str);

            index += 14;
        }

        if (instruction.indexOf("$branch_single", index) > 0)
        {
            const auto address
            {
                (static_cast<int16_t>(cpu.offset()) << 2) + cpu.pc + 4
            };

            const auto address_str
            {
                QString::number(address, 16).rightJustified(8, '0').toUpper()
            };

            result += QString("%1, 0x%2")
                      .arg(gpr[cpu.instruction.rs])
                      .arg(address_str);

            index += 14;
        }

        if (instruction.indexOf("$alu_double_imm", index) > 0)
        {
            const auto immediate
            {
                QString::number(cpu.immediate(), 16)
                .rightJustified(4, '0')
                .toUpper()
            };

            result += QString("%1, %2, 0x%3")
                      .arg(gpr[cpu.instruction.rt])
                      .arg(gpr[cpu.instruction.rs])
                      .arg(immediate);

            post_regs.push_back(PostExecutionData{ cpu.gpr[cpu.instruction.rt],
                                                   gpr[cpu.instruction.rt] });
            index += 15;
        }

        if (instruction.indexOf("$alu_single", index) > 0)
        {
            const auto imm
            {
                QString::number(cpu.immediate(), 16)
                .rightJustified(4, '0')
                .toUpper()
            };

            result += QString("%1, 0x%2").arg(gpr[cpu.instruction.rt]).arg(imm);

            post_regs.push_back(PostExecutionData{ cpu.gpr[cpu.instruction.rt],
                                                   gpr[cpu.instruction.rt] });
            index += 11;
        }

        if (instruction.indexOf("$rt_cop0", index) > 0)
        {
            result += QString("%1, %2").arg(gpr[cpu.instruction.rt])
                                       .arg(cop0[cpu.instruction.rd]);

            post_regs.push_back(PostExecutionData{ cpu.gpr[cpu.instruction.rt],
                                                   gpr[cpu.instruction.rt] });
            index += 8;
        }

        if (instruction.indexOf("$rd_cop0", index) > 0)
        {
            result += QString("%1, %2").arg(gpr[cpu.instruction.rt])
                                       .arg(cop0[cpu.instruction.rd]);

            post_regs.push_back(PostExecutionData
            {
                cpu.cop0[cpu.instruction.rd],
                cop0[cpu.instruction.rd]
            });
            index += 8;
        }

        if (instruction.indexOf("$mem", index) > 0)
        {
            const auto offset{ static_cast<int16_t>(cpu.offset()) };

            const auto offset_str
            {
                QString::number(abs(offset), 16)
                .rightJustified(4, '0')
                .toUpper()
            };

            result += QString("%1, %2%3(%4)")
                      .arg(gpr[cpu.instruction.rt])
                      .arg(offset < 0 ? "-" : "")
                      .arg(offset_str)
                      .arg(gpr[cpu.base()]);

            post_regs.push_back(PostExecutionData
            {
                cpu.gpr[cpu.instruction.rt],
                gpr[cpu.instruction.rt]
            });
            index += 4;
        }
    }
}

/// @brief Disassembles the current instruction after it is executed.
auto Disassembler::after() noexcept -> QString
{
    if (!post_regs.isEmpty())
    {
        while (result.length() < 55)
        {
            result += " ";
        }

        result += " ; ";

        auto count{ post_regs.count() };

        for (const auto& post_data : post_regs)
        {
            const auto value
            {
                QString::number(post_data.reg, 16).rightJustified(8, '0').toUpper()
            };

            result += QString("%1=0x%2").arg(post_data.name).arg(value);

            if (count-- > 1)
            {
                result += ", ";
            }
        }
        post_regs = { };
    }
    return result;
}
