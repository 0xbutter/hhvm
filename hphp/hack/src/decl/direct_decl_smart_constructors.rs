/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use parser_rust as parser;

use flatten_smart_constructors::{FlattenOp, FlattenSmartConstructors};
use oxidized::{
    aast,
    ast_defs::{ClassKind, ConstraintKind, FunKind, Id, Variance},
    errors::Errors,
    file_info::Mode,
    pos::Pos,
    shallow_decl_defs, typing_defs,
    typing_defs::{
        DeclTy, FunArity, FunElt, FunParam, FunParams, FunTparamsKind, FunType, ParamMode,
        PossiblyEnforcedTy, Reactivity, Tparam, Ty, Ty_, TypedefType,
    },
    typing_reason::Reason,
};
use parser::{
    indexed_source_text::IndexedSourceText, lexable_token::LexableToken, token_kind::TokenKind,
};
use std::borrow::Cow;
use std::collections::{HashMap, HashSet};
use std::rc::Rc;

pub use crate::direct_decl_smart_constructors_generated::*;

#[derive(Clone, Debug)]
pub struct InProgressDecls {
    pub classes: HashMap<String, Rc<shallow_decl_defs::ShallowClass>>,
    pub funs: HashMap<String, Rc<typing_defs::FunElt>>,
    pub typedefs: HashMap<String, Rc<typing_defs::TypedefType>>,
    pub consts: HashMap<String, Rc<typing_defs::DeclTy>>,
}

pub fn empty_decls() -> InProgressDecls {
    InProgressDecls {
        classes: HashMap::new(),
        funs: HashMap::new(),
        typedefs: HashMap::new(),
        consts: HashMap::new(),
    }
}

fn mangle_xhp_id(mut name: String) -> String {
    fn ignore_id(name: &str) -> bool {
        name.starts_with("class@anonymous") || name.starts_with("Closure$")
    }

    fn is_xhp(name: &str) -> bool {
        name.chars().next().map_or(false, |c| c == ':')
    }

    if !ignore_id(&name) {
        if is_xhp(&name) {
            name.replace_range(..1, "xhp_")
        }
        name.replace(":", "__").replace("-", "_")
    } else {
        name
    }
}

pub fn get_name(namespace: &str, name: &Node_) -> Result<(String, Pos), String> {
    fn qualified_name_from_parts(
        namespace: &str,
        parts: &Vec<Node_>,
        pos: &Pos,
    ) -> Result<(String, Pos), String> {
        let mut qualified_name = String::new();
        for part in parts {
            match part {
                Node_::Name(name, _pos) => qualified_name.push_str(&name),
                Node_::Backslash(_) => qualified_name.push('\\'),
                Node_::ListItem(listitem) => {
                    if let (Node_::Name(name, _), Node_::Backslash(_)) = &**listitem {
                        qualified_name.push_str(&name);
                        qualified_name.push_str("\\");
                    } else {
                        return Err(format!(
                            "Expected a name or backslash, but got {:?}",
                            listitem
                        ));
                    }
                }
                n => {
                    return Err(format!(
                        "Expected a name, backslash, or list item, but got {:?}",
                        n
                    ))
                }
            }
        }
        let name = if namespace.is_empty() {
            qualified_name // globally qualified name
        } else {
            let namespace = namespace.to_owned();
            let namespace = if namespace.ends_with("\\") {
                namespace
            } else {
                namespace + "\\"
            };
            namespace + &qualified_name
        };
        let pos = pos.clone();
        Ok((name, pos))
    }

    match name {
        Node_::Name(name, pos) => {
            // always a simple name
            let name = name.to_string();
            let name = if namespace.is_empty() {
                name
            } else {
                namespace.to_owned() + "\\" + &name
            };
            let pos = pos.clone();
            Ok((name, pos))
        }
        Node_::XhpName(name, pos) => {
            // xhp names are always unqualified
            let name = name.to_string();
            Ok((mangle_xhp_id(name), pos.clone()))
        }
        Node_::QualifiedName(parts, pos) => qualified_name_from_parts(namespace, &parts, pos),
        n => {
            return Err(format!(
                "Expected a name, XHP name, or qualified name, but got {:?}",
                n
            ))
        }
    }
}

