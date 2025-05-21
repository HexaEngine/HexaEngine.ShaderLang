#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include "ast_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class AttributeDeclaration : public ASTNode, public IHasExpressions
	{
	private:
		ast_ptr<SymbolRef> symbol;
		std::vector<ast_ptr<Expression>> parameters;
	public:
		AttributeDeclaration(TextSpan span, ast_ptr<SymbolRef> symbol, std::vector<ast_ptr<Expression>> parameters)
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

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, Symbol, symbol)

			const std::vector<ast_ptr<Expression>>& GetParameters() const noexcept
		{
			return parameters;
		}

		void SetParameters(std::vector<ast_ptr<Expression>> value) noexcept
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
		std::vector<ast_ptr<AttributeDeclaration>> attributes;
	public:
		AttributeContainer(ASTNode* self) : self(self)
		{
		}
		virtual ~AttributeContainer() {}
		void AddAttribute(ast_ptr<AttributeDeclaration> attribute)
		{
			attribute->SetParent(self);
			attributes.push_back(std::move(attribute));
		}

		const std::vector<ast_ptr<AttributeDeclaration>>& GetAttributes() const noexcept
		{
			return attributes;
		}
	};
}

#endif