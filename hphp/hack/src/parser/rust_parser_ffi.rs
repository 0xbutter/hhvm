/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 * TODO(kasper): rustfmt is getting confused in this file because of macro definition,
 * and @nolint was the only way I found to disable it
*/

#[macro_use]
extern crate ocaml;
pub mod rust_to_ocaml;
use parser_rust as parser;

use parser::file_mode::parse_mode;
use parser::minimal_syntax::MinimalValue;
use parser::minimal_token::MinimalToken;
use parser::parser::Parser;
use parser::parser_env::ParserEnv;

use parser::positioned_syntax::PositionedValue;
use parser::positioned_token::PositionedToken;

use parser::smart_constructors::NoState;
use parser::smart_constructors_wrappers::WithKind;
use parser::source_text::SourceText;
use parser::syntax_smart_constructors::SyntaxSmartConstructors;
use rust_to_ocaml::{caml_tuple, to_list, SerializationContext, ToOcaml};

type MinimalSyntaxParser<'a> =
    Parser<'a, WithKind<SyntaxSmartConstructors<MinimalToken, MinimalValue>>, NoState>;
type PositionedSyntaxParser<'a> =
    Parser<'a, WithKind<SyntaxSmartConstructors<PositionedToken, PositionedValue>>, NoState>;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

unsafe fn bool_field(block: &ocaml::Value, field: usize) -> bool {
    ocaml::Value::new(*ocaml::core::mlvalues::field(block.0, field)).i32_val() != 0
}

macro_rules! parse {
    ($name:ident, $parser:ident) => {
        caml!($name, |ocaml_source_text, opts|, <l>, {
            let content = ocaml::Str::from(
                ocaml::Value::new(*ocaml::core::mlvalues::field(ocaml_source_text.0, 2))
            );
            let data = content.data();
            let source_text = SourceText::make(&data);

            let is_experimental_mode = bool_field(&opts, 0);
            let enable_stronger_await_binding = bool_field(&opts, 1);
            let disable_unsafe_expr = bool_field(&opts, 2);
            let disable_unsafe_block = bool_field(&opts, 3);
            let force_hh = bool_field(&opts, 4);
            let enable_xhp = bool_field(&opts, 5);
            let hhvm_compat_mode = bool_field(&opts, 6);
            let php5_compat_mode = bool_field(&opts, 7);

            let env = ParserEnv {
                is_experimental_mode,
                enable_stronger_await_binding,
                disable_unsafe_expr,
                disable_unsafe_block,
                force_hh,
                enable_xhp,
                hhvm_compat_mode,
                php5_compat_mode,
            };

            let mode = parse_mode(&source_text);
            let mut parser = $parser::make(&source_text, env);
            let root = parser.parse_script();
            let errors = parser.errors();
            ocamlpool_enter();

            let context = SerializationContext::new(ocaml_source_text.0);
            let ocaml_root = root.to_ocaml(&context);
            let ocaml_errors = to_list(&errors, &context);
            let ocaml_mode = mode.to_ocaml(&context);

            let res = caml_tuple(&[
                ocaml_mode,
                ocaml_root,
                ocaml_errors
            ]);
            l = ocaml::Value::new(res);
            ocamlpool_leave();
        } -> l);
    };
}

parse!(parse_minimal, MinimalSyntaxParser);
parse!(parse_positioned, PositionedSyntaxParser);
