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
		SymbolRef* symbol;
		IdentifierInfo* semantic;

		Parameter(const TextSpan& span, IdentifierInfo* name, ParameterFlags flags, InterpolationModifier interpolationModifiers, SymbolRef* symbol, IdentifierInfo* semantic)
			: SymbolDef(span, ID, name),
			paramaterFlags(flags),
			interpolationModifiers(interpolationModifiers),
			symbol(symbol),
			semantic(semantic)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Parameter;
		static Parameter* Create(const TextSpan& span, IdentifierInfo* name, ParameterFlags flags, InterpolationModifier interpolationModifiers, SymbolRef* symbol, IdentifierInfo* semantic);

		void SetSymbolRef(SymbolRef* value)
		{
			symbol = value;
		}

		SymbolRef* GetSymbolRef()
		{
			return symbol;
		}

		SymbolDef* GetDeclaredType() const noexcept
		{
			return symbol->GetDeclaration();
		}

		DEFINE_GETTER_SETTER(ParameterFlags, ParameterFlags, paramaterFlags);
		DEFINE_GETTER_SETTER(InterpolationModifier, InterpolationModifiers, interpolationModifiers);
		DEFINE_GETTER_SETTER_PTR(IdentifierInfo*, Semantic, semantic);

		void ForEachChild(ASTChildCallback cb, void* userdata) {}
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const {}
	};

	class FunctionOverload : public SymbolDef, public TrailingObjects<FunctionOverload, Parameter*, AttributeDecl*>
	{
		friend class ASTContext;
		friend class TrailingObjects;
	protected:
		std::string cachedSignature;
		AccessModifier accessModifiers;
		FunctionFlags functionFlags;
		SymbolRef* returnSymbol;
		IdentifierInfo* semantic;
		BlockStatement* body;
		union
		{
			struct OperatorStorage
			{
				OperatorFlags operatorFlags;
				Operator _operator;
			} operatorStorage;

			struct ConstructorStorage
			{
				SymbolRef* targetTypeSymbol;
			} constructorStorage;
		};
		TrailingObjStorage<FunctionOverload, uint32_t> storage;

		FunctionOverload(const TextSpan& span, NodeType type, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, SymbolRef* returnSymbol, IdentifierInfo* semantic, BlockStatement* body)
			: SymbolDef(span, type, name),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(returnSymbol),
			semantic(semantic),
			body(body)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_FunctionOverload;
		static FunctionOverload* Create(
			const TextSpan& span,
			IdentifierInfo* name,
			AccessModifier accessModifiers,
			FunctionFlags functionFlags,
			SymbolRef* returnSymbol,
			BlockStatement* body,
			IdentifierInfo* semantic,
			const ArrayRef<Parameter*>& parameters = {},
			const ArrayRef<AttributeDecl*>& attributes = {});

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetParameters, 0, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetAttributes, 1, storage);

		SymbolRef* GetReturnSymbolRef() const noexcept
		{
			return returnSymbol;
		}

		SymbolDef* GetReturnType() const noexcept
		{
			return returnSymbol->GetDeclaration();
		}

		std::string BuildOverloadSignature(bool placeholder = false) noexcept;

		std::string BuildTemporaryOverloadSignature() noexcept
		{
			return BuildOverloadSignature(true);
		}

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] Name: " << GetName();
			return oss.str();
		}

		DEFINE_GETTER_SETTER(FunctionFlags, FunctionFlags, functionFlags);
		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers);
		DEFINE_GETTER_SETTER_PTR(SymbolRef*, ReturnSymbol, returnSymbol);
		DEFINE_GETTER_SETTER_PTR(IdentifierInfo*, Semantic, semantic);

		BlockStatement* GetBody() const noexcept;

		void SetBody(BlockStatement* value) noexcept;

		BlockStatement*& GetBodyMut() noexcept;

		void ForEachChild(ASTChildCallback cb, void* userdata);
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const;
	};

	class ConstructorOverload : public FunctionOverload
	{
		friend class ASTContext;
	private:

		ConstructorOverload(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, SymbolRef* targetTypeSymbol, SymbolRef* returnType, BlockStatement* body)
			: FunctionOverload(span, ID, name, accessModifiers, functionFlags, returnType, nullptr, body)
		{
			constructorStorage.targetTypeSymbol = targetTypeSymbol;
		}

	public:
		static constexpr NodeType ID = NodeType_ConstructorOverload;
		static ConstructorOverload* Create(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, SymbolRef* targetTypeSymbol, BlockStatement* body, const ArrayRef<Parameter*>& parameters, const ArrayRef<AttributeDecl*>& attributes);

		SymbolRef* GetTargetTypeSymbolRef()
		{
			return constructorStorage.targetTypeSymbol;
		}

		SymbolDef* GetTargetType() const noexcept
		{
			return constructorStorage.targetTypeSymbol->GetDeclaration();
		}

		std::string BuildOverloadSignature(bool placeholder = false) noexcept;

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, TargetTypeSymbol, constructorStorage.targetTypeSymbol)
	};

	class OperatorOverload : public FunctionOverload
	{
		friend class ASTContext;
	private:
	public:
		static constexpr NodeType ID = NodeType_OperatorOverload;
		OperatorOverload(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, Operator _operator, SymbolRef* returnSymbol, BlockStatement* body)
			: FunctionOverload(span, ID, name, accessModifiers, functionFlags, returnSymbol, nullptr, body)
		{
			operatorStorage.operatorFlags = operatorFlags;
			operatorStorage._operator = _operator;
		}

		static OperatorOverload* Create(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, Operator _operator, SymbolRef* returnSymbol, BlockStatement* body, const ArrayRef<Parameter*>& parameters, const ArrayRef<AttributeDecl*>& attributes);

		std::string BuildOverloadSignature(bool placeholder = false) noexcept;

		const OperatorFlags& GetOperatorFlags() const noexcept { return operatorStorage.operatorFlags; }

		void SetOperatorFlags(const OperatorFlags& value) noexcept { operatorStorage.operatorFlags = value; }

		const Operator& GetOperator() const noexcept { return operatorStorage._operator; }

		void SetOperator(const Operator& value) noexcept { operatorStorage._operator = value; }
	};

	class Field : public SymbolDef
	{
		friend class ASTContext;
	private:
		AccessModifier accessModifiers;
		StorageClass storageClass;
		InterpolationModifier interpolationModifiers;
		SymbolRef* symbol;
		IdentifierInfo* semantic;

		Field(const TextSpan& span, IdentifierInfo* name, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, SymbolRef* symbol, IdentifierInfo* semantic)
			: SymbolDef(span, ID, name),
			accessModifiers(access),
			storageClass(storageClass),
			interpolationModifiers(interpolationModifiers),
			symbol(symbol),
			semantic(semantic)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Field;
		static Field* Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, SymbolRef* symbol, IdentifierInfo* semantic);

		void SetSymbolRef(SymbolRef* ref)
		{
			symbol = ref;
		}

		SymbolRef* GetSymbolRef()
		{
			return symbol;
		}

		const StorageClass& GetStorageClass() const noexcept
		{
			return storageClass;
		}

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers);
		DEFINE_GETTER_SETTER(InterpolationModifier, InterpolationModifiers, interpolationModifiers);
		DEFINE_GETTER_SETTER_PTR(IdentifierInfo*, Semantic, semantic);

		void ForEachChild(ASTChildCallback cb, void* userdata) {}
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const {}
	};

	class ThisDef : public SymbolDef
	{
		friend class ASTContext;
	private:
		SymbolRef* parentSymbol;

		ThisDef(IdentifierInfo* name, SymbolRef* parentSymbol)
			: SymbolDef(TextSpan(), ID, name),
			parentSymbol(parentSymbol)
		{
		}
	public:
		static constexpr NodeType ID = NodeType_ThisDef;
		static ThisDef* Create(IdentifierInfo* parentIdentifier);

		void SetSymbolRef(SymbolRef* ref)
		{
			parentSymbol = ref;
		}

		SymbolRef* GetSymbolRef()
		{
			return parentSymbol;
		}

		SymbolDef* GetDeclaredType() const noexcept
		{
			return parentSymbol->GetDeclaration();
		}

		void ForEachChild(ASTChildCallback cb, void* userdata) {}
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const {}
	};

	class DataTypeBase : public Type, public TrailingObjects<DataTypeBase, Field*, Struct*, Class*, ConstructorOverload*, FunctionOverload*, OperatorOverload*>
	{
		friend class ASTContext;
	protected:
		ThisDef* thisDef;
		TrailingObjStorage<DataTypeBase, uint32_t> storage;

		DataTypeBase(const TextSpan& span, NodeType id, IdentifierInfo* name, AccessModifier access, ThisDef* thisDef)
			: Type(span, id, name, access),
			thisDef(thisDef)
		{
		}

	public:
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetFields, 0, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetStructs, 1, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetClasses, 2, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetConstructors, 3, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetFunctions, 4, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetOperators, 5, storage);

		ThisDef* GetThisDef() const
		{
			return thisDef;
		}

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] Name: " << GetName();
			return oss.str();
		}

		void ForEachChild(ASTChildCallback cb, void* userdata);
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const;
	};

	class Struct : public DataTypeBase
	{
		friend class ASTContext;
	private:

		Struct(const TextSpan& span, IdentifierInfo* name, AccessModifier access, ThisDef* thisDef)
			: DataTypeBase(span, ID, name, access, thisDef)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Struct;
		static Struct* Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, const ArrayRef<Field*>& fields, const ArrayRef<Struct*>& structs, const ArrayRef<Class*>& classes, const ArrayRef<ConstructorOverload*>& constructors, const ArrayRef<FunctionOverload*>& functions, const ArrayRef<OperatorOverload*>& operators);
		static Struct* Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, uint32_t numFields, uint32_t numStructs, uint32_t numClasses, uint32_t numConstructors, uint32_t numFunctions, uint32_t numOperators);
	};

	class Class : public DataTypeBase
	{
		friend class ASTContext;
	private:

		Class(const TextSpan& span, IdentifierInfo* name, AccessModifier access, ThisDef* thisDef)
			: DataTypeBase(span, ID, name, access, thisDef)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Class;
		static Class* Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, const ArrayRef<Field*>& fields, const ArrayRef<Struct*>& structs, const ArrayRef<Class*>& classes, const ArrayRef<ConstructorOverload*>& constructors, const ArrayRef<FunctionOverload*>& functions, const ArrayRef<OperatorOverload*>& operators);
		static Class* Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, uint32_t numFields, uint32_t numStructs, uint32_t numClasses, uint32_t numConstructors, uint32_t numFunctions, uint32_t numOperators);
	};
}

#endif