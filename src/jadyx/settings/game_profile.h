// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// A game's own profile: settings that override Settings > System for that game alone. For now its
// performance mode, so a heavy game can run Handheld (720p) on the Series S while the rest stay on
// TV (1080p). Profiles are keyed by the game's launch path, the same string the library hands the
// boot.

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "common/settings.h"

namespace Jadyx {

/// The storage key of a game's profile: FNV-1a 64 of its launch path, in hex. Stable across runs
/// and builds (std::hash is not guaranteed to be).
[[nodiscard]] inline std::string GameProfileKey(std::string_view launch_path) {
    std::uint64_t hash = 0xcbf29ce484222325ull;
    for (const unsigned char c : launch_path) {
        hash ^= c;
        hash *= 0x100000001b3ull;
    }
    constexpr char digits[] = "0123456789abcdef";
    std::string key = "game_";
    for (int shift = 60; shift >= 0; shift -= 4) key += digits[(hash >> shift) & 15];
    return key;
}

/// The next choice when the player presses A on the profile: follow Settings, TV, Handheld.
[[nodiscard]] inline std::optional<Settings::ConsoleMode> NextGameConsoleMode(
    std::optional<Settings::ConsoleMode> mode) {
    if (!mode) return Settings::ConsoleMode::Docked;
    if (*mode == Settings::ConsoleMode::Docked) return Settings::ConsoleMode::Handheld;
    return std::nullopt;
}

/// The game's performance mode, or nullopt when it follows Settings > System.
[[nodiscard]] std::optional<Settings::ConsoleMode> LoadGameConsoleMode(std::string_view launch_path);
/// Saves it (nullopt removes the override). False when the settings store cannot be written.
bool SaveGameConsoleMode(std::string_view launch_path, std::optional<Settings::ConsoleMode> mode);

} // namespace Jadyx
