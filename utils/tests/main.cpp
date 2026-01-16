#include "utils/trailing_objects.hpp"

using namespace HXSL;

class Foo : public TrailingObjects<Foo, uint64_t>
{
	float a;
	float b;
	uint32_t c;
};

int main()
{
	auto size = Foo::TotalSizeToAlloc(1);
	char* mem = new char[size];
	Foo* ptr = new (mem) Foo();

	auto f = ptr->GetTrailingObjects<0>(1);
	return 0;
}