// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! HackC emitter options.
//!
//! Canonical name for each option is chosen to be JSON key, which often
//! differs from the ones passed as CLI arguments via `-v KEY=VALUE`. The
//! names for arguments (non-flags) are derived via the `serde` framework
//! by taking the name of the field (or the one given via serde(rename=new_name)),
//! and the names for flags (boolean options) are derived by downcasing bitflags names.
//! The options are grouped their common prefix in their canonical names,
//! which is specified via macros `prefix_all` or `prefixed_flags`, respectively.
//! E.g., `prefix_all("hhvm.")`` or `prefixed_flags(..., "hhvm.", ...)` ensure that
//! an argument "reffiness_variance" or flag LOG_EXTERN_COMPILER_PERF gets the
//! canonical name "hhvm.reffiness_variance" or "hhvm.log_extern_compiler_perf", respectively.
//!
//! Non-canonical names (used when parsing from CLI) are specified by:
//! - `options_cli::CANON_BY_ALIAS.get("some_alias")`; and
//! - attribute `#[serde(alias = "some_alias")]`, for any non-flag argument.
//! The latter is mainly for convenience, so that JSON can be deserialized
//! even when the caller passes a CLI name (which would be understood for flags,
//! so it is also more consistent), but can also be handy for migration towards
//! consistent names between JSON and CLI.
//!
//! Example:
//! ```
//! let opts: Options = Options::default(); // JSON key
//! opts.doc_root.get()                     // doc_root
//! opts.hhvm.reffiness_invariance.set(32); // hhvm.reffiness_invariance
//! opts.hhvm_flags.contains(
//!     HhvmFlags::RX_IS_ENABLED);          // hhvm.rx_is_enabled
//! opts.hhvm.hack_lang_flags.set(
//!     LangFlags::ENABLE_COROUTINES        // hhvm.hack.lang.enable_coroutines
//! ```

mod options_cli;

use options_serde::prefix_all;

extern crate bitflags;
use bitflags::bitflags;

#[macro_use]
extern crate lazy_static;

use serde_derive::{Deserialize, Serialize};
use serde_json::{json, value::Value as Json};

use std::collections::{BTreeMap, BTreeSet};

/// Provides uniform access to bitflags-generated structs in JSON SerDe
trait PrefixedFlags:
    Sized
    + Copy
    + Default
    + std::fmt::Debug
    + std::ops::BitOrAssign
    + std::ops::BitAndAssign
    + std::ops::Not<Output = Self>
{
    const PREFIX: &'static str;

    // these methods (or equivalents) are implemented by bitflags!
    const EMPTY: Self;
    const ALL: Self;
    fn from_flags(flags: &Flags) -> Option<Self>;
    fn contains(&self, other: Self) -> bool;
    fn bits(&self) -> u64;
    fn to_map() -> BTreeMap<String, Self>;
}

macro_rules! prefixed_flags {
    ($class:ident, $prefix:expr, $($field:ident),*,) => { // require trailing comma

        bitflags! {
            pub struct $class: u64 {
                // TODO(leoo) expand RHS this into 1 << i, using equivalent of C++ index_sequence
                $( const $field = Flags::$field.bits(); )*
            }
        }
        impl PrefixedFlags for $class {
            const PREFIX: &'static str = $prefix;
            const EMPTY: Self = Self::empty();
            const ALL: Self = Self::all();

            // TODO(leoo) use proc_macro_hack and field_to_config_name!($field)
            // to map ("some.prefix", SOME_FIELD) into "some.prefix.some_field"
            // fn by_name(name: &'static str) -> Self {
            //     match name {
            //         $( case field_to_config_name!($prefix, $field) => Flags::$field, )*
            //     }
            // }
            fn to_map() -> BTreeMap<String, Self> {{
                let mut ret: BTreeMap<String, Self> = BTreeMap::new();
                $(
                    ret.insert(stringify!($field).to_lowercase(), Self::$field);
                )*
                ret
            }}

            fn contains(&self, other: Self) -> bool {
                self.contains(other)
            }

            fn bits(&self) -> u64 {
                self.bits()
            }

            fn from_flags(flags: &Flags) -> Option<Self> {
                Self::from_bits(flags.bits())
            }
        }
    }
}