#[derive(Clone, Debug)]
enum NamespaceType {
    Simple(String),
    Delimited(Vec<String>),
}

#[derive(Clone, Debug)]
struct NamespaceBuilder {
    namespace: NamespaceType,
    in_progress_namespace: String,
    is_building_namespace: bool,
}

impl NamespaceBuilder {
    fn new() -> NamespaceBuilder {
        NamespaceBuilder {
            namespace: NamespaceType::Delimited(Vec::new()),
            in_progress_namespace: String::new(),
            is_building_namespace: false,
        }
    }

    fn set_namespace(&mut self) {
        // This clone isn't a perf mistake because we might keep mutating
        // self.in_progress_namespace and we don't want the current namespace to
        // reflect those changes.
        self.namespace = NamespaceType::Simple(self.in_progress_namespace.clone());
    }

    fn push_namespace(&mut self) {
        // If we're currently in a simple namespace, then having a delimited
        // namespace (the kind we'd be in if we're calling this method) is an
        // error. We're not in the business of handling errors in this parser,
        // so just ignore it.
        if let NamespaceType::Delimited(ref mut vec) = self.namespace {
            // This clone isn't a perf mistake because we might keep mutating
            // self.in_progress_namespace and we don't want the namespace stack
            // to reflect those changes.
            vec.push(self.in_progress_namespace.clone());
        }
    }

    fn pop_namespace(&mut self) {
        // If we're currently in a simple namespace, then having a delimited
        // namespace (the kind we'd be in if we're calling this method) is an
        // error. We're not in the business of handling errors in this parser,
        // so just ignore it.
        if let NamespaceType::Delimited(ref mut vec) = self.namespace {
            // This clone isn't a perf mistake because we might keep mutating
            // self.in_progress_namespace and we don't want the namespace stack
            // to reflect those changes.
            // Also, while we normally try to use the Result type to surface
            // errors that we see as part of the file being malformed, the only
            // way we can hit this pop without having hit the push first is
            // through coding error, so we panic instead.
            vec.pop()
                .expect("Attempted to pop from the namespace stack when there were no entries");
            self.in_progress_namespace = self.current_namespace().to_string();
        }
    }

    fn current_namespace(&self) -> &str {
        match &self.namespace {
            NamespaceType::Simple(s) => &s,
            NamespaceType::Delimited(stack) => stack.last().map(|ns| ns.as_str()).unwrap_or(""),
        }
    }
}

#[derive(Clone, Debug)]
pub struct State<'a> {
    pub source_text: IndexedSourceText<'a>,
    pub decls: Cow<'a, InProgressDecls>,
    namespace_builder: Cow<'a, NamespaceBuilder>,
    yields: Cow<'a, Vec<bool>>,
}

impl<'a> State<'a> {
    pub fn new(source_text: IndexedSourceText) -> State {
        State {
            source_text,
            decls: Cow::Owned(empty_decls()),
            namespace_builder: Cow::Owned(NamespaceBuilder::new()),
            yields: Cow::Owned(Vec::new()),
        }
    }
}

#[derive(Clone, Debug)]
pub enum HintValue {
    Void,
    Int,
    Bool,
    Float,
    String,
    Num,
    ArrayKey,
    NoReturn,
    Apply(Box<(Id, Vec<Node_>)>),
}

#[derive(Clone, Debug)]
pub struct VariableDecl {
    hint: Node_,
    name: String,
    pos: Pos,
}

#[derive(Clone, Debug)]
pub struct FunctionDecl {
    name: Node_,
    modifiers: Node_,
    type_params: Node_,
    param_list: Node_,
    ret_hint: Node_,
}

#[derive(Clone, Debug)]
pub enum Node_ {
    List(Vec<Node_>),
    BracketedList(Box<(Pos, Vec<Node_>, Pos)>),
    Ignored,
    Name(String, Pos),
    XhpName(String, Pos),
    QualifiedName(Vec<Node_>, Pos),
    Hint(HintValue, Pos),
    Backslash(Pos), // This needs a pos since it shows up in names.
    ListItem(Box<(Node_, Node_)>),
    Variable(Box<VariableDecl>),
    FunctionHeader(Box<FunctionDecl>),
    TypeConstraint(Box<(ConstraintKind, Node_)>),
    LessThan(Pos),    // This needs a pos since it shows up in generics.
    GreaterThan(Pos), // This needs a pos since it shows up in generics.

