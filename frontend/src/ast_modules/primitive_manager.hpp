#ifndef PRIMITIVE_MANAGER_HPP
#define PRIMITIVE_MANAGER_HPP

#include "primitive.hpp"

namespace HXSL
{
	class PrimitiveManager
	{
		ASTContext* context;
	public:
		PrimitiveManager(ASTContext& context) : context(&context)
		{
			Populate();
		}

		const SymbolTable* GetSymbolTable() const
		{
			return assembly->GetSymbolTable();
		}

		SymbolHandle Resolve(const StringSpan& span) const;

	private:
		SymbolTable* GetMutableSymbolTable() const
		{
			return assembly->GetMutableSymbolTable();
		}

		PrimitiveManager() = default;

		void Populate();

		PrimitiveManager(const PrimitiveManager&) = delete;
		PrimitiveManager& operator=(const PrimitiveManager&) = delete;

		std::unique_ptr<Assembly> assembly = Assembly::Create("HXSL.Core");

		void ResolveInternal(SymbolRef* ref);
	};
}

#endif