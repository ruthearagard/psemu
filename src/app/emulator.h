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

#include <QThread>
#include "disasm.h"
#include "../libpsemu/include/ps.h"

/// @brief Defines a threaded PlayStation::System.
class Emulator : public QThread, public PlayStation::System
{
    Q_OBJECT

public:
    /// @brief Initializes the emulator.
    /// @param parent The owner of this object.
    explicit Emulator(QObject* parent) noexcept;

    /// @brief Thread entry point.
    auto run() -> void;

private:
    /// @brief Disassembler instance
    Disassembler disasm;

    /// @brief Are we generating a trace log?
    bool tracing{ false };

signals:
    /// @brief Emitted when it is time to inject the EXE.
    void time_to_inject_exe() noexcept;
};