    // Box the insides of the vector so we don't need to reallocate them when
    // we pull them out of the TypeConstraint variant.
    TypeParameter(Box<(Node_, Vec<Box<(ConstraintKind, Node_)>>)>),
    As,
    Super,
    Async,
}

impl Node_ {
    pub fn get_pos(&self) -> Result<Pos, String> {
        match self {
            Node_::Name(_, pos) => Ok(pos.clone()),
            Node_::Hint(_, pos) => Ok(pos.clone()),
            Node_::Backslash(pos) | Node_::LessThan(pos) | Node_::GreaterThan(pos) => {
                Ok(pos.clone())
            }
            Node_::ListItem(items) => {
                let fst = &items.0;
                let snd = &items.1;
                match (fst.get_pos(), snd.get_pos()) {
                    (Ok(fst_pos), Ok(snd_pos)) => Pos::merge(&fst_pos, &snd_pos),
                    (Ok(pos), Err(_)) => Ok(pos),
                    (Err(_), Ok(pos)) => Ok(pos),
                    (Err(_), Err(_)) => Err(format!("No pos found for {:?} or {:?}", fst, snd)),
                }
            }
            Node_::List(items) => self.pos_from_vec(&items),
            Node_::BracketedList(innards) => {
                let (first_pos, inner_list, second_pos) = &**innards;
                Pos::merge(
                    &first_pos,
                    &Pos::merge(&self.pos_from_vec(&inner_list)?, &second_pos)?,
                )
            }
            _ => Err(format!("No pos found for node {:?}", self)),
        }
    }

    fn pos_from_vec(&self, nodes: &Vec<Node_>) -> Result<Pos, String> {
        nodes.iter().fold(
            Err(format!("No pos found for any children under {:?}", self)),
            |acc, elem| match (acc, elem.get_pos()) {
                (Ok(acc_pos), Ok(elem_pos)) => Pos::merge(&acc_pos, &elem_pos),
                (Err(_), Ok(elem_pos)) => Ok(elem_pos),
                (acc, Err(_)) => acc,
            },
        )
    }

    fn into_vec(self) -> Vec<Self> {
        match self {
            Node_::List(items) => items,
            Node_::BracketedList(innards) => {
                let (_, items, _) = *innards;
                items
            }
            Node_::Ignored => Vec::new(),
            n => vec![n],
        }
    }
}

pub type Node = Result<Node_, String>;

