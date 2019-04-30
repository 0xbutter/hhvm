// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern crate ocaml;
use parser_rust as parser;

use ocaml::core::memory;
use ocaml::core::mlvalues::{empty_list, Size, Tag, Value};

use std::cell::RefCell;
use std::collections::HashMap;
use std::iter::Iterator;

use parser::file_mode::FileMode;
use parser::lexable_token::LexableToken;
use parser::minimal_syntax::MinimalValue;
use parser::minimal_token::MinimalToken;
use parser::minimal_trivia::MinimalTrivia;
use parser::positioned_syntax::PositionedValue;
use parser::positioned_token::PositionedToken;
use parser::positioned_trivia::PositionedTrivia;
use parser::syntax::SyntaxVariant;
use parser::syntax::{Syntax, SyntaxValueType};
use parser::syntax_error::SyntaxError;
use parser::syntax_kind::SyntaxKind;
use parser::syntax_type::SyntaxType;
use parser::token_kind::TokenKind;
use parser::trivia_kind::TriviaKind;

extern "C" {
    fn ocamlpool_reserve_block(tag: Tag, size: Size) -> Value;
    fn ocamlpool_reserve_string(size: Size) -> Value;
}

// Unsafe functions in this file should be called only:
// - while being called from OCaml process
// - between ocamlpool_enter / ocamlpool_leave invocations
unsafe fn caml_block(tag: Tag, fields: &[Value]) -> Value {
    let result = ocamlpool_reserve_block(tag, fields.len());
    for (i, field) in fields.iter().enumerate() {
        memory::store_field(result, i, *field);
    }
    return result;
}

pub unsafe fn caml_tuple(fields: &[Value]) -> Value {
    caml_block(0, fields)
}

pub struct SerializationContext {
    source_text: Value,
    shared_tokens: RefCell<HashMap<PositionedToken, Value>>,
}

impl SerializationContext {
    pub fn new(source_text: Value) -> Self {
        Self {
            source_text,
            shared_tokens: RefCell::new(HashMap::new()),
        }
    }

    pub fn get(&self, token: &PositionedToken) -> Option<Value> {
        self.shared_tokens.borrow().get(token).cloned()
    }

    pub fn set(&self, token: PositionedToken, value: Value) {
        self.shared_tokens.borrow_mut().insert(token, value);
    }
}

pub trait ToOcaml {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value;
}

pub unsafe fn to_list<T>(values: &[T], context: &SerializationContext) -> Value
where
    T: ToOcaml,
{
    let mut res = empty_list();

    for v in values.iter().rev() {
        res = caml_tuple(&[v.to_ocaml(context), res])
    }
    res
}

// Not implementing ToOcaml for integer types, because Value itself is an integer too and it makes
// it too easy to accidentally treat a pointer to heap as integer and try double convert it
fn usize_to_ocaml(x: usize) -> Value {
    (x << 1) + 1
}

fn u8_to_ocaml(x: u8) -> Value {
    usize_to_ocaml(x as usize)
}

impl ToOcaml for TokenKind {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        u8_to_ocaml(self.ocaml_tag())
    }
}

impl ToOcaml for TriviaKind {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        u8_to_ocaml(self.ocaml_tag())
    }
}

impl ToOcaml for MinimalTrivia {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        // From full_fidelity_minimal_trivia.ml:
        // type t = {
        //   kind: Full_fidelity_trivia_kind.t;
        //   width: int
        // }
        let kind = self.kind.to_ocaml(context);
        let width = usize_to_ocaml(self.width);
        caml_tuple(&[kind, width])
    }
}

impl ToOcaml for MinimalToken {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        let kind = self.kind().to_ocaml(context);
        let width = usize_to_ocaml(self.width());
        let leading = to_list(&self.leading, context);
        let trailing = to_list(&self.trailing, context);

        // From full_fidelity_minimal_token.ml:
        // type t = {
        //   kind: TokenKind.t;
        //   width: int;
        //   leading: Trivia.t list;
        //   trailing: Trivia.t list
        // }
        caml_tuple(&[kind, width, leading, trailing])
    }
}

impl ToOcaml for MinimalValue {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        // From full_fidelity_minimal_syntax.ml:
        // type t = { full_width: int }
        let full_width = usize_to_ocaml(self.full_width);
        caml_tuple(&[full_width])
    }
}

impl<Token, SyntaxValue> ToOcaml for Syntax<Token, SyntaxValue>
where
    Token: LexableToken + ToOcaml,
    SyntaxValue: SyntaxValueType<Token> + ToOcaml,
{
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        let value = self.value.to_ocaml(context);

        let syntax = match &self.syntax {
            SyntaxVariant::Missing => u8_to_ocaml(SyntaxKind::Missing.ocaml_tag()),
            SyntaxVariant::Token(token) => {
                let token_kind = token.kind();
                let token = token.to_ocaml(context);
                caml_block(SyntaxKind::Token(token_kind).ocaml_tag(), &[token])
            }
            SyntaxVariant::SyntaxList(l) => {
                let l = to_list(l, context);
                caml_block(SyntaxKind::SyntaxList.ocaml_tag(), &[l])
            }
            _ => {
                let tag = self.kind().ocaml_tag() as u8;
                let children: Vec<Value> = self
                    .children()
                    .iter()
                    .map(|x| x.to_ocaml(context))
                    .collect();
                caml_block(tag, &children)
            }
        };
        caml_tuple(&[syntax, value])
    }
}

