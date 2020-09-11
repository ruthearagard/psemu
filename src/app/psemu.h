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

#include <QObject>
#include "emulator.h"

/// @brief PSEmu main controller.
class PSEmu : public QObject
{
    Q_OBJECT

public:
    PSEmu() noexcept;

private:
    /// @brief Load a BIOS file for use by the emulator.
    /// @param file The file path to load the BIOS data from.
    auto load_bios_file(const QString& file_name) noexcept -> void;

    /// @brief Spawn a QFileDialog and force the user to choose a file, or
    /// quit the program.
    /// @param title The title of the QFileDialog.
    /// @param filter The filter of the QFileDialog.
    /// @return The file selected.
    auto file_open_force(const QString& title,
                         const QString& filter) noexcept -> QString;

    /// @brief Emulator instance
    Emulator* emu_thread;
};
