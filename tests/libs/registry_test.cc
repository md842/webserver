#include "registry.h"
#include "gtest/gtest.h"


class RegistryTest : public ::testing::Test {
protected:
  void SetUp() override { // Setup test fixture
    Registry::inst().register_mapping("Test", "/uri", "/mapping");
  }
};


TEST_F(RegistryTest, MappingTest){ // Uses test fixture
  // Get types
  std::vector<std::string> types = Registry::inst().get_types();
  EXPECT_EQ(types.at(0), "Test");

  // Get mapping
  std::map<std::string, std::vector<std::string>> map_ =
    Registry::inst().get_map("Test");
  EXPECT_EQ(map_["/uri"].at(0), "/mapping");
}


TEST_F(RegistryTest, PointerTest){ // Uses test fixture
  // get_factory() should return a nullptr since no factory was registered
  RequestHandlerFactory* handler = Registry::inst().get_factory("Test");
  EXPECT_EQ(handler, nullptr);
}
