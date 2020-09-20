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

#include <QFileDialog>
#include <QMessageBox>
#include "psemu.h"
#include "../libpsemu/include/types.h"

PSEmu::PSEmu() noexcept : emu_thread(new Emulator(this))
{
    const auto bios_file{ file_open_force("Select PlayStation BIOS",
                                          "PlayStation BIOS files (*.bin)") };

    const auto exe_file{ file_open_force("Select PS-X EXE",
                                         "PS-X EXEs (*.exe)") };

    load_bios_file(bios_file);

    connect(emu_thread, &Emulator::render_frame, &opengl, &OpenGL::render_frame);

    connect(emu_thread, &Emulator::time_to_inject_exe, this, [=]()
    {
        QFile file(exe_file);

        if (!file.open(QIODevice::ReadOnly))
        {
            const auto error_string{ QString("Unable to open %1: %2")
                                     .arg(exe_file)
                                     .arg(file.errorString()) };

            QMessageBox::critical(nullptr, tr("Error"), error_string);
            exit(EXIT_FAILURE);
        }

        const auto file_handle{ file.readAll() };
        const auto file_data{ file_handle.constData() };

        PlayStation::Word initial_pc;
        PlayStation::Word initial_gp;
        PlayStation::Word dest_in_ram;
        PlayStation::Word file_size;

        std::memcpy(&initial_pc,  &file_data[0x10], sizeof(PlayStation::Word));
        std::memcpy(&initial_gp,  &file_data[0x14], sizeof(PlayStation::Word));
        std::memcpy(&dest_in_ram, &file_data[0x18], sizeof(PlayStation::Word));
        std::memcpy(&file_size,   &file_data[0x1C], sizeof(PlayStation::Word));

        for (auto index{ 0x800 }; index != file_size;
             index += 4, dest_in_ram += 4)
        {
            std::memcpy(&emu_thread->bus.ram.data()[dest_in_ram & 0x1FFF'FFFF],
                        &file_data[index],
                        sizeof(PlayStation::Word));
        }

        file.close();

        emu_thread->cpu.pc      = initial_pc;
        emu_thread->cpu.next_pc = initial_pc + 4;

        emu_thread->cpu.instruction.word =
        emu_thread->bus.memory_access<PlayStation::Word>(emu_thread->cpu.pc);

        // The emulator thread is halted during this process, restart it now
        // that the EXE has been injected.
        emu_thread->start();
    });

    main_window.setCentralWidget(&opengl);
    main_window.show();

    emu_thread->start();
}

/// @brief Load a BIOS file for use by the emulator.
/// @param file The file path to load the BIOS data from.
auto PSEmu::load_bios_file(const QString& file_name) noexcept -> void
{
    QFile file(file_name);

    if (!file.open(QIODevice::ReadOnly))
    {
        const auto error_string{ QString("Unable to open %1: %2")
                                .arg(file_name)
                                .arg(file.errorString()) };

        QMessageBox::critical(nullptr, tr("Error"), error_string);
        exit(EXIT_FAILURE);
    }

    const auto file_data{ file.readAll() };
    PlayStation::BIOS bios;

    for (auto index{ 0 }; index < PlayStation::BIOS_SIZE; ++index)
    {
        bios[index] = file_data[index];
    }
    file.close();

    emu_thread->set_bios_data(bios);
}

/// @brief Spawn a QFileDialog and force the user to choose a file, or quit the
/// program.
/// @param title The title of the QFileDialog.
/// @param filter The filter of the QFileDialog.
/// @return The file selected.
auto PSEmu::file_open_force(const QString& title,
                            const QString& filter) noexcept -> QString
{
    for (;;)
    {
        const auto file_name{ QFileDialog::getOpenFileName(nullptr,
                                                           title,
                                                           "",
                                                           filter) };

        if (file_name.isEmpty())
        {
            const auto response
            {
                QMessageBox::critical(nullptr,
                                      tr("Error"),
                                      tr("You must select a file."),
                                      QMessageBox::Retry | QMessageBox::Cancel)
            };

            switch (response)
            {
                case QMessageBox::Retry:
                    continue;

                case QMessageBox::Cancel:
                default:
                    exit(EXIT_FAILURE);
            }
        }
        else
        {
            return file_name;
        }
    }
}
