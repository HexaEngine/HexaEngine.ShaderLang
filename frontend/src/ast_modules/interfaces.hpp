#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include "ast_base.hpp"

#include <memory>
#include <vector>

namespace HXSL
{
#define DEFINE_GET_SET_MOVE_REG_EXPR(type, name, field)    \
    type Get##name() const noexcept { return field; } \
    void Set##name(type value) noexcept { UnregisterExpression(field); field = value; RegisterExpression(this, field); } \
	type Detach##name() noexcept { UnregisterExpression(field); return std::move(field); }

	class IHasExpressions
	{
	private:

	protected:

	public:
	};
}

#endif