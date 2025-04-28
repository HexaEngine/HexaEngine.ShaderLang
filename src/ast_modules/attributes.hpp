#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include "ast_base.hpp"

namespace HXSL
{
#define DEFINE_GET_SET_MOVE_REG_EXPR(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(type&& value) noexcept { UnregisterExpression(field.get()); field = std::move(value); RegisterExpression(field.get()); } \
	type Detach##name() noexcept { UnregisterExpression(field.get()); return std::move(field); }

	class AttributeDeclaration : public ASTNode, public IHasExpressions
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::vector<std::unique_ptr<Expression>> parameters;
	public:
		AttributeDeclaration(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, std::vector<std::unique_ptr<Expression>> parameters)
			: ASTNode(span, parent, NodeType_AttributeDeclaration),
			symbol(std::move(symbol)),
			parameters(std::move(parameters))
		{
			for (auto& param : this->parameters)
			{
				RegisterExpression(param.get());
			}
		}
		AttributeDeclaration(TextSpan span, ASTNode* parent)
			: ASTNode(span, parent, NodeType_AttributeDeclaration)
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			const std::vector<std::unique_ptr<Expression>>& GetParameters() const noexcept
		{
			return parameters;
		}

		void SetParameters(std::vector<std::unique_ptr<Expression>> value) noexcept
		{
			for (auto& param : parameters)
			{
				UnregisterExpression(param.get());
			}
			parameters = std::move(value);
			for (auto& param : parameters)
			{
				RegisterExpression(param.get());
			}
		}
	};

	class AttributeContainer
	{
	protected:
		std::vector<std::unique_ptr<AttributeDeclaration>> attributes;
	public:
		virtual ~AttributeContainer() {}
		void AddAttribute(std::unique_ptr<AttributeDeclaration> attribute)
		{
			attribute->SetParent(dynamic_cast<ASTNode*>(this));
			attributes.push_back(std::move(attribute));
		}

		const std::vector<std::unique_ptr<AttributeDeclaration>>& GetAttributes() const noexcept
		{
			return attributes;
		}
	};
}

#endif