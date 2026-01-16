#ifndef AST_BUILDER_HPP
#define AST_BUILDER_HPP

#include "ast_base.hpp"
#include <logging/logger_adapter.hpp>

namespace HXSL
{
	enum class DeclContainerFlags : uint16_t
	{
		None,
		AllowFields,
		AllowStructs,
		AllowClasses,
		AllowEnums,
		AllowConstructors,
		AllowFunctions,
		AllowOperators,
		AllowNamespaces,
		AllowUsings,

		AllowPresetStruct = AllowFields | AllowStructs | AllowClasses | AllowConstructors | AllowFunctions | AllowOperators,
		AllowPresetNamespace = AllowFields | AllowStructs | AllowClasses | AllowEnums | AllowNamespaces | AllowFunctions | AllowUsings,
		AllowPresetCompilationUnit = AllowNamespaces | AllowUsings,
	};

	DEFINE_FLAGS_OPERATORS(DeclContainerFlags, uint16_t);

	struct DeclContainerBuilder : public LoggerAdapter
	{
		std::vector<Field*> fields;
		std::vector<Struct*> structs;
		std::vector<Class*> classes;
		std::vector<ConstructorOverload*> constructors;
		std::vector<FunctionOverload*> functions;
		std::vector<OperatorOverload*> operators;
		std::vector<Namespace*> namespaces;
		std::vector<UsingDecl*> usings;
		DeclContainerFlags flags;

		DeclContainerBuilder(ILogger* logger, DeclContainerFlags flags) : LoggerAdapter(logger), flags(flags)
		{
		}

		template<typename T, DeclContainerFlags Flags>
		void Append(ASTNode* node, std::vector<T*>& vec, DiagnosticCode code)
		{
			if ((flags & Flags) != Flags)
			{
				Log(code, node->GetSpan());
			}
			else
			{
				vec.push_back(cast<T>(node));
			}
		}

		void AddDeclaration(ASTNode* decl);
	};

	class CompilationUnitBuilder
	{
		DeclContainerBuilder builder;

	public:
		CompilationUnitBuilder(ILogger* logger) : builder(DeclContainerBuilder(logger, DeclContainerFlags::AllowPresetCompilationUnit)) {}
		DeclContainerBuilder& GetBuilder() noexcept { return builder; }
		CompilationUnit* Build(bool isExtern = false);
	};
}

#endif