impl ToOcaml for PositionedTrivia {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        // From full_fidelity_positioned_trivia.ml:
        // type t = {
        //   kind: TriviaKind.t;
        //   source_text : SourceText.t;
        //   offset : int;
        //   width : int
        // }
        caml_tuple(&[
            self.kind.to_ocaml(context),
            context.source_text,
            usize_to_ocaml(self.offset),
            usize_to_ocaml(self.width),
        ])
    }
}

// TODO (kasper): we replicate LazyTrivia memory saving bit-packing when converting from Rust to
// OCaml values, but Rust values themselves are not packed. We should consider porting this
// optimization there too.
fn trivia_kind_mask(kind: TriviaKind) -> usize {
    1 << (62 - (kind.ocaml_tag()))
}

fn build_lazy_trivia(trivia_list: &[PositionedTrivia], acc: Option<usize>) -> Option<usize> {
    trivia_list
        .iter()
        .fold(acc, |acc, trivia| match (acc, trivia.kind) {
            (None, _) | (_, TriviaKind::AfterHaltCompiler) | (_, TriviaKind::ExtraTokenError) => {
                None
            }
            (Some(mask), kind) => Some(mask | trivia_kind_mask(kind)),
        })
}

impl ToOcaml for PositionedToken {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        if let Some(res) = context.get(self) {
            return res;
        }

        let kind = self.kind().to_ocaml(context);
        let offset = usize_to_ocaml(self.offset());
        let leading_width = usize_to_ocaml(self.leading_width());
        let width = usize_to_ocaml(self.width());
        let trailing_width = usize_to_ocaml(self.trailing_width());

        let lazy_trivia_mask = Some(0);
        let lazy_trivia_mask = build_lazy_trivia(&self.leading(), lazy_trivia_mask);
        let lazy_trivia_mask = build_lazy_trivia(&self.trailing(), lazy_trivia_mask);

        let trivia = match lazy_trivia_mask {
            Some(mask) => usize_to_ocaml(mask),
            None => {
                //( Trivia.t list * Trivia.t list)
                let leading = to_list(self.leading(), context);
                let trailing = to_list(self.trailing(), context);
                caml_tuple(&[leading, trailing])
            }
        };
        // From full_fidelity_positioned_token.ml:
        // type t = {
        //   kind: TokenKind.t;
        //   source_text: SourceText.t;
        //   offset: int; (* Beginning of first trivia *)
        //   leading_width: int;
        //   width: int; (* Width of actual token, not counting trivia *)
        //   trailing_width: int;
        //   trivia: LazyTrivia.t;
        // }
        let res = caml_tuple(&[
            kind,
            context.source_text,
            offset,
            leading_width,
            width,
            trailing_width,
            trivia,
        ]);
        context.set(Self::clone_rc(self), res);
        res
    }
}

const TOKEN_VALUE_VARIANT: u8 = 0;
const TOKEN_SPAN_VARIANT: u8 = 1;
const MISSING_VALUE_VARIANT: u8 = 2;

impl ToOcaml for PositionedValue {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        match self {
            PositionedValue::TokenValue(t) => {
                let token = t.to_ocaml(context);
                // TokenValue of  ...
                caml_block(TOKEN_VALUE_VARIANT, &[token])
            }
            PositionedValue::TokenSpan { left, right } => {
                let left = left.to_ocaml(context);
                let right = right.to_ocaml(context);
                // TokenSpan { left: Token.t; right: Token.t }
                caml_block(TOKEN_SPAN_VARIANT, &[left, right])
            }
            PositionedValue::Missing { offset } => {
                let offset = usize_to_ocaml(*offset);
                // Missing of {...}
                caml_block(MISSING_VALUE_VARIANT, &[context.source_text, offset])
            }
        }
    }
}

impl ToOcaml for Option<FileMode> {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        match self {
            None => usize_to_ocaml(0),
            Some(x) => {
                let tag: u8 = match x {
                    FileMode::Mphp => 0,
                    FileMode::Mdecl => 1,
                    FileMode::Mstrict => 2,
                    FileMode::Mpartial => 3,
                    FileMode::Mexperimental => 4,
                };
                caml_tuple(&[u8_to_ocaml(tag)])
            }
        }
    }
}

impl ToOcaml for SyntaxError {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        // type error_type = ParseError | RuntimeError
        //
        // type t = {
        //   child        : t option;
        //   start_offset : int;
        //   end_offset   : int;
        //   error_type   : error_type;
        //   message      : string;
        // }

        let child = usize_to_ocaml(0); // None
        let start_offset = usize_to_ocaml(self.start_offset);
        let end_offset = usize_to_ocaml(self.end_offset);
        let error_type = usize_to_ocaml(0); // ParseError

        let m = self.message.as_bytes();
        let message = ocamlpool_reserve_string(m.len());
        let mut str_ = ocaml::Str::from(ocaml::Value::new(message));
        str_.data_mut().copy_from_slice(m);

        caml_tuple(&[child, start_offset, end_offset, error_type, message])
    }
}
