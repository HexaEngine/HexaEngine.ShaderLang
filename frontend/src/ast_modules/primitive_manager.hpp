#ifndef PRIMITIVE_MANAGER_HPP
#define PRIMITIVE_MANAGER_HPP

#include "primitive.hpp"

namespace HXSL
{
	class PrimitiveManager
	{
	public:
		static PrimitiveManager& GetInstance()
		{
			static PrimitiveManager instance;

			std::call_once(initFlag, []() {
				instance.assembly = Assembly::Create("HXSL.Core");
				instance.Populate();
				instance.assembly->Seal();
				});

			return instance;
		}

		const SymbolTable* GetSymbolTable() const
		{
			return assembly->GetSymbolTable();
		}

		SymbolHandle Resolve(const StringSpan& span) const;

	private:
		static std::once_flag initFlag;

		SymbolTable* GetMutableSymbolTable() const
		{
			return assembly->GetMutableSymbolTable();
		}

		PrimitiveManager() = default;

		void Populate();

		PrimitiveManager(const PrimitiveManager&) = delete;
		PrimitiveManager& operator=(const PrimitiveManager&) = delete;

		std::unique_ptr<Assembly> assembly;

		void AddPrimClass(const std::string& name, Class** outClass = nullptr, SymbolHandle* symbolOut = nullptr);
		void ResolveInternal(SymbolRef* ref);
	};
}

#endif