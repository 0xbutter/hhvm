// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_ocamlpool::ocaml_ffi;
use oxidized::file_info;
use parser_rust::{
    lexer::Lexer,
    minimal_token::MinimalToken,
    minimal_trivia::MinimalTrivia,
    operator::{Assoc, Operator},
    source_text::SourceText,
    token_kind::TokenKind,
};
use syntax_tree::mode_parser::parse_mode;

#[macro_export]
macro_rules! parse {
    ($name:ident, $parser:ident, $syntax:ty) => {
        use ocamlpool_rust::caml_raise;
        use std::panic::AssertUnwindSafe;
        use $name::{ocamlpool_enter, ocamlpool_leave};
        mod $name {
            use super::*;
            extern "C" {
                pub fn ocamlpool_enter();
                pub fn ocamlpool_leave();
            }

            use ocaml::Value;
            use ocamlrep::{Allocator, OcamlRep};
            use ocamlrep_ocamlpool::Pool;
            use ocamlpool_rust::{caml_raise, utils::*};
            use parser_rust::{
                self as parser,
                lexer::Lexer,
                parser_env::ParserEnv,
                source_text::SourceText,
                stack_limit::StackLimit,
            };
            use rust_to_ocaml::{to_list, SerializationContext, ToOcaml};
            use syntax_tree::{mode_parser::parse_mode, SyntaxTree};
            use oxidized::{
                full_fidelity_parser_env::FullFidelityParserEnv,
                relative_path::RelativePath,
            };

            pub unsafe fn parse(ocaml_source_text : usize, env : usize) -> usize {
                    let ocaml_source_text_value = ocaml_source_text;

                    let env = FullFidelityParserEnv::from_ocaml(env).unwrap();
                    let leak_rust_tree = env.leak_rust_tree;
                    let env = ParserEnv::from(env);

                    // Note: Determining the current thread size cannot be done portably,
                    // therefore assume the worst (running on non-main thread with min size, 2MiB)
                    const KI: usize = 1024;
                    const MI: usize = KI * KI;
                    const MAX_STACK_SIZE: usize = 1024 * MI;
                    let mut stack_size = 2 * MI;
                    let mut default_stack_size_sufficient = true;
                    parser::stack_limit::init();
                    loop {
                        if stack_size > MAX_STACK_SIZE {
                            panic!("Rust FFI exceeded maximum allowed stack of {} KiB", MAX_STACK_SIZE / KI);
                        }

                        // Avoid eagerly wasting of space that will not be used in practice (WWW),
                        // but only for degenerate test cases (/test/{slow,quick}), by starting off
                        // with small stack (default thread) then fall back to bigger ones (custom thread).
                        // Since we're doubling the stack the time is: t + 2*t + 4*t + ...
                        // where the total parse time with unbounded stack is T=k*t, which is
                        // bounded by 2*T (much less in practice due to superlinear parsing time).
                        let next_stack_size = if default_stack_size_sufficient {
                            13 * MI // assume we need much more if default stack size isn't enough
                        } else { // exponential backoff to limit parsing time to at most twice as long
                            2 * stack_size
                        };
                        // Note: detect almost full stack by setting "slack" of 60% for StackLimit because
                        // Syntax::to_ocaml is deeply & mutually recursive and uses nearly 2.5x of stack
                        // TODO: rewrite to_ocaml iteratively & reduce it to "stack_size - MB" as in HHVM
                        // (https://github.com/facebook/hhvm/blob/master/hphp/runtime/base/request-info.h)
                        let relative_stack_size = stack_size - stack_size*6/10;
                        let content = str_field(&ocaml::Value(ocaml_source_text), 2);
                        let relative_path_raw = block_field(&ocaml::Value(ocaml_source_text), 0);

                        let env = env.clone();
                        let try_parse = move || {
                            let stack_limit = StackLimit::relative(relative_stack_size);
                            stack_limit.reset();
                            let stack_limit_ref = &stack_limit;
                            let relative_path = RelativePath::from_ocaml(relative_path_raw.0).unwrap();
                            let file_path = relative_path.path_str().to_owned();
                            let relative_path = AssertUnwindSafe(relative_path);
                            let mut pool = Pool::new();
                            let parse_result = std::panic::catch_unwind(move || {
                                let source_text = SourceText::make_with_raw(
                                    &*relative_path,
                                    &content.data(),
                                    ocaml_source_text_value,
                                );
                                let mut parser = $parser::make(&source_text, env);
                                let root = parser.parse_script(Some(&stack_limit_ref));
                                let errors = parser.errors();
                                let state = parser.sc_state();

                                // traversing the parsed syntax tree uses about 1/3 of the stack
                                let context = SerializationContext::new(ocaml_source_text_value);
                                let ocaml_root = root.to_ocaml(&context);
                                let ocaml_errors = pool.add(&errors);
                                let ocaml_state = state.to_ocaml(&context);
                                let tree = if leak_rust_tree {
                                    let required_stack_size = if default_stack_size_sufficient {
                                        None
                                    } else {
                                        Some(stack_size)
                                    };
                                    let mode = parse_mode(&source_text);
                                    let tree = Box::new(SyntaxTree::build(&source_text, root, errors, mode, (), required_stack_size));
                                    Some(Box::leak(tree) as *const SyntaxTree<$syntax, ()> as usize)
                                } else {
                                    None
                                };
                                let ocaml_tree = pool.add(&tree);

                                let res = pool.block_with_size(4);
                                Pool::set_field(res, 0, ocamlrep::Value::from_bits(ocaml_state));
                                Pool::set_field(res, 1, ocamlrep::Value::from_bits(ocaml_root));
                                Pool::set_field(res, 2, ocaml_errors);
                                Pool::set_field(res, 3, ocaml_tree);
                                res as usize
                            });
                            match parse_result {
                                Ok(result) => Some(result),
                                Err(_) if stack_limit.exceeded() => {
                                    // Not always printing warning here because this would fail some HHVM tests
                                    let istty = libc::isatty(libc::STDERR_FILENO as i32) != 0;
                                    if istty || std::env::var_os("HH_TEST_MODE").is_some() {
                                        eprintln!("[hrust] warning: parser exceeded stack of {} KiB on: {}",
                                                  stack_limit.get() / KI,
                                                  file_path,
                                        );
                                    }
                                    None
                                }
                                Err(msg) => panic!(msg),
                            }
                        };
                        stack_size = next_stack_size;

                        let result_opt = if default_stack_size_sufficient {
                            try_parse()
                        } else {
                            std::thread::Builder::new().stack_size(stack_size).spawn(try_parse)
                                .expect("ERROR: thread::spawn")
                                .join().expect("ERROR: failed to wait on new thread")
                        };

                        match result_opt {
                            Some(ocaml_result) => return ocaml_result,
                            _ => default_stack_size_sufficient = false,
                        }
                    }
            }
        }

        // We don't use the ocaml_ffi! macro here because we want precise
        // control over the Pool--when a parse fails, we want to free the old
        // pool and create a new one.
        #[no_mangle]
        pub extern "C" fn $name(ocaml_source_text: usize, env: usize) -> usize {
            ocamlrep_ocamlpool::catch_unwind(|| unsafe {
                $name::parse(ocaml_source_text, env)
            })
        }
    }
}

