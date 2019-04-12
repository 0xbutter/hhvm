/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use std::marker::PhantomData;

use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::parser_trait::{Context, ParserTrait};
use crate::smart_constructors::SmartConstructors;
use crate::syntax_error::SyntaxError;

pub struct ExpressionParser<'a, S>
where
    S: SmartConstructors,
{
    lexer: Lexer<'a, S::Token>,
    env: ParserEnv,
    context: Context<S::Token>,
    errors: Vec<SyntaxError>,
    precedence: usize,
    allow_as_expressions: bool,
    _phantom: PhantomData<S>,
}

impl<'a, S> std::clone::Clone for ExpressionParser<'a, S>
where
    S: SmartConstructors,
{
    fn clone(&self) -> Self {
        Self {
            lexer: self.lexer.clone(),
            context: self.context.clone(),
            env: self.env.clone(),
            errors: self.errors.clone(),
            precedence: self.precedence,
            _phantom: self._phantom.clone(),
            allow_as_expressions: self.allow_as_expressions,
        }
    }
}

impl<'a, S> ParserTrait<'a, S> for ExpressionParser<'a, S>
where
    S: SmartConstructors,
{
    fn make(
        lexer: Lexer<'a, S::Token>,
        env: ParserEnv,
        context: Context<S::Token>,
        errors: Vec<SyntaxError>,
    ) -> Self {
        Self {
            lexer,
            env,
            precedence: 0,
            context,
            errors,
            allow_as_expressions: true,
            _phantom: PhantomData,
        }
    }

    fn into_parts(self) -> (Lexer<'a, S::Token>, Context<S::Token>, Vec<SyntaxError>) {
        (self.lexer, self.context, self.errors)
    }

    fn lexer(&self) -> &Lexer<'a, S::Token> {
        &self.lexer
    }

    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::Token> {
        &mut self.lexer
    }

    fn continue_from<P: ParserTrait<'a, S>>(&mut self, other: P) {
        let (lexer, context, errors) = other.into_parts();
        self.lexer = lexer;
        self.context = context;
        self.errors = errors;
    }

    fn add_error(&mut self, error: SyntaxError) {
        self.errors.push(error)
    }

    fn skipped_tokens_mut(&mut self) -> &mut Vec<S::Token> {
        &mut self.context.skipped_tokens
    }

    fn skipped_tokens(&self) -> &[S::Token] {
        &self.context.skipped_tokens
    }

    fn context_mut(&mut self) -> &mut Context<S::Token> {
        &mut self.context
    }

    fn context(&self) -> &Context<S::Token> {
        &self.context
    }
}
