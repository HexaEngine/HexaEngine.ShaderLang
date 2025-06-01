#ifndef DECLARATIONS_HPP
#define DECLARATIONS_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "attributes.hpp"
#include "container.hpp"

namespace HXSL
{
	class Parameter : public SymbolDef
	{
		friend class ASTContext;
	private:
		ParameterFlags paramaterFlags;
		InterpolationModifier interpolationModifiers;
		ast_ptr<SymbolRef> symbol;
		IdentifierInfo* semantic;

		Parameter(const TextSpan& span, IdentifierInfo* name, ParameterFlags flags, InterpolationModifier interpolationModifiers, ast_ptr<SymbolRef>&& symbol, IdentifierInfo* semantic)
			: SymbolDef(span, ID, name),
			paramaterFlags(flags),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Parameter;
		static Parameter* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ParameterFlags flags, InterpolationModifier interpolationModifiers, ast_ptr<SymbolRef>&& symbol, IdentifierInfo* semantic);

		void SetSymbolRef(ast_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		SymbolDef* GetDeclaredType() const noexcept
		{
			return symbol->GetDeclaration();
		}

		DEFINE_GETTER_SETTER(ParameterFlags, ParameterFlags, paramaterFlags)

			DEFINE_GETTER_SETTER(InterpolationModifier, InterpolationModifiers, interpolationModifiers)

			DEFINE_GETTER_SETTER_PTR(IdentifierInfo*, Semantic, semantic)
	};

	class FunctionOverload : public SymbolDef, public AttributeContainer, protected TrailingObjects<FunctionOverload, ast_ptr<Parameter>>
	{
		friend class ASTContext;
		friend class TrailingObjects;
	protected:
		std::string cachedSignature;
		AccessModifier accessModifiers;
		FunctionFlags functionFlags;
		ast_ptr<SymbolRef> returnSymbol;
		IdentifierInfo* semantic;
		ast_ptr<BlockStatement> body;
		uint32_t numParameters;

		FunctionOverload(const TextSpan& span, NodeType type, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef> returnSymbol)
			: SymbolDef(span, type, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}

