/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "runtime/ext_hhvm/ext_hhvm.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/base/array/array_init.h"
#include "runtime/ext/ext.h"
#include "runtime/vm/class.h"
#include "runtime/vm/runtime.h"
#include <exception>

namespace HPHP {

Value* fh_mb_list_encodings(Value* _rv) asm("_ZN4HPHP19f_mb_list_encodingsEv");

TypedValue* fg_mb_list_encodings(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_mb_list_encodings(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("mb_list_encodings", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_list_encodings_alias_names(TypedValue* _rv, Value* name) asm("_ZN4HPHP31f_mb_list_encodings_alias_namesERKNS_6StringE");

void fg1_mb_list_encodings_alias_names(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_list_encodings_alias_names(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_list_encodings_alias_names(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_list_encodings_alias_names(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_list_encodings_alias_names(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_list_encodings_alias_names(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_list_encodings_alias_names", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_list_mime_names(TypedValue* _rv, Value* name) asm("_ZN4HPHP20f_mb_list_mime_namesERKNS_6StringE");

void fg1_mb_list_mime_names(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_list_mime_names(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_list_mime_names(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_list_mime_names(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_list_mime_names(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_list_mime_names(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_list_mime_names", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mb_check_encoding(Value* var, Value* encoding) asm("_ZN4HPHP19f_mb_check_encodingERKNS_6StringES2_");

void fg1_mb_check_encoding(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_check_encoding(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mb_check_encoding((count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mb_check_encoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mb_check_encoding((count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mb_check_encoding(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_check_encoding", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_convert_case(TypedValue* _rv, Value* str, int mode, Value* encoding) asm("_ZN4HPHP17f_mb_convert_caseERKNS_6StringEiS2_");

void fg1_mb_convert_case(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_convert_case(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_convert_case(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_convert_case(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_convert_case(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_convert_case(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_convert_case", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_convert_encoding(TypedValue* _rv, Value* str, Value* to_encoding, TypedValue* from_encoding) asm("_ZN4HPHP21f_mb_convert_encodingERKNS_6StringES2_RKNS_7VariantE");

void fg1_mb_convert_encoding(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_convert_encoding(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_convert_encoding(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_convert_encoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_convert_encoding(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_convert_encoding(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_convert_encoding", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_convert_kana(TypedValue* _rv, Value* str, Value* option, Value* encoding) asm("_ZN4HPHP17f_mb_convert_kanaERKNS_6StringES2_S2_");

void fg1_mb_convert_kana(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_convert_kana(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_convert_kana(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_convert_kana(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_convert_kana(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_convert_kana(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_convert_kana", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_convert_variables(TypedValue* _rv, int64_t _argc, Value* to_encoding, TypedValue* from_encoding, TypedValue* vars, Value* _argv) asm("_ZN4HPHP22f_mb_convert_variablesEiRKNS_6StringERKNS_7VariantERKNS_14VRefParamValueERKNS_5ArrayE");

void fg1_mb_convert_variables(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_convert_variables(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);

  Array extraArgs;
  {
    ArrayInit ai(count-3);
    for (int32_t i = 3; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-3);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-3, tvAsVariant(extraArg));
      } else {
        ai.set(i-3, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_mb_convert_variables(rv, count, &args[-0].m_data, (args-1), (args-2), (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_convert_variables(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {

      Array extraArgs;
      {
        ArrayInit ai(count-3);
        for (int32_t i = 3; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-3);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-3, tvAsVariant(extraArg));
          } else {
            ai.set(i-3, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_mb_convert_variables(rv, count, &args[-0].m_data, (args-1), (args-2), (Value*)(&extraArgs));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_convert_variables(rv, ar, count);
    }
  } else {
    throw_missing_arguments_nr("mb_convert_variables", 3, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_decode_mimeheader(TypedValue* _rv, Value* str) asm("_ZN4HPHP22f_mb_decode_mimeheaderERKNS_6StringE");

void fg1_mb_decode_mimeheader(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_decode_mimeheader(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_decode_mimeheader(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_decode_mimeheader(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_decode_mimeheader(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_decode_mimeheader(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_decode_mimeheader", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_decode_numericentity(TypedValue* _rv, Value* str, TypedValue* convmap, Value* encoding) asm("_ZN4HPHP25f_mb_decode_numericentityERKNS_6StringERKNS_7VariantES2_");

void fg1_mb_decode_numericentity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_decode_numericentity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_decode_numericentity(rv, &args[-0].m_data, (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_decode_numericentity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_decode_numericentity(rv, &args[-0].m_data, (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_decode_numericentity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_decode_numericentity", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_detect_encoding(TypedValue* _rv, Value* str, TypedValue* encoding_list, TypedValue* strict) asm("_ZN4HPHP20f_mb_detect_encodingERKNS_6StringERKNS_7VariantES5_");

void fg1_mb_detect_encoding(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_detect_encoding(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_detect_encoding(rv, &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&null_variant), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_detect_encoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_detect_encoding(rv, &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&null_variant), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_detect_encoding(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_detect_encoding", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_detect_order(TypedValue* _rv, TypedValue* encoding_list) asm("_ZN4HPHP17f_mb_detect_orderERKNS_7VariantE");

TypedValue* fg_mb_detect_order(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    fh_mb_detect_order(rv, (count > 0) ? (args-0) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("mb_detect_order", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_encode_mimeheader(TypedValue* _rv, Value* str, Value* charset, Value* transfer_encoding, Value* linefeed, int indent) asm("_ZN4HPHP22f_mb_encode_mimeheaderERKNS_6StringES2_S2_S2_i");

void fg1_mb_encode_mimeheader(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_encode_mimeheader(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal3 = "\r\n";
  fh_mb_encode_mimeheader(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_encode_mimeheader(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      String defVal3 = "\r\n";
      fh_mb_encode_mimeheader(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_encode_mimeheader(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_encode_mimeheader", count, 1, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_encode_numericentity(TypedValue* _rv, Value* str, TypedValue* convmap, Value* encoding) asm("_ZN4HPHP25f_mb_encode_numericentityERKNS_6StringERKNS_7VariantES2_");

void fg1_mb_encode_numericentity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_encode_numericentity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_encode_numericentity(rv, &args[-0].m_data, (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_encode_numericentity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_encode_numericentity(rv, &args[-0].m_data, (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_encode_numericentity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_encode_numericentity", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_get_info(TypedValue* _rv, Value* type) asm("_ZN4HPHP13f_mb_get_infoERKNS_6StringE");

void fg1_mb_get_info(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_get_info(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_get_info(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_get_info(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_get_info(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_get_info(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_get_info", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_http_input(TypedValue* _rv, Value* type) asm("_ZN4HPHP15f_mb_http_inputERKNS_6StringE");

void fg1_mb_http_input(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_http_input(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_http_input(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_http_input(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_http_input(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_http_input(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_http_input", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_http_output(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP16f_mb_http_outputERKNS_6StringE");

void fg1_mb_http_output(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_http_output(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_http_output(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_http_output(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_http_output(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_http_output(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_http_output", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_internal_encoding(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP22f_mb_internal_encodingERKNS_6StringE");

void fg1_mb_internal_encoding(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_internal_encoding(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_internal_encoding(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_internal_encoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_internal_encoding(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_internal_encoding(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_internal_encoding", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_language(TypedValue* _rv, Value* language) asm("_ZN4HPHP13f_mb_languageERKNS_6StringE");

void fg1_mb_language(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_language(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_language(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_language(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_language(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_language(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_language", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mb_output_handler(Value* _rv, Value* contents, int status) asm("_ZN4HPHP19f_mb_output_handlerERKNS_6StringEi");

void fg1_mb_output_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_output_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_mb_output_handler(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_output_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_mb_output_handler(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mb_output_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_output_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mb_parse_str(Value* encoded_string, TypedValue* result) asm("_ZN4HPHP14f_mb_parse_strERKNS_6StringERKNS_14VRefParamValueE");

void fg1_mb_parse_str(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_parse_str(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal1 = uninit_null();
  rv->m_data.num = (fh_mb_parse_str(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
}

TypedValue* fg_mb_parse_str(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal1 = uninit_null();
      rv->m_data.num = (fh_mb_parse_str(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
    } else {
      fg1_mb_parse_str(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_parse_str", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_preferred_mime_name(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP24f_mb_preferred_mime_nameERKNS_6StringE");

void fg1_mb_preferred_mime_name(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_preferred_mime_name(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_preferred_mime_name(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_preferred_mime_name(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_preferred_mime_name(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_preferred_mime_name(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_preferred_mime_name", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_substr(TypedValue* _rv, Value* str, int start, int length, Value* encoding) asm("_ZN4HPHP11f_mb_substrERKNS_6StringEiiS2_");

void fg1_mb_substr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_substr(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_substr(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0x7FFFFFFF), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_substr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_substr(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0x7FFFFFFF), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_substr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_substr", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strcut(TypedValue* _rv, Value* str, int start, int length, Value* encoding) asm("_ZN4HPHP11f_mb_strcutERKNS_6StringEiiS2_");

void fg1_mb_strcut(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strcut(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strcut(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0x7FFFFFFF), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strcut(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strcut(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0x7FFFFFFF), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strcut(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strcut", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strimwidth(TypedValue* _rv, Value* str, int start, int width, Value* trimmarker, Value* encoding) asm("_ZN4HPHP15f_mb_strimwidthERKNS_6StringEiiS2_S2_");

void fg1_mb_strimwidth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strimwidth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strimwidth(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strimwidth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strimwidth(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strimwidth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strimwidth", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_stripos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP12f_mb_striposERKNS_6StringES2_iS2_");

void fg1_mb_stripos(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_stripos(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_stripos(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_stripos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_stripos(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_stripos(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_stripos", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strripos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP13f_mb_strriposERKNS_6StringES2_iS2_");

void fg1_mb_strripos(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strripos(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strripos(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strripos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strripos(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strripos(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strripos", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_stristr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP12f_mb_stristrERKNS_6StringES2_bS2_");

void fg1_mb_stristr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_stristr(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_stristr(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_stristr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_stristr(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_stristr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_stristr", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strlen(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP11f_mb_strlenERKNS_6StringES2_");

void fg1_mb_strlen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strlen(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strlen(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strlen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strlen(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strlen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strlen", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strpos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP11f_mb_strposERKNS_6StringES2_iS2_");

void fg1_mb_strpos(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strpos(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strpos(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strpos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strpos(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strpos(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strpos", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strrpos(TypedValue* _rv, Value* haystack, Value* needle, TypedValue* offset, Value* encoding) asm("_ZN4HPHP12f_mb_strrposERKNS_6StringES2_RKNS_7VariantES2_");

void fg1_mb_strrpos(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strrpos(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2 = 0;
  fh_mb_strrpos(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strrpos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      Variant defVal2 = 0;
      fh_mb_strrpos(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strrpos(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strrpos", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strrchr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP12f_mb_strrchrERKNS_6StringES2_bS2_");

void fg1_mb_strrchr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strrchr(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strrchr(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strrchr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strrchr(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strrchr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strrchr", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strrichr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP13f_mb_strrichrERKNS_6StringES2_bS2_");

void fg1_mb_strrichr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strrichr(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strrichr(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strrichr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strrichr(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strrichr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strrichr", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strstr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP11f_mb_strstrERKNS_6StringES2_bS2_");

void fg1_mb_strstr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strstr(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strstr(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strstr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strstr(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strstr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strstr", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strtolower(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP15f_mb_strtolowerERKNS_6StringES2_");

void fg1_mb_strtolower(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strtolower(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strtolower(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strtolower(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strtolower(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strtolower(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strtolower", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strtoupper(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP15f_mb_strtoupperERKNS_6StringES2_");

void fg1_mb_strtoupper(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strtoupper(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strtoupper(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strtoupper(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strtoupper(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strtoupper(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strtoupper", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_strwidth(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP13f_mb_strwidthERKNS_6StringES2_");

void fg1_mb_strwidth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_strwidth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_strwidth(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_strwidth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_strwidth(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_strwidth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_strwidth", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_substitute_character(TypedValue* _rv, TypedValue* substrchar) asm("_ZN4HPHP25f_mb_substitute_characterERKNS_7VariantE");

TypedValue* fg_mb_substitute_character(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    fh_mb_substitute_character(rv, (count > 0) ? (args-0) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("mb_substitute_character", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_substr_count(TypedValue* _rv, Value* haystack, Value* needle, Value* encoding) asm("_ZN4HPHP17f_mb_substr_countERKNS_6StringES2_S2_");

void fg1_mb_substr_count(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_substr_count(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_substr_count(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_substr_count(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_substr_count(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_substr_count(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_substr_count", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mb_ereg_match(Value* pattern, Value* str, Value* option) asm("_ZN4HPHP15f_mb_ereg_matchERKNS_6StringES2_S2_");

void fg1_mb_ereg_match(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_ereg_match(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mb_ereg_match(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mb_ereg_match(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mb_ereg_match(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mb_ereg_match(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_ereg_match", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_ereg_replace(TypedValue* _rv, TypedValue* pattern, Value* replacement, Value* str, Value* option) asm("_ZN4HPHP17f_mb_ereg_replaceERKNS_7VariantERKNS_6StringES5_S5_");

void fg1_mb_ereg_replace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_ereg_replace(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_mb_ereg_replace(rv, (args-0), &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_ereg_replace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      fh_mb_ereg_replace(rv, (args-0), &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_ereg_replace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_ereg_replace", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_eregi_replace(TypedValue* _rv, TypedValue* pattern, Value* replacement, Value* str, Value* option) asm("_ZN4HPHP18f_mb_eregi_replaceERKNS_7VariantERKNS_6StringES5_S5_");

void fg1_mb_eregi_replace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_eregi_replace(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_mb_eregi_replace(rv, (args-0), &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_eregi_replace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      fh_mb_eregi_replace(rv, (args-0), &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_eregi_replace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_eregi_replace", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_mb_ereg_search_getpos() asm("_ZN4HPHP23f_mb_ereg_search_getposEv");

TypedValue* fg_mb_ereg_search_getpos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_mb_ereg_search_getpos();
  } else {
    throw_toomany_arguments_nr("mb_ereg_search_getpos", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mb_ereg_search_setpos(int position) asm("_ZN4HPHP23f_mb_ereg_search_setposEi");

void fg1_mb_ereg_search_setpos(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_ereg_search_setpos(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mb_ereg_search_setpos((int)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_mb_ereg_search_setpos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mb_ereg_search_setpos((int)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_mb_ereg_search_setpos(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_ereg_search_setpos", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_ereg_search_getregs(TypedValue* _rv) asm("_ZN4HPHP24f_mb_ereg_search_getregsEv");

TypedValue* fg_mb_ereg_search_getregs(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_mb_ereg_search_getregs(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("mb_ereg_search_getregs", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mb_ereg_search_init(Value* str, Value* pattern, Value* option) asm("_ZN4HPHP21f_mb_ereg_search_initERKNS_6StringES2_S2_");

void fg1_mb_ereg_search_init(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_ereg_search_init(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mb_ereg_search_init(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mb_ereg_search_init(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mb_ereg_search_init(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mb_ereg_search_init(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_ereg_search_init", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_ereg_search(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP16f_mb_ereg_searchERKNS_6StringES2_");

void fg1_mb_ereg_search(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_ereg_search(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_mb_ereg_search(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_ereg_search(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_ereg_search(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_ereg_search(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_ereg_search", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_ereg_search_pos(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP20f_mb_ereg_search_posERKNS_6StringES2_");

void fg1_mb_ereg_search_pos(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_ereg_search_pos(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_mb_ereg_search_pos(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_ereg_search_pos(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_ereg_search_pos(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_ereg_search_pos(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_ereg_search_pos", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_ereg_search_regs(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP21f_mb_ereg_search_regsERKNS_6StringES2_");

void fg1_mb_ereg_search_regs(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_ereg_search_regs(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_mb_ereg_search_regs(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_ereg_search_regs(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_ereg_search_regs(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_ereg_search_regs(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_ereg_search_regs", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_ereg(TypedValue* _rv, TypedValue* pattern, Value* str, TypedValue* regs) asm("_ZN4HPHP9f_mb_eregERKNS_7VariantERKNS_6StringERKNS_14VRefParamValueE");

void fg1_mb_ereg(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_ereg(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  VRefParamValue defVal2 = uninit_null();
  fh_mb_ereg(rv, (args-0), &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_ereg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if (IS_STRING_TYPE((args - 1)->m_type)) {
      VRefParamValue defVal2 = uninit_null();
      fh_mb_ereg(rv, (args-0), &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_ereg(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_ereg", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_eregi(TypedValue* _rv, TypedValue* pattern, Value* str, TypedValue* regs) asm("_ZN4HPHP10f_mb_eregiERKNS_7VariantERKNS_6StringERKNS_14VRefParamValueE");

void fg1_mb_eregi(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_eregi(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  VRefParamValue defVal2 = uninit_null();
  fh_mb_eregi(rv, (args-0), &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_eregi(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if (IS_STRING_TYPE((args - 1)->m_type)) {
      VRefParamValue defVal2 = uninit_null();
      fh_mb_eregi(rv, (args-0), &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_eregi(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_eregi", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_regex_encoding(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP19f_mb_regex_encodingERKNS_6StringE");

void fg1_mb_regex_encoding(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_regex_encoding(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mb_regex_encoding(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_regex_encoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      fh_mb_regex_encoding(rv, (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_regex_encoding(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_regex_encoding", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mb_regex_set_options(Value* _rv, Value* options) asm("_ZN4HPHP22f_mb_regex_set_optionsERKNS_6StringE");

void fg1_mb_regex_set_options(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_regex_set_options(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_mb_regex_set_options(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_regex_set_options(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfString;
      fh_mb_regex_set_options(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mb_regex_set_options(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("mb_regex_set_options", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mb_split(TypedValue* _rv, Value* pattern, Value* str, int count) asm("_ZN4HPHP10f_mb_splitERKNS_6StringES2_i");

void fg1_mb_split(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_split(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_mb_split(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mb_split(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mb_split(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mb_split(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_split", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mb_send_mail(Value* to, Value* subject, Value* message, Value* headers, Value* extra_cmd) asm("_ZN4HPHP14f_mb_send_mailERKNS_6StringES2_S2_S2_S2_");

void fg1_mb_send_mail(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mb_send_mail(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mb_send_mail(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mb_send_mail(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mb_send_mail(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mb_send_mail(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mb_send_mail", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
