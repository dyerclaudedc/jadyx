// SPDX-FileCopyrightText: Copyright 2026 Jadyx Project
// SPDX-FileCopyrightText: Copyright 2026 Dyer Play contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "jadyx/ui/ui_fonts.h"

#include <filesystem>
#include <mutex>

#include <dwrite_3.h>
#include <wrl/client.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Storage.h>

namespace Jadyx {
using Microsoft::WRL::ComPtr;

namespace {

constexpr const wchar_t* SansFamily = L"Inter Tight";
constexpr const wchar_t* MonoFamily = L"IBM Plex Mono";

struct Fonts {
    ComPtr<IDWriteFontCollection> collection;
    bool sans{};
    bool mono{};
};

bool HasFamily(IDWriteFontCollection* collection, const wchar_t* family) {
    UINT32 index{};
    BOOL exists{};
    return SUCCEEDED(collection->FindFamilyName(family, &index, &exists)) && exists;
}

Fonts LoadFonts(IDWriteFactory* write) {
    ComPtr<IDWriteFactory5> factory;
    if (FAILED(write->QueryInterface(IID_PPV_ARGS(&factory)))) return {};
    std::filesystem::path root;
    try {
        root = std::filesystem::path{winrt::Windows::ApplicationModel::Package::Current()
                                         .InstalledLocation().Path().c_str()} / L"Assets" / L"Fonts";
    } catch (const winrt::hresult_error&) {
        return {}; // Not packaged (PC tests).
    }
    ComPtr<IDWriteFontSetBuilder1> builder;
    if (FAILED(factory->CreateFontSetBuilder(&builder))) return {};
    // A missing or unreadable file only loses its own weight; the rest still load.
    for (const wchar_t* name : {L"InterTight-Light.ttf", L"InterTight-Regular.ttf",
                                L"InterTight-Medium.ttf", L"InterTight-SemiBold.ttf",
                                L"IBMPlexMono-Regular.ttf", L"IBMPlexMono-Medium.ttf"}) {
        ComPtr<IDWriteFontFile> file;
        if (SUCCEEDED(factory->CreateFontFileReference((root / name).c_str(), nullptr, &file)))
            (void)builder->AddFontFile(file.Get());
    }
    ComPtr<IDWriteFontSet> set;
    ComPtr<IDWriteFontCollection1> collection;
    if (FAILED(builder->CreateFontSet(&set)) ||
        FAILED(factory->CreateFontCollectionFromFontSet(set.Get(), &collection)))
        return {};
    Fonts fonts;
    fonts.sans = HasFamily(collection.Get(), SansFamily);
    fonts.mono = HasFamily(collection.Get(), MonoFamily);
    if (fonts.sans || fonts.mono) fonts.collection = collection;
    return fonts;
}

const Fonts& Loaded(IDWriteFactory* write) {
    static std::once_flag once;
    static Fonts fonts;
    std::call_once(once, [&] { fonts = LoadFonts(write); });
    return fonts;
}

} // Anonymous namespace

InterfaceTypeface InterfaceFonts(IDWriteFactory* write) {
    const auto& fonts = Loaded(write);
    if (fonts.sans) return {fonts.collection.Get(), SansFamily};
    return {nullptr, L"Segoe UI"};
}

InterfaceTypeface MonoFonts(IDWriteFactory* write) {
    const auto& fonts = Loaded(write);
    if (fonts.mono) return {fonts.collection.Get(), MonoFamily};
    return {nullptr, L"Consolas"};
}

} // namespace Jadyx
