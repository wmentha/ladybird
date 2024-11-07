/*
 * Copyright (c) 2021, Ali Mohammad Pur <mpfard@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/HashMap.h>
#include <AK/TemporaryChange.h>
#include <LibWasm/AbstractMachine/AbstractMachine.h>
#include <LibWasm/Printer/Printer.h>

namespace Wasm {

struct Names {
    static HashMap<OpCode, String> instruction_names;
    static HashMap<String, OpCode> instructions_by_name;
};

String instruction_name(OpCode const& opcode)
{
    return Names::instruction_names.get(opcode).value_or("<unknown>");
}

Optional<OpCode> instruction_from_name(StringView name)
{
    if (Names::instructions_by_name.is_empty()) {
        for (auto& entry : Names::instruction_names)
            Names::instructions_by_name.set(entry.value, entry.key);
    }

    return Names::instructions_by_name.get(name);
}

void Printer::print_indent()
{
    for (size_t i = 0; i < m_indent; ++i)
        m_stream.write_until_depleted("  "sv.bytes()).release_value_but_fixme_should_propagate_errors();
}

void Printer::print(Wasm::BlockType const& type)
{
    print_indent();
    print("(type block ");
    switch (type.kind()) {
    case Wasm::BlockType::Kind::Index:
        print("index {})\n", type.type_index().value());
        return;
    case Wasm::BlockType::Kind::Type: {
        print("type\n");
        {
            TemporaryChange change { m_indent, m_indent + 1 };
            print(type.value_type());
        }
        print_indent();
        print(")\n");
        return;
    }
    case Wasm::BlockType::Kind ::Empty:
        print("empty)\n");
        return;
    }
    VERIFY_NOT_REACHED();
}

void Printer::print(Wasm::CodeSection const& section)
{
    if (section.functions().is_empty())
        return;
    print_indent();
    print("(section code\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& code : section.functions())
            print(code);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::CodeSection::Code const& code)
{
    print(code.func());
}

void Printer::print(Wasm::CustomSection const& section)
{
    print_indent();
    print("(section custom\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print_indent();
        print("(name `{}')\n", section.name());
        print_indent();
        print("(contents {} bytes)\n", section.contents().size());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::DataCountSection const& section)
{
    if (!section.count().has_value())
        return;
    print_indent();
    print("(section data count\n");
    if (section.count().has_value()) {
        TemporaryChange change { m_indent, m_indent + 1 };
        print_indent();
        print("(count `{}')\n", *section.count());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::DataSection const& section)
{
    if (section.data().is_empty())
        return;
    print_indent();
    print("(section data\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& entry : section.data())
            print(entry);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::DataSection::Data const& data)
{
    print_indent();
    print("(data with value\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        data.value().visit(
            [this](DataSection::Data::Passive const& value) {
                print_indent();
                print("(passive init {}xu8 (", value.init.size());
                print(MUST(String::join(' ', value.init, "{:x}"sv)));
                print(")\n");
            },
            [this](DataSection::Data::Active const& value) {
                print_indent();
                print("(active init {}xu8 (", value.init.size());
                print(MUST(String::join(' ', value.init, "{:x}"sv)));
                print("\n");
                {
                    TemporaryChange change { m_indent, m_indent + 1 };
                    print_indent();
                    print("(offset\n");
                    {
                        TemporaryChange change { m_indent, m_indent + 1 };
                        print(value.offset);
                    }
                    print_indent();
                    print(")\n");
                }
                {
                    TemporaryChange change { m_indent, m_indent + 1 };
                    print_indent();
                    print("(index {})\n", value.index.value());
                }
            });
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::ElementSection const& section)
{
    if (section.segments().is_empty())
        return;
    print_indent();
    print("(section element\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& entry : section.segments())
            print(entry);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::ElementSection::Element const& element)
{
    print_indent();
    print("(element ");
    {
        TemporaryChange<size_t> change { m_indent, 0 };
        print(element.type);
    }
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print_indent();
        print("(init\n");
        {
            TemporaryChange change { m_indent, m_indent + 1 };
            for (auto& entry : element.init)
                print(entry);
        }
        print_indent();
        print(")\n");
        print_indent();
        print("(mode ");
        element.mode.visit(
            [this](ElementSection::Active const& active) {
                print("\n");
                {
                    TemporaryChange change { m_indent, m_indent + 1 };
                    print_indent();
                    print("(active index {}\n", active.index.value());
                    {
                        print(active.expression);
                    }
                    print_indent();
                    print(")\n");
                }
                print_indent();
            },
            [this](ElementSection::Passive const&) { print("passive"); },
            [this](ElementSection::Declarative const&) { print("declarative"); });
        print(")\n");
    }
}

void Printer::print(Wasm::ExportSection const& section)
{
    if (section.entries().is_empty())
        return;
    print_indent();
    print("(section export\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& entry : section.entries())
            print(entry);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::ExportSection::Export const& entry)
{
    print_indent();
    print("(export `{}' as\n", entry.name());
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print_indent();
        entry.description().visit(
            [this](FunctionIndex const& index) { print("(function index {})\n", index.value()); },
            [this](TableIndex const& index) { print("(table index {})\n", index.value()); },
            [this](MemoryIndex const& index) { print("(memory index {})\n", index.value()); },
            [this](GlobalIndex const& index) { print("(global index {})\n", index.value()); });
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::Expression const& expression)
{
    TemporaryChange change { m_indent, m_indent + 1 };
    for (auto& instr : expression.instructions())
        print(instr);
}

void Printer::print(Wasm::CodeSection::Func const& func)
{
    print_indent();
    print("(function\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        {
            print_indent();
            print("(locals\n");
            {
                TemporaryChange change { m_indent, m_indent + 1 };
                for (auto& locals : func.locals())
                    print(locals);
            }
            print_indent();
            print(")\n");
        }
        print_indent();
        print("(body\n");
        print(func.body());
        print_indent();
        print(")\n");
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::FunctionSection const& section)
{
    if (section.types().is_empty())
        return;
    print_indent();
    print("(section function\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& index : section.types()) {
            print_indent();
            print("(type index {})\n", index.value());
        }
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::FunctionType const& type)
{
    print_indent();
    print("(type function\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print_indent();
        print("(parameters\n");
        {
            TemporaryChange change { m_indent, m_indent + 1 };
            for (auto& param : type.parameters())
                print(param);
        }
        print_indent();
        print(")\n");
    }
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print_indent();
        print("(results\n");
        {
            TemporaryChange change { m_indent, m_indent + 1 };
            for (auto& type : type.results())
                print(type);
        }
        print_indent();
        print(")\n");
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::GlobalSection const& section)
{
    if (section.entries().is_empty())
        return;
    print_indent();
    print("(section global\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& entry : section.entries())
            print(entry);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::GlobalSection::Global const& entry)
{
    print_indent();
    print("(global\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print_indent();
        print("(type\n");
        {
            TemporaryChange change { m_indent, m_indent + 1 };
            print(entry.type());
        }
        print_indent();
        print(")\n");
    }
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print_indent();
        print("(init\n");
        {
            TemporaryChange change { m_indent, m_indent + 1 };
            print(entry.expression());
        }
        print_indent();
        print(")\n");
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::GlobalType const& type)
{
    print_indent();
    print("(type global {}mutable\n", type.is_mutable() ? "" : "im");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print(type.type());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::ImportSection const& section)
{
    if (section.imports().is_empty())
        return;
    print_indent();
    print("(section import\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& import : section.imports())
            print(import);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::ImportSection::Import const& import)
{
    print_indent();
    print("(import `{}' from `{}' as\n", import.name(), import.module());
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        import.description().visit(
            [this](auto const& type) {
                print(type);
            },
            [this](TypeIndex const& index) {
                print_indent();
                print("(type index {})\n", index.value());
            });
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::Instruction const& instruction)
{
    print_indent();
    print("({}", instruction_name(instruction.opcode()));
    if (instruction.arguments().has<u8>()) {
        print(")\n");
    } else {
        print(" ");
        instruction.arguments().visit(
            [&](BlockType const& type) { print(type); },
            [&](DataIndex const& index) { print("(data index {})", index.value()); },
            [&](ElementIndex const& index) { print("(element index {})", index.value()); },
            [&](FunctionIndex const& index) { print("(function index {})", index.value()); },
            [&](GlobalIndex const& index) { print("(global index {})", index.value()); },
            [&](LabelIndex const& index) { print("(label index {})", index.value()); },
            [&](LocalIndex const& index) { print("(local index {})", index.value()); },
            [&](TableIndex const& index) { print("(table index {})", index.value()); },
            [&](Instruction::IndirectCallArgs const& args) { print("(indirect (type index {}) (table index {}))", args.type.value(), args.table.value()); },
            [&](Instruction::MemoryArgument const& args) { print("(memory index {} (align {}) (offset {}))", args.memory_index.value(), args.align, args.offset); },
            [&](Instruction::MemoryAndLaneArgument const& args) { print("(memory index {} (align {}) (offset {})) (lane {})", args.memory.memory_index.value(), args.memory.align, args.memory.offset, args.lane); },
            [&](Instruction::MemoryInitArgs const& args) { print("(memory index {}) (data index {})", args.memory_index.value(), args.data_index.value()); },
            [&](Instruction::MemoryCopyArgs const& args) { print("(from (memory index {}) to (memory index {}))", args.src_index.value(), args.dst_index.value()); },
            [&](Instruction::MemoryIndexArgument const& args) { print("(memory index {})", args.memory_index.value()); },
            [&](Instruction::LaneIndex const& args) { print("(lane {})", args.lane); },
            [&](Instruction::ShuffleArgument const& args) {
                print("{{ {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} }}",
                    args.lanes[0], args.lanes[1], args.lanes[2], args.lanes[3],
                    args.lanes[4], args.lanes[5], args.lanes[6], args.lanes[7],
                    args.lanes[8], args.lanes[9], args.lanes[10], args.lanes[11],
                    args.lanes[12], args.lanes[13], args.lanes[14], args.lanes[15]);
            },
            [&](Instruction::StructuredInstructionArgs const& args) {
                print("(structured\n");
                TemporaryChange change { m_indent, m_indent + 1 };
                print(args.block_type);
                print_indent();
                print("(else {}) (end {}))", args.else_ip.has_value() ? String::number(args.else_ip->value()) : "(none)", args.end_ip.value());
            },
            [&](Instruction::TableBranchArgs const& args) {
                print("(table_branch");
                for (auto& label : args.labels)
                    print(" (label {})", label.value());
                print(" (label {}))", args.default_.value());
            },
            [&](Instruction::TableElementArgs const& args) { print("(table_element (table index {}) (element index {}))", args.table_index.value(), args.element_index.value()); },
            [&](Instruction::TableTableArgs const& args) { print("(table_table (table index {}) (table index {}))", args.lhs.value(), args.rhs.value()); },
            [&](ValueType const& type) { print(type); },
            [&](Vector<ValueType> const&) { print("(types...)"); },
            [&](auto const& value) { print("{}", value); });

        print(")\n");
    }
}

void Printer::print(Wasm::Limits const& limits)
{
    print_indent();
    print("(limits min={}", limits.min());
    if (limits.max().has_value())
        print(" max={}", limits.max().value());
    else
        print(" unbounded");
    print(")\n");
}

void Printer::print(Wasm::Locals const& local)
{
    print_indent();
    print("(local x{} of type\n", local.n());
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print(local.type());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::MemorySection const& section)
{
    if (section.memories().is_empty())
        return;
    print_indent();
    print("(section memory\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& memory : section.memories())
            print(memory);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::MemorySection::Memory const& memory)
{
    print_indent();
    print("(memory\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print(memory.type());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::MemoryType const& type)
{
    print_indent();
    print("(type memory\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print(type.limits());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::Module const& module)
{
    print_indent();
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print("(module\n");
        for (auto& custom_section : module.custom_sections())
            print(custom_section);
        print(module.type_section());
        print(module.import_section());
        print(module.function_section());
        print(module.table_section());
        print(module.memory_section());
        print(module.global_section());
        print(module.export_section());
        print(module.start_section());
        print(module.element_section());
        print(module.code_section());
        print(module.data_section());
        print(module.data_count_section());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::StartSection const& section)
{
    if (!section.function().has_value())
        return;
    print_indent();
    print("(section start\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print(*section.function());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::StartSection::StartFunction const& function)
{
    print_indent();
    print("(start function index {})\n", function.index().value());
}

void Printer::print(Wasm::TableSection const& section)
{
    if (section.tables().is_empty())
        return;
    print_indent();
    print("(section table\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& table : section.tables())
            print(table);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::TableSection::Table const& table)
{
    print_indent();
    print("(table\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print(table.type());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::TableType const& type)
{
    print_indent();
    print("(type table min:{}", type.limits().min());
    if (type.limits().max().has_value())
        print(" max:{}", type.limits().max().value());
    print("\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        print(type.element_type());
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::TypeSection const& section)
{
    if (section.types().is_empty())
        return;
    print_indent();
    print("(section type\n");
    {
        TemporaryChange change { m_indent, m_indent + 1 };
        for (auto& type : section.types())
            print(type);
    }
    print_indent();
    print(")\n");
}

void Printer::print(Wasm::ValueType const& type)
{
    print_indent();
    print("(type {})\n", ValueType::kind_name(type.kind()));
}

void Printer::print(Wasm::Value const& value, Wasm::ValueType const& type)
{
    print_indent();
    switch (type.kind()) {
    case ValueType::I32:
        print(MUST(String::formatted("{}", value.to<i32>())));
        break;
    case ValueType::I64:
        print(MUST(String::formatted("{}", value.to<i64>())));
        break;
    case ValueType::F32:
        print(MUST(String::formatted("{}", value.to<f32>())));
        break;
    case ValueType::F64:
        print(MUST(String::formatted("{}", value.to<f64>())));
        break;
    case ValueType::V128:
        print(MUST(String::formatted("v128({:x})", value.value())));
        break;
    case ValueType::FunctionReference:
    case ValueType::ExternReference:
        print(MUST(String::formatted("addr({})",
            value.to<Reference>().ref().visit(
                [](Wasm::Reference::Null const&) { return "null"_string; },
                [](auto const& ref) { return String::number(ref.address.value()); }))));
        break;
    }
    TemporaryChange<size_t> change { m_indent, 0 };
}

void Printer::print(Wasm::Value const& value)
{
    print_indent();
    print("{:x}", value.value());
    TemporaryChange<size_t> change { m_indent, 0 };
}

void Printer::print(Wasm::Reference const& value)
{
    print_indent();
    print(
        "addr({})\n",
        value.ref().visit(
            [](Wasm::Reference::Null const&) { return "null"_string; },
            [](auto const& ref) { return String::number(ref.address.value()); }));
}
}

HashMap<Wasm::OpCode, String> Wasm::Names::instruction_names {
    { Instructions::unreachable, "unreachable"_string },
    { Instructions::nop, "nop"_string },
    { Instructions::block, "block"_string },
    { Instructions::loop, "loop"_string },
    { Instructions::if_, "if"_string },
    { Instructions::br, "br"_string },
    { Instructions::br_if, "br.if"_string },
    { Instructions::br_table, "br.table"_string },
    { Instructions::return_, "return"_string },
    { Instructions::call, "call"_string },
    { Instructions::call_indirect, "call.indirect"_string },
    { Instructions::drop, "drop"_string },
    { Instructions::select, "select"_string },
    { Instructions::select_typed, "select.typed"_string },
    { Instructions::local_get, "local.get"_string },
    { Instructions::local_set, "local.set"_string },
    { Instructions::local_tee, "local.tee"_string },
    { Instructions::global_get, "global.get"_string },
    { Instructions::global_set, "global.set"_string },
    { Instructions::table_get, "table.get"_string },
    { Instructions::table_set, "table.set"_string },
    { Instructions::i32_load, "i32.load"_string },
    { Instructions::i64_load, "i64.load"_string },
    { Instructions::f32_load, "f32.load"_string },
    { Instructions::f64_load, "f64.load"_string },
    { Instructions::i32_load8_s, "i32.load8_s"_string },
    { Instructions::i32_load8_u, "i32.load8_u"_string },
    { Instructions::i32_load16_s, "i32.load16_s"_string },
    { Instructions::i32_load16_u, "i32.load16_u"_string },
    { Instructions::i64_load8_s, "i64.load8_s"_string },
    { Instructions::i64_load8_u, "i64.load8_u"_string },
    { Instructions::i64_load16_s, "i64.load16_s"_string },
    { Instructions::i64_load16_u, "i64.load16_u"_string },
    { Instructions::i64_load32_s, "i64.load32_s"_string },
    { Instructions::i64_load32_u, "i64.load32_u"_string },
    { Instructions::i32_store, "i32.store"_string },
    { Instructions::i64_store, "i64.store"_string },
    { Instructions::f32_store, "f32.store"_string },
    { Instructions::f64_store, "f64.store"_string },
    { Instructions::i32_store8, "i32.store8"_string },
    { Instructions::i32_store16, "i32.store16"_string },
    { Instructions::i64_store8, "i64.store8"_string },
    { Instructions::i64_store16, "i64.store16"_string },
    { Instructions::i64_store32, "i64.store32"_string },
    { Instructions::memory_size, "memory.size"_string },
    { Instructions::memory_grow, "memory.grow"_string },
    { Instructions::i32_const, "i32.const"_string },
    { Instructions::i64_const, "i64.const"_string },
    { Instructions::f32_const, "f32.const"_string },
    { Instructions::f64_const, "f64.const"_string },
    { Instructions::i32_eqz, "i32.eqz"_string },
    { Instructions::i32_eq, "i32.eq"_string },
    { Instructions::i32_ne, "i32.ne"_string },
    { Instructions::i32_lts, "i32.lts"_string },
    { Instructions::i32_ltu, "i32.ltu"_string },
    { Instructions::i32_gts, "i32.gts"_string },
    { Instructions::i32_gtu, "i32.gtu"_string },
    { Instructions::i32_les, "i32.les"_string },
    { Instructions::i32_leu, "i32.leu"_string },
    { Instructions::i32_ges, "i32.ges"_string },
    { Instructions::i32_geu, "i32.geu"_string },
    { Instructions::i64_eqz, "i64.eqz"_string },
    { Instructions::i64_eq, "i64.eq"_string },
    { Instructions::i64_ne, "i64.ne"_string },
    { Instructions::i64_lts, "i64.lts"_string },
    { Instructions::i64_ltu, "i64.ltu"_string },
    { Instructions::i64_gts, "i64.gts"_string },
    { Instructions::i64_gtu, "i64.gtu"_string },
    { Instructions::i64_les, "i64.les"_string },
    { Instructions::i64_leu, "i64.leu"_string },
    { Instructions::i64_ges, "i64.ges"_string },
    { Instructions::i64_geu, "i64.geu"_string },
    { Instructions::f32_eq, "f32.eq"_string },
    { Instructions::f32_ne, "f32.ne"_string },
    { Instructions::f32_lt, "f32.lt"_string },
    { Instructions::f32_gt, "f32.gt"_string },
    { Instructions::f32_le, "f32.le"_string },
    { Instructions::f32_ge, "f32.ge"_string },
    { Instructions::f64_eq, "f64.eq"_string },
    { Instructions::f64_ne, "f64.ne"_string },
    { Instructions::f64_lt, "f64.lt"_string },
    { Instructions::f64_gt, "f64.gt"_string },
    { Instructions::f64_le, "f64.le"_string },
    { Instructions::f64_ge, "f64.ge"_string },
    { Instructions::i32_clz, "i32.clz"_string },
    { Instructions::i32_ctz, "i32.ctz"_string },
    { Instructions::i32_popcnt, "i32.popcnt"_string },
    { Instructions::i32_add, "i32.add"_string },
    { Instructions::i32_sub, "i32.sub"_string },
    { Instructions::i32_mul, "i32.mul"_string },
    { Instructions::i32_divs, "i32.divs"_string },
    { Instructions::i32_divu, "i32.divu"_string },
    { Instructions::i32_rems, "i32.rems"_string },
    { Instructions::i32_remu, "i32.remu"_string },
    { Instructions::i32_and, "i32.and"_string },
    { Instructions::i32_or, "i32.or"_string },
    { Instructions::i32_xor, "i32.xor"_string },
    { Instructions::i32_shl, "i32.shl"_string },
    { Instructions::i32_shrs, "i32.shrs"_string },
    { Instructions::i32_shru, "i32.shru"_string },
    { Instructions::i32_rotl, "i32.rotl"_string },
    { Instructions::i32_rotr, "i32.rotr"_string },
    { Instructions::i64_clz, "i64.clz"_string },
    { Instructions::i64_ctz, "i64.ctz"_string },
    { Instructions::i64_popcnt, "i64.popcnt"_string },
    { Instructions::i64_add, "i64.add"_string },
    { Instructions::i64_sub, "i64.sub"_string },
    { Instructions::i64_mul, "i64.mul"_string },
    { Instructions::i64_divs, "i64.divs"_string },
    { Instructions::i64_divu, "i64.divu"_string },
    { Instructions::i64_rems, "i64.rems"_string },
    { Instructions::i64_remu, "i64.remu"_string },
    { Instructions::i64_and, "i64.and"_string },
    { Instructions::i64_or, "i64.or"_string },
    { Instructions::i64_xor, "i64.xor"_string },
    { Instructions::i64_shl, "i64.shl"_string },
    { Instructions::i64_shrs, "i64.shrs"_string },
    { Instructions::i64_shru, "i64.shru"_string },
    { Instructions::i64_rotl, "i64.rotl"_string },
    { Instructions::i64_rotr, "i64.rotr"_string },
    { Instructions::f32_abs, "f32.abs"_string },
    { Instructions::f32_neg, "f32.neg"_string },
    { Instructions::f32_ceil, "f32.ceil"_string },
    { Instructions::f32_floor, "f32.floor"_string },
    { Instructions::f32_trunc, "f32.trunc"_string },
    { Instructions::f32_nearest, "f32.nearest"_string },
    { Instructions::f32_sqrt, "f32.sqrt"_string },
    { Instructions::f32_add, "f32.add"_string },
    { Instructions::f32_sub, "f32.sub"_string },
    { Instructions::f32_mul, "f32.mul"_string },
    { Instructions::f32_div, "f32.div"_string },
    { Instructions::f32_min, "f32.min"_string },
    { Instructions::f32_max, "f32.max"_string },
    { Instructions::f32_copysign, "f32.copysign"_string },
    { Instructions::f64_abs, "f64.abs"_string },
    { Instructions::f64_neg, "f64.neg"_string },
    { Instructions::f64_ceil, "f64.ceil"_string },
    { Instructions::f64_floor, "f64.floor"_string },
    { Instructions::f64_trunc, "f64.trunc"_string },
    { Instructions::f64_nearest, "f64.nearest"_string },
    { Instructions::f64_sqrt, "f64.sqrt"_string },
    { Instructions::f64_add, "f64.add"_string },
    { Instructions::f64_sub, "f64.sub"_string },
    { Instructions::f64_mul, "f64.mul"_string },
    { Instructions::f64_div, "f64.div"_string },
    { Instructions::f64_min, "f64.min"_string },
    { Instructions::f64_max, "f64.max"_string },
    { Instructions::f64_copysign, "f64.copysign"_string },
    { Instructions::i32_wrap_i64, "i32.wrap_i64"_string },
    { Instructions::i32_trunc_sf32, "i32.trunc_sf32"_string },
    { Instructions::i32_trunc_uf32, "i32.trunc_uf32"_string },
    { Instructions::i32_trunc_sf64, "i32.trunc_sf64"_string },
    { Instructions::i32_trunc_uf64, "i32.trunc_uf64"_string },
    { Instructions::i64_extend_si32, "i64.extend_si32"_string },
    { Instructions::i64_extend_ui32, "i64.extend_ui32"_string },
    { Instructions::i64_trunc_sf32, "i64.trunc_sf32"_string },
    { Instructions::i64_trunc_uf32, "i64.trunc_uf32"_string },
    { Instructions::i64_trunc_sf64, "i64.trunc_sf64"_string },
    { Instructions::i64_trunc_uf64, "i64.trunc_uf64"_string },
    { Instructions::f32_convert_si32, "f32.convert_si32"_string },
    { Instructions::f32_convert_ui32, "f32.convert_ui32"_string },
    { Instructions::f32_convert_si64, "f32.convert_si64"_string },
    { Instructions::f32_convert_ui64, "f32.convert_ui64"_string },
    { Instructions::f32_demote_f64, "f32.demote_f64"_string },
    { Instructions::f64_convert_si32, "f64.convert_si32"_string },
    { Instructions::f64_convert_ui32, "f64.convert_ui32"_string },
    { Instructions::f64_convert_si64, "f64.convert_si64"_string },
    { Instructions::f64_convert_ui64, "f64.convert_ui64"_string },
    { Instructions::f64_promote_f32, "f64.promote_f32"_string },
    { Instructions::i32_reinterpret_f32, "i32.reinterpret_f32"_string },
    { Instructions::i64_reinterpret_f64, "i64.reinterpret_f64"_string },
    { Instructions::f32_reinterpret_i32, "f32.reinterpret_i32"_string },
    { Instructions::f64_reinterpret_i64, "f64.reinterpret_i64"_string },
    { Instructions::i32_extend8_s, "i32.extend8_s"_string },
    { Instructions::i32_extend16_s, "i32.extend16_s"_string },
    { Instructions::i64_extend8_s, "i64.extend8_s"_string },
    { Instructions::i64_extend16_s, "i64.extend16_s"_string },
    { Instructions::i64_extend32_s, "i64.extend32_s"_string },
    { Instructions::ref_null, "ref.null"_string },
    { Instructions::ref_is_null, "ref.is.null"_string },
    { Instructions::ref_func, "ref.func"_string },
    { Instructions::i32_trunc_sat_f32_s, "i32.trunc_sat_f32_s"_string },
    { Instructions::i32_trunc_sat_f32_u, "i32.trunc_sat_f32_u"_string },
    { Instructions::i32_trunc_sat_f64_s, "i32.trunc_sat_f64_s"_string },
    { Instructions::i32_trunc_sat_f64_u, "i32.trunc_sat_f64_u"_string },
    { Instructions::i64_trunc_sat_f32_s, "i64.trunc_sat_f32_s"_string },
    { Instructions::i64_trunc_sat_f32_u, "i64.trunc_sat_f32_u"_string },
    { Instructions::i64_trunc_sat_f64_s, "i64.trunc_sat_f64_s"_string },
    { Instructions::i64_trunc_sat_f64_u, "i64.trunc_sat_f64_u"_string },
    { Instructions::memory_init, "memory.init"_string },
    { Instructions::data_drop, "data.drop"_string },
    { Instructions::memory_copy, "memory.copy"_string },
    { Instructions::memory_fill, "memory.fill"_string },
    { Instructions::table_init, "table.init"_string },
    { Instructions::elem_drop, "elem.drop"_string },
    { Instructions::table_copy, "table.copy"_string },
    { Instructions::table_grow, "table.grow"_string },
    { Instructions::table_size, "table.size"_string },
    { Instructions::table_fill, "table.fill"_string },
    { Instructions::v128_load, "v128.load"_string },
    { Instructions::v128_load8x8_s, "v128.load8x8_s"_string },
    { Instructions::v128_load8x8_u, "v128.load8x8_u"_string },
    { Instructions::v128_load16x4_s, "v128.load16x4_s"_string },
    { Instructions::v128_load16x4_u, "v128.load16x4_u"_string },
    { Instructions::v128_load32x2_s, "v128.load32x2_s"_string },
    { Instructions::v128_load32x2_u, "v128.load32x2_u"_string },
    { Instructions::v128_load8_splat, "v128.load8_splat"_string },
    { Instructions::v128_load16_splat, "v128.load16_splat"_string },
    { Instructions::v128_load32_splat, "v128.load32_splat"_string },
    { Instructions::v128_load64_splat, "v128.load64_splat"_string },
    { Instructions::v128_store, "v128.store"_string },
    { Instructions::v128_const, "v128.const"_string },
    { Instructions::i8x16_shuffle, "i8x16.shuffle"_string },
    { Instructions::i8x16_swizzle, "i8x16.swizzle"_string },
    { Instructions::i8x16_splat, "i8x16.splat"_string },
    { Instructions::i16x8_splat, "i16x8.splat"_string },
    { Instructions::i32x4_splat, "i32x4.splat"_string },
    { Instructions::i64x2_splat, "i64x2.splat"_string },
    { Instructions::f32x4_splat, "f32x4.splat"_string },
    { Instructions::f64x2_splat, "f64x2.splat"_string },
    { Instructions::i8x16_extract_lane_s, "i8x16.extract_lane_s"_string },
    { Instructions::i8x16_extract_lane_u, "i8x16.extract_lane_u"_string },
    { Instructions::i8x16_replace_lane, "i8x16.replace_lane"_string },
    { Instructions::i16x8_extract_lane_s, "i16x8.extract_lane_s"_string },
    { Instructions::i16x8_extract_lane_u, "i16x8.extract_lane_u"_string },
    { Instructions::i16x8_replace_lane, "i16x8.replace_lane"_string },
    { Instructions::i32x4_extract_lane, "i32x4.extract_lane"_string },
    { Instructions::i32x4_replace_lane, "i32x4.replace_lane"_string },
    { Instructions::i64x2_extract_lane, "i64x2.extract_lane"_string },
    { Instructions::i64x2_replace_lane, "i64x2.replace_lane"_string },
    { Instructions::f32x4_extract_lane, "f32x4.extract_lane"_string },
    { Instructions::f32x4_replace_lane, "f32x4.replace_lane"_string },
    { Instructions::f64x2_extract_lane, "f64x2.extract_lane"_string },
    { Instructions::f64x2_replace_lane, "f64x2.replace_lane"_string },
    { Instructions::i8x16_eq, "i8x16.eq"_string },
    { Instructions::i8x16_ne, "i8x16.ne"_string },
    { Instructions::i8x16_lt_s, "i8x16.lt_s"_string },
    { Instructions::i8x16_lt_u, "i8x16.lt_u"_string },
    { Instructions::i8x16_gt_s, "i8x16.gt_s"_string },
    { Instructions::i8x16_gt_u, "i8x16.gt_u"_string },
    { Instructions::i8x16_le_s, "i8x16.le_s"_string },
    { Instructions::i8x16_le_u, "i8x16.le_u"_string },
    { Instructions::i8x16_ge_s, "i8x16.ge_s"_string },
    { Instructions::i8x16_ge_u, "i8x16.ge_u"_string },
    { Instructions::i16x8_eq, "i16x8.eq"_string },
    { Instructions::i16x8_ne, "i16x8.ne"_string },
    { Instructions::i16x8_lt_s, "i16x8.lt_s"_string },
    { Instructions::i16x8_lt_u, "i16x8.lt_u"_string },
    { Instructions::i16x8_gt_s, "i16x8.gt_s"_string },
    { Instructions::i16x8_gt_u, "i16x8.gt_u"_string },
    { Instructions::i16x8_le_s, "i16x8.le_s"_string },
    { Instructions::i16x8_le_u, "i16x8.le_u"_string },
    { Instructions::i16x8_ge_s, "i16x8.ge_s"_string },
    { Instructions::i16x8_ge_u, "i16x8.ge_u"_string },
    { Instructions::i32x4_eq, "i32x4.eq"_string },
    { Instructions::i32x4_ne, "i32x4.ne"_string },
    { Instructions::i32x4_lt_s, "i32x4.lt_s"_string },
    { Instructions::i32x4_lt_u, "i32x4.lt_u"_string },
    { Instructions::i32x4_gt_s, "i32x4.gt_s"_string },
    { Instructions::i32x4_gt_u, "i32x4.gt_u"_string },
    { Instructions::i32x4_le_s, "i32x4.le_s"_string },
    { Instructions::i32x4_le_u, "i32x4.le_u"_string },
    { Instructions::i32x4_ge_s, "i32x4.ge_s"_string },
    { Instructions::i32x4_ge_u, "i32x4.ge_u"_string },
    { Instructions::f32x4_eq, "f32x4.eq"_string },
    { Instructions::f32x4_ne, "f32x4.ne"_string },
    { Instructions::f32x4_lt, "f32x4.lt"_string },
    { Instructions::f32x4_gt, "f32x4.gt"_string },
    { Instructions::f32x4_le, "f32x4.le"_string },
    { Instructions::f32x4_ge, "f32x4.ge"_string },
    { Instructions::f64x2_eq, "f64x2.eq"_string },
    { Instructions::f64x2_ne, "f64x2.ne"_string },
    { Instructions::f64x2_lt, "f64x2.lt"_string },
    { Instructions::f64x2_gt, "f64x2.gt"_string },
    { Instructions::f64x2_le, "f64x2.le"_string },
    { Instructions::f64x2_ge, "f64x2.ge"_string },
    { Instructions::v128_not, "v128.not"_string },
    { Instructions::v128_and, "v128.and"_string },
    { Instructions::v128_andnot, "v128.andnot"_string },
    { Instructions::v128_or, "v128.or"_string },
    { Instructions::v128_xor, "v128.xor"_string },
    { Instructions::v128_bitselect, "v128.bitselect"_string },
    { Instructions::v128_any_true, "v128.any_true"_string },
    { Instructions::v128_load8_lane, "v128.load8_lane"_string },
    { Instructions::v128_load16_lane, "v128.load16_lane"_string },
    { Instructions::v128_load32_lane, "v128.load32_lane"_string },
    { Instructions::v128_load64_lane, "v128.load64_lane"_string },
    { Instructions::v128_store8_lane, "v128.store8_lane"_string },
    { Instructions::v128_store16_lane, "v128.store16_lane"_string },
    { Instructions::v128_store32_lane, "v128.store32_lane"_string },
    { Instructions::v128_store64_lane, "v128.store64_lane"_string },
    { Instructions::v128_load32_zero, "v128.load32_zero"_string },
    { Instructions::v128_load64_zero, "v128.load64_zero"_string },
    { Instructions::f32x4_demote_f64x2_zero, "f32x4.demote_f64x2_zero"_string },
    { Instructions::f64x2_promote_low_f32x4, "f64x2.promote_low_f32x4"_string },
    { Instructions::i8x16_abs, "i8x16.abs"_string },
    { Instructions::i8x16_neg, "i8x16.neg"_string },
    { Instructions::i8x16_popcnt, "i8x16.popcnt"_string },
    { Instructions::i8x16_all_true, "i8x16.all_true"_string },
    { Instructions::i8x16_bitmask, "i8x16.bitmask"_string },
    { Instructions::i8x16_narrow_i16x8_s, "i8x16.narrow_i16x8_s"_string },
    { Instructions::i8x16_narrow_i16x8_u, "i8x16.narrow_i16x8_u"_string },
    { Instructions::f32x4_ceil, "f32x4.ceil"_string },
    { Instructions::f32x4_floor, "f32x4.floor"_string },
    { Instructions::f32x4_trunc, "f32x4.trunc"_string },
    { Instructions::f32x4_nearest, "f32x4.nearest"_string },
    { Instructions::i8x16_shl, "i8x16.shl"_string },
    { Instructions::i8x16_shr_s, "i8x16.shr_s"_string },
    { Instructions::i8x16_shr_u, "i8x16.shr_u"_string },
    { Instructions::i8x16_add, "i8x16.add"_string },
    { Instructions::i8x16_add_sat_s, "i8x16.add_sat_s"_string },
    { Instructions::i8x16_add_sat_u, "i8x16.add_sat_u"_string },
    { Instructions::i8x16_sub, "i8x16.sub"_string },
    { Instructions::i8x16_sub_sat_s, "i8x16.sub_sat_s"_string },
    { Instructions::i8x16_sub_sat_u, "i8x16.sub_sat_u"_string },
    { Instructions::f64x2_ceil, "f64x2.ceil"_string },
    { Instructions::f64x2_floor, "f64x2.floor"_string },
    { Instructions::i8x16_min_s, "i8x16.min_s"_string },
    { Instructions::i8x16_min_u, "i8x16.min_u"_string },
    { Instructions::i8x16_max_s, "i8x16.max_s"_string },
    { Instructions::i8x16_max_u, "i8x16.max_u"_string },
    { Instructions::f64x2_trunc, "f64x2.trunc"_string },
    { Instructions::i8x16_avgr_u, "i8x16.avgr_u"_string },
    { Instructions::i16x8_extadd_pairwise_i8x16_s, "i16x8.extadd_pairwise_i8x16_s"_string },
    { Instructions::i16x8_extadd_pairwise_i8x16_u, "i16x8.extadd_pairwise_i8x16_u"_string },
    { Instructions::i32x4_extadd_pairwise_i16x8_s, "i32x4.extadd_pairwise_i16x8_s"_string },
    { Instructions::i32x4_extadd_pairwise_i16x8_u, "i32x4.extadd_pairwise_i16x8_u"_string },
    { Instructions::i16x8_abs, "i16x8.abs"_string },
    { Instructions::i16x8_neg, "i16x8.neg"_string },
    { Instructions::i16x8_q15mulr_sat_s, "i16x8.q15mulr_sat_s"_string },
    { Instructions::i16x8_all_true, "i16x8.all_true"_string },
    { Instructions::i16x8_bitmask, "i16x8.bitmask"_string },
    { Instructions::i16x8_narrow_i32x4_s, "i16x8.narrow_i32x4_s"_string },
    { Instructions::i16x8_narrow_i32x4_u, "i16x8.narrow_i32x4_u"_string },
    { Instructions::i16x8_extend_low_i8x16_s, "i16x8.extend_low_i8x16_s"_string },
    { Instructions::i16x8_extend_high_i8x16_s, "i16x8.extend_high_i8x16_s"_string },
    { Instructions::i16x8_extend_low_i8x16_u, "i16x8.extend_low_i8x16_u"_string },
    { Instructions::i16x8_extend_high_i8x16_u, "i16x8.extend_high_i8x16_u"_string },
    { Instructions::i16x8_shl, "i16x8.shl"_string },
    { Instructions::i16x8_shr_s, "i16x8.shr_s"_string },
    { Instructions::i16x8_shr_u, "i16x8.shr_u"_string },
    { Instructions::i16x8_add, "i16x8.add"_string },
    { Instructions::i16x8_add_sat_s, "i16x8.add_sat_s"_string },
    { Instructions::i16x8_add_sat_u, "i16x8.add_sat_u"_string },
    { Instructions::i16x8_sub, "i16x8.sub"_string },
    { Instructions::i16x8_sub_sat_s, "i16x8.sub_sat_s"_string },
    { Instructions::i16x8_sub_sat_u, "i16x8.sub_sat_u"_string },
    { Instructions::f64x2_nearest, "f64x2.nearest"_string },
    { Instructions::i16x8_mul, "i16x8.mul"_string },
    { Instructions::i16x8_min_s, "i16x8.min_s"_string },
    { Instructions::i16x8_min_u, "i16x8.min_u"_string },
    { Instructions::i16x8_max_s, "i16x8.max_s"_string },
    { Instructions::i16x8_max_u, "i16x8.max_u"_string },
    { Instructions::i16x8_avgr_u, "i16x8.avgr_u"_string },
    { Instructions::i16x8_extmul_low_i8x16_s, "i16x8.extmul_low_i8x16_s"_string },
    { Instructions::i16x8_extmul_high_i8x16_s, "i16x8.extmul_high_i8x16_s"_string },
    { Instructions::i16x8_extmul_low_i8x16_u, "i16x8.extmul_low_i8x16_u"_string },
    { Instructions::i16x8_extmul_high_i8x16_u, "i16x8.extmul_high_i8x16_u"_string },
    { Instructions::i32x4_abs, "i32x4.abs"_string },
    { Instructions::i32x4_neg, "i32x4.neg"_string },
    { Instructions::i32x4_all_true, "i32x4.all_true"_string },
    { Instructions::i32x4_bitmask, "i32x4.bitmask"_string },
    { Instructions::i32x4_extend_low_i16x8_s, "i32x4.extend_low_i16x8_s"_string },
    { Instructions::i32x4_extend_high_i16x8_s, "i32x4.extend_high_i16x8_s"_string },
    { Instructions::i32x4_extend_low_i16x8_u, "i32x4.extend_low_i16x8_u"_string },
    { Instructions::i32x4_extend_high_i16x8_u, "i32x4.extend_high_i16x8_u"_string },
    { Instructions::i32x4_shl, "i32x4.shl"_string },
    { Instructions::i32x4_shr_s, "i32x4.shr_s"_string },
    { Instructions::i32x4_shr_u, "i32x4.shr_u"_string },
    { Instructions::i32x4_add, "i32x4.add"_string },
    { Instructions::i32x4_sub, "i32x4.sub"_string },
    { Instructions::i32x4_mul, "i32x4.mul"_string },
    { Instructions::i32x4_min_s, "i32x4.min_s"_string },
    { Instructions::i32x4_min_u, "i32x4.min_u"_string },
    { Instructions::i32x4_max_s, "i32x4.max_s"_string },
    { Instructions::i32x4_max_u, "i32x4.max_u"_string },
    { Instructions::i32x4_dot_i16x8_s, "i32x4.dot_i16x8_s"_string },
    { Instructions::i32x4_extmul_low_i16x8_s, "i32x4.extmul_low_i16x8_s"_string },
    { Instructions::i32x4_extmul_high_i16x8_s, "i32x4.extmul_high_i16x8_s"_string },
    { Instructions::i32x4_extmul_low_i16x8_u, "i32x4.extmul_low_i16x8_u"_string },
    { Instructions::i32x4_extmul_high_i16x8_u, "i32x4.extmul_high_i16x8_u"_string },
    { Instructions::i64x2_abs, "i64x2.abs"_string },
    { Instructions::i64x2_neg, "i64x2.neg"_string },
    { Instructions::i64x2_all_true, "i64x2.all_true"_string },
    { Instructions::i64x2_bitmask, "i64x2.bitmask"_string },
    { Instructions::i64x2_extend_low_i32x4_s, "i64x2.extend_low_i32x4_s"_string },
    { Instructions::i64x2_extend_high_i32x4_s, "i64x2.extend_high_i32x4_s"_string },
    { Instructions::i64x2_extend_low_i32x4_u, "i64x2.extend_low_i32x4_u"_string },
    { Instructions::i64x2_extend_high_i32x4_u, "i64x2.extend_high_i32x4_u"_string },
    { Instructions::i64x2_shl, "i64x2.shl"_string },
    { Instructions::i64x2_shr_s, "i64x2.shr_s"_string },
    { Instructions::i64x2_shr_u, "i64x2.shr_u"_string },
    { Instructions::i64x2_add, "i64x2.add"_string },
    { Instructions::i64x2_sub, "i64x2.sub"_string },
    { Instructions::i64x2_mul, "i64x2.mul"_string },
    { Instructions::i64x2_eq, "i64x2.eq"_string },
    { Instructions::i64x2_ne, "i64x2.ne"_string },
    { Instructions::i64x2_lt_s, "i64x2.lt_s"_string },
    { Instructions::i64x2_gt_s, "i64x2.gt_s"_string },
    { Instructions::i64x2_le_s, "i64x2.le_s"_string },
    { Instructions::i64x2_ge_s, "i64x2.ge_s"_string },
    { Instructions::i64x2_extmul_low_i32x4_s, "i64x2.extmul_low_i32x4_s"_string },
    { Instructions::i64x2_extmul_high_i32x4_s, "i64x2.extmul_high_i32x4_s"_string },
    { Instructions::i64x2_extmul_low_i32x4_u, "i64x2.extmul_low_i32x4_u"_string },
    { Instructions::i64x2_extmul_high_i32x4_u, "i64x2.extmul_high_i32x4_u"_string },
    { Instructions::f32x4_abs, "f32x4.abs"_string },
    { Instructions::f32x4_neg, "f32x4.neg"_string },
    { Instructions::f32x4_sqrt, "f32x4.sqrt"_string },
    { Instructions::f32x4_add, "f32x4.add"_string },
    { Instructions::f32x4_sub, "f32x4.sub"_string },
    { Instructions::f32x4_mul, "f32x4.mul"_string },
    { Instructions::f32x4_div, "f32x4.div"_string },
    { Instructions::f32x4_min, "f32x4.min"_string },
    { Instructions::f32x4_max, "f32x4.max"_string },
    { Instructions::f32x4_pmin, "f32x4.pmin"_string },
    { Instructions::f32x4_pmax, "f32x4.pmax"_string },
    { Instructions::f64x2_abs, "f64x2.abs"_string },
    { Instructions::f64x2_neg, "f64x2.neg"_string },
    { Instructions::f64x2_sqrt, "f64x2.sqrt"_string },
    { Instructions::f64x2_add, "f64x2.add"_string },
    { Instructions::f64x2_sub, "f64x2.sub"_string },
    { Instructions::f64x2_mul, "f64x2.mul"_string },
    { Instructions::f64x2_div, "f64x2.div"_string },
    { Instructions::f64x2_min, "f64x2.min"_string },
    { Instructions::f64x2_max, "f64x2.max"_string },
    { Instructions::f64x2_pmin, "f64x2.pmin"_string },
    { Instructions::f64x2_pmax, "f64x2.pmax"_string },
    { Instructions::i32x4_trunc_sat_f32x4_s, "i32x4.trunc_sat_f32x4_s"_string },
    { Instructions::i32x4_trunc_sat_f32x4_u, "i32x4.trunc_sat_f32x4_u"_string },
    { Instructions::f32x4_convert_i32x4_s, "f32x4.convert_i32x4_s"_string },
    { Instructions::f32x4_convert_i32x4_u, "f32x4.convert_i32x4_u"_string },
    { Instructions::i32x4_trunc_sat_f64x2_s_zero, "i32x4.trunc_sat_f64x2_s_zero"_string },
    { Instructions::i32x4_trunc_sat_f64x2_u_zero, "i32x4.trunc_sat_f64x2_u_zero"_string },
    { Instructions::f64x2_convert_low_i32x4_s, "f64x2.convert_low_i32x4_s"_string },
    { Instructions::f64x2_convert_low_i32x4_u, "f64x2.convert_low_i32x4_u"_string },
    { Instructions::structured_else, "synthetic:else"_string },
    { Instructions::structured_end, "synthetic:end"_string },
};
HashMap<String, Wasm::OpCode> Wasm::Names::instructions_by_name;
