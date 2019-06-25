/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity -- --rust
 *
 **
 *
 */
use crate::coroutine_smart_constructors::*;
use crate::lexable_token::LexableToken;
use crate::parser_env::ParserEnv;
use crate::smart_constructors::SmartConstructors;
use crate::source_text::SourceText;
use crate::syntax::*;
use crate::syntax_smart_constructors::SyntaxSmartConstructors;

impl<'a, S, Token, Value> SmartConstructors<'a, Bool<'a>>
    for CoroutineSmartConstructors<S>
where
    Token: LexableToken,
    Value: SyntaxValueType<Token>,
    S: SyntaxType<Token=Token, Value=Value>,
{
    type Token = Token;
    type R = S;

    fn initial_state<'b: 'a>(env: &ParserEnv, src: &'b SourceText<'b>) -> Bool<'a> {
        <Self as SyntaxSmartConstructors<'a, Self::R, State<Self::R>>>::initial_state(env, src)
    }

    fn make_missing(s: Bool<'a>, offset: usize) -> (Bool<'a>, Self::R) {
       <Self as SyntaxSmartConstructors<'a, Self::R, State<Self::R>>>::make_missing(s, offset)
    }

    fn make_token(s: Bool<'a>, offset: Self::Token) -> (Bool<'a>, Self::R) {
       <Self as SyntaxSmartConstructors<'a, Self::R, State<Self::R>>>::make_token(s, offset)
    }

    fn make_list(
        s: Bool<'a>,
        lst: Box<Vec<Self::R>>,
        offset: usize,
    ) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<'a, Self::R, State<Self::R>>>::make_list(s, lst, offset)
    }

    fn make_end_of_file(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_end_of_file(s, arg0)
    }

    fn make_script(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_script(s, arg0)
    }

    fn make_qualified_name(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_qualified_name(s, arg0)
    }

    fn make_simple_type_specifier(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_simple_type_specifier(s, arg0)
    }

    fn make_literal_expression(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_literal_expression(s, arg0)
    }

    fn make_prefixed_string_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_prefixed_string_expression(s, arg0, arg1)
    }

    fn make_variable_expression(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_variable_expression(s, arg0)
    }

    fn make_pipe_variable_expression(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pipe_variable_expression(s, arg0)
    }

    fn make_file_attribute_specification(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_file_attribute_specification(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_enum_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_enum_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_enumerator(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_enumerator(s, arg0, arg1, arg2, arg3)
    }

    fn make_record_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_record_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_record_field(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_record_field(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_alias_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_alias_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_property_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_property_declaration(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_property_declarator(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_property_declarator(s, arg0, arg1)
    }

    fn make_namespace_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_namespace_declaration(s, arg0, arg1, arg2)
    }

    fn make_namespace_body(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_namespace_body(s, arg0, arg1, arg2)
    }

    fn make_namespace_empty_body(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_namespace_empty_body(s, arg0)
    }

    fn make_namespace_use_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_namespace_use_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_namespace_group_use_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_namespace_group_use_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_namespace_use_clause(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_namespace_use_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_function_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_function_declaration(s, arg0, arg1, arg2)
    }

    fn make_function_declaration_header(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_function_declaration_header(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_where_clause(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_where_clause(s, arg0, arg1)
    }

    fn make_where_constraint(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_where_constraint(s, arg0, arg1, arg2)
    }

    fn make_methodish_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_methodish_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_methodish_trait_resolution(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_methodish_trait_resolution(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_classish_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_classish_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
    }

    fn make_classish_body(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_classish_body(s, arg0, arg1, arg2)
    }

    fn make_trait_use_precedence_item(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_trait_use_precedence_item(s, arg0, arg1, arg2)
    }

    fn make_trait_use_alias_item(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_trait_use_alias_item(s, arg0, arg1, arg2, arg3)
    }

    fn make_trait_use_conflict_resolution(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_trait_use_conflict_resolution(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_trait_use(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_trait_use(s, arg0, arg1, arg2)
    }

    fn make_require_clause(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_require_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_const_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_const_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_constant_declarator(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_constant_declarator(s, arg0, arg1)
    }

    fn make_type_const_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_type_const_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_decorated_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_decorated_expression(s, arg0, arg1)
    }

    fn make_parameter_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_parameter_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_variadic_parameter(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_variadic_parameter(s, arg0, arg1, arg2)
    }

    fn make_attribute_specification(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_attribute_specification(s, arg0, arg1, arg2)
    }

    fn make_inclusion_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_inclusion_expression(s, arg0, arg1)
    }

    fn make_inclusion_directive(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_inclusion_directive(s, arg0, arg1)
    }

    fn make_compound_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_compound_statement(s, arg0, arg1, arg2)
    }

    fn make_expression_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_expression_statement(s, arg0, arg1)
    }

    fn make_markup_section(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_markup_section(s, arg0, arg1, arg2, arg3)
    }

    fn make_markup_suffix(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_markup_suffix(s, arg0, arg1)
    }

    fn make_unset_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_unset_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_let_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_let_statement(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_block_scoped(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_using_statement_block_scoped(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_function_scoped(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_using_statement_function_scoped(s, arg0, arg1, arg2, arg3)
    }

    fn make_while_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_while_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_if_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_if_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_elseif_clause(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_elseif_clause(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_else_clause(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_else_clause(s, arg0, arg1)
    }

    fn make_try_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_try_statement(s, arg0, arg1, arg2, arg3)
    }

    fn make_catch_clause(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_catch_clause(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_finally_clause(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_finally_clause(s, arg0, arg1)
    }

    fn make_do_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_do_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_for_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_for_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_foreach_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_foreach_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_switch_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_switch_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_switch_section(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_switch_section(s, arg0, arg1, arg2)
    }

    fn make_switch_fallthrough(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_switch_fallthrough(s, arg0, arg1)
    }

    fn make_case_label(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_case_label(s, arg0, arg1, arg2)
    }

    fn make_default_label(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_default_label(s, arg0, arg1)
    }

    fn make_return_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_return_statement(s, arg0, arg1, arg2)
    }

    fn make_goto_label(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_goto_label(s, arg0, arg1)
    }

    fn make_goto_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_goto_statement(s, arg0, arg1, arg2)
    }

    fn make_throw_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_throw_statement(s, arg0, arg1, arg2)
    }

    fn make_break_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_break_statement(s, arg0, arg1, arg2)
    }

    fn make_continue_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_continue_statement(s, arg0, arg1, arg2)
    }

    fn make_echo_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_echo_statement(s, arg0, arg1, arg2)
    }

    fn make_concurrent_statement(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_concurrent_statement(s, arg0, arg1)
    }

    fn make_simple_initializer(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_simple_initializer(s, arg0, arg1)
    }

    fn make_anonymous_class(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_anonymous_class(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_anonymous_function(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_anonymous_function(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_anonymous_function_use_clause(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_anonymous_function_use_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_lambda_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_lambda_expression(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_lambda_signature(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_lambda_signature(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_cast_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_cast_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_scope_resolution_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_scope_resolution_expression(s, arg0, arg1, arg2)
    }

    fn make_member_selection_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_safe_member_selection_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_safe_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_embedded_member_selection_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_embedded_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_yield_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_yield_expression(s, arg0, arg1)
    }

    fn make_yield_from_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_yield_from_expression(s, arg0, arg1, arg2)
    }

    fn make_prefix_unary_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_prefix_unary_expression(s, arg0, arg1)
    }

    fn make_postfix_unary_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_postfix_unary_expression(s, arg0, arg1)
    }

    fn make_binary_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_binary_expression(s, arg0, arg1, arg2)
    }

    fn make_instanceof_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_instanceof_expression(s, arg0, arg1, arg2)
    }

    fn make_is_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_is_expression(s, arg0, arg1, arg2)
    }

    fn make_as_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_as_expression(s, arg0, arg1, arg2)
    }

    fn make_nullable_as_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_nullable_as_expression(s, arg0, arg1, arg2)
    }

    fn make_conditional_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_conditional_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_eval_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_eval_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_define_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_define_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_halt_compiler_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_halt_compiler_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_isset_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_isset_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_function_call_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_function_call_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_parenthesized_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_parenthesized_expression(s, arg0, arg1, arg2)
    }

    fn make_braced_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_braced_expression(s, arg0, arg1, arg2)
    }

    fn make_embedded_braced_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_embedded_braced_expression(s, arg0, arg1, arg2)
    }

    fn make_list_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_list_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_collection_literal_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_collection_literal_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_object_creation_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_object_creation_expression(s, arg0, arg1)
    }

    fn make_constructor_call(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_constructor_call(s, arg0, arg1, arg2, arg3)
    }

    fn make_record_creation_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_record_creation_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_array_creation_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_array_creation_expression(s, arg0, arg1, arg2)
    }

    fn make_array_intrinsic_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_array_intrinsic_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_darray_intrinsic_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_darray_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_dictionary_intrinsic_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_dictionary_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_intrinsic_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_keyset_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_varray_intrinsic_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_varray_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_intrinsic_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_vector_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_element_initializer(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_element_initializer(s, arg0, arg1, arg2)
    }

    fn make_subscript_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_subscript_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_embedded_subscript_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_embedded_subscript_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_awaitable_creation_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_awaitable_creation_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_children_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_children_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_children_parenthesized_list(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_children_parenthesized_list(s, arg0, arg1, arg2)
    }

    fn make_xhp_category_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_category_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_enum_type(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_enum_type(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_xhp_lateinit(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_lateinit(s, arg0, arg1)
    }

    fn make_xhp_required(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_required(s, arg0, arg1)
    }

    fn make_xhp_class_attribute_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_class_attribute_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_class_attribute(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_class_attribute(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_simple_class_attribute(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_simple_class_attribute(s, arg0)
    }

    fn make_xhp_simple_attribute(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_simple_attribute(s, arg0, arg1, arg2)
    }

    fn make_xhp_spread_attribute(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_spread_attribute(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_open(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_open(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_expression(s, arg0, arg1, arg2)
    }

    fn make_xhp_close(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_xhp_close(s, arg0, arg1, arg2)
    }

    fn make_type_constant(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_type_constant(s, arg0, arg1, arg2)
    }

    fn make_vector_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_vector_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_keyset_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_tuple_type_explicit_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_tuple_type_explicit_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_varray_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_varray_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_array_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_vector_array_type_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_type_parameter(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_type_parameter(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_type_constraint(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_type_constraint(s, arg0, arg1)
    }

    fn make_darray_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_darray_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_map_array_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_map_array_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_dictionary_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_dictionary_type_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_closure_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_closure_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_closure_parameter_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_closure_parameter_type_specifier(s, arg0, arg1)
    }

    fn make_classname_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_classname_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_field_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_field_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_field_initializer(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_field_initializer(s, arg0, arg1, arg2)
    }

    fn make_shape_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_shape_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_shape_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_shape_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_tuple_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_tuple_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_generic_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_generic_type_specifier(s, arg0, arg1)
    }

    fn make_nullable_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_nullable_type_specifier(s, arg0, arg1)
    }

    fn make_like_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_like_type_specifier(s, arg0, arg1)
    }

    fn make_soft_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_soft_type_specifier(s, arg0, arg1)
    }

    fn make_reified_type_argument(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_reified_type_argument(s, arg0, arg1)
    }

    fn make_type_arguments(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_type_arguments(s, arg0, arg1, arg2)
    }

    fn make_type_parameters(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_type_parameters(s, arg0, arg1, arg2)
    }

    fn make_tuple_type_specifier(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_tuple_type_specifier(s, arg0, arg1, arg2)
    }

    fn make_error(s: Bool<'a>, arg0: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_error(s, arg0)
    }

    fn make_list_item(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_list_item(s, arg0, arg1)
    }

    fn make_pocket_atom_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pocket_atom_expression(s, arg0, arg1)
    }

    fn make_pocket_identifier_expression(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pocket_identifier_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_pocket_atom_mapping_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pocket_atom_mapping_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_enum_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pocket_enum_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_field_type_expr_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pocket_field_type_expr_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_field_type_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pocket_field_type_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_mapping_id_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pocket_mapping_id_declaration(s, arg0, arg1)
    }

    fn make_pocket_mapping_type_declaration(s: Bool<'a>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (Bool<'a>, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, State<Self::R>>>::make_pocket_mapping_type_declaration(s, arg0, arg1, arg2, arg3)
    }

}
