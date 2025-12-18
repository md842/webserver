#include "registry.h"
#include "gtest/gtest.h"


TEST(RegistryTest, PointerTest){ // Uses test fixture
  // get_factory() should return a nullptr since no factory was registered
  RequestHandlerFactory* handler = Registry::inst().get_factory("Test");
  EXPECT_EQ(handler, nullptr);
}
