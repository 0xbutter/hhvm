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
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 */
use crate::parser_env::ParserEnv;
use crate::positioned_syntax::{PositionedSyntax, PositionedValue};
use crate::positioned_token::PositionedToken;
use crate::smart_constructors::{NoState, SmartConstructors, StateType};
use crate::source_text::SourceText;
use crate::syntax_smart_constructors::SyntaxSmartConstructors;

pub struct PositionedSmartConstructors<State = NoState> {
    phantom_state: std::marker::PhantomData<State>,
}

impl<'a, State: StateType<'a, PositionedSyntax>>
SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>
    for PositionedSmartConstructors<State> {}

impl<'a, State: StateType<'a, PositionedSyntax>> SmartConstructors<'a, State::T>
    for PositionedSmartConstructors<State>
{
    type Token = PositionedToken;
    type R = PositionedSyntax;

    fn initial_state<'b: 'a>(env: &ParserEnv, src: &'b SourceText<'b>) -> State::T {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::initial_state(env, src)
    }

    fn make_missing(s: State::T, offset: usize) -> (State::T, Self::R) {
       <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_missing(s, offset)
    }

    fn make_token(s: State::T, offset: Self::Token) -> (State::T, Self::R) {
       <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_token(s, offset)
    }

    fn make_list(
        s: State::T,
        lst: Box<Vec<Self::R>>,
        offset: usize,
    ) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_list(s, lst, offset)
    }

    fn make_end_of_file(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_end_of_file(s, arg0)
    }

    fn make_script(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_script(s, arg0)
    }

    fn make_qualified_name(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_qualified_name(s, arg0)
    }

    fn make_simple_type_specifier(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_simple_type_specifier(s, arg0)
    }

    fn make_literal_expression(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_literal_expression(s, arg0)
    }

    fn make_prefixed_string_expression(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_prefixed_string_expression(s, arg0, arg1)
    }

    fn make_variable_expression(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_variable_expression(s, arg0)
    }

    fn make_pipe_variable_expression(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pipe_variable_expression(s, arg0)
    }

    fn make_file_attribute_specification(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_file_attribute_specification(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_enum_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_enum_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_enumerator(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_enumerator(s, arg0, arg1, arg2, arg3)
    }

    fn make_record_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_record_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_record_field(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_record_field(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_alias_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_alias_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_property_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_property_declaration(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_property_declarator(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_property_declarator(s, arg0, arg1)
    }

    fn make_namespace_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_namespace_declaration(s, arg0, arg1, arg2)
    }

    fn make_namespace_body(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_namespace_body(s, arg0, arg1, arg2)
    }

    fn make_namespace_empty_body(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_namespace_empty_body(s, arg0)
    }

    fn make_namespace_use_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_namespace_use_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_namespace_group_use_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_namespace_group_use_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_namespace_use_clause(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_namespace_use_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_function_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_function_declaration(s, arg0, arg1, arg2)
    }

    fn make_function_declaration_header(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_function_declaration_header(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_where_clause(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_where_clause(s, arg0, arg1)
    }

    fn make_where_constraint(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_where_constraint(s, arg0, arg1, arg2)
    }

    fn make_methodish_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_methodish_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_methodish_trait_resolution(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_methodish_trait_resolution(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_classish_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_classish_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_classish_body(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_classish_body(s, arg0, arg1, arg2)
    }

    fn make_trait_use_precedence_item(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_trait_use_precedence_item(s, arg0, arg1, arg2)
    }

    fn make_trait_use_alias_item(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_trait_use_alias_item(s, arg0, arg1, arg2, arg3)
    }

    fn make_trait_use_conflict_resolution(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_trait_use_conflict_resolution(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_trait_use(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_trait_use(s, arg0, arg1, arg2)
    }

    fn make_require_clause(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_require_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_const_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_const_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_constant_declarator(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_constant_declarator(s, arg0, arg1)
    }

    fn make_type_const_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_type_const_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_decorated_expression(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_decorated_expression(s, arg0, arg1)
    }

    fn make_parameter_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_parameter_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_variadic_parameter(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_variadic_parameter(s, arg0, arg1, arg2)
    }

    fn make_attribute_specification(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_attribute_specification(s, arg0, arg1, arg2)
    }

    fn make_inclusion_expression(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_inclusion_expression(s, arg0, arg1)
    }

    fn make_inclusion_directive(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_inclusion_directive(s, arg0, arg1)
    }

    fn make_compound_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_compound_statement(s, arg0, arg1, arg2)
    }

    fn make_alternate_loop_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_alternate_loop_statement(s, arg0, arg1, arg2, arg3)
    }

    fn make_expression_statement(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_expression_statement(s, arg0, arg1)
    }

    fn make_markup_section(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_markup_section(s, arg0, arg1, arg2, arg3)
    }

    fn make_markup_suffix(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_markup_suffix(s, arg0, arg1)
    }

    fn make_unset_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_unset_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_let_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_let_statement(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_block_scoped(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_using_statement_block_scoped(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_function_scoped(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_using_statement_function_scoped(s, arg0, arg1, arg2, arg3)
    }

    fn make_declare_directive_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_declare_directive_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_declare_block_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_declare_block_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_while_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_while_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_if_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_if_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_elseif_clause(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_elseif_clause(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_else_clause(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_else_clause(s, arg0, arg1)
    }

    fn make_alternate_if_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_alternate_if_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_alternate_elseif_clause(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_alternate_elseif_clause(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_alternate_else_clause(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_alternate_else_clause(s, arg0, arg1, arg2)
    }

    fn make_try_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_try_statement(s, arg0, arg1, arg2, arg3)
    }

    fn make_catch_clause(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_catch_clause(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_finally_clause(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_finally_clause(s, arg0, arg1)
    }

    fn make_do_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_do_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_for_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_for_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_foreach_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_foreach_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_switch_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_switch_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_alternate_switch_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_alternate_switch_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_switch_section(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_switch_section(s, arg0, arg1, arg2)
    }

    fn make_switch_fallthrough(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_switch_fallthrough(s, arg0, arg1)
    }

    fn make_case_label(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_case_label(s, arg0, arg1, arg2)
    }

    fn make_default_label(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_default_label(s, arg0, arg1)
    }

    fn make_return_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_return_statement(s, arg0, arg1, arg2)
    }

    fn make_goto_label(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_goto_label(s, arg0, arg1)
    }

    fn make_goto_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_goto_statement(s, arg0, arg1, arg2)
    }

    fn make_throw_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_throw_statement(s, arg0, arg1, arg2)
    }

    fn make_break_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_break_statement(s, arg0, arg1, arg2)
    }

    fn make_continue_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_continue_statement(s, arg0, arg1, arg2)
    }

    fn make_echo_statement(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_echo_statement(s, arg0, arg1, arg2)
    }

    fn make_concurrent_statement(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_concurrent_statement(s, arg0, arg1)
    }

    fn make_simple_initializer(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_simple_initializer(s, arg0, arg1)
    }

    fn make_anonymous_class(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_anonymous_class(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_anonymous_function(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_anonymous_function(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_php7_anonymous_function(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_php7_anonymous_function(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_anonymous_function_use_clause(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_anonymous_function_use_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_lambda_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_lambda_expression(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_lambda_signature(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_lambda_signature(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_cast_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_cast_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_scope_resolution_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_scope_resolution_expression(s, arg0, arg1, arg2)
    }

    fn make_member_selection_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_safe_member_selection_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_safe_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_embedded_member_selection_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_embedded_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_yield_expression(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_yield_expression(s, arg0, arg1)
    }

    fn make_yield_from_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_yield_from_expression(s, arg0, arg1, arg2)
    }

    fn make_prefix_unary_expression(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_prefix_unary_expression(s, arg0, arg1)
    }

    fn make_postfix_unary_expression(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_postfix_unary_expression(s, arg0, arg1)
    }

    fn make_binary_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_binary_expression(s, arg0, arg1, arg2)
    }

    fn make_instanceof_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_instanceof_expression(s, arg0, arg1, arg2)
    }

    fn make_is_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_is_expression(s, arg0, arg1, arg2)
    }

    fn make_as_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_as_expression(s, arg0, arg1, arg2)
    }

    fn make_nullable_as_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_nullable_as_expression(s, arg0, arg1, arg2)
    }

    fn make_conditional_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_conditional_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_eval_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_eval_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_empty_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_empty_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_define_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_define_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_halt_compiler_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_halt_compiler_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_isset_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_isset_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_function_call_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_function_call_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_parenthesized_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_parenthesized_expression(s, arg0, arg1, arg2)
    }

    fn make_braced_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_braced_expression(s, arg0, arg1, arg2)
    }

    fn make_embedded_braced_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_embedded_braced_expression(s, arg0, arg1, arg2)
    }

    fn make_list_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_list_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_collection_literal_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_collection_literal_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_object_creation_expression(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_object_creation_expression(s, arg0, arg1)
    }

    fn make_constructor_call(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_constructor_call(s, arg0, arg1, arg2, arg3)
    }

    fn make_record_creation_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_record_creation_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_array_creation_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_array_creation_expression(s, arg0, arg1, arg2)
    }

    fn make_array_intrinsic_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_array_intrinsic_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_darray_intrinsic_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_darray_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_dictionary_intrinsic_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_dictionary_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_intrinsic_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_keyset_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_varray_intrinsic_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_varray_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_intrinsic_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_vector_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_element_initializer(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_element_initializer(s, arg0, arg1, arg2)
    }

    fn make_subscript_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_subscript_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_embedded_subscript_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_embedded_subscript_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_awaitable_creation_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_awaitable_creation_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_children_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_children_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_children_parenthesized_list(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_children_parenthesized_list(s, arg0, arg1, arg2)
    }

    fn make_xhp_category_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_category_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_enum_type(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_enum_type(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_xhp_required(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_required(s, arg0, arg1)
    }

    fn make_xhp_class_attribute_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_class_attribute_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_class_attribute(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_class_attribute(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_simple_class_attribute(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_simple_class_attribute(s, arg0)
    }

    fn make_xhp_simple_attribute(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_simple_attribute(s, arg0, arg1, arg2)
    }

    fn make_xhp_spread_attribute(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_spread_attribute(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_open(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_open(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_expression(s, arg0, arg1, arg2)
    }

    fn make_xhp_close(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_xhp_close(s, arg0, arg1, arg2)
    }

    fn make_type_constant(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_type_constant(s, arg0, arg1, arg2)
    }

    fn make_vector_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_vector_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_keyset_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_tuple_type_explicit_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_tuple_type_explicit_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_varray_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_varray_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_array_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_vector_array_type_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_type_parameter(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_type_parameter(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_type_constraint(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_type_constraint(s, arg0, arg1)
    }

    fn make_darray_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_darray_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_map_array_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_map_array_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_dictionary_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_dictionary_type_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_closure_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_closure_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_closure_parameter_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_closure_parameter_type_specifier(s, arg0, arg1)
    }

    fn make_classname_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_classname_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_field_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_field_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_field_initializer(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_field_initializer(s, arg0, arg1, arg2)
    }

    fn make_shape_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_shape_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_shape_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_shape_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_tuple_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_tuple_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_generic_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_generic_type_specifier(s, arg0, arg1)
    }

    fn make_nullable_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_nullable_type_specifier(s, arg0, arg1)
    }

    fn make_like_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_like_type_specifier(s, arg0, arg1)
    }

    fn make_soft_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_soft_type_specifier(s, arg0, arg1)
    }

    fn make_reified_type_argument(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_reified_type_argument(s, arg0, arg1)
    }

    fn make_type_arguments(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_type_arguments(s, arg0, arg1, arg2)
    }

    fn make_type_parameters(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_type_parameters(s, arg0, arg1, arg2)
    }

    fn make_tuple_type_specifier(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_tuple_type_specifier(s, arg0, arg1, arg2)
    }

    fn make_error(s: State::T, arg0: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_error(s, arg0)
    }

    fn make_list_item(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_list_item(s, arg0, arg1)
    }

    fn make_pocket_atom_expression(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pocket_atom_expression(s, arg0, arg1)
    }

    fn make_pocket_identifier_expression(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pocket_identifier_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_pocket_atom_mapping_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pocket_atom_mapping_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_enum_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pocket_enum_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_field_type_expr_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pocket_field_type_expr_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_field_type_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pocket_field_type_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_mapping_id_declaration(s: State::T, arg0: Self::R, arg1: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pocket_mapping_id_declaration(s, arg0, arg1)
    }

    fn make_pocket_mapping_type_declaration(s: State::T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (State::T, Self::R) {
        <Self as SyntaxSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue, State>>::make_pocket_mapping_type_declaration(s, arg0, arg1, arg2, arg3)
    }

}
