/*
 * Copyright (c) 2018-2020, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2021, Max Wipfli <max.wipfli@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/String.h>
#include <AK/Vector.h>

// On Linux distros that use mlibc `basename` is defined as a macro that expands to `__mlibc_gnu_basename` or `__mlibc_gnu_basename_c`, so we undefine it.
#if defined(AK_OS_LINUX) && defined(basename)
#    undef basename
#endif

namespace AK {

class LexicalPath {
public:
    enum StripExtension {
        No,
        Yes
    };

    explicit LexicalPath(String);

    bool is_absolute() const { return !m_string.is_empty() && m_string.starts_with('/'); }
    String const& string() const { return m_string; }

    StringView dirname() const { return m_dirname; }
    StringView basename(StripExtension s = StripExtension::No) const { return s == StripExtension::No ? m_basename : m_basename.substring_view(0, m_basename.length() - m_extension.length() - 1); }
    StringView title() const { return m_title; }
    StringView extension() const { return m_extension; }

    Vector<StringView> const& parts_view() const { return m_parts; }
    [[nodiscard]] Vector<String> parts() const;

    bool has_extension(StringView) const;
    bool is_child_of(LexicalPath const& possible_parent) const;

    [[nodiscard]] LexicalPath append(StringView) const;
    [[nodiscard]] LexicalPath prepend(StringView) const;
    [[nodiscard]] LexicalPath parent() const;

    [[nodiscard]] static String canonicalized_path(String);
    [[nodiscard]] static String absolute_path(String dir_path, String target);
    [[nodiscard]] static String relative_path(StringView absolute_path, StringView prefix);

    template<typename... S>
    [[nodiscard]] static LexicalPath join(StringView first, S&&... rest)
    {
        StringBuilder builder;
        builder.append(first);
        ((builder.append('/'), builder.append(forward<S>(rest))), ...);

        return LexicalPath { builder.to_string() };
    }

    [[nodiscard]] static String dirname(String path)
    {
        auto lexical_path = LexicalPath(move(path));
        return lexical_path.dirname();
    }

    [[nodiscard]] static String basename(String path, StripExtension s = StripExtension::No)
    {
        auto lexical_path = LexicalPath(move(path));
        return lexical_path.basename(s);
    }

    [[nodiscard]] static String title(String path)
    {
        auto lexical_path = LexicalPath(move(path));
        return lexical_path.title();
    }

    [[nodiscard]] static String extension(String path)
    {
        auto lexical_path = LexicalPath(move(path));
        return lexical_path.extension();
    }

private:
    Vector<StringView> m_parts;
    String m_string;
    StringView m_dirname;
    StringView m_basename;
    StringView m_title;
    StringView m_extension;
};

template<>
struct Formatter<LexicalPath> : Formatter<StringView> {
    ErrorOr<void> format(FormatBuilder& builder, LexicalPath const& value)
    {
        return Formatter<StringView>::format(builder, value.string());
    }
};

};

#if USING_AK_GLOBALLY
using AK::LexicalPath;
#endif
