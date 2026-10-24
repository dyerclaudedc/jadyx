// SPDX-FileCopyrightText: Copyright 2026 Jadyx Project
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// The frontend's drawing vocabulary on a Direct2D render target: rounded panels, text in the
// interface language, the vector icons, rows, page headers, switches and button glyphs. The
// library canvas draws on its swapchain with it, and the in-game menu on a CPU bitmap, so both
// look the same from one implementation.

#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

#include "jadyx/ui/ui_icon.h"

namespace Jadyx {

/// Which of the two Dyer Play themes the calling thread draws with. The library and settings are
/// light (warm paper, ink and one red accent); what is drawn over a running game (the in-game
/// menu, its keyboard, Pulse) is dark glass, so the game stays the brightest thing on screen.
namespace Theme {
inline thread_local bool dark = false;

/// Draws with the dark theme until it goes out of scope (the previous theme comes back).
class DarkScope {
public:
    DarkScope() noexcept : previous{dark} { dark = true; }
    ~DarkScope() { dark = previous; }
    DarkScope(const DarkScope&) = delete;
    DarkScope& operator=(const DarkScope&) = delete;

private:
    bool previous;
};
} // namespace Theme

/// The frontend's colors (0xRRGGBB), taken from the Dyer Play screens. Each entry has a light and
/// a dark value and reads as the one of the calling thread's theme, so every surface keeps using
/// the same names.
namespace Palette {
struct Color {
    unsigned light;
    unsigned dark;
    operator unsigned() const noexcept { return Theme::dark ? dark : light; }
};
inline constexpr Color Canvas{0xf5f2ed, 0x14110f};   ///< The base layer, behind everything.
inline constexpr Color Surface{0xffffff, 0x1f1b19};  ///< Cards, panels and dialogs.
inline constexpr Color Graphite{0xe4dfd8, 0x2e2926}; ///< Dividers, tracks, badges.
inline constexpr Color Backdrop = Canvas;
inline constexpr Color Panel = Surface;
inline constexpr Color Row{0xefebe5, 0x26211e};
/// The focused row: white on paper, a lighter ink over a game.
inline constexpr Color RowActive{0xffffff, 0x3a3431};
/// "On" and primary actions: ink on paper (cream over a game), with OnAccent on top of it.
inline constexpr Color Accent{0x1f1b19, 0xf5f2ed};
inline constexpr Color OnAccent{0xf5f2ed, 0x1f1b19};
inline constexpr Color Text{0x1f1b19, 0xf5f2ed};
inline constexpr Color TextSecondary{0x5e5750, 0xb9b3ac};
inline constexpr Color TextHint{0x4a443e, 0xd3cec8};
inline constexpr Color Warning{0xa4521c, 0xf0c27a};
inline constexpr Color Track = Graphite;
/// Hairlines and translucent fills drawn at low alpha: ink on paper, white over a game.
inline constexpr Color Hairline{0x1f1b19, 0xffffff};
/// The Dyer red: the focus ring, switches that are on, the brand dot.
inline constexpr Color Brand{0xd9483b, 0xd9483b};
inline constexpr Color Focus = Brand;
/// Soft drop shadows: warm brown on paper, black over a game.
inline constexpr Color Shadow{0x46281a, 0x000000};
} // namespace Palette

/// The type scale, in 1280x720 layout units (the design's CSS pixels).
enum class TypeRole : unsigned {
    Hero,           ///< 56/64 light, the focused game's title.
    HeadlineLarge,  ///< 40/48 light, page titles.
    HeadlineMedium, ///< 28/36 regular.
    HeadlineSmall,  ///< 22/28 medium, section titles.
    BodyLarge,      ///< 18/26.
    Body,           ///< 15/22.
    LabelLarge,     ///< 14/20 medium.
    Label,          ///< 12/16 IBM Plex Mono: chips, figures, button letters.
    Hint,           ///< 13/18 medium, controller prompts.
    Caps,           ///< 12/16 IBM Plex Mono, widely tracked: the kicker over a title (pass it upper-cased).
    Count,
};

enum class TextAlign : unsigned { Leading, Center, Trailing };

/// Semantic targets emitted at the same place and size that a manager draws its controls.
enum class UiTarget : unsigned { ManagerTab, ManagerRow, ManagerCard };

class UiPainter {
public:
    virtual ~UiPainter() = default;
    virtual void Target(UiTarget kind, std::size_t index, float x, float y, float w, float h) {
        (void)kind, (void)index, (void)x, (void)y, (void)w, (void)h;
    }

    /// Draws on `target` from now on; texts are laid out by `write`.
    void Attach(ID2D1RenderTarget* target, IDWriteFactory* write);
    /// Text formats follow the interface language; call once per frame.
    void SyncLanguage();