fn trivia_lexer<'a>(source_text: SourceText<'a>, offset: usize) -> Lexer<'a, MinimalToken> {
    let is_experimental_mode = false;
    Lexer::make_at(&source_text, is_experimental_mode, offset)
}

ocaml_ffi! {
    fn rust_parse_mode(source_text: SourceText) -> Option<file_info::Mode> {
        parse_mode(&source_text)
    }

    fn scan_leading_xhp_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        trivia_lexer(source_text, offset).scan_leading_xhp_trivia()
    }
    fn scan_trailing_xhp_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        trivia_lexer(source_text, offset).scan_trailing_xhp_trivia()
    }
    fn scan_leading_php_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        trivia_lexer(source_text, offset).scan_leading_php_trivia()
    }
    fn scan_trailing_php_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        trivia_lexer(source_text, offset).scan_trailing_php_trivia()
    }

    fn trailing_from_token(token: TokenKind) -> Operator {
        Operator::trailing_from_token(token)
    }
    fn prefix_unary_from_token(token: TokenKind) -> Operator {
        Operator::prefix_unary_from_token(token)
    }

    fn is_trailing_operator_token(token: TokenKind) -> bool {
        Operator::is_trailing_operator_token(token)
    }
    fn is_binary_operator_token(token: TokenKind) -> bool {
        Operator::is_binary_operator_token(token)
    }

    fn is_comparison(op: Operator) -> bool {
        op.is_comparison()
    }
    fn is_assignment(op: Operator) -> bool {
        op.is_assignment()
    }

    fn rust_precedence_helper(op: Operator) -> usize {
        // NOTE: ParserEnv is not used in operator::precedence(), so we just create an empty ParserEnv
        // If operator::precedence() starts using ParserEnv, this function and the callsites in OCaml must be updated
        use parser_rust::parser_env::ParserEnv;
        op.precedence(&ParserEnv::default())
    }

    fn rust_precedence_for_assignment_in_expressions_helper() -> usize {
        Operator::precedence_for_assignment_in_expressions()
    }

    fn rust_associativity_helper(op: Operator) -> Assoc {
        // NOTE: ParserEnv is not used in operator::associativity(), so we just create an empty ParserEnv
        // If operator::associativity() starts using ParserEnv, this function and the callsites in OCaml must be updated
        use parser_rust::parser_env::ParserEnv;
        op.associativity(&ParserEnv::default())
    }
}