impl DirectDeclSmartConstructors<'_> {
    fn node_to_ty(&self, node: &Node_, type_variables: &HashSet<String>) -> Result<Ty, String> {
        match node {
            Node_::Hint(hv, pos) => {
                let reason = Reason::Rhint(pos.clone());
                let ty_ = match hv {
                    HintValue::Void => Ty_::Tprim(aast::Tprim::Tvoid),
                    HintValue::Int => Ty_::Tprim(aast::Tprim::Tint),
                    HintValue::Bool => Ty_::Tprim(aast::Tprim::Tbool),
                    HintValue::Float => Ty_::Tprim(aast::Tprim::Tfloat),
                    HintValue::String => Ty_::Tprim(aast::Tprim::Tstring),
                    HintValue::Num => Ty_::Tprim(aast::Tprim::Tnum),
                    HintValue::ArrayKey => Ty_::Tprim(aast::Tprim::Tarraykey),
                    HintValue::NoReturn => Ty_::Tprim(aast::Tprim::Tnoreturn),
                    HintValue::Apply(innards) => {
                        let (id, inner_types) = &**innards;
                        Ty_::Tapply(
                            id.clone(),
                            inner_types
                                .into_iter()
                                .map(|node| self.node_to_ty(&node, type_variables))
                                .collect::<Result<Vec<_>, String>>()?,
                        )
                    }
                };
                Ok(Ty(reason, Box::new(ty_)))
            }
            node => {
                let (name, pos) = get_name("", node)?;
                let reason = Reason::Rhint(pos.clone());
                let ty_ = if type_variables.contains(&name) {
                    Ty_::Tgeneric(name)
                } else {
                    let name = self.prefix_ns(name);
                    Ty_::Tapply(Id(pos, name), Vec::new())
                };
                Ok(Ty(reason, Box::new(ty_)))
            }
        }
    }

    // I'd really prefer that this function take a &str instead of a String, but
    // I wanted to avoid having to unnecessarily copy it in the cases where it
    // already starts with a backslash.
    fn prefix_ns(&self, name: String) -> String {
        if name.starts_with("\\") {
            name
        } else if self.state.namespace_builder.current_namespace().is_empty() {
            "\\".to_owned() + &name
        } else {
            "\\".to_owned() + self.state.namespace_builder.current_namespace() + "\\" + &name
        }
    }

    // I'd really prefer that this function take a &str instead of a String, but
    // I wanted to avoid having to unnecessarily copy it in the cases where it
    // already starts with a backslash.
    fn prefix_slash(&self, name: String) -> String {
        if name.starts_with("\\") {
            name
        } else {
            "\\".to_owned() + &name
        }
    }

    fn into_variables_list(
        &self,
        list: Node_,
        type_variables: &HashSet<String>,
    ) -> Result<FunParams<DeclTy>, String> {
        match list {
            Node_::List(nodes) => {
                nodes
                    .into_iter()
                    .fold(Ok(Vec::new()), |acc, variable| match (acc, variable) {
                        (Ok(mut variables), Node_::Variable(innards)) => {
                            let VariableDecl { hint, name, pos } = *innards;
                            variables.push(FunParam {
                                pos,
                                name: Some(name),
                                type_: PossiblyEnforcedTy {
                                    enforced: false,
                                    type_: self.node_to_ty(&hint, type_variables)?,
                                },
                                kind: ParamMode::FPnormal,
                                accept_disposable: false,
                                mutability: None,
                                rx_annotation: None,
                            });
                            Ok(variables)
                        }
                        (Ok(_), n) => Err(format!("Expected a variable, but got {:?}", n)),
                        (acc @ Err(_), _) => acc,
                    })
            }
            Node_::Ignored => Ok(Vec::new()),
            n => Err(format!("Expected a list of variables, but got {:?}", n)),
        }
    }
}

impl<'a> FlattenOp for DirectDeclSmartConstructors<'_> {
    type S = Node;

    fn flatten(lst: Vec<Self::S>) -> Self::S {
        let mut r = lst
            .into_iter()
            .map(|s| match s {
                Ok(Node_::List(children)) => children.into_iter().map(|x| Ok(x)).collect(),
                x => {
                    if Self::is_zero(&x) {
                        vec![]
                    } else {
                        vec![x]
                    }
                }
            })
            .flatten()
            .collect::<Result<Vec<_>, String>>()?;
        Ok(match r.as_slice() {
            [] => Node_::Ignored,
            [_] => r.pop().unwrap(),
            _ => Node_::List(r),
        })
    }

    fn zero() -> Self::S {
        Ok(Node_::Ignored)
    }

    fn is_zero(s: &Self::S) -> bool {
        s.is_ok()
    }
}

