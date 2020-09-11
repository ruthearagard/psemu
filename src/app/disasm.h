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
#include <QObject>
#include "../libpsemu/include/ps.h"

/// @brief Defines the disassembler class.
class Disassembler : public QObject
{
    Q_OBJECT

public:
    Disassembler(const PlayStation::CPU& c,
                 const PlayStation::SystemBus& b) noexcept;

    /// @brief Disassembles the current instruction before it is executed.
    auto before() noexcept -> void;

    /// @brief Disassembles the current instruction after it is executed.
    auto after() noexcept -> QString;

private:
    struct PostExecutionData
    {
        /// @brief Reference to the register.
        const uint32_t& reg;

        /// @brief Conventional name of the register.
        const QString name;
    };

    /// @brief The registers to output to the disassembly result.
    QVector<struct PostExecutionData> post_regs;

    /// @brief Conventional names of the general purpose registers
    const std::array<const QString, 32> gpr =
    {
        "$zero", // 0
        "$at",   // 1
        "$v0",   // 2
        "$v1",   // 3
        "$a0",   // 4
        "$a1",   // 5
        "$a2",   // 6
        "$a3",   // 7
        "$t0",   // 8
        "$t1",   // 9
        "$t2",   // 10
        "$t3",   // 11
        "$t4",   // 12
        "$t5",   // 13
        "$t6",   // 14
        "$t7",   // 15
        "$s0",   // 16
        "$s1",   // 17
        "$s2",   // 18
        "$s3",   // 19
        "$s4",   // 20
        "$s5",   // 21
        "$s6",   // 22
        "$s7",   // 23
        "$t8",   // 24
        "$t9",   // 25
        "$k0",   // 26
        "$k1",   // 27
        "$gp",   // 28
        "$sp",   // 29
        "$fp",   // 30
        "$ra"    // 31
    };

    /// System control co-processor (COP0) registers
    const std::array<const QString, 32> cop0 =
    {
        "UNKNOWN0",  // 0
        "UNKNOWN1",  // 1
        "UNKNOWN2",  // 2
        "BPC",       // 3
        "UNKNOWN4",  // 4
        "BDA",       // 5
        "TAR",       // 6
        "DCIC",      // 7
        "BadA",      // 8
        "BDAM",      // 9
        "UNKNOWN10", // 10
        "BPCM",      // 11
        "SR",        // 12
        "Cause",     // 13
        "EPC",       // 14
        "PRId",      // 15
        "UNKNOWN16", // 16
        "UNKNOWN17", // 17
        "UNKNOWN18", // 18
        "UNKNOWN19", // 19
        "UNKNOWN20", // 20
        "UNKNOWN21", // 21
        "UNKNOWN22", // 22
        "UNKNOWN23", // 23
        "UNKNOWN24", // 24
        "UNKNOWN25", // 25
        "UNKNOWN26", // 26
        "UNKNOWN27", // 27
        "UNKNOWN28", // 28
        "UNKNOWN29", // 29
        "UNKNOWN30", // 30
        "UNKNOWN31"  // 31
    };

