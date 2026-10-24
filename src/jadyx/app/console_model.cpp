// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "jadyx/app/console_model.h"

#include <atomic>

#include <windows.h>
#include <gamingdeviceinformation.h>
#include <winrt/Windows.System.Profile.h>

namespace Jadyx {
namespace {

ConsoleModel QueryConsoleModel() {
    bool xbox = false;
    try {
        xbox = winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo().DeviceFamily() ==
               L"Windows.Xbox";
    } catch (...) {
        return ConsoleModel::Pc;
    }
    if (!xbox) return ConsoleModel::Pc;
    GAMING_DEVICE_MODEL_INFORMATION info{};
    if (FAILED(GetGamingDeviceModelInformation(&info))) return ConsoleModel::XboxUnknown;
    return ConsoleModelFromIds(true, static_cast<std::uint32_t>(info.vendorId),
                               static_cast<std::uint32_t>(info.deviceId));
}

std::atomic<OutputPreference> output_preference{OutputPreference::Automatic};

} // namespace

ConsoleModel DetectConsoleModel() {
    static const ConsoleModel model = QueryConsoleModel();
    return model;
}

void SetOutputPreference(OutputPreference preference) {
    output_preference.store(preference, std::memory_order_relaxed);
}

OutputSize OutputSizeForWindow(OutputSize window) {
    return ChooseOutputSize(DetectConsoleModel(), window,
                            output_preference.load(std::memory_order_relaxed));
}

} // namespace Jadyx
