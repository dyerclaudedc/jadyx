// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
// Header-only: cl /std:c++20 /I src tools/xbox/tests/console-model.cpp
#include <cstdio>
#include <cstdlib>
#include "jadyx/app/console_model.h"

int main() {
    using namespace Jadyx;
    using namespace Jadyx::ConsoleModelIds;
    auto check = [](bool ok) { if (!ok) std::abort(); };
    static_assert(ConsoleModelFromIds(true, VendorMicrosoft, XboxSeriesS) == ConsoleModel::XboxSeriesS);
    check(ConsoleModelFromIds(false, VendorMicrosoft, XboxSeriesS) == ConsoleModel::Pc);
    check(ConsoleModelFromIds(true, VendorMicrosoft, XboxSeriesX) == ConsoleModel::XboxSeriesX);
    check(ConsoleModelFromIds(true, VendorMicrosoft, XboxSeriesXDevkit) == ConsoleModel::XboxSeriesX);
    check(ConsoleModelFromIds(true, VendorMicrosoft, XboxOneX) == ConsoleModel::XboxOne);
    check(ConsoleModelFromIds(true, VendorMicrosoft, 0x12345678) == ConsoleModel::XboxUnknown);
    check(ConsoleModelFromIds(true, 0x1234, XboxSeriesS) == ConsoleModel::XboxUnknown);
    check(ConsoleModelName(ConsoleModel::XboxSeriesS) == "Xbox Series S");
    // Output size: 1440p on the Series S, the window everywhere else, boot.cfg overrides both.
    constexpr OutputSize window{1920, 1080};
    static_assert(ChooseOutputSize(ConsoleModel::XboxSeriesS, window, OutputPreference::Automatic) ==
                  OutputSize{2560, 1440});
    check(ChooseOutputSize(ConsoleModel::XboxSeriesX, window, OutputPreference::Automatic) == window);
    check(ChooseOutputSize(ConsoleModel::Pc, {1600, 900}, OutputPreference::Automatic) == OutputSize{1600, 900});
    check(ChooseOutputSize(ConsoleModel::XboxSeriesS, window, OutputPreference::Window) == window);
    check(ChooseOutputSize(ConsoleModel::XboxSeriesX, window, OutputPreference::P1440) == OutputSize{2560, 1440});
    check(ParseOutputPreference("1080p") == OutputPreference::P1080);
    check(ParseOutputPreference("4k") == OutputPreference::P2160);
    check(ParseOutputPreference("window") == OutputPreference::Window);
    check(ParseOutputPreference("auto") == OutputPreference::Automatic);
    check(ParseOutputPreference("") == OutputPreference::Automatic);
    std::puts("console model: ids and names PASS");
}
