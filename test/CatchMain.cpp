#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include "ustevent/core/Context.h"

int main(int argc, char* argv[])
{
  ustevent::installMainContext();

  int result = Catch::Session().run(argc, argv);

  return result;
}