/// An option of non-boolean type T (i.e., not a flag)
#[derive(Serialize, Deserialize, Debug, Default, PartialEq)]
pub struct Arg<T> {
    global_value: T,
}
impl<T> Arg<T> {
    pub fn get(&self) -> &T {
        &self.global_value
    }

    pub fn get_mut(&mut self) -> &mut T {
        &mut self.global_value
    }

    fn new(global_value: T) -> Arg<T> {
        Arg { global_value }
    }
}

// group options by JSON config prefix to avoid error-prone repetition & boilerplate in SerDe

// TODO move this "lonely wolf" to group "hack.compiler." (as hinted in D7057454);
// however, this is a breaking change for stuff that passes this option as JSON key
prefixed_flags!(EvalFlags, "eval.", DISASSEMBLER_SOURCE_MAPPING,);
impl Default for EvalFlags {
    fn default() -> EvalFlags {
        EvalFlags::empty()
    }
}

prefixed_flags!(
    CompilerFlags,
    "hack.compiler.",
    CONSTANT_FOLDING,
    OPTIMIZE_NULL_CHECKS,
    RELABEL,
);
impl Default for CompilerFlags {
    fn default() -> CompilerFlags {
        CompilerFlags::CONSTANT_FOLDING | CompilerFlags::RELABEL
    }
}

prefixed_flags!(
    HhvmFlags,
    "hhvm.",
    ARRAY_PROVENANCE,
    CREATE_IN_OUT_WRAPPER_FUNCTIONS,
    EMIT_CLS_METH_POINTERS,
    EMIT_FUNC_POINTERS,
    EMIT_INST_METH_POINTERS,
    EMIT_METH_CALLER_FUNC_POINTERS,
    ENABLE_INTRINSICS_EXTENSION,
    LOG_EXTERN_COMPILER_PERF,
    JIT_ENABLE_RENAME_FUNCTION,
    HACK_ARR_COMPAT_NOTICES,
    HACK_ARR_DV_ARRS,
    NOTICE_ON_BY_REF_ARGUMENT_TYPEHINT_VIOLATION,
    RX_IS_ENABLED,
);
impl Default for HhvmFlags {
    fn default() -> HhvmFlags {
        HhvmFlags::CREATE_IN_OUT_WRAPPER_FUNCTIONS
            | HhvmFlags::EMIT_CLS_METH_POINTERS
            | HhvmFlags::EMIT_FUNC_POINTERS
            | HhvmFlags::EMIT_INST_METH_POINTERS
            | HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS
    }
}

#[prefix_all("hhvm.")]
#[derive(Serialize, Deserialize, Debug, Default, PartialEq)]
pub struct Hhvm {
    #[serde(default)]
    pub aliased_namespaces: Arg<Vec<(String, String)>>,

    #[serde(default)]
    pub dynamic_invoke_functions: Arg<BTreeSet<String>>,

    #[serde(default)]
    pub include_roots: Arg<BTreeMap<String, String>>, // TODO(leoo) change to HashMap if order doesn't matter

    #[serde(default, alias = "eval.reffinessinvariance")]
    pub reffiness_invariance: Arg<isize>,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub flags: HhvmFlags,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub hack_lang_flags: LangFlags,
}

