// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Which machine the app runs on. Xbox Series S and Series X share the CPU family and the Dev Mode
// memory limit, but the Series S GPU has about a third of the compute and less bandwidth, so the
// renderer and the performance mode can lean on this to pick lighter defaults there.
//
// The ids are the GAMING_DEVICE_* values of <gamingdeviceinformation.h>, spelled as numbers so an
// older Windows SDK without the Series constants still builds. The vendor is
// GAMING_DEVICE_VENDOR_ID_MICROSOFT (0xC2EC5032), not the USB vendor id 0x045E; the Series ids are
// the ones Chuck Walbourn documents for UWP on Xbox Series X|S.

#include <cstdint>
#include <string_view>

namespace Jadyx {

enum class ConsoleModel : std::uint8_t {
    Pc,          ///< Not an Xbox (a Windows PC running the same package).
    XboxOne,     ///< Xbox One, One S, One X (and their dev kits).
    XboxSeriesS,
    XboxSeriesX, ///< Series X and the Series X|S dev kit.
    XboxUnknown, ///< An Xbox this build does not know yet.
};

namespace ConsoleModelIds {
inline constexpr std::uint32_t VendorMicrosoft = 0xC2EC5032;
inline constexpr std::uint32_t XboxOne = 0x768BAE26;
inline constexpr std::uint32_t XboxOneS = 0x2A7361D9;
inline constexpr std::uint32_t XboxOneX = 0x5AD617C7;
inline constexpr std::uint32_t XboxOneXDevkit = 0x10F7CDE3;
inline constexpr std::uint32_t XboxSeriesS = 0x1D27FABB;
inline constexpr std::uint32_t XboxSeriesX = 0x2F7A3DFF;
inline constexpr std::uint32_t XboxSeriesXDevkit = 0xDE8A5661;
} // namespace ConsoleModelIds

/// Pure mapping, kept apart from the OS query so it can be tested anywhere.
[[nodiscard]] constexpr ConsoleModel ConsoleModelFromIds(bool xbox, std::uint32_t vendor,
                                                         std::uint32_t device) {
    using namespace ConsoleModelIds;
    if (!xbox) return ConsoleModel::Pc;
    if (vendor != VendorMicrosoft) return ConsoleModel::XboxUnknown;
    switch (device) {
    case XboxOne:
    case XboxOneS:
    case XboxOneX:
    case XboxOneXDevkit:
        return ConsoleModel::XboxOne;
    case XboxSeriesS:
        return ConsoleModel::XboxSeriesS;
    case XboxSeriesX:
    case XboxSeriesXDevkit:
        return ConsoleModel::XboxSeriesX;
    default:
        return ConsoleModel::XboxUnknown;
    }
}

[[nodiscard]] constexpr std::string_view ConsoleModelName(ConsoleModel model) {
    switch (model) {
    case ConsoleModel::Pc:          return "PC";
    case ConsoleModel::XboxOne:     return "Xbox One";
    case ConsoleModel::XboxSeriesS: return "Xbox Series S";
    case ConsoleModel::XboxSeriesX: return "Xbox Series X";
    case ConsoleModel::XboxUnknown: return "Xbox";
    }
    return "Xbox";
}

/// Asked once, then cached: the answer cannot change while the app runs.
[[nodiscard]] ConsoleModel DetectConsoleModel();

/// The size of the swapchains (library and game), in pixels.
struct OutputSize {
    std::uint32_t width;
    std::uint32_t height;
    constexpr bool operator==(const OutputSize&) const = default;
};

/// `output=` in boot.cfg: automatic, or a fixed size for testing.
enum class OutputPreference : std::uint8_t { Automatic, Window, P1080, P1440, P2160 };

[[nodiscard]] constexpr OutputPreference ParseOutputPreference(std::string_view value) {
    if (value == "window") return OutputPreference::Window;
    if (value == "1080" || value == "1080p") return OutputPreference::P1080;
    if (value == "1440" || value == "1440p") return OutputPreference::P1440;
    if (value == "2160" || value == "2160p" || value == "4k") return OutputPreference::P2160;
    return OutputPreference::Automatic;
}

/// What the app presents at: the window's size everywhere, unless boot.cfg asks for another.
/// A 2560x1440 swapchain on the Series S (which Microsoft documents as supported) stalled there:
/// its frame-latency waitable went a second at a time without a frame, every couple of seconds,
/// until the device was reset (Super Mario Bros. Wonder, 0.3.8.108/109). The window's 1920x1080
/// is what Jadyx presents at and what the community reports on the Series S were played at.
[[nodiscard]] constexpr OutputSize ChooseOutputSize(ConsoleModel model, OutputSize window,
                                                    OutputPreference preference) {
    switch (preference) {
    case OutputPreference::Window: return window;
    case OutputPreference::P1080:  return {1920, 1080};
    case OutputPreference::P1440:  return {2560, 1440};
    case OutputPreference::P2160:  return {3840, 2160};
    case OutputPreference::Automatic: break;
    }
    (void)model;
    return window;
}

/// The `output=` preference read from boot.cfg at startup, kept for the rest of the run.
void SetOutputPreference(OutputPreference preference);
/// ChooseOutputSize for this machine and that preference: what a swapchain for a window of this
/// size should be (the library calls it again when the window is resized).
[[nodiscard]] OutputSize OutputSizeForWindow(OutputSize window);

} // namespace Jadyx
