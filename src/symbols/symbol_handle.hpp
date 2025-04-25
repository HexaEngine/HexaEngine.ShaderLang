#ifndef SYMBOL_HANDLE_HPP
#define SYMBOL_HANDLE_HPP

#include "span.h"

#include <iostream>
#include <memory>
#include <string>

namespace HXSL
{
	class SymbolTable;
	struct SymbolTableNode;
	class SymbolMetadata;

	struct SymbolHandle
	{
	private:
		const SymbolTable* table = nullptr;
		std::weak_ptr<size_t> handle;

	public:
		SymbolHandle(const SymbolTable* table, std::shared_ptr<size_t> ptr) : table(table), handle(ptr) {}

		SymbolHandle() = default;

		static constexpr size_t Invalid = -1;

		bool invalid() const
		{
			return handle.expired();
		}

		bool valid() const
		{
			return !invalid();
		}

		size_t GetIndex() const
		{
			if (auto locked = handle.lock())
			{
				return *locked;
			}
			return Invalid;
		}

		const SymbolTable* GetTable() const
		{
			if (handle.expired())
			{
				return nullptr;
			}
			return table;
		}

		const SymbolTableNode& GetNode() const;

		const SymbolMetadata* GetMetadata() const;

		std::string GetFullyQualifiedName() const;

		SymbolHandle FindFullPath(const StringSpan& span, const SymbolTable* alt = nullptr) const;

		SymbolHandle FindPart(const StringSpan& span) const;
	};
}

#endif