prefixed_flags!(
    LangFlags,
    "hhvm.hack.lang.",
    ABSTRACT_STATIC_PROPS,
    ALLOW_NEW_ATTRIBUTE_SYNTAX,
    CONST_DEFAULT_FUNC_ARGS,
    CONST_STATIC_PROPS,
    DISABLE_LEGACY_ATTRIBUTE_SYNTAX,
    DISABLE_LEGACY_SOFT_TYPEHINTS,
    DISABLE_LVAL_AS_AN_EXPRESSION,
    DISABLE_UNSET_CLASS_CONST,
    ENABLE_CLASS_LEVEL_WHERE_CLAUSES,
    ENABLE_CONSTANT_VISIBILITY_MODIFIERS,
    ENABLE_COROUTINES,
    ENABLE_POCKET_UNIVERSES,
    HACKSPERIMENTAL,
);
impl Default for LangFlags {
    fn default() -> LangFlags {
        LangFlags::ENABLE_COROUTINES
    }
}

prefixed_flags!(
    PhpismFlags,
    "hhvm.hack.lang.phpism.",
    DISALLOW_EXECUTION_OPERATOR,
    DISABLE_NONTOPLEVEL_DECLARATIONS,
    DISABLE_STATIC_CLOSURES,
    DISABLE_HALT_COMPILER,
);
impl Default for PhpismFlags {
    fn default() -> PhpismFlags {
        PhpismFlags::empty()
    }
}

prefixed_flags!(
    Php7Flags,
    "hhvm.php7.",
    INT_SEMANTICS, //
    LTR_ASSIGN,    //
    UVS,           //
);
impl Default for Php7Flags {
    fn default() -> Php7Flags {
        Php7Flags::empty()
    }
}

prefixed_flags!(
    RepoFlags,
    "hhvm.repo.",
    AUTHORITATIVE, //
);
impl Default for RepoFlags {
    fn default() -> RepoFlags {
        RepoFlags::empty()
    }
}

#[prefix_all("hhvm.server.")]
#[derive(Serialize, Deserialize, Default, PartialEq, Debug)]
pub struct Server {
    #[serde(default)]
    pub include_search_paths: Arg<Vec<String>>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug)]
pub struct Options {
    #[serde(default)]
    pub doc_root: Arg<String>,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub eval_flags: EvalFlags,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub hack_compiler_flags: CompilerFlags,

    #[serde(flatten, default)]
    pub hhvm: Hhvm,

    #[serde(default = "defaults::max_array_elem_size_on_the_stack")]
    pub max_array_elem_size_on_the_stack: Arg<isize>,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub phpism_flags: PhpismFlags,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub php7_flags: Php7Flags,

    #[serde(
        flatten,
        serialize_with = "serialize_flags",
        deserialize_with = "deserialize_flags"
    )]
    pub repo_flags: RepoFlags,

    #[serde(flatten, default)]
    pub server: Server,
}
impl Default for Options {
    fn default() -> Options {
        Options {
            max_array_elem_size_on_the_stack: defaults::max_array_elem_size_on_the_stack(),
            hack_compiler_flags: CompilerFlags::default(),
            eval_flags: EvalFlags::default(),
            hhvm: Hhvm::default(),
            phpism_flags: PhpismFlags::default(),
            php7_flags: Php7Flags::default(),
            repo_flags: RepoFlags::default(),
            server: Server::default(),
            // the rest is zeroed out (cannot do ..Default::default() as it'd be recursive)
            doc_root: Arg::new("".to_owned()),
        }
    }
}

/// Non-zero argument defaults for use in both Default::default & SerDe framework
mod defaults {
    use super::*;

    pub fn max_array_elem_size_on_the_stack() -> Arg<isize> {
        Arg::new(64)
    }
}

impl Options {
    pub fn to_string(&self) -> String {
        serde_json::to_string_pretty(&self).expect("failed to parse JSON")
    }

    pub fn from_json(s: &str) -> Result<Self, String> {
        let opts: serde_json::Result<Self> = serde_json::from_str(s);
        opts.map_err(|e| format!("failed to load config JSON:\n{}", e))
    }