	public:
		static constexpr NodeType ID = NodeType_FunctionOverload;
		static FunctionOverload* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef>&& returnSymbol, ArrayRef<ast_ptr<Parameter>>& parameters);
		static FunctionOverload* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef>&& returnSymbol, uint32_t numParameters);

		ArrayRef<ast_ptr<Parameter>> GetParameters()
		{
			return { GetTrailingObjects<0>(numParameters), numParameters };
		}

		ast_ptr<SymbolRef>& GetReturnSymbolRef()
		{
			return returnSymbol;
		}

		SymbolDef* GetReturnType() const noexcept
		{
			return returnSymbol->GetDeclaration();
		}

		std::string BuildOverloadSignature(bool placeholder) noexcept;

		std::string BuildOverloadSignature() noexcept
		{
			return BuildOverloadSignature(false);
		}

		std::string BuildTemporaryOverloadSignature() noexcept
		{
			return BuildOverloadSignature(true);
		}

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] Name: " << name;
			return oss.str();
		}

		DEFINE_GETTER_SETTER(FunctionFlags, FunctionFlags, functionFlags)
			DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
			DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, ReturnSymbol, returnSymbol)
			DEFINE_GETTER_SETTER_PTR(IdentifierInfo*, Semantic, semantic)

			const ast_ptr<BlockStatement>& GetBody() const noexcept;

		void SetBody(ast_ptr<BlockStatement>&& value) noexcept;

		ast_ptr<BlockStatement>& GetBodyMut() noexcept;
	};

	class ConstructorOverload : public FunctionOverload
	{
		friend class ASTContext;
	private:
		ast_ptr<SymbolRef> targetTypeSymbol;

		ConstructorOverload(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef> targetTypeSymbol, ast_ptr<SymbolRef>&& returnType)
			: FunctionOverload(span, ID, name, accessModifiers, functionFlags, std::move(returnType)),
			targetTypeSymbol(std::move(targetTypeSymbol))
		{
		}

	public:
		static constexpr NodeType ID = NodeType_ConstructorOverload;
		static ConstructorOverload* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef>&& targetTypeSymbol, ArrayRef<ast_ptr<Parameter>>& parameters);
		static ConstructorOverload* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef>&& targetTypeSymbol, uint32_t numParameters);

		ast_ptr<SymbolRef>& GetTargetTypeSymbolRef()
		{
			return targetTypeSymbol;
		}

		SymbolDef* GetTargetType() const noexcept
		{
			return targetTypeSymbol->GetDeclaration();
		}

		std::string BuildOverloadSignature(bool placeholder) noexcept;

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, TargetTypeSymbol, targetTypeSymbol)
	};

	class OperatorOverload : public FunctionOverload
	{
		friend class ASTContext;
	private:
		OperatorFlags operatorFlags;
		Operator _operator;
	public:
		static constexpr NodeType ID = NodeType_OperatorOverload;
		OperatorOverload(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, Operator _operator, ast_ptr<SymbolRef>&& returnSymbol)
			: FunctionOverload(span, ID, name, accessModifiers, functionFlags, std::move(returnSymbol)),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		static OperatorOverload* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, Operator _operator, ast_ptr<SymbolRef>&& returnSymbol, ArrayRef<ast_ptr<Parameter>>& parameters);
		static OperatorOverload* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, Operator _operator, ast_ptr<SymbolRef>&& returnSymbol, uint32_t numParameters);

		std::string BuildOverloadSignature(bool placeholder) noexcept;

		const OperatorFlags& GetOperatorFlags() const noexcept { return operatorFlags; }

		void SetOperatorFlags(const OperatorFlags& value) noexcept { operatorFlags = value; }

		const Operator& GetOperator() const noexcept { return _operator; }

		void SetOperator(const Operator& value) noexcept { _operator = value; }
	};

	class Field : public SymbolDef
	{
		friend class ASTContext;
	private:
		AccessModifier accessModifiers;
		StorageClass storageClass;
		InterpolationModifier interpolationModifiers;
		ast_ptr<SymbolRef> symbol;
		IdentifierInfo* semantic;

		Field(const TextSpan& span, IdentifierInfo* name, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, ast_ptr<SymbolRef>&& symbol, IdentifierInfo* semantic)
			: SymbolDef(span, ID, name),
			accessModifiers(access),
			storageClass(storageClass),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Field;
		static Field* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, ast_ptr<SymbolRef>&& symbol, IdentifierInfo* semantic);

		void SetSymbolRef(ast_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		const StorageClass& GetStorageClass() const noexcept
		{
			return storageClass;
		}

		bool IsConstant() const override
		{
			return (storageClass & StorageClass_Const) != 0;
		}

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)

			DEFINE_GETTER_SETTER(InterpolationModifier, InterpolationModifiers, interpolationModifiers)

			DEFINE_GETTER_SETTER_PTR(IdentifierInfo*, Semantic, semantic)
	};

	class ThisDef : public SymbolDef
	{
		friend class ASTContext;
	private:
		ast_ptr<SymbolRef> parentSymbol;

		ThisDef(IdentifierInfo* name, ast_ptr<SymbolRef>&& parentSymbol)
			: SymbolDef(TextSpan(), ID, name),
			parentSymbol(std::move(parentSymbol))
		{
		}
	public:
		static constexpr NodeType ID = NodeType_ThisDef;
		static ThisDef* Create(ASTContext* context, IdentifierInfo* parentIdentifier);

		void SetSymbolRef(ast_ptr<SymbolRef> ref)
		{
			parentSymbol = std::move(ref);
		}

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return parentSymbol;
		}

		SymbolDef* GetDeclaredType() const noexcept
		{
			return parentSymbol->GetDeclaration();
		}
	};

	class Struct : public Type, TrailingObjects<Struct, ast_ptr<Field>, ast_ptr<Struct>, ast_ptr<Class>, ast_ptr<ConstructorOverload>, ast_ptr<FunctionOverload>, ast_ptr<OperatorOverload>>
	{
		friend class ASTContext;
	private:
		ast_ptr<ThisDef> thisDef;
		uint32_t numFields;
		uint32_t numClasses;
		uint32_t numStructs;
		uint32_t numConstructors;
		uint32_t numFunctions;
		uint32_t numOperators;

		Struct(const TextSpan& span, IdentifierInfo* name, AccessModifier access, ast_ptr<ThisDef>&& thisDef)
			: Type(span, ID, name, access),
			thisDef(std::move(thisDef))
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Struct;
		static Struct* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, ArrayRef<ast_ptr<Field>>& fields, ArrayRef<ast_ptr<Struct>>& structs, ArrayRef<ast_ptr<Class>>& classes, ArrayRef<ast_ptr<ConstructorOverload>>& constructors, ArrayRef<ast_ptr<FunctionOverload>>& functions, ArrayRef<ast_ptr<OperatorOverload>>& operators);
		static Struct* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, uint32_t numFields, uint32_t numStructs, uint32_t numClasses, uint32_t numConstructors, uint32_t numFunctions, uint32_t numOperators);

		ArrayRef<ast_ptr<Field>> GetFields()
		{
			return { GetTrailingObjects<0>(numFields, numStructs, numClasses,  numConstructors, numFunctions, numOperators), numFields };
		}

		ArrayRef<ast_ptr<Struct>> GetStructs()
		{
			return { GetTrailingObjects<1>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numStructs };
		}

		ArrayRef<ast_ptr<Class>> GetClasses()
		{
			return { GetTrailingObjects<2>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numClasses };
		}

		ArrayRef<ast_ptr<ConstructorOverload>> GetConstructors()
		{
			return { GetTrailingObjects<3>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numConstructors };
		}

		ArrayRef<ast_ptr<FunctionOverload>> GetFunctions()
		{
			return { GetTrailingObjects<4>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numFunctions };
		}

		ArrayRef<ast_ptr<OperatorOverload>> GetOperators()
		{
			return { GetTrailingObjects<5>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numOperators };
		}

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] Name: " << name;
			return oss.str();
		}

		const ast_ptr<ThisDef>& GetThisDef() const
		{
			return thisDef;
		}
	};

	class Class : public Type, TrailingObjects<Class, ast_ptr<Field>, ast_ptr<Struct>, ast_ptr<Class>, ast_ptr<ConstructorOverload>, ast_ptr<FunctionOverload>, ast_ptr<OperatorOverload>>
	{
		friend class ASTContext;
	private:
		uint32_t numFields;
		uint32_t numClasses;
		uint32_t numStructs;
		uint32_t numConstructors;
		uint32_t numFunctions;
		uint32_t numOperators;
		ast_ptr<ThisDef> thisDef;

		Class(const TextSpan& span, IdentifierInfo* name, AccessModifier access, ast_ptr<ThisDef>&& thisDef)
			: Type(span, ID, name, access),
			thisDef(std::move(thisDef))
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Class;
		static Class* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, ArrayRef<ast_ptr<Field>>& fields, ArrayRef<ast_ptr<Struct>>& structs, ArrayRef<ast_ptr<Class>>& classes, ArrayRef<ast_ptr<ConstructorOverload>>& constructors, ArrayRef<ast_ptr<FunctionOverload>>& functions, ArrayRef<ast_ptr<OperatorOverload>>& operators);
		static Class* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, uint32_t numFields, uint32_t numStructs, uint32_t numClasses, uint32_t numConstructors, uint32_t numFunctions, uint32_t numOperators);

		ArrayRef<ast_ptr<Field>> GetFields()
		{
			return { GetTrailingObjects<0>(numFields, numStructs, numClasses,  numConstructors, numFunctions, numOperators), numFields };
		}

		ArrayRef<ast_ptr<Struct>> GetStructs()
		{
			return { GetTrailingObjects<1>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numStructs };
		}

		ArrayRef<ast_ptr<Class>> GetClasses()
		{
			return { GetTrailingObjects<2>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numClasses };
		}

		ArrayRef<ast_ptr<ConstructorOverload>> GetConstructors()
		{
			return { GetTrailingObjects<3>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numConstructors };
		}

		ArrayRef<ast_ptr<FunctionOverload>> GetFunctions()
		{
			return { GetTrailingObjects<4>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numFunctions };
		}

		ArrayRef<ast_ptr<OperatorOverload>> GetOperators()
		{
			return { GetTrailingObjects<5>(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), numOperators };
		}

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] Name: " << name;
			return oss.str();
		}
	};
}

#endif