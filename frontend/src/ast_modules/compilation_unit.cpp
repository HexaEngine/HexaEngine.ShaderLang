#include "compilation_unit.hpp"

namespace HXSL
{
	Namespace* CompilationUnit::AddNamespace(const NamespaceDeclaration& declaration)
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);

		for (auto& ns : namespaces)
		{
			const StringSpan& sp = ns->GetName();
			if (sp == declaration.Name->name)
			{
				return ns.get();
			}
		}

		lock.unlock();
		std::unique_lock<std::shared_mutex> uniqueLock(_mutex);

		auto ns = make_ast_ptr<Namespace>(declaration);
		ns->SetParent(this);
		auto pNs = ns.get();
		namespaces.push_back(std::move(ns));
		return pNs;
	}
}