    fn from_cli_args(args: &[&str]) -> Result<Json, String> {
        let mut json = json!({});
        for arg in args {
            match arg.find('=') {
                Some(pos) => {
                    let (key, val) = arg.split_at(pos);
                    let val = &val[1..]; // strip '='
                    let key: &str = options_cli::CANON_BY_ALIAS.get(key).unwrap_or(&key);
                    if let Some(val) = options_cli::to_json(key)(&val) {
                        json.as_object_mut().map(|m| {
                            m.insert(
                                key.to_owned(),
                                json!({
                                    "global_value": val,
                                }),
                            )
                        });
                    } else {
                        return Err(format!("Invalid format for CLI arg key: {}", key));
                    }
                }
                None => return Err(format!("Missing '=' key-value separator in: {}", arg)),
            }
        }
        Ok(json)
    }

    /// Merges src JSON into dst JSON, recursively adding or overwriting existing entries.
    /// This method cleverly avoids the need to represent each option as Option<Type>,
    /// since only the ones that are specified by JSON will be actually overridden.
    fn merge(dst: &mut Json, src: &Json) {
        match (dst, src) {
            (&mut Json::Object(ref mut dst), &Json::Object(ref src)) => {
                for (k, v) in src {
                    Self::merge(dst.entry(k.clone()).or_insert(Json::Null), v);
                }
            }
            (dst, src) => {
                *dst = src.clone();
            }
        }
    }

    pub fn from_config(json: Option<&str>, cli_args: &[&str]) -> Result<Self, String> {
        let mut merged = json!({});
        if let Some(json) = json {
            merged = serde_json::from_str(json).map_err(|e| e.to_string())?;
        }
        let overrides = Self::from_cli_args(cli_args)?;
        Self::merge(&mut merged, &overrides);
        let opts: serde_json::Result<Self> = serde_json::value::from_value(merged);
        opts.map_err(|e| e.to_string())
    }
}

use serde::de::{self, Deserializer, MapAccess, Visitor};
use serde::{ser::SerializeMap, Serializer};

fn serialize_flags<S: Serializer, P: PrefixedFlags>(flags: &P, s: S) -> Result<S::Ok, S::Error> {
    // TODO(leoo) iterate over each set bit: flags.bits() & ~(flags.bits() + 1)
    let mut map = s.serialize_map(None)?;
    for (key, value) in P::to_map().into_iter() {
        let bool_val = flags.contains(value);
        map.serialize_entry(&format!("{}{}", &P::PREFIX, key), &Arg::new(bool_val))?;
    }
    map.end()
}

#[derive(Deserialize)]
#[serde(untagged)]
enum GlobalValue {
    String(String),
    Bool(bool),
    Int(isize),
    VecStr(Vec<String>),
    MapStr(BTreeMap<String, String>),
}

