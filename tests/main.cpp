#include "generated/localization.hpp"
#include "common.hpp"

int main(int argc, char** argv)
{
	SetLocale("en_US");
	EnableErrorOutput = false;
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}