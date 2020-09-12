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

#include <QFile>
#include <QTextStream>
#include "emulator.h"

/// @brief Initializes the emulator.
/// @param parent The owner of this object.
Emulator::Emulator(QObject* parent) noexcept : QThread(parent),
                                               disasm(cpu, bus)
{ }

/// @brief Thread entry point.
auto Emulator::run() -> void
{
    QFile trace_file{ "trace.txt" };
    trace_file.open(QIODevice::WriteOnly);
    QTextStream out(&trace_file);

    for (;;)
    {
        if (cpu.pc == 0x80030000)
        {
            __debugbreak();
#if 0
            emit time_to_inject_exe();
            tracing = true;

            // The thread will be restarted when the EXE is loaded.
            quit();
            break;
#endif
        }

        if (cpu.pc == 0x000000A0)
        {
            switch (cpu.gpr[9])
            {
                case 0x3C:
                    QTextStream(stdout) << static_cast<char>(cpu.gpr[4]);
                    break;
            }
        }

        if (cpu.pc == 0x000000B0)
        {
            switch (cpu.gpr[9])
            {
                case 0x3D:
                    QTextStream(stdout) << static_cast<char>(cpu.gpr[4]);
                    break;
            }
        }

        if (tracing)
        {
            disasm.before();
        }
        step();

        if (tracing)
        {
            out << disasm.after() << "\n";
            out.flush();
        }
    }
}
