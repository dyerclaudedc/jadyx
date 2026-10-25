// SPDX-FileCopyrightText: Copyright 2026 Jadyx Project
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <limits>

namespace Jadyx {
/// How much code the JIT prewarm may compile per core, learned per game from its last sessions.
/// The prewarm ends before the game grows, so it cannot judge its own cost by the headroom at the
/// time: a game whose profile has filled up (Game C: 115 MiB on three cores, 1 GiB of commit)
/// only runs out minutes later. A session that comes close to the limit lowers the next one's
/// budget a step; a clean session with plenty to spare raises it back. Pure, no clocks or I/O.
class PrewarmBudget {
public:
    static constexpr std::uint64_t MiB = 1024 * 1024;
    /// Per core, from the most down. Step 0 (64 MiB) is the default and what a stored 0 means; the
    /// plan warms gameplay and shared blocks first, so the lower steps keep the blocks that matter
    /// most. 115 cost a full profile (Game C) 1 GiB of a 5 GiB limit, so 96 is never the default:
    /// a game earns it (step -1) only after a long session with lots of memory to spare. Super
    /// Smash Bros. Ultimate filled 64 MiB on two cores with 27 000 blocks left over, compiled
    /// during play as hitches, while 2.2 GiB of its 5 GiB stayed free.
    static constexpr std::array<std::uint32_t, 4> STEPS_MIB{96, 64, 32, 16};
    static constexpr std::int32_t BOOST_STEP = -1;
    static constexpr std::int32_t LOWEST_STEP = static_cast<std::int32_t>(STEPS_MIB.size()) - 2;
    /// A session whose headroom fell below this lowers the next one's budget, at once.
    static constexpr std::uint64_t LOW_HEADROOM = 256 * MiB;
    /// A clean session whose headroom never fell below this raises it back a step...
    static constexpr std::uint64_t HIGH_HEADROOM = 1024 * MiB;
    /// ...and above the default only with this much: the boost costs up to 32 MiB of code per
    /// core, plus its metadata.
    static constexpr std::uint64_t BOOST_HEADROOM = 1536 * MiB;
    /// ...if it lasted long enough to have seen the game's heavier parts.
    static constexpr std::uint32_t RAISE_SECONDS = 180;

    /// `step` as stored (0 is the default budget, -1 the boost); out of range values clamp.
    constexpr explicit PrewarmBudget(std::int32_t stored = 0) noexcept
        : step{stored < BOOST_STEP ? BOOST_STEP : stored > LOWEST_STEP ? LOWEST_STEP : stored} {}

    [[nodiscard]] constexpr std::int32_t Step() const noexcept { return step; }
    [[nodiscard]] constexpr std::uint32_t MiBPerCore() const noexcept {
        return STEPS_MIB[static_cast<std::size_t>(step - BOOST_STEP)];
    }
    [[nodiscard]] constexpr std::uint64_t BytesPerCore() const noexcept {
        return std::uint64_t{MiBPerCore()} * MiB;
    }

    /// The headroom during play. True when the next session's budget just went down a step
    /// (once per session): store Step() now, the session may end in a crash or a kill.
    constexpr bool Observe(std::uint64_t headroom) noexcept {
        if (headroom < min_headroom) {
            min_headroom = headroom;
        }
        if (lowered || headroom >= LOW_HEADROOM) {
            return false;
        }
        lowered = true;
        if (step >= LOWEST_STEP) {
            return false;
        }
        ++step;
        return true;
    }

    /// The session ended cleanly after `seconds` of play. True when the budget went up a step.
    constexpr bool Finish(std::uint32_t seconds) noexcept {
        if (lowered || step == BOOST_STEP || seconds < RAISE_SECONDS) {
            return false;
        }
        if (min_headroom < (step == 0 ? BOOST_HEADROOM : HIGH_HEADROOM)) {
            return false;
        }
        --step;
        return true;
    }

    [[nodiscard]] constexpr std::uint64_t MinHeadroom() const noexcept { return min_headroom; }

private:
    std::int32_t step;
    std::uint64_t min_headroom{std::numeric_limits<std::uint64_t>::max()};
    bool lowered{};
};

/// The per-core JIT code cache when boot.cfg does not set one. A full cache is cleared whole, and
/// every block the game runs next is compiled again: the worst CPU hitch there is. With the
/// boosted prewarm (96 MiB) a 128 MiB cache left a large game 32 MiB to grow into. The cache
/// commits only what is emitted, so a bigger one costs nothing until the game needs it; the
/// App-mode limit (about 1 GiB) keeps the old size.
[[nodiscard]] constexpr std::uint32_t JitCacheMiBForLimit(std::uint64_t app_limit) noexcept {
    return app_limit >= 4096 * PrewarmBudget::MiB ? 192 : 128;
}
} // namespace Jadyx
