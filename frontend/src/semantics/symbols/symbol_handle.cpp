#include "symbol_handle.hpp"
#include "symbol_table.hpp"

namespace HXSL
{
	const SymbolTableNode& SymbolHandle::GetNode() const
	{
		size_t index = GetIndex();
		if (index == Invalid)
		{
			static const SymbolTableNode emptyNode = {};
			return emptyNode;
		}
		return table->GetNode(index);
	}

	const SymbolMetadata* SymbolHandle::GetMetadata() const
	{
		return GetNode().Metadata.get();
	}

	std::string SymbolHandle::GetFullyQualifiedName() const
	{
		size_t index = GetIndex();
		if (index == Invalid)
		{
			return {};
		}
		return table->GetFullyQualifiedName(index);
	}

	SymbolHandle SymbolHandle::FindFullPath(const StringSpan& span, const SymbolTable* alt) const
	{
		size_t index = GetIndex();
		if (index == Invalid)
		{
			if (alt)
			{
				return alt->FindNodeIndexFullPath(span, 0);
			}
			return {};
		}

		SymbolHandle handle = table->FindNodeIndexFullPath(span, index);
		return handle;
	}

	SymbolHandle SymbolHandle::FindPart(const StringSpan& span) const
	{
		size_t index = GetIndex();
		if (index == Invalid)
		{
			return {};
		}

		SymbolHandle handle = table->FindNodeIndexPart(span, index);
		return handle;
	}
}