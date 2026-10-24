// SPDX-FileCopyrightText: Copyright 2026 Jadyx Project
// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// The interface typefaces of the Dyer Play screens: Inter Tight for text and IBM Plex Mono for
// kickers, metadata chips and button letters. Both ship in the package (Assets/Fonts, SIL OFL
// 1.1); outside a package, or when a file is missing, Segoe UI and Consolas stand in.

#include <dwrite.h>

namespace Jadyx {

struct InterfaceTypeface {
    IDWriteFontCollection* collection; ///< nullptr: the system collection.
    const wchar_t* family;
};

/// Built once from the first factory that asks; DirectWrite's shared factory is a singleton, so
/// every surface (library, in-game menu, Pulse) gets the same collection.
InterfaceTypeface InterfaceFonts(IDWriteFactory* write);
/// The monospaced companion, from the same collection.
InterfaceTypeface MonoFonts(IDWriteFactory* write);

} // namespace Jadyx
