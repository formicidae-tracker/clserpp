#include "clserpp.hpp"
#include <gtest/gtest.h>

TEST(Foo, Bar) {
	EXPECT_NO_THROW({ fort::clserpp::Serial::GetManufacturerInfos(); });
}
