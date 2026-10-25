// SPDX-FileCopyrightText: Copyright 2026 Jadyx Project
// SPDX-License-Identifier: GPL-3.0-or-later

// The checks must run in any configuration: an NDEBUG build would otherwise pass vacuously.
#undef NDEBUG
#include <cassert>
#include <iostream>
#include "jadyx/app/prewarm_budget.h"

namespace {
using Jadyx::PrewarmBudget;
constexpr std::uint64_t M = PrewarmBudget::MiB;

void TestDefaultsAndClamp() {
    assert(PrewarmBudget{}.MiBPerCore() == 64);
    assert(PrewarmBudget{}.BytesPerCore() == 64 * M);
    assert(PrewarmBudget{-3}.Step() == PrewarmBudget::BOOST_STEP);
    assert(PrewarmBudget{-1}.MiBPerCore() == 96);
    assert(PrewarmBudget{99}.MiBPerCore() == 16);
    assert(Jadyx::JitCacheMiBForLimit(5120 * M) == 192);
    assert(Jadyx::JitCacheMiBForLimit(1024 * M) == 128);
    assert(Jadyx::JitCacheMiBForLimit(0) == 128);
}

void TestLowersOncePerSession() {
    PrewarmBudget budget;
    assert(!budget.Observe(2048 * M));
    assert(!budget.Observe(257 * M));
    assert(budget.Observe(255 * M)); // Game C at the limit: 64 -> 32 for the next session
    assert(budget.MiBPerCore() == 32);
    assert(!budget.Observe(0)); // once per session, however low it goes
    assert(budget.MiBPerCore() == 32);
    assert(!budget.Finish(3600)); // a session that came close never raises it
    assert(budget.MinHeadroom() == 0);

    PrewarmBudget lowest{2};
    assert(!lowest.Observe(0) && lowest.MiBPerCore() == 16);
}

void TestRaisesAfterAComfortableSession() {
    PrewarmBudget budget{1};
    assert(!budget.Observe(1500 * M));
    assert(!budget.Finish(PrewarmBudget::RAISE_SECONDS - 1)); // too short to judge
    PrewarmBudget long_enough{2};
    assert(!long_enough.Observe(1500 * M));
    assert(long_enough.Finish(PrewarmBudget::RAISE_SECONDS));
    assert(long_enough.MiBPerCore() == 32);

    PrewarmBudget tight{1};
    assert(!tight.Observe(1023 * M)); // between the thresholds: keep
    assert(!tight.Finish(3600) && tight.MiBPerCore() == 32);

    PrewarmBudget top{PrewarmBudget::BOOST_STEP};
    assert(!top.Observe(4096 * M));
    assert(!top.Finish(3600) && top.Step() == PrewarmBudget::BOOST_STEP);

    // Above the default only with lots to spare (Smash: 2.2 GiB free for ten minutes).
    PrewarmBudget near{};
    assert(!near.Observe(1400 * M));
    assert(!near.Finish(3600) && near.MiBPerCore() == 64);
    PrewarmBudget roomy{};
    assert(!roomy.Observe(2200 * M));
    assert(roomy.Finish(600) && roomy.Step() == PrewarmBudget::BOOST_STEP && roomy.MiBPerCore() == 96);
    PrewarmBudget boosted{PrewarmBudget::BOOST_STEP};
    assert(boosted.Observe(200 * M) && boosted.MiBPerCore() == 64); // and back down at once

    PrewarmBudget unobserved{1}; // no samples: the minimum stays at its maximum
    assert(unobserved.Finish(PrewarmBudget::RAISE_SECONDS) && unobserved.Step() == 0);
    assert(unobserved.Finish(PrewarmBudget::RAISE_SECONDS) && unobserved.Step() == PrewarmBudget::BOOST_STEP);
}
} // namespace

int main() {
    TestDefaultsAndClamp();
    TestLowersOncePerSession();
    TestRaisesAfterAComfortableSession();
    std::cout << "PASS: defaults, clamping, lowering once per session, raising after a "
                 "comfortable session\n";
}
