// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Minimal test harness. Each TEST(id, name) is run in isolation; any exception (including the stub's
// std::logic_error "not implemented") is reported as a clean FAIL. Exit code 1 if any test fails.
// Before running, FTZ+DAZ are enabled exactly as Rack does for its engine threads (C-017:
// Rack src/system.cpp resetFpuFlags(), MXCSR |= 0x8040 on x64).
#pragma once
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#if defined(__x86_64__) || defined(_M_X64)
#include <xmmintrin.h>
#endif

namespace th
{
struct Case { const char* id; const char* name; std::function<void()> fn; };
inline std::vector<Case>& registry() { static std::vector<Case> r; return r; }
struct Reg { Reg (const char* id, const char* n, std::function<void()> f) { registry().push_back ({ id, n, std::move (f) }); } };
struct Fail : std::runtime_error { using std::runtime_error::runtime_error; };

inline void setRackFpuFlags()
{
#if defined(__x86_64__) || defined(_M_X64)
    _mm_setcsr ((_mm_getcsr() | 0x8040u) & ~0x6000u);
#endif
}

inline int runAll (int argc, char** argv)
{
    setRackFpuFlags();
    const char* only = argc > 1 ? argv[1] : nullptr;
    int fails = 0, run = 0;
    for (auto& c : registry())
    {
        if (only && std::string (only) != c.id) continue;
        ++run;
        try { c.fn(); std::printf ("PASS %s %s\n", c.id, c.name); }
        catch (const std::exception& e) { ++fails; std::printf ("FAIL %s %s: %s\n", c.id, c.name, e.what()); }
        catch (...) { ++fails; std::printf ("FAIL %s %s: unknown exception\n", c.id, c.name); }
        std::fflush (stdout);
    }
    std::printf ("SUMMARY %d/%d passed\n", run - fails, run);
    return fails ? 1 : 0;
}
} // namespace th

#define TH_CAT2(a, b) a##b
#define TH_CAT(a, b) TH_CAT2 (a, b)
#define TEST(ID, NAME)                                                               \
    static void TH_CAT (th_test_, __LINE__)();                                       \
    static th::Reg TH_CAT (th_reg_, __LINE__) (ID, NAME, TH_CAT (th_test_, __LINE__)); \
    static void TH_CAT (th_test_, __LINE__)()
#define CHECK_MSG(cond, ...)                                                            \
    do { if (! (cond)) { char b_[768]; int k_ = std::snprintf (b_, sizeof b_, "%s:%d [%s] ", __FILE__, __LINE__, #cond); \
         std::snprintf (b_ + k_, sizeof b_ - (size_t) k_, __VA_ARGS__); throw th::Fail (b_); } } while (0)
#define CHECK(cond) CHECK_MSG (cond, "%s", "")
#define TEST_MAIN int main (int argc, char** argv) { return th::runAll (argc, argv); }
