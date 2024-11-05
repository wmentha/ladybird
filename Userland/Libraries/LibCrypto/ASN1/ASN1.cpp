/*
 * Copyright (c) 2021, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/GenericLexer.h>
#include <LibCrypto/ASN1/ASN1.h>

namespace Crypto::ASN1 {

String kind_name(Kind kind)
{
    switch (kind) {
    case Kind::Eol:
        return "EndOfList"_string;
    case Kind::Boolean:
        return "Boolean"_string;
    case Kind::Integer:
        return "Integer"_string;
    case Kind::BitString:
        return "BitString"_string;
    case Kind::OctetString:
        return "OctetString"_string;
    case Kind::Null:
        return "Null"_string;
    case Kind::ObjectIdentifier:
        return "ObjectIdentifier"_string;
    case Kind::ObjectDescriptor:
        return "ObjectDescriptor"_string;
    case Kind::External:
        return "External"_string;
    case Kind::Real:
        return "Real"_string;
    case Kind::Enumerated:
        return "Enumerated"_string;
    case Kind::EmbeddedPdv:
        return "EmbeddedPdv"_string;
    case Kind::Utf8String:
        return "Utf8String"_string;
    case Kind::RelativeOid:
        return "RelativeOid"_string;
    case Kind::Time:
        return "Time"_string;
    case Kind::Reserved:
        return "Reserved"_string;
    case Kind::Sequence:
        return "Sequence"_string;
    case Kind::Set:
        return "Set"_string;
    case Kind::NumericString:
        return "NumericString"_string;
    case Kind::PrintableString:
        return "PrintableString"_string;
    case Kind::T61String:
        return "T61String"_string;
    case Kind::VideotexString:
        return "VideotexString"_string;
    case Kind::IA5String:
        return "IA5String"_string;
    case Kind::UTCTime:
        return "UTCTime"_string;
    case Kind::GeneralizedTime:
        return "GeneralizedTime"_string;
    case Kind::GraphicString:
        return "GraphicString"_string;
    case Kind::VisibleString:
        return "VisibleString"_string;
    case Kind::GeneralString:
        return "GeneralString"_string;
    case Kind::UniversalString:
        return "UniversalString"_string;
    case Kind::CharacterString:
        return "CharacterString"_string;
    case Kind::BMPString:
        return "BMPString"_string;
    case Kind::Date:
        return "Date"_string;
    case Kind::TimeOfDay:
        return "TimeOfDay"_string;
    case Kind::DateTime:
        return "DateTime"_string;
    case Kind::Duration:
        return "Duration"_string;
    case Kind::OidIri:
        return "OidIri"_string;
    case Kind::RelativeOidIri:
        return "RelativeOidIri"_string;
    }

    return "InvalidKind"_string;
}

String class_name(Class class_)
{
    switch (class_) {
    case Class::Application:
        return "Application"_string;
    case Class::Context:
        return "Context"_string;
    case Class::Private:
        return "Private"_string;
    case Class::Universal:
        return "Universal"_string;
    }

    return "InvalidClass"_string;
}

String type_name(Type type)
{
    switch (type) {
    case Type::Constructed:
        return "Constructed"_string;
    case Type::Primitive:
        return "Primitive"_string;
    }

    return "InvalidType"_string;
}

Optional<UnixDateTime> parse_utc_time(StringView time)
{
    // YYMMDDhhmm[ss]Z or YYMMDDhhmm[ss](+|-)hhmm
    GenericLexer lexer(time);
    auto year_in_century = lexer.consume(2).to_number<unsigned>();
    auto month = lexer.consume(2).to_number<unsigned>();
    auto day = lexer.consume(2).to_number<unsigned>();
    auto hour = lexer.consume(2).to_number<unsigned>();
    auto minute = lexer.consume(2).to_number<unsigned>();
    Optional<unsigned> seconds, offset_hours, offset_minutes;
    [[maybe_unused]] bool negative_offset = false;

    if (lexer.next_is(is_any_of("0123456789"sv))) {
        seconds = lexer.consume(2).to_number<unsigned>();
        if (!seconds.has_value()) {
            return {};
        }
    }

    if (lexer.next_is('Z')) {
        lexer.consume();
    } else if (lexer.next_is(is_any_of("+-"sv))) {
        negative_offset = lexer.consume() == '-';
        offset_hours = lexer.consume(2).to_number<unsigned>();
        offset_minutes = lexer.consume(2).to_number<unsigned>();
        if (!offset_hours.has_value() || !offset_minutes.has_value()) {
            return {};
        }
    } else {
        return {};
    }

    if (!year_in_century.has_value() || !month.has_value() || !day.has_value() || !hour.has_value() || !minute.has_value()) {
        return {};
    }

    // RFC5280 section 4.1.2.5.1.
    auto full_year = year_in_century.value();
    full_year += (full_year < 50) ? 2000 : 1900;
    auto full_seconds = seconds.value_or(0);

    // FIXME: Handle offsets!
    if (offset_hours.has_value() || offset_minutes.has_value())
        dbgln("FIXME: Implement UTCTime with offset!");

    return UnixDateTime::from_unix_time_parts(full_year, month.value(), day.value(), hour.value(), minute.value(), full_seconds, 0);
}

Optional<UnixDateTime> parse_generalized_time(StringView time)
{
    // YYYYMMDDhh[mm[ss[.fff]]] or YYYYMMDDhh[mm[ss[.fff]]]Z or YYYYMMDDhh[mm[ss[.fff]]](+|-)hhmm
    GenericLexer lexer(time);
    auto year = lexer.consume(4).to_number<unsigned>();
    auto month = lexer.consume(2).to_number<unsigned>();
    auto day = lexer.consume(2).to_number<unsigned>();
    auto hour = lexer.consume(2).to_number<unsigned>();
    Optional<unsigned> minute, seconds, milliseconds, offset_hours, offset_minutes;
    [[maybe_unused]] bool negative_offset = false;

    if (!lexer.is_eof()) {
        if (lexer.consume_specific('Z'))
            goto done_parsing;

        if (!lexer.next_is(is_any_of("+-"sv))) {
            minute = lexer.consume(2).to_number<unsigned>();
            if (!minute.has_value()) {
                return {};
            }
            if (lexer.is_eof() || lexer.consume_specific('Z'))
                goto done_parsing;
        }

        if (!lexer.next_is(is_any_of("+-"sv))) {
            seconds = lexer.consume(2).to_number<unsigned>();
            if (!seconds.has_value()) {
                return {};
            }
            if (lexer.is_eof() || lexer.consume_specific('Z'))
                goto done_parsing;
        }

        if (lexer.consume_specific('.')) {
            milliseconds = lexer.consume(3).to_number<unsigned>();
            if (!milliseconds.has_value()) {
                return {};
            }
            if (lexer.is_eof() || lexer.consume_specific('Z'))
                goto done_parsing;
        }

        if (lexer.next_is(is_any_of("+-"sv))) {
            negative_offset = lexer.consume() == '-';
            offset_hours = lexer.consume(2).to_number<unsigned>();
            offset_minutes = lexer.consume(2).to_number<unsigned>();
            if (!offset_hours.has_value() || !offset_minutes.has_value()) {
                return {};
            }
        }

        // Any character would be garbage.
        if (!lexer.is_eof()) {
            return {};
        }
    }

done_parsing:;

    if (!year.has_value() || !month.has_value() || !day.has_value() || !hour.has_value()) {
        return {};
    }

    // FIXME: Handle offsets!
    if (offset_hours.has_value() || offset_minutes.has_value())
        dbgln("FIXME: Implement GeneralizedTime with offset!");

    return UnixDateTime::from_unix_time_parts(year.value(), month.value(), day.value(), hour.value(), minute.value_or(0), seconds.value_or(0), milliseconds.value_or(0));
}
}