    void Round(float x, float y, float w, float h, float radius, unsigned color, float alpha = 1);
    /// A rounded hairline (stroke centred on the rectangle's edge).
    void Outline(float x, float y, float w, float h, float radius, unsigned color, float alpha,
                 float stroke = 1);
    /// A dialog or card surface: Surface with a white hairline at 8%.
    void Card(float x, float y, float w, float h, float radius = 16);
    /// The screens' glass panel: translucent white with a white edge on a warm shadow on paper,
    /// translucent ink with a faint cream edge over a game.
    void Glass(float x, float y, float w, float h, float radius = 24);
    /// Game-management cards share a quiet surface and a thin white focus ring.
    void SelectionCard(float x, float y, float w, float h, bool focused, float radius = 16);
    /// Clips what is drawn to the rectangle until PopClip (lists that scroll under a fixed edge).
    void PushClip(float x, float y, float w, float h);
    void PopClip();
    /// Page chrome shared by mods, cheats and installed content; coordinates stay local.
    void ManagementSurface(float x, float y, float w, float h, float footer);
    /// Text in the type scale, clipped to `width` and at most `lines` lines (ellipsis after).
    /// Returns the height it took.
    float Type(std::wstring_view text, TypeRole role, float x, float y, float width, unsigned color,
               float alpha = 1, TextAlign align = TextAlign::Leading, unsigned lines = 1);
    float MeasureType(std::wstring_view text, TypeRole role);
    /// The tvOS focus: a diffuse shadow, a white ring and a faint halo around the rectangle,
    /// `amount` (0..1) of the way in.
    void FocusGlow(float x, float y, float w, float h, float radius, float amount);
    /// A capsule button: white with dark text when primary, graphite otherwise. A badge with the
    /// button that presses it sits at its end. Returns its width.
    float PillButton(std::wstring_view label, std::wstring_view letters, unsigned letters_color,
                     float x, float y, bool primary, float height = 44);
    float PillWidth(std::wstring_view label, std::wstring_view letters);
    /// A quiet capsule of metadata (a version, a play time); returns its width.
    float Chip(std::wstring_view text, float x, float y, unsigned color, float alpha = 1);
    /// A controller button badge: a 22 px graphite circle (a capsule for longer names) with a
    /// hairline, centred on (cx, cy); the letters take `color`. Returns its width.
    float Badge(std::wstring_view letters, float cx, float cy, unsigned color);
    float BadgeWidth(std::wstring_view letters);
    void Text(std::wstring_view text, float x, float y, float size, unsigned color, float width,
              float height = 40, bool bold = false, bool wrap = false, bool center = false,
              bool trailing = false);
    float MeasureText(std::wstring_view text, float size, bool bold);
    /// Native vector strokes: no texture loads, font glyphs or geometry allocation.
    /// Draws `icon` in a `size` box (the icons are drawn for 32).
    void Icon(UiIcon icon, float x, float y, unsigned color, float size = 32);
    /// The ">" at the end of a row that opens a page.
    void Chevron(float cx, float cy, unsigned color, bool left = false);
    /// A small "v" telling that a page goes on below what it shows.
    void MoreBelow(float cx, float cy, float alpha = 1);
    /// A selectable row: background, accent bar when active and the icon in its tile.
    /// Passive list position: track/thumb and available directions, hidden when everything fits.
    void ScrollIndicator(float x, float y, float height, std::size_t first,
                         std::size_t visible, std::size_t count, bool more_below = false);
    void RowFrame(float x, float y, float width, float height, bool active, UiIcon icon);
    void PageHeader(float x, float y, UiIcon icon, std::wstring_view title, std::wstring_view subtitle);
    /// An on/off switch, 52x28 at (x, y).
    void Switch(float x, float y, bool on, bool active);
    /// A controller button drawn natively: a ring with its letters (A, B, X, Y, LB, RB...).
    void ButtonGlyph(std::wstring_view letters, float x, float y, unsigned color);
    /// A button glyph and its label; returns the width it took.
    float Prompt(std::wstring_view letters, unsigned color, std::wstring_view label, float x, float y);
    /// The picture in `file`, cropped to fill the rectangle, with rounded corners. False when this
    /// surface draws no pictures or the file cannot be decoded: the caller draws a placeholder.
    virtual bool Picture(const std::filesystem::path& file, float x, float y, float w, float h) {
        (void)file, (void)x, (void)y, (void)w, (void)h;
        return false;
    }

protected:
    IDWriteTextFormat* Format(float size, bool bold, bool wrap, bool center, bool trailing);
    IDWriteTextLayout* TypeLayout(std::wstring_view text, TypeRole role, float width, TextAlign align,
                              unsigned lines);

    Microsoft::WRL::ComPtr<ID2D1RenderTarget> surface;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
    Microsoft::WRL::ComPtr<IDWriteFactory> write;

private:
    struct CachedLayout {
        Microsoft::WRL::ComPtr<IDWriteTextLayout> layout;
        unsigned frame;
    };

    std::wstring format_locale{L"en"};
    std::unordered_map<unsigned, Microsoft::WRL::ComPtr<IDWriteTextFormat>> formats;
    std::unordered_map<unsigned, Microsoft::WRL::ComPtr<IDWriteTextFormat>> role_formats;
    /// Laid-out texts by content and box: a redraw lays out only what changed. Entries unused for
    /// a while are dropped at SyncLanguage.
    std::unordered_map<std::wstring, CachedLayout> layouts;
    unsigned layout_frame{};
};

/// Colors of the controller button letters, for ButtonGlyph. Dyer Play draws them monochrome, as
/// the screens do: ink on paper, cream over a game.
namespace ButtonColor {
inline constexpr Palette::Color A = Palette::Text;
inline constexpr Palette::Color B = Palette::Text;
inline constexpr Palette::Color X = Palette::Text;
inline constexpr Palette::Color Y = Palette::Text;
inline constexpr Palette::Color Neutral = Palette::Text;
} // namespace ButtonColor

} // namespace Jadyx