impl<'a> FlattenSmartConstructors<'a, State<'a>> for DirectDeclSmartConstructors<'a> {
    fn make_token(&mut self, token: Self::Token) -> Self::R {
        let token_text = || {
            String::from_utf8_lossy(self.state.source_text.source_text().sub(
                token.leading_start_offset().unwrap_or(0) + token.leading_width(),
                token.width(),
            ))
            .to_string()
        };
        let token_pos = || {
            self.state
                .source_text
                .relative_pos(token.start_offset(), token.end_offset() + 1)
        };
        let kind = token.kind();

        // So... this part gets pretty disgusting. We're setting a lot of parser
        // state in here, and we're unsetting a lot of it down in the actual
        // make_namespace_XXXX methods. I tried a few other ways of handling
        // namespaces, but ultimately rejected them.
        //
        // The reason we even need this code at all is because we insert entries
        // into our state.decls struct as soon as we see them, but because the
        // FFP calls its make_XXXX functions in inside out order (which is the
        // most reasonable thing for a parser to do), we don't actually know
        // which namespace we're in until we leave it, at which point we've
        // already inserted all of the decls from inside that namespace.
        //
        // One idea I attempted was having both the state.decls struct as well
        // as a new state.in_progress_decls struct, and if we hit a
        // make_namespace_XXXX method, we would rename every decl within that
        // struct to have the correct namespace, then merge
        // state.in_progress_decls into state.decls. The problem that I ran into
        // with this approach had to do with the insides of the decls. While
        // it's easy enough to rename every decl's name, it's much harder to dig
        // into each decl and rename every single Tapply, which we have to do if
        // a decl references a type within the same namespace.
        //
        // The facts parser, meanwhile, solves this problem by keeping track of
        // a micro-AST, and only inserting names into the returned facts struct
        // at the very end. This would be convenient for us, but it would
        // require going back to allocating and returning a micro-AST, then
        // walking over that again, instead of building up the decls struct as
        // we go, which is what we do now. Unfortunately, since decls are much
        // more complex than facts (which are just lists of toplevel symbols),
        // the cost of doing things that way is too high for our purposes.
        //
        // So we can't allocate sub-trees for each namespace, and we can't just
        // wait until we hit make_namespace_XXXX then rewrite all the names, so
        // what can we do? The answer is that we can encode a really gross
        // namespace-tracking state machine inside of our parser, and take
        // advantage of some mildly unspoken quasi-implementation details. We
        // always call the make_XXXX methods after we finish the full parsing of
        // whatever we're making... but since make_token() only depends on a
        // single token, it gets called *before* we enter the namespace!
        //
        // The general idea, then, is that when we see a namespace token we know
        // that we're about to parse a namespace name, so we enter a state where
        // we know we're building up a namespace name (as opposed to the many,
        // many other kinds of names). While we're in that state, every name and
        // backslash token gets appended to the in-progress namespace name. Once
        // we see either an opening curly brace or a semicolon, we know that
        // we've finished building the namespace name, so we exit the "building
        // a namespace" state and push the newly created namespace onto our
        // namespace stack. Then, in all of the various make_namespace_XXXX
        // methods, we pop the namespace (since we know we've just exited it).
        //
        // So there we have it. Fully inline namespace tracking, and all it cost
        // was a little elbow grease and a lot of dignity.
        Ok(match kind {
            TokenKind::Name | TokenKind::Variable => {
                let name = token_text();
                if self.state.namespace_builder.is_building_namespace {
                    self.state
                        .namespace_builder
                        .to_mut()
                        .in_progress_namespace
                        .push_str(&name.to_string());
                    Node_::Ignored
                } else {
                    Node_::Name(name, token_pos())
                }
            }
            TokenKind::XHPClassName => Node_::XhpName(token_text(), token_pos()),
            TokenKind::String => Node_::Hint(HintValue::String, token_pos()),
            TokenKind::Int => Node_::Hint(HintValue::Int, token_pos()),
            TokenKind::Float => Node_::Hint(HintValue::Float, token_pos()),
            TokenKind::Double => Node_::Hint(
                HintValue::Apply(Box::new((Id(token_pos(), token_text()), Vec::new()))),
                token_pos(),
            ),
            TokenKind::Num => Node_::Hint(HintValue::Num, token_pos()),
            TokenKind::Bool => Node_::Hint(HintValue::Bool, token_pos()),
            TokenKind::Boolean => Node_::Hint(
                HintValue::Apply(Box::new((Id(token_pos(), token_text()), Vec::new()))),
                token_pos(),
            ),
            TokenKind::Void => Node_::Hint(HintValue::Void, token_pos()),
            TokenKind::Backslash => {
                if self.state.namespace_builder.is_building_namespace {
                    self.state
                        .namespace_builder
                        .to_mut()
                        .in_progress_namespace
                        .push('\\');
                    Node_::Ignored
                } else {
                    Node_::Backslash(token_pos())
                }
            }
            TokenKind::LessThan => Node_::LessThan(token_pos()),
            TokenKind::GreaterThan => Node_::GreaterThan(token_pos()),
            TokenKind::As => Node_::As,
            TokenKind::Super => Node_::Super,
            TokenKind::Async => Node_::Async,
            TokenKind::Function => {
                self.state.yields.to_mut().push(false);
                Node_::Ignored
            }
            TokenKind::Yield => {
                // First pop the one that we pushed when we saw the function keyword.
                self.state.yields.to_mut().pop();
                self.state.yields.to_mut().push(true);
                Node_::Ignored
            }
            TokenKind::Namespace => {
                self.state.namespace_builder.to_mut().is_building_namespace = true;
                if !self.state.namespace_builder.current_namespace().is_empty() {
                    self.state
                        .namespace_builder
                        .to_mut()
                        .in_progress_namespace
                        .push('\\')
                }
                Node_::Ignored
            }
            TokenKind::LeftBrace | TokenKind::Semicolon => {
                if self.state.namespace_builder.is_building_namespace {
                    if kind == TokenKind::LeftBrace {
                        self.state.namespace_builder.to_mut().push_namespace();
                    } else {
                        self.state.namespace_builder.to_mut().set_namespace();
                    }
                    self.state.namespace_builder.to_mut().is_building_namespace = false;
                }
                Node_::Ignored
            }
            _ => Node_::Ignored,
        })
    }

