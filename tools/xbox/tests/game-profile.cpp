// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
// The pure half of game_profile.h: storage keys and the order of the choices.
#include <cstdio>
#include <cstdlib>
#include "jadyx/settings/game_profile.h"

int main() {
    using namespace Jadyx;
    auto check = [](bool ok) { if (!ok) std::abort(); };
    // FNV-1a 64 of "" is its offset basis; the key is stable and differs per path.
    check(GameProfileKey("") == "game_cbf29ce484222325");
    check(GameProfileKey("a") == "game_af63dc4c8601ec8c");
    check(GameProfileKey("C:\\games\\x.nsp") != GameProfileKey("C:\\games\\y.nsp"));
    check(GameProfileKey("C:\\games\\x.nsp").size() == 21);
    // Settings, then TV, then Handheld, then Settings again.
    auto mode = NextGameConsoleMode(std::nullopt);
    check(mode == Settings::ConsoleMode::Docked);
    mode = NextGameConsoleMode(mode);
    check(mode == Settings::ConsoleMode::Handheld);
    check(!NextGameConsoleMode(mode));
    std::puts("game profile: keys and choices PASS");
}
