
#ifndef __GENERATED_cls_DomainException_h34b60206__
#define __GENERATED_cls_DomainException_h34b60206__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/LogicException.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/exception.php line 229 */
FORWARD_DECLARE_CLASS(DomainException);
extern const ObjectStaticCallbacks cw_DomainException;
class c_DomainException : public c_LogicException {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_COMMON_NO_SWEEP(DomainException, DomainException)
  c_DomainException(const ObjectStaticCallbacks *cb = &cw_DomainException) : c_LogicException(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
};
ObjectData *coo_DomainException() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_DomainException_h34b60206__