fn deserialize_flags<'de, D: Deserializer<'de>, P: PrefixedFlags>(
    deserializer: D,
) -> Result<P, D::Error> {
    use std::fmt;
    use std::marker::PhantomData;
    struct Phantom<P>(PhantomData<P>);

    impl<'de, P: PrefixedFlags> Visitor<'de> for Phantom<P> {
        type Value = P;

        fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
            formatter.write_str("flag with string global_value")
        }

        fn visit_map<M: MapAccess<'de>>(self, mut map: M) -> Result<Self::Value, M::Error> {
            // TODO(leoo) proc macro to traverse Flags struct & iter over assoc. constants
            let mut flags = P::default();
            let by_name = P::to_map();
            let prefix_len = P::PREFIX.len();
            let from_str = |v: &str| match v {
                "true" => Ok(true),
                "false" => Ok(false),
                num => num.parse::<i32>().map(|v| v > 0).map_err(de::Error::custom),
            };
            while let Some((ref k, ref v)) = map.next_entry::<String, Arg<GlobalValue>>()? {
                let truish = match v.get() {
                    GlobalValue::String(s) => from_str(s)?,
                    GlobalValue::Bool(b) => *b,
                    GlobalValue::Int(n) => *n != 0,
                    _ => continue, // types such as VecStr aren't parsable as flags
                };
                // println!("k={:?} v={:?}~{} flag={:?}", k, v, truish, by_name.get(k));
                let mut found = None;
                let k: &str = &k;
                let k: &str = options_cli::CANON_BY_ALIAS.get(k).unwrap_or(&k);
                if k.starts_with(P::PREFIX) {
                    found = by_name.get(&k[prefix_len..]).map(|p| *p);
                }

                if let Some(flag) = found {
                    if truish {
                        flags |= flag
                    } else {
                        flags &= !flag
                    }
                }
            }
            Ok(flags)
        }
    }

    deserializer.deserialize_map(Phantom(PhantomData::<P>))
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq; // make assert_eq print huge diffs more human-readable

    const HHVM_1: &'static str = r#"{
  "hhvm.aliased_namespaces": {
    "global_value": [
      [
        "foo",
        "bar"
      ],
      [
        "bar",
        "baz"
      ]
    ]
  },
  "hhvm.array_provenance": {
    "global_value": false
  },
  "hhvm.create_in_out_wrapper_functions": {
    "global_value": true
  },
  "hhvm.dynamic_invoke_functions": {
    "global_value": []
  },
  "hhvm.emit_cls_meth_pointers": {
    "global_value": false
  },
  "hhvm.emit_func_pointers": {
    "global_value": false
  },
  "hhvm.emit_inst_meth_pointers": {
    "global_value": false
  },
  "hhvm.emit_meth_caller_func_pointers": {
    "global_value": false
  },
  "hhvm.enable_intrinsics_extension": {
    "global_value": false
  },
  "hhvm.hack.lang.abstract_static_props": {
    "global_value": false
  },
  "hhvm.hack.lang.allow_new_attribute_syntax": {
    "global_value": false
  },
  "hhvm.hack.lang.const_default_func_args": {
    "global_value": false
  },
  "hhvm.hack.lang.const_static_props": {
    "global_value": false
  },
  "hhvm.hack.lang.disable_legacy_attribute_syntax": {
    "global_value": false
  },
  "hhvm.hack.lang.disable_legacy_soft_typehints": {
    "global_value": false
  },
  "hhvm.hack.lang.disable_lval_as_an_expression": {
    "global_value": false
  },
  "hhvm.hack.lang.disable_unset_class_const": {
    "global_value": false
  },
  "hhvm.hack.lang.enable_class_level_where_clauses": {
    "global_value": false
  },
  "hhvm.hack.lang.enable_constant_visibility_modifiers": {
    "global_value": false
  },
  "hhvm.hack.lang.enable_coroutines": {
    "global_value": true
  },
  "hhvm.hack.lang.enable_pocket_universes": {
    "global_value": false
  },
  "hhvm.hack.lang.hacksperimental": {
    "global_value": false
  },
  "hhvm.hack_arr_compat_notices": {
    "global_value": false
  },
  "hhvm.hack_arr_dv_arrs": {
    "global_value": false
  },
  "hhvm.include_roots": {
    "global_value": {}
  },
  "hhvm.jit_enable_rename_function": {
    "global_value": false
  },
  "hhvm.log_extern_compiler_perf": {
    "global_value": false
  },
  "hhvm.notice_on_by_ref_argument_typehint_violation": {
    "global_value": true
  },
  "hhvm.reffiness_invariance": {
    "global_value": 5
  },
  "hhvm.rx_is_enabled": {
    "global_value": false
  }
}"#;

    #[test]
    fn test_hhvm_json_ser() {
        assert_eq!(1, 1);
        let hhvm = json!(Hhvm {
            aliased_namespaces: Arg::new(vec![
                ("foo".to_owned(), "bar".to_owned()),
                ("bar".to_owned(), "baz".to_owned()),
            ]),
            flags: (HhvmFlags::CREATE_IN_OUT_WRAPPER_FUNCTIONS
                | HhvmFlags::NOTICE_ON_BY_REF_ARGUMENT_TYPEHINT_VIOLATION),
            reffiness_invariance: Arg::new(5),
            ..Default::default()
        });
        assert_eq!(HHVM_1, serde_json::to_string_pretty(&hhvm).unwrap(),);
    }

    #[test]
    fn test_hhvm_json_de() {
        let j = serde_json::from_str(
            r#"{
            "hhvm.aliased_namespaces": { "global_value": [ ["foo", "bar"] ] },
            "hhvm.create_in_out_wrapper_functions": { "global_value": "1333" },
            "hhvm.jit_enable_rename_function": { "global_value": 2 },
            "hhvm.log_extern_compiler_perf": { "global_value": false },
            "hhvm.notice_on_by_ref_argument_typehint_violation": { "global_value": "1" },
            "hhvm.reffiness_invariance": { "global_value": 42 }
            }"#,
        )
        .unwrap();
        let hhvm: Hhvm = serde_json::from_value(j).unwrap();
        println!("{:?}", hhvm);
        assert!(hhvm.flags.contains(
            HhvmFlags::CREATE_IN_OUT_WRAPPER_FUNCTIONS
                | HhvmFlags::JIT_ENABLE_RENAME_FUNCTION
                | HhvmFlags::NOTICE_ON_BY_REF_ARGUMENT_TYPEHINT_VIOLATION
        ));
        assert!(!hhvm.flags.contains(HhvmFlags::LOG_EXTERN_COMPILER_PERF));
        assert_eq!(hhvm.reffiness_invariance.global_value, 42);
    }

    #[test]
    fn test_hhvm_json_de_defaults_overrideable() {
        let hhvm: Hhvm = serde_json::value::from_value(json!({})).unwrap();
        assert_eq!(hhvm.flags, HhvmFlags::default());
        assert!(hhvm
            .flags
            .contains(HhvmFlags::CREATE_IN_OUT_WRAPPER_FUNCTIONS));

        // now override a true-by-default option with a false value
        let hhvm: Hhvm = serde_json::value::from_value(json!({
            "hhvm.create_in_out_wrapper_functions": { "global_value": "false" },
        }))
        .unwrap();
        assert!(!hhvm
            .flags
            .contains(HhvmFlags::CREATE_IN_OUT_WRAPPER_FUNCTIONS));
    }

    #[test]
    fn test_hhvm_flags_alias_json_de() {
        // sanity check for defaults (otherwise this test doesn't do much!)
        assert!(!HhvmFlags::default().contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION));
        assert!(HhvmFlags::default().contains(HhvmFlags::CREATE_IN_OUT_WRAPPER_FUNCTIONS));

        let hhvm: Hhvm = serde_json::from_str(
            r#"{ "eval.jitenablerenamefunction": { "global_value": "true" },
                 "eval.createinoutwrapperfunctions": { "global_value": "false" } }"#,
        )
        .unwrap();
        assert!(hhvm // verify a false-by-default flag was parsed as true
            .flags
            .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION));

        assert!(!hhvm // verify a true-by-default flag was parsed as false
            .flags
            .contains(HhvmFlags::CREATE_IN_OUT_WRAPPER_FUNCTIONS));
    }

    #[test]
    fn test_options_flat_arg_alias_json_de() {
        let act: Options = serde_json::value::from_value(json!({
            "eval.reffinessinvariance": {
                "global_value": 123,
            },
            "eval.jitenablerenamefunction": {
                "global_value": "true",
            },
        }))
        .expect("failed to deserialize");
        assert_eq!(act.hhvm.reffiness_invariance.global_value, 123);
        assert!(act
            .hhvm
            .flags
            .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION));
    }

    #[test]
    fn test_options_nonzero_defaults_json_de() {
        let act: Options = serde_json::value::from_value(json!({})).unwrap();
        assert_eq!(act, Options::default());
    }

    #[test]
    fn test_options_merge() {
        let mut dst = json!({
            "uniqueAtDst": "DST",
            "person" : { "firstName": "John", "lastName": "Doe" },
            "flat": [ "will", "be", "overridden" ],
        });
        let src = json!({
            "uniqueAtSrc": "SRC",
            "person" : { "firstName" : "Jane (not John)" },
            "flat": "overrides dst's field",
        });
        Options::merge(&mut dst, &src);
        assert_eq!(
            dst,
            json!({
                "flat": "overrides dst's field",
                "person": {
                    "firstName": "Jane (not John)",
                    "lastName": "Doe"
                },
                "uniqueAtDst": "DST",
                "uniqueAtSrc": "SRC",
            })
        );
    }

    #[test]
    fn test_hhvm_flags_cli_de_missing_equals() {
        let args = ["eval.jitenablerenamefunction"];
        let exp = Options::from_cli_args(args.as_ref());
        assert!(exp.is_err());
        let err = exp.unwrap_err();
        assert!(err.starts_with("Missing '='"));
        assert!(err.ends_with("function"));
    }

    #[test]
    fn test_hhvm_flags_cli_de_to_json() {
        let args = [
            "eval.logexterncompilerperf=true",
            "eval.jitenablerenamefunction=1",
        ];
        let act = Options::from_cli_args(&args);
        assert_eq!(
            act,
            Ok(json!({
                "hhvm.jit_enable_rename_function": {
                    "global_value": "1",
                },
                "hhvm.log_extern_compiler_perf": {
                    "global_value": "true",
                },
            })),
        );
    }

    #[test]
    fn test_options_de_from_cli_override_json() {
        let cli_args = [
            "eval.jitenablerenamefunction=1",
            "eval.reffinessinvariance=999",
        ];
        let json = json!({
            "hhvm.reffiness_invariance": {
                "global_value": 123,
            },
            "hhvm.log_extern_compiler_perf": {
                "global_value": "true",
            },
        });
        let act = Options::from_config(Some(&json.to_string()), &cli_args).unwrap();
        assert_eq!(act.hhvm.reffiness_invariance.global_value, 999);
    }

    #[test]
    fn test_options_de_from_cli_comma_separated_strings() {
        let mut exp_dynamic_invoke_functions = BTreeSet::<String>::new();
        exp_dynamic_invoke_functions.insert("foo".into());
        exp_dynamic_invoke_functions.insert("bar".into());
        let act = Options::from_config(None, &["hhvm.dynamic_invoke_functions=foo,bar"]).unwrap();
        assert_eq!(
            act.hhvm.dynamic_invoke_functions.global_value,
            exp_dynamic_invoke_functions,
        );
    }

    #[test]
    fn test_options_de_from_cli_comma_separated_key_value() {
        let mut exp_include_roots = BTreeMap::<String, String>::new();
        exp_include_roots.insert("foo".into(), "bar".into());
        exp_include_roots.insert("bar".into(), "baz".into());
        const CLI_ARG: &str = "hhvm.include_roots=foo:bar,bar:baz";
        let act = Options::from_config(None, &[CLI_ARG]).unwrap();
        assert_eq!(act.hhvm.include_roots.global_value, exp_include_roots,);
    }

    #[test]
    fn test_major_outlier_source_mapping_serde() {
        use serde::ser::Serialize;
        fn mk_source_mapping<T: Serialize>(v: T) -> Json {
            json!({
                "eval.disassembler_source_mapping": {
                    "global_value": v
                }
            })
        }
        fn serialize_source_mapping(opts: Options) -> Json {
            let mut j = serde_json::to_value(opts).unwrap();
            // remove everything from Options JSON except this key
            const KEY: &str = "eval.disassembler_source_mapping";
            let m = j.as_object_mut().unwrap();
            let sm = m.remove(KEY);
            m.clear();
            sm.map(|val| m.insert(KEY.to_owned(), val));
            j
        }
        fn test<T: Serialize + std::fmt::Debug>(exp_val: bool, val: T) {
            let j = mk_source_mapping(dbg!(val));
            let opts: Options = serde_json::value::from_value(j).unwrap();
            assert_eq!(
                exp_val,
                opts.eval_flags
                    .contains(EvalFlags::DISASSEMBLER_SOURCE_MAPPING)
            );
            let j_act = serialize_source_mapping(opts);
            let j_exp = mk_source_mapping(exp_val);
            assert_eq!(j_exp, j_act);
        }
        test(true, true);
        test(true, 1);
        test(true, "true");
        test(false, 0);
        test(false, false);
        test(false, "false");
    }
}

