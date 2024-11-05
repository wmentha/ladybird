/*
 * Copyright (c) 2020-2023, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2021, Linus Groh <linusg@serenityos.org>
 * Copyright (c) 2021, Luke Wilde <lukew@serenityos.org>
 * Copyright (c) 2022, Ali Mohammad Pur <mpfard@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/CharacterTypes.h>
#include <AK/GenericLexer.h>
#include <LibIDL/Types.h>

namespace IDL {

class Parser {
public:
    Parser(String filename, StringView contents, String import_base_path);
    Interface& parse();

    Vector<String> imported_files() const;

private:
    // https://webidl.spec.whatwg.org/#dfn-special-operation
    // A special operation is a getter, setter or deleter.
    enum class IsSpecialOperation {
        No,
        Yes,
    };

    enum class IsStatic {
        No,
        Yes,
    };

    Parser(Parser* parent, String filename, StringView contents, String import_base_path);

    void assert_specific(char ch);
    void assert_string(StringView expected);
    void consume_whitespace();
    Optional<Interface&> resolve_import(auto path);

    HashMap<String, String> parse_extended_attributes();
    void parse_attribute(HashMap<String, String>& extended_attributes, Interface&, IsStatic is_static = IsStatic::No);
    void parse_interface(Interface&);
    void parse_namespace(Interface&);
    void parse_non_interface_entities(bool allow_interface, Interface&);
    void parse_enumeration(HashMap<String, String>, Interface&);
    void parse_typedef(Interface&);
    void parse_interface_mixin(Interface&);
    void parse_dictionary(Interface&);
    void parse_callback_function(HashMap<String, String>& extended_attributes, Interface&);
    void parse_constructor(HashMap<String, String>& extended_attributes, Interface&);
    void parse_getter(HashMap<String, String>& extended_attributes, Interface&);
    void parse_setter(HashMap<String, String>& extended_attributes, Interface&);
    void parse_deleter(HashMap<String, String>& extended_attributes, Interface&);
    void parse_stringifier(HashMap<String, String>& extended_attributes, Interface&);
    void parse_iterable(Interface&);
    void parse_setlike(Interface&, bool is_readonly);
    Function parse_function(HashMap<String, String>& extended_attributes, Interface&, IsStatic is_static = IsStatic::No, IsSpecialOperation is_special_operation = IsSpecialOperation::No);
    Vector<Parameter> parse_parameters();
    NonnullRefPtr<Type const> parse_type();
    void parse_constant(Interface&);
    String parse_identifier_until(AK::Function<bool(char)> predicate);
    String parse_identifier_ending_with(auto... possible_terminating_characters);
    String parse_identifier_ending_with_space();
    String parse_identifier_ending_with_space_or(auto... possible_terminating_characters);

    String import_base_path;
    String filename;
    StringView input;
    LineTrackingLexer lexer;

    HashTable<NonnullOwnPtr<Interface>>& top_level_interfaces();
    HashTable<NonnullOwnPtr<Interface>> interfaces;
    HashMap<String, Interface*>& top_level_resolved_imports();
    HashMap<String, Interface*> resolved_imports;
    Parser* top_level_parser();
    Parser* parent = nullptr;
};

}
