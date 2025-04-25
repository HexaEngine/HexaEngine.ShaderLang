#ifndef PRIMITIVE_MANAGER_HPP
#define PRIMITIVE_MANAGER_HPP

#include "primitive.hpp"

#include <mutex>

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

		void Populate();

	private:
		static std::once_flag initFlag;

		SymbolTable* GetMutableSymbolTable() const
		{
			return assembly->GetMutableSymbolTable();
		}

		PrimitiveManager() = default;

		PrimitiveManager(const PrimitiveManager&) = delete;
		PrimitiveManager& operator=(const PrimitiveManager&) = delete;

		std::unique_ptr<Assembly> assembly;

		void AddPrimClass(TextSpan name, Class** outClass = nullptr, SymbolHandle* symbolOut = nullptr);
		void AddPrim(PrimitiveKind kind, PrimitiveClass primitiveClass, uint rows, uint columns);
		void ResolveInternal(SymbolRef* ref);
	};
}

#endif