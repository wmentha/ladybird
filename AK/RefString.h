/*
 * Copyright (c) 2024, William JK Mentha <william@mentha.au>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Forward.h>
#include <AK/String.h>

namespace AK {

    // RefString is a convenience reference counted wrapper for String.
    // Suitable for passing around as a value type.
    // Same as passing around NonnullRefPtr<String>
    // with an identical forwarding String API.
    //
    // Note that RefString is an immutable object that cannot shrink or grow.
    // Its allocation size is snugly tailored to the specific string it contains.
    // Once constructed, RefString cannot be reassigned to a different value.
    // Copying a RefString is very efficient, as copying only requires modifying
    // the ref count.
    //
    // There are four main ways to construct a new RefString from a string:
    //
    //    auto string = String("some existing string");
    //    auto ref_string = RefString(move(string));
    //
    //    auto ref_string = "some literal"_ref_string;
    //
    //    auto ref_string - RefString::formatted("{} little piggies", m_piggies);
    //
    //     StringBuilder builder;
    //     builder.append("abc");
    //     builder.append("123");
    //     auto ref_string = builder.to_ref_string();

class RefString {

public:   
    RefString() = delete;

    RefString(RefString const&) = default;

    RefString(RefString&&) = default;

    RefString(String const&) = delete;

    RefString(String&& string)
        : m_data(move(string))
    {
    }

    RefString& operator=(RefString&& other) = delete;
    // {
    //     if (this != &other)
    //         m_data = move(other.m_data);
    //     return *this;
    // }

    RefString& operator=(RefString const& other) = delete;
    // {
    //     if (this != &other)
    //         m_data = const_cast<RefString&>(other).m_data;
    //     return *this;
    // }

    RefString& operator=(String&& other) = delete;
    // {
    //     if (this->m_data != &other)
    //         m_data = move(other);
    //     return *this;
    // }

    RefString& operator=(String const& other) = delete;
    // {
    //     if (this->m_data != &other)
    //         m_data = const_cast<String&>(other);
    //     return *this;
    // }

    // String Forwarding Functions:

    // Creates a new String by case-transforming this String. Using these methods require linking LibUnicode into your application.
    ALWAYS_INLINE ErrorOr<String> to_lowercase(Optional<StringView> const& locale = {}) const
    {
        return m_data->to_lowercase(locale);
    }

    ALWAYS_INLINE ErrorOr<String> to_uppercase(Optional<StringView> const& locale = {}) const
    {
        return m_data->to_uppercase(locale);
    }

    ALWAYS_INLINE ErrorOr<String> to_titlecase(Optional<StringView> const& locale = {}, TrailingCodePointTransformation trailing_code_point_transformation = TrailingCodePointTransformation::Lowercase) const
    {
        return m_data->to_titlecase(locale, trailing_code_point_transformation);
    }

    ALWAYS_INLINE ErrorOr<String> to_casefold()
    {
        return m_data->to_casefold();
    }

    ALWAYS_INLINE [[nodiscard]] String to_ascii_lowercase() const
    {
        return m_data->to_ascii_lowercase();
    }

    ALWAYS_INLINE [[nodiscard]] String to_ascii_uppercase() const
    {
        return m_data->to_ascii_uppercase();
    }

    // Compare this String against another string with caseless matching. Using this method requires linking LibUnicode into your application.
    ALWAYS_INLINE [[nodiscard]] bool equals_ignoring_case(String const& other) const
    {
        return m_data->equals_ignoring_case(other);
    }

    ALWAYS_INLINE [[nodiscard]] bool equals_ignoring_ascii_case(String const& other) const
    {
        return m_data->equals_ignoring_ascii_case(other);
    }

    ALWAYS_INLINE [[nodiscard]] bool equals_ignoring_ascii_case(StringView other) const
    {
        return m_data->equals_ignoring_case(other);
    }

    ALWAYS_INLINE [[nodiscard]] bool starts_with(u32 code_point) const
    {
        return m_data->starts_with(code_point);
    }

    ALWAYS_INLINE [[nodiscard]] bool starts_with_bytes(StringView bytes, CaseSensitivity case_sensitivity = CaseSensitivity::CaseSensitive) const
    {
        return m_data->starts_with_bytes(bytes, case_sensitivity);
    }

    ALWAYS_INLINE [[nodiscard]] bool ends_with(u32 code_point) const
    {
        return m_data->ends_with(code_point);
    }

    ALWAYS_INLINE [[nodiscard]] bool ends_with_bytes(StringView bytes, CaseSensitivity case_sensitivity = CaseSensitivity::CaseSensitive) const
    {
        return m_data->ends_with_bytes(bytes, case_sensitivity);
    }

    // Creates a substring with a deep copy of the specified data window.
    ALWAYS_INLINE ErrorOr<String> substring_from_byte_offset(size_t start, size_t byte_count) const
    {
        return m_data->substring_from_byte_offset(start, byte_count);
    }

    ALWAYS_INLINE ErrorOr<String> substring_from_byte_offset(size_t start) const
    {
        return m_data->substring_from_byte_offset(start);
    }

    // Creates a substring that strongly references the origin superstring instead of making a deep copy of the data.
    ALWAYS_INLINE ErrorOr<String> substring_from_byte_offset_with_shared_superstring(size_t start, size_t byte_count) const
    {
        return m_data->substring_from_byte_offset_with_shared_superstring(start, byte_count);
    }

    ALWAYS_INLINE ErrorOr<String> substring_from_byte_offset_with_shared_superstring(size_t start) const
    {
        return m_data->substring_from_byte_offset_with_shared_superstring(start);
    }

    // Returns an iterable view over the Unicode code points.
    ALWAYS_INLINE [[nodiscard]] Utf8View code_points() const&
    {
        return m_data->code_points();
    }

    [[nodiscard]] Utf8View code_points() const&& = delete;

    // Returns true if the String is zero-length.
    ALWAYS_INLINE [[nodiscard]] bool is_empty() const
    {
        return m_data->is_empty();
    }

    // Returns a StringView covering the full length of the string. Note that iterating this will go byte-at-a-time, not code-point-at-a-time.
    ALWAYS_INLINE [[nodiscard]] StringView bytes_as_string_view() const&
    {
        return m_data->bytes_as_string_view();
    }

    [[nodiscard]] StringView bytes_as_string_view() const&& = delete;

    ALWAYS_INLINE [[nodiscard]] size_t count(StringView needle) const
    {
        return StringUtils::count(m_data->bytes_as_string_view(), needle);
    }

    ALWAYS_INLINE ErrorOr<String> replace(StringView needle, StringView replacement, ReplaceMode replace_mode) const
    {
        return m_data->replace(needle, replacement, replace_mode);
    }

    ALWAYS_INLINE ErrorOr<String> reverse() const
    {
        return m_data->reverse();
    }

    ALWAYS_INLINE ErrorOr<String> trim(Utf8View const& code_points_to_trim, TrimMode mode = TrimMode::Both) const
    {
        return m_data->trim(code_points_to_trim, mode);
    }

    ALWAYS_INLINE ErrorOr<String> trim(StringView code_points_to_trim, TrimMode mode = TrimMode::Both) const
    {
        return m_data->trim(code_points_to_trim, mode);
    }

    ALWAYS_INLINE ErrorOr<String> trim_ascii_whitespace(TrimMode mode = TrimMode::Both) const
    {
        return m_data->trims_ascii_whitespace(mode);
    }

    ALWAYS_INLINE ErrorOr<Vector<String>> split_limit(u32 separator, size_t limit, SplitBehavior split_behavior = SplitBehavior::Nothing) const
    {
        return m_data->split_limit(seperator, limit, split_behavior);
    }

    ALWAYS_INLINE ErrorOr<Vector<String>> split(u32 separator, SplitBehavior split_behavior = SplitBehavior::Nothing) const
    {
        return m_data->split(seperator, split_behavior);
    }

    ALWAYS_INLINE Optional<size_t> find_byte_offset(u32 code_point, size_t from_byte_offset = 0) const
    {
        return m_data->find_byte_offset(code_point, from_byte_offset);
    }

    ALWAYS_INLINE Optional<size_t> find_byte_offset(StringView substring, size_t from_byte_offset = 0) const
    {
        return m_data->find_byte_offset(substring, from_byte_offset);
    }

    // Using this method requires linking LibUnicode into your application.
    ALWAYS_INLINE Optional<size_t> find_byte_offset_ignoring_case(StringView needle, size_t from_byte_offset = 0) const
    {
        return m_data->find_byte_offset(needle, from_byte_offset);
    }

    ALWAYS_INLINE [[nodiscard]] bool operator==(String const& other) const
    {
        return m_data->operator==(other);
    }

    ALWAYS_INLINE [[nodiscard]] bool operator==(FlyString const& other) const
    {
        return m_data->operator==(other);
    }
    
    ALWAYS_INLINE [[nodiscard]] bool operator==(StringView other) const
    {
        return m_data->operator==(other);
    }
    
    ALWAYS_INLINE [[nodiscard]] bool operator==(char const* cstring) const
    {
        return m_data->operator==(cstring);
    }
    
    // NOTE: UTF-8 is defined in a way that lexicographic ordering of code points is equivalent to lexicographic ordering of bytes.
    ALWAYS_INLINE [[nodiscard]] int operator<=>(String const& other) const {
        return m_data->operator<=>(other);
    }

    template<typename... Ts>
    [[nodiscard]] ALWAYS_INLINE constexpr bool is_one_of(Ts&&... strings) const
    {
        return (m_data->operator==(forward<Ts>(strings)) || ...);
    }
    
    ALWAYS_INLINE [[nodiscard]] bool contains(StringView needle, CaseSensitivity case_sensitivity = CaseSensitivity::CaseSensitive) const
    {
        return m_data->contains(needle, case_sensitivity);
    }
    ALWAYS_INLINE [[nodiscard]] bool contains(u32 needle, CaseSensitivity case_sensitivity = CaseSensitivity::CaseSensitive) const
    {
        return m_data->contains(needle, case_sensitivity);
    }

    ALWAYS_INLINE [[nodiscard]] u32 ascii_case_insensitive_hash() const
    {
        return m_data->ascii_case_insensitive_hash();
    }

    template<Arithmetic T>
    ALWAYS_INLINE Optional<T> to_number(TrimWhitespace trim_whitespace = TrimWhitespace::Yes) const
    {
        return m_data->to_number(trim_whitespace);
    }

    template<typename... Parameters>
    static ErrorOr<RefString> formatted(CheckedFormatString<Parameters...>&& fmtstr, Parameters const&... parameters)
    {
        return m_data->formatted(forward(fmtstr), parameters);
    }

    template<class SeparatorType, class CollectionType>
    static ErrorOr<RefString> join(SeparatorType const& separator, CollectionType const& collection, StringView fmtstr = "{}"sv)
    {
        return m_data->join(seperator, collection, fmtstr);
    }


    // BaseString Forwarding Functions:

    // Returns the underlying UTF-8 encoded bytes.
    // NOTE: There is no guarantee about null-termination.
    ALWAYS_INLINE [[nodiscard]] ReadonlyBytes bytes(Badge<StringView>) const
    {
        return m_data->bytes();
    }

    ALWAYS_INLINE [[nodiscard]] u32 hash() const
    {
        m_data->hash();
    }

    ALWAYS_INLINE[[nodiscard]] size_t byte_count() const
    {
        return m_data->byte_count();
    }

private:
    NonnullRefPtr<String const> m_data;
}

template<>
struct Traits<RefString> : public DefualtTraits<RefString> {
    ALWAYS_INLINE static unsigned hash(RefString const& ref_string)
    {
        return ref_string->hash();
    }
}

template<>
struct Formatter<RefString> : Formatter<StringView> {
    ErrorOr<void> format(FormatBuilder&, RefString const&);
};

struct ASCIICaseInsensitiveRefStringTraits : public Traits<RefString> {
    static unsigned hash(RefString const& s) { return s->ascii_case_insensitive_hash(); }
    static bool equals(RefString const& a, RefString const& b) { return a->bytes_as_string_view().equals_ignoring_ascii_case(b->bytes_as_string_view()); }
};

}

[[nodiscard]] ALWAYS_INLINE AK::RefString operator""_ref_string(char const* cstring, size_t length)
{
    return AK::String::from_utf8(AK::StringView(cstring, length)).release_value();
}