// boilerplate code that could eventually be avoided via procedural macros

bitflags! {
    struct Flags: u64 {
        const CONSTANT_FOLDING = 1 << 0;
        const OPTIMIZE_NULL_CHECKS = 1 << 1;
        const DISASSEMBLER_SOURCE_MAPPING = 1 << 2;
        const UVS = 1 << 3;
        const LTR_ASSIGN = 1 << 4;

        /// If true, then renumber labels after generating code for a method
        /// body. Semantic diff doesn't care about labels, but for visual diff against
        /// HHVM it's helpful to renumber in order that the labels match more closely
        const RELABEL = 1 << 5;
        const CREATE_IN_OUT_WRAPPER_FUNCTIONS = 1 << 6;
        const HACK_ARR_COMPAT_NOTICES = 1 << 7;
        const HACK_ARR_DV_ARRS = 1 << 8;
        const AUTHORITATIVE = 1 << 9;
        const JIT_ENABLE_RENAME_FUNCTION = 1 << 10;
        const INT_SEMANTICS = 1 << 11;
        const ENABLE_COROUTINES = 1 << 12;
        const HACKSPERIMENTAL = 1 << 13;
        const LOG_EXTERN_COMPILER_PERF = 1 << 14;
        const ENABLE_INTRINSICS_EXTENSION = 1 << 15;
        const ENABLE_HHJS = 1 << 16;
        const DISALLOW_EXECUTION_OPERATOR = 1 << 21;
        const DISABLE_NONTOPLEVEL_DECLARATIONS = 1 << 22;
        const DISABLE_STATIC_CLOSURES = 1 << 23;
        const DISABLE_HALT_COMPILER = 1 << 24;
        const EMIT_FUNC_POINTERS = 1 << 25;
        const EMIT_CLS_METH_POINTERS = 1 << 26;
        const EMIT_INST_METH_POINTERS = 1 << 27;
        const EMIT_METH_CALLER_FUNC_POINTERS = 1 << 28;
        const RX_IS_ENABLED = 1 << 29;
        const DISABLE_LVAL_AS_AN_EXPRESSION = 1 << 30;
        const ENABLE_POCKET_UNIVERSES = 1 << 31;
        const NOTICE_ON_BY_REF_ARGUMENT_TYPEHINT_VIOLATION = 1 << 32;
        const ARRAY_PROVENANCE = 1 << 33;
        const ENABLE_CONSTANT_VISIBILITY_MODIFIERS = 1 << 34;
        const ENABLE_CLASS_LEVEL_WHERE_CLAUSES = 1 << 35;
        const DISABLE_LEGACY_SOFT_TYPEHINTS = 1 << 36;
        const ALLOW_NEW_ATTRIBUTE_SYNTAX = 1 << 37;
        const DISABLE_LEGACY_ATTRIBUTE_SYNTAX = 1 << 38;
        const CONST_DEFAULT_FUNC_ARGS = 1 << 39;
        const CONST_STATIC_PROPS = 1 << 40;
        const ABSTRACT_STATIC_PROPS = 1 << 41;
        const DISABLE_UNSET_CLASS_CONST = 1 << 42;
    }
}
