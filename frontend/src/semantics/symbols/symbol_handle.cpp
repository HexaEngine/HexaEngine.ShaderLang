#include "symbol_handle.hpp"
#include "symbol_table.hpp"

namespace HXSL
{
	SymbolTableNode* SymbolHandle::GetNode() const
	{
		return node;
	}

	const SymbolMetadata* SymbolHandle::GetMetadata() const
	{
		return GetNode()->GetMetadata().Get();
	}

	std::string SymbolHandle::GetFullyQualifiedName() const
	{
		if (invalid())
		{
			return {};
		}
		return table->GetFullyQualifiedName(node);
	}

	SymbolHandle SymbolHandle::FindFullPath(const StringSpan& span, const SymbolTable* alt) const
	{
		if (invalid())
		{
			if (alt)
			{
				return alt->FindNodeIndexFullPath(span, nullptr);
			}
			return {};
		}

		SymbolHandle handle = table->FindNodeIndexFullPath(span, node);
		return handle;
	}

	SymbolHandle SymbolHandle::FindPart(const StringSpan& span) const
	{
		if (invalid())
		{
			return {};
		}

		SymbolHandle handle = table->FindNodeIndexPart(span, node);
		return handle;
	}
}