    fn make_missing(&mut self, _: usize) -> Self::R {
        Ok(Node_::Ignored)
    }

    fn make_list(&mut self, items: Vec<Self::R>, _: usize) -> Self::R {
        let result = if !items.is_empty()
            && !items.iter().all(|r| match r {
                Ok(Node_::Ignored) => true,
                _ => false,
            }) {
            let items = items.into_iter().collect::<Result<Vec<_>, String>>()?;
            Node_::List(items)
        } else {
            Node_::Ignored
        };
        Ok(result)
    }

    fn make_qualified_name(&mut self, arg0: Self::R) -> Self::R {
        let arg0 = arg0?;
        let pos = arg0.get_pos();
        Ok(match arg0 {
            Node_::Ignored => Node_::Ignored,
            Node_::List(nodes) => Node_::QualifiedName(nodes, pos?),
            node => Node_::QualifiedName(vec![node], pos?),
        })
    }

    fn make_simple_type_specifier(&mut self, arg0: Self::R) -> Self::R {
        // Return this explicitly because flatten filters out non-error nodes.
        arg0
    }

    fn make_simple_initializer(&mut self, _arg0: Self::R, arg1: Self::R) -> Self::R {
        arg1
    }

    fn make_list_item(&mut self, item: Self::R, sep: Self::R) -> Self::R {
        Ok(match (item?, sep?) {
            (Node_::Ignored, Node_::Ignored) => Node_::Ignored,
            (x, Node_::Ignored) | (Node_::Ignored, x) => x,
            (x, y) => Node_::ListItem(Box::new((x, y))),
        })
    }

    fn make_type_arguments(
        &mut self,
        less_than: Self::R,
        arguments: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        Ok(Node_::BracketedList(Box::new((
            less_than?.get_pos()?,
            arguments?.into_vec(),
            greater_than?.get_pos()?,
        ))))
    }

    fn make_generic_type_specifier(
        &mut self,
        class_type: Self::R,
        argument_list: Self::R,
    ) -> Self::R {
        let (class_type, argument_list) = (class_type?, argument_list?);
        let (class_type, pos) = get_name(
            self.state.namespace_builder.current_namespace(),
            &class_type,
        )?;
        let full_pos = match argument_list.get_pos() {
            Ok(p2) => Pos::merge(&pos, &p2)?,
            Err(_) => pos.clone(),
        };
        Ok(Node_::Hint(
            HintValue::Apply(Box::new((
                Id(pos, "\\".to_string() + &class_type),
                argument_list.into_vec(),
            ))),
            full_pos,
        ))
    }

    fn make_alias_declaration(
        &mut self,
        _attributes: Self::R,
        _keyword: Self::R,
        name: Self::R,
        _generic_params: Self::R,
        _constraint: Self::R,
        _equal: Self::R,
        aliased_type: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let (name, aliased_type) = (name?, aliased_type?);
        match name {
            Node_::Ignored => (),
            _ => {
                let (name, pos) =
                    get_name(self.state.namespace_builder.current_namespace(), &name)?;
                match self.node_to_ty(&aliased_type, &HashSet::new()) {
                    Ok(ty) => {
                        self.state.decls.to_mut().typedefs.insert(
                            name,
                            Rc::new(TypedefType {
                                pos,
                                vis: aast::TypedefVisibility::Transparent,
                                tparams: Vec::new(),
                                constraint: None,
                                type_: ty,
                                decl_errors: None,
                            }),
                        );
                    }
                    Err(msg) => {
                        return Err(format!(
                            "Expected hint or name for type alias {}, but was {:?} ({})",
                            name, aliased_type, msg
                        ))
                    }
                }
            }
        };
        Ok(Node_::Ignored)
    }

