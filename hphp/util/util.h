/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

//////////////////////////////////////////////////////////////////////

/*
 * This header is deprecated, please don't include it in anything new
 * or add new code to this header.
 *
 * If you need something defined in this header, pull it out to a
 * smaller header and include that.
 *
 * TODO(#3468751): split this header up
 */

//////////////////////////////////////////////////////////////////////

#ifndef incl_HPHP_UTIL_H_
#define incl_HPHP_UTIL_H_

#include <cassert>
#include <atomic>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h> // For htonl().

namespace HPHP { namespace Util {

///////////////////////////////////////////////////////////////////////////////

/**
 * Make sure path exists. Same as "mkdir -p", but "a/b" will only make sure
 * "a/" exists, treating "b" as a file name.
 */
bool mkdir(const std::string &path, int mode = 0777);

/**
 * Make dest directory look identical to src by copying files and directories,
 * without copying identical files (so they keep the same timestamp as before).
 */
void syncdir(const std::string &dest, const std::string &src,
             bool keepSrc = false);

/**
 * Copy srcfile to dstfile, return 0 on success, -1 otherwise
 */
int copy(const char *srcfile, const char *dstfile);

/**
 * Like copy but using little disk-cache
 */
int directCopy(const char *srcfile, const char *dstfile);

/**
 * Like rename(2), but takes care of cross-filesystem rename.
 */
int rename(const char *oldname, const char *newname);

/**
 * Like rename but using little disk-cache
 */
int directRename(const char *oldname, const char *newname);

/**
 * Like system(3), but automatically print errors if execution fails.
 */
int ssystem(const char *command);

/**
 * Find the relative path from a directory with trailing slash to the file
 */
std::string relativePath(const std::string fromDir, const std::string toFile);

/**
 * Canonicalize path to remove "..", "." and "\/", etc..
 */
std::string canonicalize(const std::string& path);
char* canonicalize(const char* path, size_t len,
                   bool collapse_slashes = true);

/**
 * Makes sure there is ending slash by changing "path/name" to "path/name/".
 */
std::string normalizeDir(const std::string &dirname);

/**
 * Thread-safe dirname().
 */
std::string safe_dirname(const char *path, int len);
std::string safe_dirname(const char *path);
std::string safe_dirname(const std::string& path);

/**
 * Helper function for safe_dirname.
 */
size_t dirname_helper(char *path, int len);

/**
 * Round up value to the nearest power of two
 */
template<typename Int>
inline Int roundUpToPowerOfTwo(Int value) {
#ifdef DEBUG
  (void) (0 / value); // fail for 0; ASSERT is a pain.
#endif
  --value;
  // Verified that gcc is smart enough to unroll this and emit
  // constant shifts.
  for (unsigned i = 1; i < sizeof(Int) * 8; i *= 2)
    value |= value >> i;
  ++value;
  return value;
}

/**
 * Search for PHP or non-PHP files under a directory.
 */
void find(std::vector<std::string> &out,
          const std::string &root, const char *path, bool php,
          const std::set<std::string> *excludeDirs = nullptr,
          const std::set<std::string> *excludeFiles = nullptr);

inline void assert_native_stack_aligned() {
#ifndef NDEBUG
  assert(reinterpret_cast<uintptr_t>(__builtin_frame_address(0)) % 16 == 0);
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
}

#endif
