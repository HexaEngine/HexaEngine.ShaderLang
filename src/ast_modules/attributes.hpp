#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include "ast_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
#define DEFINE_GET_SET_MOVE_REG_EXPR(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(type&& value) noexcept { UnregisterExpression(field.get()); field = std::move(value); RegisterExpression(this, field.get()); } \
	type Detach##name() noexcept { UnregisterExpression(field.get()); return std::move(field); }

	class AttributeDeclaration : public ASTNode, public IHasExpressions
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::vector<std::unique_ptr<Expression>> parameters;
	public:
		AttributeDeclaration(TextSpan span, std::unique_ptr<SymbolRef> symbol, std::vector<std::unique_ptr<Expression>> parameters)
			: ASTNode(span, NodeType_AttributeDeclaration),
			symbol(std::move(symbol)),
			parameters(std::move(parameters))
		{
			RegisterExpressions(this, this->parameters);
		}

		AttributeDeclaration(TextSpan span)
			: ASTNode(span, NodeType_AttributeDeclaration)
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
			UnregisterExpression(parameters);
			parameters = std::move(value);
			RegisterExpressions(this, parameters);
		}
	};

	class AttributeContainer
	{
	protected:
		ASTNode* self;
		std::vector<std::unique_ptr<AttributeDeclaration>> attributes;
	public:
		AttributeContainer(ASTNode* self) : self(self)
		{
		}
		virtual ~AttributeContainer() {}
		void AddAttribute(std::unique_ptr<AttributeDeclaration> attribute)
		{
			attribute->SetParent(self);
			attributes.push_back(std::move(attribute));
		}

		const std::vector<std::unique_ptr<AttributeDeclaration>>& GetAttributes() const noexcept
		{
			return attributes;
		}
	};
}

#endif