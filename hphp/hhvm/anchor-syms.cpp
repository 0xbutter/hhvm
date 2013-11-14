#include "hphp/runtime/ext/extension.h"

namespace HPHP {

/**
 * Prevent over-eager linkers from stripping seemingly unused
 * symbols from the resulting binary by linking them from here.
 */
extern Extension s_zip_extension;

const Extension *g_anchor_extensions[] = {
  &s_zip_extension,
};

} // HPHP