    /// @brief Instructions that are referenced by bits [31:26] of the current
    /// opcode.
    const std::array<const QString, 63> instructions =
    {
        "GROUP_SPECIAL",         // 0x00
        "GROUP_BCOND",           // 0x01
        "j $branch_address",     // 0x02
        "jal $branch_address",   // 0x03
        "beq $branch_double",    // 0x04
        "bne $branch_double",    // 0x05
        "blez $branch_single",   // 0x06
        "bgtz $branch_single",   // 0x07
        "addi $alu_double_imm",  // 0x08
        "addiu $alu_double_imm", // 0x09
        "slti $alu_double_imm",  // 0x0A
        "sltiu $alu_double_imm", // 0x0B
        "andi $alu_double_imm",  // 0x0C
        "ori $alu_double_imm",   // 0x0D
        "xori $alu_double_imm",  // 0x0E
        "lui $alu_single",       // 0x0F
        "GROUP_COP0",            // 0x10
        "illegal",               // 0x11
        "GROUP_COP2",            // 0x12
        "illegal",               // 0x13
        "illegal",               // 0x14
        "illegal",               // 0x15
        "illegal",               // 0x16
        "illegal",               // 0x17
        "illegal",               // 0x18
        "illegal",               // 0x19
        "illegal",               // 0x1A
        "illegal",               // 0x1B
        "illegal",               // 0x1C
        "illegal",               // 0x1D
        "illegal",               // 0x1E
        "illegal",               // 0x1F
        "lb $mem",               // 0x20
        "lh $mem",               // 0x21
        "lwl $mem",              // 0x22
        "lw $mem",               // 0x23
        "lbu $mem",              // 0x24
        "lhu $mem",              // 0x25
        "lwr $mem",              // 0x26
        "illegal",               // 0x27
        "sb $mem",               // 0x28
        "sh $mem",               // 0x29
        "swl $mem",              // 0x2A
        "sw $mem",               // 0x2B
        "illegal",               // 0x2C
        "illegal",               // 0x2D
        "swr $mem",              // 0x2E
        "illegal",               // 0x2F
        "illegal",               // 0x30
        "illegal",               // 0x31
        "lwc2 $cp2_mem",         // 0x32
        "illegal",               // 0x33
        "illegal",               // 0x34
        "illegal",               // 0x35
        "illegal",               // 0x36
        "illegal",               // 0x37
        "illegal",               // 0x38
        "illegal",               // 0x39
        "swc2 $cp2_mem",         // 0x3A
        "illegal",               // 0x3B
        "illegal",               // 0x3C
        "illegal",               // 0x3D
        "illegal"                // 0x3E
    };

    /// @brief Instructions that are referenced by bits [6:0] of the current
    /// opcode.
    const std::array<const QString, 63> special_instructions =
    {
        "sll $shift_sa",                 // 0x00
        "illegal",                       // 0x01
        "srl $shift_sa",                 // 0x02
        "sra $shift_sa",                 // 0x03
        "sllv $shift_gpr",               // 0x04
        "illegal",                       // 0x05
        "srlv $shift_gpr",               // 0x06
        "srav $shift_gpr",               // 0x07
        "jr $absolute_jump",             // 0x08
        "jalr $jump_with_link",          // 0x09
        "illegal",                       // 0x0A
        "illegal",                       // 0x0B
        "syscall",                       // 0x0C
        "break",                         // 0x0D
        "illegal",                       // 0x0E
        "illegal",                       // 0x0F
        "mfhi $hi_lo",                   // 0x10
        "mthi $hi_lo",                   // 0x11
        "mflo $hi_lo",                   // 0x12
        "mtlo $hi_lo",                   // 0x13
        "illegal",                       // 0x14
        "illegal",                       // 0x15
        "illegal",                       // 0x16
        "illegal",                       // 0x17
        "mult $hi_lo",                   // 0x18
        "multu $hi_lo",                  // 0x19
        "div $hi_lo",                    // 0x1A
        "divu $hi_lo",                   // 0x1B
        "illegal",                       // 0x1C
        "illegal",                       // 0x1D
        "illegal",                       // 0x1E
        "illegal",                       // 0x1F
        "add $alu_reg",                  // 0x20
        "addu $alu_reg",                 // 0x21
        "sub $alu_reg",                  // 0x22
        "subu $alu_reg",                 // 0x23
        "and $alu_reg",                  // 0x24
        "or $alu_reg",                   // 0x25
        "xor $alu_reg",                  // 0x26
        "nor $alu_reg",                  // 0x27
        "illegal",                       // 0x28
        "illegal",                       // 0x29
        "slt $alu_reg",                  // 0x2A
        "sltu $alu_reg",                 // 0x2B
        "illegal",                       // 0x2C
        "illegal",                       // 0x2D
        "illegal",                       // 0x2E
        "illegal",                       // 0x2F
        "illegal",                       // 0x30
        "illegal",                       // 0x31
        "illegal",                       // 0x32
        "illegal",                       // 0x33
        "illegal",                       // 0x34
        "illegal",                       // 0x35
        "illegal",                       // 0x36
        "illegal",                       // 0x37
        "illegal",                       // 0x38
        "illegal",                       // 0x39
        "illegal",                       // 0x3A
        "illegal",                       // 0x3B
        "illegal",                       // 0x3C
        "illegal",                       // 0x3D
        "illegal"                        // 0x3E
    };

    /// @brief PlayStation CPU instance
    const PlayStation::CPU& cpu;

    /// @brief PlayStation system bus instance
    const PlayStation::SystemBus& bus;

    /// @brief Current disassembly result
    QString result;
};
