// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod arena;
mod block;
mod error;
mod impls;
mod slab;
mod value;

pub mod from;
pub mod rc;

pub use arena::Arena;
pub use block::Block;
pub use error::{FromError, SlabIntegrityError};
pub use slab::OwnedSlab;
pub use value::{OpaqueValue, Value};

pub trait OcamlRep: Sized {
    fn to_ocamlrep<'a>(&self, arena: &mut Arena<'a>) -> Value<'a>;
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError>;

    unsafe fn from_ocaml(value: usize) -> Result<Self, FromError> {
        Self::from_ocamlrep(Value::from_bits(value))
    }
}
