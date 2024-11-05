/*
 * Copyright (c) 2018-2020, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2021, networkException <networkexception@serenityos.org>
 * Copyright (c) 2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Forward.h>
#include <AK/HashMap.h>
#include <AK/OwnPtr.h>
#include <AK/RefCounted.h>
#include <AK/RefPtr.h>
#include <AK/String.h>
#include <AK/Vector.h>
#include <LibCore/File.h>

namespace Core {

class ConfigFile : public RefCounted<ConfigFile> {
public:
    enum class AllowWriting {
        Yes,
        No,
    };

    static ErrorOr<NonnullRefPtr<ConfigFile>> open_for_lib(String const& lib_name, AllowWriting = AllowWriting::No);
    static ErrorOr<NonnullRefPtr<ConfigFile>> open_for_app(String const& app_name, AllowWriting = AllowWriting::No);
    static ErrorOr<NonnullRefPtr<ConfigFile>> open_for_system(String const& app_name, AllowWriting = AllowWriting::No);
    static ErrorOr<NonnullRefPtr<ConfigFile>> open(String const& filename, AllowWriting = AllowWriting::No);
    static ErrorOr<NonnullRefPtr<ConfigFile>> open(String const& filename, int fd);
    static ErrorOr<NonnullRefPtr<ConfigFile>> open(String const& filename, NonnullOwnPtr<Core::File>);
    ~ConfigFile();

    bool has_group(String const&) const;
    bool has_key(String const& group, String const& key) const;

    Vector<String> groups() const;
    Vector<String> keys(String const& group) const;

    size_t num_groups() const { return m_groups.size(); }

    String read_entry(String const& group, String const& key, String const& default_value = {}) const
    {
        return read_entry_optional(group, key).value_or(default_value);
    }
    Optional<String> read_entry_optional(String const& group, String const& key) const;
    bool read_bool_entry(String const& group, String const& key, bool default_value = false) const;

    template<Integral T = int>
    T read_num_entry(String const& group, String const& key, T default_value = 0) const
    {
        if (!has_key(group, key))
            return default_value;

        return read_entry(group, key, "").to_number<T>().value_or(default_value);
    }

    void write_entry(String const& group, String const& key, String const& value);
    void write_bool_entry(String const& group, String const& key, bool value);

    template<Integral T = int>
    void write_num_entry(String const& group, String const& key, T value)
    {
        write_entry(group, key, String::number(value));
    }

    void dump() const;

    bool is_dirty() const { return m_dirty; }

    ErrorOr<void> sync();

    void add_group(String const& group);
    void remove_group(String const& group);
    void remove_entry(String const& group, String const& key);

    String const& filename() const { return m_filename; }

private:
    ConfigFile(String const& filename, OwnPtr<InputBufferedFile> open_file);

    ErrorOr<void> reparse();

    String m_filename;
    OwnPtr<InputBufferedFile> m_file;
    HashMap<String, HashMap<String, String>> m_groups;
    bool m_dirty { false };
};

}
