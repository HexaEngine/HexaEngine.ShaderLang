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

		AttributeDecl(const TextSpan& span, SymbolRef* symbol)
			: ASTNode(span, ID),
			symbol(symbol)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_AttributeDeclaration;

		static AttributeDecl* Create(const TextSpan& span, SymbolRef* symbol, const Span<Expression*>& parameters);
		static AttributeDecl* Create(const TextSpan& span, SymbolRef* symbol, uint32_t numParameters);

		SymbolRef*& GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetParameters, 0, storage);

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, Symbol, symbol);

		void ForEachChild(ASTChildCallback cb, void* userdata);
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const;
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