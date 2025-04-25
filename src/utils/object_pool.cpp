#include "object_pool.hpp"

namespace HXSL
{
	std::unique_ptr<PoolRegistry> PoolRegistry::shared;
	std::once_flag PoolRegistry::initFlag;
}