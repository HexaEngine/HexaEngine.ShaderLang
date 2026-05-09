#pragma once

#include "module.hpp"

namespace HXSL
{
	namespace Backend
	{
		class NameMangler
		{
			using ModuleId = LayoutDataTypes::ModuleId;
			dense_map<const Module*, ModuleId> moduleMap;
			vector<const Module*> modules;
			Module& module;
			ModuleReferenceTableBuilder& referenceTableBuilder;

		public:
			NameMangler(Module& module, ModuleReferenceTableBuilder& referenceTableBuilder);

			ModuleId GetModuleId(const Module* module);
			void Mangle(const Module* mod, std::string& out);
			void Mangle(const FunctionLayout* func, std::string& out);
			void Mangle(const OperatorLayout* op, std::string& out);
			void Mangle(const ConstructorLayout* ctor, std::string& out);
			void Mangle(const ParameterLayout* param, std::string& out);
			void Mangle(const FieldLayout* field, std::string& out);
			void Mangle(const EnumItemLayout* item, std::string& out);
			void Mangle(const NamespaceLayout* ns, std::string& out);
			void Mangle(const TypeLayout* type, std::string& out);
			void Mangle(const Layout* lay, std::string& out);
			void AppendRoot(const Module* module, std::string& out);
			void ManglePath(const Layout* layout, std::string& out) const;
			void MangleExtern(const Layout* layout, std::string& out);
		};
	}
}