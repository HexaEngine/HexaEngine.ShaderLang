#ifndef COMPILATION_HPP
#define COMPILATION_HPP

#include "vector.h"
#include "log.h"
#include "text_span.h"
#include "nodes.hpp"

#include <string>
namespace HXSL
{
	class HXSLCompilation : virtual public HXSLNode, public HXSLContainer, public ILogger
	{
	private:
		std::atomic<size_t> currentID = 0;
		std::vector<std::unique_ptr<HXSLNamespace>> namespaces;
		std::vector<UsingDeclaration> Usings;

	public:
		HXSLCompilation(bool isExtern = false)
			: HXSLNode({ }, nullptr, HXSLNodeType_Compilation, isExtern), HXSLContainer({ }, nullptr, HXSLNodeType_Compilation, isExtern), ILogger()
		{
			AssignId();
		}

		size_t GetNextID()
		{
			return currentID.fetch_add(1, std::memory_order_relaxed);
		}

		HXSLCompilation* GetCompilation() override
		{
			return this;
		}

		HXSLNamespace* AddNamespace(const NamespaceDeclaration& declaration)
		{
			for (auto& ns : namespaces)
			{
				if (ns->GetName() == declaration.Name)
				{
					return ns.get();
				}
			}
			auto ns = std::make_unique<HXSLNamespace>(this, declaration);
			auto pNs = ns.get();
			namespaces.push_back(std::move(ns));
			return pNs;
		}

		void AddNamespace(std::unique_ptr<HXSLNamespace> ns)
		{
			HXSL_ASSERT(isExtern, "Cannot add namespace manually on non extern compilations.");
			ns->SetParent(this);
			namespaces.push_back(std::move(ns));
		}

		const std::vector<std::unique_ptr<HXSLNamespace>>& GetNamespaces() const noexcept
		{
			return namespaces;
		}

		void AddUsing(UsingDeclaration _using)
		{
			Usings.push_back(_using);
		}

		void Clear()
		{
			Usings.clear();
			namespaces.clear();
			Functions.clear();
			Classes.clear();
			Structs.clear();
			Fields.clear();
		}

		std::vector<UsingDeclaration>& GetUsings() noexcept { return Usings; }
	};
}
#endif