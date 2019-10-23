// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;
use std::num::NonZeroUsize;

use crate::{Allocator, FromError, OcamlRep, Value};

/// Unsafe pointer to an OCaml value which is (possibly) managed by the garbage
/// collector.
///
/// Take care that the value stays rooted or the garbage collector does not run
/// while an UnsafeOcamlPtr wrapper for it exists.
///
/// While this can be used with an ocamlrep::Arena via to_ocamlrep, caution is
/// required--the pointed-to value will *not* be cloned into the Arena, so a
/// data structure containing UnsafeOcamlPtrs which is allocated into an Arena
/// may contain pointers into the OCaml GC-ed heap.
#[repr(transparent)]
#[derive(Clone, Copy, Hash, PartialEq, Eq)]
pub struct UnsafeOcamlPtr(NonZeroUsize);

impl UnsafeOcamlPtr {
    pub unsafe fn new(ptr: usize) -> Self {
        Self(NonZeroUsize::new(ptr).unwrap())
    }

    pub fn as_usize(self) -> usize {
        self.0.get()
    }
}

impl fmt::Debug for UnsafeOcamlPtr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "0x{:x}", self.0)
    }
}

impl OcamlRep for UnsafeOcamlPtr {
    fn to_ocamlrep<'a, A: Allocator<'a>>(&self, _alloc: &mut A) -> Value<'a> {
        unsafe { Value::from_bits(self.0.get()) }
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        if value.is_immediate() {
            return Err(FromError::ExpectedBlock(value.as_int().unwrap()));
        }
        Ok(unsafe { Self::new(value.to_bits()) })
    }
}
