#include "symbol_handle.hpp"
#include "symbol_table.hpp"

namespace HXSL
{
	const SymbolTable* SymbolHandle::GetTable() const
	{
		return node->GetTable();
	}

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
		return GetTable()->GetFullyQualifiedName(node);
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

		SymbolHandle handle = GetTable()->FindNodeIndexFullPath(span, node);
		return handle;
	}

	SymbolHandle SymbolHandle::FindPart(const StringSpan& span) const
	{
		if (invalid())
		{
			return {};
		}

		SymbolHandle handle = GetTable()->FindNodeIndexPart(span, node);
		return handle;
	}
}