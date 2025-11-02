#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include "ast_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class AttributeDecl : public ASTNode, public IHasExpressions, public TrailingObjects<AttributeDecl, Expression*>
	{
		friend class ASTContext;
	private:
		SymbolRef* symbol;
		TrailingObjStorage<AttributeDecl, uint32_t> storage;

	public:
		static constexpr NodeType ID = NodeType_AttributeDeclaration;
		AttributeDecl(TextSpan span, SymbolRef* symbol)
			: ASTNode(span, ID),
			symbol(symbol)
		{
		}

		AttributeDecl(TextSpan span)
			: ASTNode(span, ID)
		{
		}

		static AttributeDecl* Create(TextSpan span, SymbolRef* symbol, const ArrayRef<Expression*>& parameters);

		SymbolRef*& GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetParameters, 0, storage);

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, Symbol, symbol)
	};

	class AttributeContainer
	{
	protected:
		ASTNode* self;
		std::vector<ast_ptr<AttributeDecl>> attributes;
	public:
		AttributeContainer(ASTNode* self) : self(self)
		{
		}
		virtual ~AttributeContainer() {}
		void AddAttribute(ast_ptr<AttributeDecl> attribute)
		{
			attribute->SetParent(self);
			attributes.push_back(std::move(attribute));
		}

		const std::vector<ast_ptr<AttributeDecl>>& GetAttributes() const noexcept
		{
			return attributes;
		}
	};
}

#endif