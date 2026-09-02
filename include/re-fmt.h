#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

template <>
struct fmt::formatter<RE::NiPoint3>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    auto format(const RE::NiPoint3& value, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "({}, {}, {})", value.x, value.y, value.z);
    }
};

template <>
struct fmt::formatter<RE::BSFixedString>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    auto format(const RE::BSFixedString& value, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "{}", value.c_str());
    }
};
