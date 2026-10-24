// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "jadyx/app/console_model.h"

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

} // namespace

ConsoleModel DetectConsoleModel() {
    static const ConsoleModel model = QueryConsoleModel();
    return model;
}

} // namespace Jadyx
