#ifndef DECLARATIONS_HPP
#define DECLARATIONS_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "attributes.hpp"
#include "container.hpp"

namespace HXSL
{
	class Parameter : public SymbolDef, public IHasSymbolRef
	{
	private:
		ParameterFlags flags;
		InterpolationModifier interpolationModifiers;
		ast_ptr<SymbolRef> symbol;
		std::string semantic;

	public:
		Parameter()
			: ASTNode(TextSpan(), NodeType_Parameter, true),
			SymbolDef(TextSpan(), NodeType_Parameter, TextSpan(), true),
			flags(ParameterFlags_In),
			interpolationModifiers(InterpolationModifier_Linear)
		{
		}
		Parameter(TextSpan span, ParameterFlags flags, InterpolationModifier interpolationModifiers, ast_ptr<SymbolRef> symbol, TextSpan name, TextSpan semantic)
			: ASTNode(span, NodeType_Parameter),
			SymbolDef(span, NodeType_Parameter, name),
			flags(flags),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic.str())
		{
		}

		void SetSymbolRef(ast_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		ast_ptr<SymbolRef>& GetSymbolRef() override
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

		~Parameter() override
		{
		}
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
			: ASTNode(span, type),
			SymbolDef(span, type, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags)
		{
		}
		FunctionOverload(TextSpan span, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, ast_ptr<SymbolRef> returnSymbol, std::vector<ast_ptr<Parameter>> parameters, TextSpan semantic)
			: ASTNode(span, type),
			SymbolDef(span, type, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol)),
			parameters(std::move(parameters)),
			semantic(semantic.str())
		{
			RegisterChildren(this->parameters);
		}
		FunctionOverload(TextSpan span, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, ast_ptr<SymbolRef> returnSymbol)
			: ASTNode(span, type),
			SymbolDef(span, type, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}

	public:
		FunctionOverload()
			: ASTNode(TextSpan(), NodeType_FunctionOverload, true),
			SymbolDef(TextSpan(), NodeType_FunctionOverload, TextSpan(), true),
			AttributeContainer(this),
			accessModifiers(AccessModifier_Private),
			functionFlags(FunctionFlags_None)
		{
		}
		FunctionOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, ast_ptr<SymbolRef> returnSymbol, std::vector<ast_ptr<Parameter>> parameters, TextSpan semantic)
			: ASTNode(span, NodeType_FunctionOverload),
			SymbolDef(span, NodeType_FunctionOverload, name),
			AttributeContainer(this),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol)),
			parameters(std::move(parameters)),
			semantic(semantic.str())
		{
			RegisterChildren(this->parameters);
		}

		FunctionOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, ast_ptr<SymbolRef> returnSymbol)
			: ASTNode(span, NodeType_FunctionOverload),
			SymbolDef(span, NodeType_FunctionOverload, name),
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
					oss << param->GetID();
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
			oss << "[" << HXSL::ToString(type) << "] ID: " << GetID() << " Name: " << name;
			return oss.str();
		}
		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
			DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, ReturnSymbol, returnSymbol)
			DEFINE_GET_SET_MOVE_CHILDREN(std::vector<ast_ptr<Parameter>>, Parameters, parameters)
			DEFINE_GETTER_SETTER(std::string, Semantic, semantic)

			const ast_ptr<BlockStatement>& GetBody() const noexcept;

		void SetBody(ast_ptr<BlockStatement>&& value) noexcept;

		ast_ptr<BlockStatement>& GetBodyMut() noexcept;
	};

	class OperatorOverload : public FunctionOverload
	{
	private:
		OperatorFlags operatorFlags;
		Operator _operator;
	public:
		OperatorOverload()
			: FunctionOverload(TextSpan(), NodeType_OperatorOverload, AccessModifier_Private, FunctionFlags_None, TextSpan()),
			ASTNode(TextSpan(), NodeType_OperatorOverload, true),
			_operator(Operator_Unknown),
			operatorFlags(OperatorFlags_None)
		{
		}
		OperatorOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator, ast_ptr<SymbolRef> returnSymbol, std::vector<ast_ptr<Parameter>> parameters, TextSpan semantic)
			: FunctionOverload(span, NodeType_OperatorOverload, accessModifiers, functionFlags, name, std::move(returnSymbol), std::move(parameters), semantic),
			ASTNode(span, NodeType_OperatorOverload),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		OperatorOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator)
			: FunctionOverload(span, NodeType_OperatorOverload, accessModifiers, functionFlags, name),
			ASTNode(span, NodeType_OperatorOverload),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		OperatorOverload(TextSpan span, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator, ast_ptr<SymbolRef> returnSymbol)
			: FunctionOverload(span, NodeType_OperatorOverload, accessModifiers, functionFlags, name, std::move(returnSymbol)),
			ASTNode(span, NodeType_OperatorOverload),
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
					str.append(std::to_string(GetID()));
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
					str.append(std::to_string(param->GetID()));
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

	class Field : public SymbolDef, public IHasSymbolRef
	{
	private:
		AccessModifier accessModifiers;
		StorageClass storageClass;
		InterpolationModifier interpolationModifiers;
		ast_ptr<SymbolRef> symbol;
		std::string semantic;
	public:
		Field()
			: ASTNode(TextSpan(), NodeType_Field, true),
			SymbolDef(TextSpan(), NodeType_Field, TextSpan(), true),
			accessModifiers(AccessModifier_Private),
			storageClass(StorageClass_None),
			interpolationModifiers(InterpolationModifier_Linear)
		{
		}
		Field(TextSpan span, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, TextSpan name, ast_ptr<SymbolRef> symbol, TextSpan semantic)
			: ASTNode(span, NodeType_Field),
			SymbolDef(span, NodeType_Field, name),
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

		ast_ptr<SymbolRef>& GetSymbolRef() override
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
	};

	class Struct : public Type, public Container
	{
	private:
		AccessModifier accessModifiers;
	public:
		Struct()
			: Type(TextSpan(), NodeType_Struct, TextSpan(), true),
			Container(TextSpan(), NodeType_Struct),
			ASTNode(TextSpan(), NodeType_Struct),
			accessModifiers(AccessModifier_Private)
		{
		}
		Struct(TextSpan span, AccessModifier access, TextSpan name)
			: Type(span, NodeType_Struct, name),
			Container(span, NodeType_Struct),
			ASTNode(span, NodeType_Struct),
			accessModifiers(access)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] ID: " << GetID() << " Name: " << name;
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

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
	};

	class Class : public Type, public Container
	{
	private:
		AccessModifier accessModifiers;
	public:
		Class()
			: Type(TextSpan(), NodeType_Class, TextSpan(), true),
			Container(TextSpan(), NodeType_Class, true),
			ASTNode(TextSpan(), NodeType_Class, true),
			accessModifiers(AccessModifier_Private)
		{
		}
		Class(TextSpan span, AccessModifier access, TextSpan name)
			: Type(span, NodeType_Class, name),
			Container(span, NodeType_Class, true),
			ASTNode(span, NodeType_Class, true),
			accessModifiers(access)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << HXSL::ToString(type) << "] ID: " << GetID() << " Name: " << name;
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

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
	};
}

#endif