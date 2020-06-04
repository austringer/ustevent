#ifndef USTEVENT_CORE_CONTEXTSTRATEGYWRAPPER_H_
#define USTEVENT_CORE_CONTEXTSTRATEGYWRAPPER_H_

#include "boost/fiber/algo/algorithm.hpp"
#include "ustevent/core/Context.h"

namespace ustevent
{

template <typename Strategy>
class ContextStrategyWrapper : public Strategy
{
public:
  template <typename ... Args>
  ContextStrategyWrapper(Context & context, Args ... args);

  ~ContextStrategyWrapper() noexcept override = default;

  void suspend_until(::std::chrono::steady_clock::time_point const& time_point) noexcept override;

  ContextStrategyWrapper(ContextStrategyWrapper const&) = delete;
  auto & operator=(ContextStrategyWrapper const&)
    -> ContextStrategyWrapper & = delete;

protected:

  auto _launchPostedTask()
    -> ::std::size_t;

private:
  Context & _context;
};

}

#include "ustevent/core/ContextStrategyWrapper.hpp"

#endif // USTEVENT_CORE_CONTEXTSTRATEGYWRAPPER_H_