    fn make_type_constraint(&mut self, kind: Self::R, value: Self::R) -> Self::R {
        let kind = match kind? {
            Node_::As => ConstraintKind::ConstraintAs,
            Node_::Super => ConstraintKind::ConstraintSuper,
            n => return Err(format!("Expected either As or Super, but was {:?}", n)),
        };
        Ok(Node_::TypeConstraint(Box::new((kind, value?))))
    }

    fn make_type_parameter(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        name: Self::R,
        constraints: Self::R,
    ) -> Self::R {
        let constraints = constraints?.into_vec().into_iter().fold(
            Ok(Vec::new()),
            |acc: Result<Vec<_>, String>, node| match acc {
                acc @ Err(_) => acc,
                Ok(mut acc) => match node {
                    Node_::TypeConstraint(innards) => {
                        acc.push(innards);
                        Ok(acc)
                    }
                    Node_::Ignored => Ok(acc),
                    n => Err(format!("Expected a type constraint, but was {:?}", n)),
                },
            },
        )?;
        Ok(Node_::TypeParameter(Box::new((name?, constraints))))
    }

    fn make_type_parameters(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        Ok(Node_::BracketedList(Box::new((
            arg0?.get_pos()?,
            arg1?.into_vec(),
            arg2?.get_pos()?,
        ))))
    }

