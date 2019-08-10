// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ord;

#[derive(Clone, Debug)]
enum SetImpl<T: Ord> {
    Empty,
    Node {
        l: Box<SetImpl<T>>,
        v: T,
        r: Box<SetImpl<T>>,
        h: usize,
    },
}

use SetImpl::*;

#[derive(Clone, Debug)]
pub struct Set<T: Ord>(SetImpl<T>);

impl<T: Ord> Set<T> {
    pub fn empty() -> Self {
        Set(Empty)
    }
}
