// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "jadyx/settings/game_profile.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>

namespace Jadyx {
namespace {

// One container keeps every game's profile apart from the system settings; each game is one
// composite value under its key, so a profile grows new fields without new keys.
constexpr auto ContainerName = L"game_profiles";
constexpr auto ConsoleModeField = L"console_mode";

winrt::Windows::Storage::ApplicationDataContainer Profiles() {
    using winrt::Windows::Storage::ApplicationData;
    using winrt::Windows::Storage::ApplicationDataCreateDisposition;
    return ApplicationData::Current().LocalSettings().CreateContainer(
        ContainerName, ApplicationDataCreateDisposition::Always);
}

} // namespace

std::optional<Settings::ConsoleMode> LoadGameConsoleMode(std::string_view launch_path) {
    try {
        const auto values = Profiles().Values();
        const auto key = winrt::to_hstring(GameProfileKey(launch_path));
        if (!values.HasKey(key)) return std::nullopt;
        const auto profile = values.Lookup(key).as<winrt::Windows::Storage::ApplicationDataCompositeValue>();
        if (!profile.HasKey(ConsoleModeField)) return std::nullopt;
        const auto raw = winrt::unbox_value<int32_t>(profile.Lookup(ConsoleModeField));
        // A value this build does not know (an older or newer one's) follows Settings.
        if (raw == static_cast<int32_t>(Settings::ConsoleMode::Docked)) return Settings::ConsoleMode::Docked;
        if (raw == static_cast<int32_t>(Settings::ConsoleMode::Handheld)) return Settings::ConsoleMode::Handheld;
    } catch (...) {
    }
    return std::nullopt;
}

bool SaveGameConsoleMode(std::string_view launch_path, std::optional<Settings::ConsoleMode> mode) {
    try {
        const auto values = Profiles().Values();
        const auto key = winrt::to_hstring(GameProfileKey(launch_path));
        if (!mode) {
            if (values.HasKey(key)) values.Remove(key);
            return true;
        }
        winrt::Windows::Storage::ApplicationDataCompositeValue profile;
        profile.Insert(ConsoleModeField, winrt::box_value(static_cast<int32_t>(*mode)));
        values.Insert(key, profile);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace Jadyx
