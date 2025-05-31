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
	private:
		ParameterFlags paramaterFlags;
		InterpolationModifier interpolationModifiers;
		ast_ptr<SymbolRef> symbol;
		std::string semantic;

	public:
		static constexpr NodeType ID = NodeType_Parameter;
		Parameter()
			: SymbolDef(TextSpan(), ID, TextSpan(), true),
			paramaterFlags(ParameterFlags_In),
			interpolationModifiers(InterpolationModifier_Linear)
		{
		}
		Parameter(TextSpan span, ParameterFlags flags, InterpolationModifier interpolationModifiers, ast_ptr<SymbolRef> symbol, TextSpan name, TextSpan semantic)
			: SymbolDef(span, ID, name),
			paramaterFlags(flags),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic.str())
		{
		}

		void SetSymbolRef(ast_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Parameter;
		}

		SymbolDef* GetDeclaredType() const noexcept
		{
			return symbol->GetDeclaration();
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(ParameterFlags, ParameterFlags, paramaterFlags)

			DEFINE_GETTER_SETTER(InterpolationModifier, InterpolationModifiers, interpolationModifiers)

			DEFINE_GETTER_SETTER(std::string, Semantic, semantic)
	};

	class FunctionOverload : public SymbolDef, public AttributeContainer
	{
	protected:
		std::string cachedSignature;
		AccessModifier accessModifiers;
		FunctionFlags functionFlags;
		ast_ptr<SymbolRef> returnSymbol;
		std::vector<ast_ptr<Parameter>> parameters;
		std::string semantic;
		ast_ptr<BlockStatement> body;

		FunctionOverload(TextSpan span, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name)
			: SymbolDef(span, type, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags)
		{
		}
		FunctionOverload(TextSpan span, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, ast_ptr<SymbolRef> returnSymbol)
			: SymbolDef(span, type, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}
		FunctionOverload(TextSpan span, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, const std::string& name)
			: SymbolDef(span, type, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags)
		{
		}
		FunctionOverload(TextSpan span, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, const std::string& name, ast_ptr<SymbolRef> returnSymbol)
			: SymbolDef(span, type, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}
	public:
		static constexpr NodeType ID = NodeType_FunctionOverload;
		FunctionOverload()
			: SymbolDef(TextSpan(), ID, TextSpan(), true),
			AttributeContainer(this),
			accessModifiers(AccessModifier_Private),
			functionFlags(FunctionFlags_None)
		{
		}
		FunctionOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, ast_ptr<SymbolRef> returnSymbol)
			: SymbolDef(span, ID, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}

		void AddParameter(ast_ptr<Parameter> parameter)
		{
			parameter->SetParent(this);
			parameters.push_back(std::move(parameter));
		}

		ast_ptr<SymbolRef>& GetReturnSymbolRef()
		{
			return returnSymbol;
		}

		SymbolDef* GetReturnType() const noexcept
		{
			return returnSymbol->GetDeclaration();
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Function;
		}

		virtual std::string BuildOverloadSignature(bool placeholder) noexcept
		{
			if (!placeholder && !cachedSignature.empty())
			{
				return cachedSignature;
			}

			std::ostringstream oss;
			oss << name << "(";
			bool first = true;
			for (auto& param : parameters)
			{
				if (!first)
				{
					oss << ",";
				}
				first = false;
				if (placeholder)
				{
					oss << reinterpret_cast<size_t>(param.get());
				}
				else
				{
					oss << param->GetSymbolRef()->GetFullyQualifiedName();
				}
			}
			oss << ")";

			if (placeholder)
			{
				return oss.str();
			}
			else
			{
				cachedSignature = oss.str();
				return cachedSignature;
			}
		}

		std::string BuildOverloadSignature() noexcept
		{
			return BuildOverloadSignature(false);
		}

		std::string BuildTemporaryOverloadSignature() noexcept
		{
			return BuildOverloadSignature(true);
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override;

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] " << " Name: " << name;
			return oss.str();
		}

		DEFINE_GETTER_SETTER(FunctionFlags, FunctionFlags, functionFlags)
			DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
			DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, ReturnSymbol, returnSymbol)
			DEFINE_GET_SET_MOVE_CHILDREN(std::vector<ast_ptr<Parameter>>, Parameters, parameters)
			DEFINE_GETTER_SETTER(std::string, Semantic, semantic)

			const ast_ptr<BlockStatement>& GetBody() const noexcept;

		void SetBody(ast_ptr<BlockStatement>&& value) noexcept;

		ast_ptr<BlockStatement>& GetBodyMut() noexcept;
	};

	class ConstructorOverload : public FunctionOverload
	{
	private:
		ast_ptr<SymbolRef> targetTypeSymbol;
	public:
		static constexpr NodeType ID = NodeType_ConstructorOverload;
		ConstructorOverload()
			: FunctionOverload(TextSpan(), ID, AccessModifier_Private, FunctionFlags_None, "#ctor", make_ast_ptr<SymbolRef>("void", SymbolRefType_Type, true))
		{
		}
		ConstructorOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags)
			: FunctionOverload(span, ID, accessModifiers, functionFlags, "#ctor", make_ast_ptr<SymbolRef>("void", SymbolRefType_Type, true))
		{
		}

		ConstructorOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef> targetTypeSymbol)
			: FunctionOverload(span, ID, accessModifiers, functionFlags, "#ctor", make_ast_ptr<SymbolRef>("void", SymbolRefType_Type, true)),
			targetTypeSymbol(std::move(targetTypeSymbol))
		{
		}

		ast_ptr<SymbolRef>& GetTargetTypeSymbolRef()
		{
			return targetTypeSymbol;
		}

		SymbolDef* GetTargetType() const noexcept
		{
			return targetTypeSymbol->GetDeclaration();
		}

		std::string BuildOverloadSignature(bool placeholder) noexcept override
		{
			if (!placeholder && !cachedSignature.empty())
			{
				return cachedSignature;
			}

			std::string str = "#ctor(";

			bool first = true;
			for (auto& param : parameters)
			{
				if (!first)
				{
					str.push_back(',');
				}
				first = false;

				if (placeholder)
				{
					str.append(std::to_string(reinterpret_cast<size_t>(param.get())));
				}
				else
				{
					str.append(param->GetSymbolRef()->GetFullyQualifiedName());
				}
			}
			str.push_back(')');

			if (placeholder)
			{
				return str;
			}
			else
			{
				cachedSignature = str;
				return cachedSignature;
			}
		}

		std::string BuildOverloadSignature() noexcept
		{
			return BuildOverloadSignature(false);
		}

		std::string BuildTemporaryOverloadSignature() noexcept
		{
			return BuildOverloadSignature(true);
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Constructor;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override;

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, TargetTypeSymbol, targetTypeSymbol)
	};

	class OperatorOverload : public FunctionOverload
	{
	private:
		OperatorFlags operatorFlags;
		Operator _operator;
	public:
		static constexpr NodeType ID = NodeType_OperatorOverload;
		OperatorOverload()
			: FunctionOverload(TextSpan(), ID, AccessModifier_Private, FunctionFlags_None, TextSpan()),
			_operator(Operator_Unknown),
			operatorFlags(OperatorFlags_None)
		{
		}

		OperatorOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator)
			: FunctionOverload(span, ID, accessModifiers, functionFlags, name),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		OperatorOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator, ast_ptr<SymbolRef> returnSymbol)
			: FunctionOverload(span, ID, accessModifiers, functionFlags, name, std::move(returnSymbol)),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		std::string BuildOverloadSignature(bool placeholder) noexcept override
		{
			if (!placeholder && !cachedSignature.empty())
			{
				return cachedSignature;
			}

			std::string str;

			size_t size = 0;

			str.resize(2);
			str[0] = ToLookupChar(_operator);

			if (_operator == Operator_Cast)
			{
				str[1] = '#';
				if (placeholder)
				{
					str.append(std::to_string(reinterpret_cast<size_t>(this)));
				}
				else
				{
					str.append(returnSymbol->GetFullyQualifiedName());
				}
				str.push_back('(');
			}
			else
			{
				str[1] = '(';
			}

			bool first = true;
			for (auto& param : parameters)
			{
				if (!first)
				{
					str.push_back(',');
				}
				first = false;

				if (placeholder)
				{
					str.append(std::to_string(reinterpret_cast<size_t>(param.get())));
				}
				else
				{
					str.append(param->GetSymbolRef()->GetFullyQualifiedName());
				}
			}
			str.push_back(')');

			if (placeholder)
			{
				return str;
			}
			else
			{
				cachedSignature = str;
				return cachedSignature;
			}
		}

		std::string BuildOverloadSignature() noexcept
		{
			return BuildOverloadSignature(false);
		}

		std::string BuildTemporaryOverloadSignature() noexcept
		{
			return BuildOverloadSignature(true);
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Operator;
		}

		const OperatorFlags& GetOperatorFlags() const noexcept { return operatorFlags; }

		void SetOperatorFlags(const OperatorFlags& value) noexcept { operatorFlags = value; }

		const Operator& GetOperator() const noexcept { return _operator; }

		void SetOperator(const Operator& value) noexcept { _operator = value; }

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override;
	};

	class Field : public SymbolDef
	{
	private:
		AccessModifier accessModifiers;
		StorageClass storageClass;
		InterpolationModifier interpolationModifiers;
		ast_ptr<SymbolRef> symbol;
		std::string semantic;
	public:
		static constexpr NodeType ID = NodeType_Field;
		Field()
			: SymbolDef(TextSpan(), ID, TextSpan(), true),
			accessModifiers(AccessModifier_Private),
			storageClass(StorageClass_None),
			interpolationModifiers(InterpolationModifier_Linear)
		{
		}
		Field(TextSpan span, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, TextSpan name, ast_ptr<SymbolRef> symbol, TextSpan semantic)
			: SymbolDef(span, ID, name),
			accessModifiers(access),
			storageClass(storageClass),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic.str())
		{
		}

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

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Field;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)

			DEFINE_GETTER_SETTER(InterpolationModifier, InterpolationModifiers, interpolationModifiers)

			DEFINE_GETTER_SETTER(std::string, Semantic, semantic)
	};

	class ThisDef : public SymbolDef
	{
		ast_ptr<SymbolRef> symbol = make_ast_ptr<SymbolRef>("", SymbolRefType_Type, false);

	public:
		static constexpr NodeType ID = NodeType_ThisDef;
		ThisDef()
			: SymbolDef(TextSpan(), ID, "this")
		{
		}

		void SetSymbolRef(ast_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Variable;
		}

		SymbolDef* GetDeclaredType() const noexcept
		{
			return symbol->GetDeclaration();
		}

		void Write(Stream& stream) const override {}

		void Read(Stream& stream, StringPool& container) override {}

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override {}
	};

	class Struct : public TypeContainer
	{
	private:
		ast_ptr<ThisDef> thisDef = make_ast_ptr<ThisDef>();
	public:
		static constexpr NodeType ID = NodeType_Struct;
		Struct()
			: TypeContainer(TextSpan(), ID, TextSpan(), AccessModifier_Private, true)
		{
		}

		Struct(TextSpan span, AccessModifier access, TextSpan name)
			: TypeContainer(span, ID, name, access)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] " << " Name: " << name;
			return oss.str();
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Struct;
		}

		const ast_ptr<ThisDef>& GetThisDef() const
		{
			return thisDef;
		}

		size_t GetFieldOffset(Field* field) const override;

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override;
	};

	class Class : public TypeContainer
	{
	public:
		static constexpr NodeType ID = NodeType_Class;
		Class()
			: TypeContainer(TextSpan(), ID, TextSpan(), AccessModifier_Private, true)
		{
		}
		Class(TextSpan span, AccessModifier access, TextSpan name)
			: TypeContainer(span, ID, name, access)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] " << " Name: " << name;
			return oss.str();
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Struct;
		}

		size_t GetFieldOffset(Field* field) const override;

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override;
	};
}

#endif