    fn make_parameter_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        hint: Self::R,
        name: Self::R,
        _arg5: Self::R,
    ) -> Self::R {
        let (name, pos) = get_name("", &name?)?;
        let hint = hint?;
        Ok(Node_::Variable(Box::new(VariableDecl { hint, name, pos })))
    }

    fn make_function_declaration(
        &mut self,
        _attributes: Self::R,
        header: Self::R,
        _body: Self::R,
    ) -> Self::R {
        Ok(match header? {
            Node_::FunctionHeader(decl) => {
                let (name, pos) =
                    get_name(self.state.namespace_builder.current_namespace(), &decl.name)?;
                let mut type_variables = HashSet::new();
                let type_params = decl
                    .type_params
                    .into_vec()
                    .into_iter()
                    .map(|node| {
                        let (name, constraints) = match node {
                            Node_::TypeParameter(innards) => *innards,
                            n => return Err(format!("Expected a type parameter, but got {:?}", n)),
                        };
                        let (name, pos) = get_name("", &name)?;
                        let constraints = constraints
                            .into_iter()
                            .map(|constraint| {
                                let (kind, value) = *constraint;
                                Ok((kind, self.node_to_ty(&value, &HashSet::new())?))
                            })
                            .collect::<Result<Vec<_>, String>>()?;
                        type_variables.insert(name.clone());
                        Ok(Tparam {
                            variance: Variance::Invariant,
                            name: Id(pos, name),
                            constraints,
                            reified: aast::ReifyKind::Erased,
                            user_attributes: Vec::new(),
                        })
                    })
                    .collect::<Result<Vec<_>, String>>()?;
                let params = self.into_variables_list(decl.param_list, &type_variables)?;
                let type_ = self.node_to_ty(&decl.ret_hint, &type_variables)?;
                let modifiers = decl.modifiers.into_vec();
                let async_ = modifiers.iter().any(|node| match node {
                    Node_::Async => true,
                    _ => false,
                });
                let fun_kind = if self
                    .state
                    .yields
                    .to_mut()
                    .pop()
                    .expect("Attempted to pop from an empty yield stack")
                {
                    if async_ {
                        FunKind::FAsyncGenerator
                    } else {
                        FunKind::FGenerator
                    }
                } else {
                    if async_ {
                        FunKind::FAsync
                    } else {
                        FunKind::FSync
                    }
                };
                self.state.decls.to_mut().funs.insert(
                    name,
                    Rc::new(FunElt {
                        deprecated: None,
                        type_: Ty(
                            Reason::Rwitness(pos.clone()),
                            Box::new(Ty_::Tfun(FunType {
                                is_coroutine: false,
                                arity: FunArity::Fstandard(
                                    params.len() as isize,
                                    params.len() as isize,
                                ),
                                tparams: (type_params, FunTparamsKind::FTKtparams),
                                where_constraints: Vec::new(),
                                params,
                                ret: PossiblyEnforcedTy {
                                    enforced: false,
                                    type_,
                                },
                                fun_kind,
                                reactive: Reactivity::Nonreactive,
                                return_disposable: false,
                                mutability: None,
                                returns_mutable: false,
                                returns_void_to_rx: false,
                            })),
                        ),
                        decl_errors: None,
                        pos,
                    }),
                );
                Node_::Ignored
            }
            _ => Node_::Ignored,
        })
    }

    fn make_function_declaration_header(
        &mut self,
        modifiers: Self::R,
        _keyword: Self::R,
        name: Self::R,
        type_params: Self::R,
        _left_parens: Self::R,
        param_list: Self::R,
        _right_parens: Self::R,
        _colon: Self::R,
        ret_hint: Self::R,
        _where: Self::R,
    ) -> Self::R {
        Ok(match name? {
            Node_::Ignored => Node_::Ignored,
            name => Node_::FunctionHeader(Box::new(FunctionDecl {
                name,
                modifiers: modifiers?,
                type_params: type_params?,
                param_list: param_list?,
                ret_hint: ret_hint?,
            })),
        })
    }

    fn make_const_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        decls: Self::R,
        _arg4: Self::R,
    ) -> Self::R {
        // None of the Node_::Ignoreds should happen in a well-formed file, but they could happen in
        // a malformed one.
        let hint = hint?;
        Ok(match decls? {
            Node_::List(nodes) => match nodes.as_slice() {
                [Node_::List(nodes)] => match nodes.as_slice() {
                    [name, _initializer] => {
                        let (name, _) =
                            get_name(self.state.namespace_builder.current_namespace(), name)?;
                        match self.node_to_ty(&hint, &HashSet::new()) {
                            Ok(ty) => self.state.decls.to_mut().consts.insert(name, Rc::new(ty)),
                            Err(msg) => {
                                return Err(format!(
                                    "Expected hint or name for constant {}, but was {:?} ({})",
                                    name, hint, msg
                                ))
                            }
                        };
                        Node_::Ignored
                    }
                    _ => Node_::Ignored,
                },
                _ => Node_::Ignored,
            },
            _ => Node_::Ignored,
        })
    }

    fn make_constant_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        let (name, initializer) = (name?, initializer?);
        Ok(match name {
            Node_::Ignored => Node_::Ignored,
            _ => Node_::List(vec![name, initializer]),
        })
    }

    fn make_namespace_declaration(
        &mut self,
        _keyword: Self::R,
        _name: Self::R,
        _body: Self::R,
    ) -> Self::R {
        self.state.namespace_builder.to_mut().pop_namespace();
        Ok(Node_::Ignored)
    }

    fn make_classish_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        name: Self::R,
        _arg4: Self::R,
        _arg5: Self::R,
        _arg6: Self::R,
        _arg7: Self::R,
        _arg8: Self::R,
        _arg9: Self::R,
        _arg10: Self::R,
    ) -> Self::R {
        let (name, pos) = get_name(self.state.namespace_builder.current_namespace(), &name?)?;
        let key = name.clone();
        let name = self.prefix_slash(name);
        self.state.decls.to_mut().classes.insert(
            key,
            Rc::new(shallow_decl_defs::ShallowClass {
                mode: Mode::Mstrict,
                final_: false,
                is_xhp: false,
                kind: ClassKind::Cnormal,
                name: Id(pos, name),
                tparams: Vec::new(),
                where_constraints: Vec::new(),
                extends: Vec::new(),
                uses: Vec::new(),
                method_redeclarations: Vec::new(),
                xhp_attr_uses: Vec::new(),
                req_extends: Vec::new(),
                req_implements: Vec::new(),
                implements: Vec::new(),
                consts: Vec::new(),
                typeconsts: Vec::new(),
                pu_enums: Vec::new(),
                props: Vec::new(),
                sprops: Vec::new(),
                constructor: None,
                static_methods: Vec::new(),
                methods: Vec::new(),
                user_attributes: Vec::new(),
                enum_type: None,
                decl_errors: Errors::empty(),
            }),
        );
        Ok(Node_::Ignored)
    }
}
