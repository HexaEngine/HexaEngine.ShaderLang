#ifndef COMPILATION_HPP
#define COMPILATION_HPP

#include "namespace.hpp"
#include "io/logger.hpp"

#include <mutex>
#include <shared_mutex>

namespace HXSL
{
	class Compilation : virtual public ASTNode, public ILogger
	{
	private:
		std::atomic<size_t> currentID = 0;
		std::vector<std::unique_ptr<Namespace>> namespaces;
		std::vector<UsingDeclaration> usings;

		std::vector<std::unique_ptr<Primitive>> primitives;
		std::vector<std::unique_ptr<Class>> primitiveClasses;

		std::vector<std::unique_ptr<Array>> arrays;

		std::shared_mutex _mutex;

		friend class PrimitiveManager;
		friend class PrimitiveBuilder;
		friend class SymbolResolver;

		void AddArray(std::unique_ptr<Array> array)
		{
			arrays.push_back(std::move(array));
		}

	public:
		Compilation(bool isExtern = false)
			: ASTNode({ }, NodeType_Compilation, isExtern), ILogger()
		{
			AssignId();
		}

		size_t GetNextID()
		{
			return currentID.fetch_add(1, std::memory_order_relaxed);
		}

		Compilation* GetCompilation() override
		{
			return this;
		}

		Namespace* AddNamespace(const NamespaceDeclaration& declaration)
		{
			std::shared_lock<std::shared_mutex> lock(_mutex);

			for (auto& ns : namespaces)
			{
				if (ns->GetName() == declaration.Name)
				{
					return ns.get();
				}
			}

			lock.unlock();
			std::unique_lock<std::shared_mutex> uniqueLock(_mutex);

			auto ns = std::make_unique<Namespace>(declaration);
			ns->SetParent(this);
			auto pNs = ns.get();
			namespaces.push_back(std::move(ns));
			return pNs;
		}

		void AddNamespace(std::unique_ptr<Namespace> ns)
		{
			HXSL_ASSERT(isExtern, "Cannot add namespace HXSL manually on non extern compilations.");
			ns->SetParent(this);
			namespaces.push_back(std::move(ns));
		}

		const std::vector<std::unique_ptr<Namespace>>& GetNamespaces() const noexcept
		{
			return namespaces;
		}

		void AddUsing(UsingDeclaration _using)
		{
			usings.push_back(_using);
		}

		void Clear()
		{
			usings.clear();
			namespaces.clear();
		}

		std::vector<UsingDeclaration>& GetUsings() noexcept { return usings; }
	};
}

#endif