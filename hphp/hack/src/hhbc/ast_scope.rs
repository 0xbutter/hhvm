// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::{ast, ast_defs::FunKind, pos::Pos, s_map};
use rx_rust as rx;
use std::borrow::Cow;

#[derive(Clone)]
pub struct LongLambda {
    pub is_static: bool,
    pub is_async: bool,
    pub rx_level: Option<rx::Level>,
}

#[derive(Clone)]
pub struct Lambda {
    pub is_async: bool,
    pub rx_level: Option<rx::Level>,
}

#[derive(Clone)]
pub enum ScopeItem {
    Class(ast::Class_),
    Function(ast::Fun_),
    Method(ast::Method_),
    LongLambda(LongLambda),
    Lambda(Lambda),
}

impl ScopeItem {
    pub fn is_in_lambda(&self) -> bool {
        match self {
            ScopeItem::Lambda(_) | ScopeItem::LongLambda(_) => true,
            _ => false,
        }
    }
}

#[derive(Clone)]
pub struct Scope {
    items: Vec<ScopeItem>,
}

impl Scope {
    pub fn toplevel() -> Self {
        Scope { items: vec![] }
    }

    pub fn push_item(&mut self, s: ScopeItem) {
        self.items.push(s)
    }

    pub fn iter(&self) -> impl ExactSizeIterator<Item = &ScopeItem> {
        self.items.iter().rev()
    }

    pub fn iter_subscopes(&self) -> impl Iterator<Item = &[ScopeItem]> {
        (1..self.items.len()).rev().map(move |i| &self.items[..i])
    }

    pub fn get_subscope_class(sub_scope: &[ScopeItem]) -> Option<&ast::Class_> {
        for scope_item in sub_scope.iter().rev() {
            if let ScopeItem::Class(cd) = scope_item {
                return Some(&cd);
            }
        }
        None
    }

    pub fn get_class(&self) -> Option<&ast::Class_> {
        Self::get_subscope_class(&self.items[..])
    }

    pub fn get_span(&self) -> Pos {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(cd) => {
                    return cd.span.clone();
                }
                ScopeItem::Function(fd) => {
                    return fd.span.clone();
                }
                ScopeItem::Method(md) => {
                    return md.span.clone();
                }
                _ => (),
            }
        }
        Pos::make_none()
    }

    pub fn get_tparams(&self) -> Vec<ast::Tparam> {
        let mut tparams = vec![];
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(cd) => {
                    tparams.extend_from_slice(cd.tparams.list.as_slice());
                }
                ScopeItem::Function(fd) => {
                    tparams.extend_from_slice(fd.tparams.as_slice());
                }
                ScopeItem::Method(md) => {
                    tparams.extend_from_slice(md.tparams.as_slice());
                }
                _ => (),
            }
        }
        tparams
    }

    pub fn get_fun_tparams(&self) -> Option<&Vec<ast::Tparam>> {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return None;
                }
                ScopeItem::Function(fd) => {
                    return Some(&fd.tparams);
                }
                ScopeItem::Method(md) => {
                    return Some(&md.tparams);
                }
                _ => (),
            }
        }
        None
    }

    pub fn get_class_params(&self) -> Cow<ast::ClassTparams> {
        for scope_item in self.items.iter().rev() {
            if let ScopeItem::Class(cd) = scope_item {
                return Cow::Borrowed(&cd.tparams);
            }
        }
        Cow::Owned(ast::ClassTparams {
            list: vec![],
            constraints: s_map::SMap::new(),
        })
    }

    pub fn has_this(&self) -> bool {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(_) | ScopeItem::Function(_) => {
                    return false;
                }
                ScopeItem::Method(_) => {
                    return true;
                }
                _ => (),
            }
        }
        false
    }

    pub fn is_in_async(&self) -> bool {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return false;
                }
                ScopeItem::Method(ast::Method_ { fun_kind, .. })
                | ScopeItem::Function(ast::Fun_ { fun_kind, .. }) => {
                    return *fun_kind == FunKind::FAsync || *fun_kind == FunKind::FAsyncGenerator;
                }
                _ => (),
            }
        }
        false
    }

    pub fn is_toplevel(&self) -> bool {
        self.items.is_empty()
    }

    pub fn is_in_static_method(&self) -> bool {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Method(md) => {
                    return md.static_;
                }
                ScopeItem::LongLambda(LongLambda { is_static, .. }) => {
                    if *is_static {
                        return false;
                    }
                }
                ScopeItem::Lambda(_) => (),
                _ => return false,
            }
        }
        false
    }

    pub fn is_in_lambda(&self) -> bool {
        self.items
            .last()
            .map(&ScopeItem::is_in_lambda)
            .unwrap_or(false)
    }

    pub fn rx_of_scope(&self) -> rx::Level {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return rx::Level::NonRx;
                }
                ScopeItem::Method(ast::Method_ {
                    user_attributes, ..
                })
                | ScopeItem::Function(ast::Fun_ {
                    user_attributes, ..
                }) => {
                    return rx::Level::from_ast(user_attributes).unwrap_or(rx::Level::NonRx);
                }
                ScopeItem::Lambda(Lambda {
                    rx_level: Some(rl), ..
                })
                | ScopeItem::LongLambda(LongLambda {
                    rx_level: Some(rl), ..
                }) => {
                    return *rl;
                }
                _ => (),
            }
        }
        rx::Level::NonRx
    }

    pub fn has_function_attribute(&self, attr_name: String) -> bool {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Method(ast::Method_ {
                    user_attributes, ..
                })
                | ScopeItem::Function(ast::Fun_ {
                    user_attributes, ..
                }) => {
                    return user_attributes.iter().any(|attr| attr.name.1 == attr_name);
                }
                _ => (),
            }
        }
        false
    }

    pub fn is_static(&self) -> bool {
        for x in self.items.iter().rev() {
            match x {
                ScopeItem::LongLambda(x) => {
                    if x.is_static {
                        return true;
                    }
                }
                ScopeItem::Function(_) => return true,
                ScopeItem::Method(md) => return md.static_,
                ScopeItem::Lambda(_) => continue,
                _ => return true,
            }
        }
        true
    }
}
