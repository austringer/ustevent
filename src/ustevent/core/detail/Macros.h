#ifndef USTEVENT_CORE_DETAIL_MACROS_H_
#define USTEVENT_CORE_DETAIL_MACROS_H_

#include <cstddef> // size_t.

namespace ustevent
{
namespace detail
{

template <typename T, ::std::size_t N>
char ( &ArraySizeHelper( T (&array)[N] ))[N];

static inline
long likely(long expression)
{
  return __builtin_expect(expression, 1);
}

static inline
long unlikely(long expression)
{
  return __builtin_expect(expression, 0);
}

}
}

#define USTEVENT_ARRAY_SIZE(array) (sizeof(::ustevent::detail::ArraySizeHelper(array)))

#define USTEVENT_LIKELY(exp) ::ustevent::detail::likely(exp)
#define USTEVENT_UNLIKELY(exp) ::ustevent::detail::unlikely(exp)

#endif // USTEVENT_CORE_DETAIL_MACROS_H_
