#ifndef SYMBOL_HANDLE_HPP
#define SYMBOL_HANDLE_HPP

#include "utils/span.hpp"

namespace HXSL
{
	class SymbolTable;
	class SymbolTableNode;
	class SymbolMetadata;

	struct SymbolHandle
	{
	private:
		SymbolTableNode* node = nullptr;

	public:
		SymbolHandle(SymbolTableNode* node) : node(node) {}

		SymbolHandle() = default;

		bool invalid() const { return node == nullptr; }

		bool valid() const { return node != nullptr; }

		constexpr operator SymbolTableNode*() const
		{
			return node;
		}

		const SymbolTable* GetTable() const;

		SymbolTableNode* GetNode() const;

		const SymbolMetadata* GetMetadata() const;

		std::string GetFullyQualifiedName() const;

		SymbolHandle FindFullPath(const StringSpan& span, const SymbolTable* alt = nullptr) const;

		SymbolHandle FindPart(const StringSpan& span) const;
	};
}

#endif