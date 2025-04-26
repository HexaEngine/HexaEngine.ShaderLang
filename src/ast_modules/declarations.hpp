#ifndef DECLARATIONS_HPP
#define DECLARATIONS_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "attributes.hpp"
#include "container.hpp"

namespace HXSL
{
	class Parameter : virtual public ASTNode, public SymbolDef, public IHasSymbolRef
	{
	private:
		ParameterFlags flags;
		InterpolationModifier interpolationModifiers;
		std::unique_ptr<SymbolRef> symbol;
		TextSpan semantic;

	public:
		Parameter()
			: SymbolDef(TextSpan(), nullptr, NodeType_Parameter, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Parameter, true),
			flags(ParameterFlags_In),
			interpolationModifiers(InterpolationModifier_Linear),
			semantic(TextSpan())
		{
		}
		Parameter(TextSpan span, ASTNode* parent, ParameterFlags flags, InterpolationModifier interpolationModifiers, std::unique_ptr<SymbolRef> symbol, TextSpan name, TextSpan semantic)
			: SymbolDef(span, parent, NodeType_Parameter, name),
			ASTNode(span, parent, NodeType_Parameter),
			flags(flags),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic)
		{
		}

		void SetSymbolRef(std::unique_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Parameter;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

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
		std::unique_ptr<SymbolRef> returnSymbol;
		std::vector<std::unique_ptr<Parameter>> parameters;
		TextSpan semantic;
		std::unique_ptr<BlockStatement> body;

		FunctionOverload(TextSpan span, ASTNode* parent, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name)
			: SymbolDef(span, parent, type, name),
			ASTNode(span, parent, type),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags)
		{
		}
		FunctionOverload(TextSpan span, ASTNode* parent, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol, std::vector<std::unique_ptr<Parameter>> parameters, TextSpan semantic)
			: SymbolDef(span, parent, type, name),
			ASTNode(span, parent, type),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol)),
			parameters(std::move(parameters)),
			semantic(semantic)
		{
		}
		FunctionOverload(TextSpan span, ASTNode* parent, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol)
			: SymbolDef(span, parent, type, name),
			ASTNode(span, parent, type),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}

	public:
		FunctionOverload()
			: SymbolDef(TextSpan(), nullptr, NodeType_FunctionOverload, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_FunctionOverload, true),
			accessModifiers(AccessModifier_Private),
			functionFlags(FunctionFlags_None),
			semantic(TextSpan())
		{
		}
		FunctionOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol, std::vector<std::unique_ptr<Parameter>> parameters, TextSpan semantic)
			: SymbolDef(span, parent, NodeType_FunctionOverload, name),
			ASTNode(span, parent, NodeType_FunctionOverload),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol)),
			parameters(std::move(parameters)),
			semantic(semantic)
		{
		}

		FunctionOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol)
			: SymbolDef(span, parent, NodeType_FunctionOverload, name),
			ASTNode(span, parent, NodeType_FunctionOverload),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}

		void AddParameter(std::unique_ptr<Parameter> parameter)
		{
			parameter->SetParent(this);
			parameters.push_back(std::move(parameter));
		}

		std::unique_ptr<SymbolRef>& GetReturnSymbolRef()
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
			oss << name.toString() << "(";
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

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Name: " + name.toString();
			return oss.str();
		}
		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
			DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, ReturnSymbol, returnSymbol)
			DEFINE_GETTER_SETTER(TextSpan, Name, name)
			DEFINE_GET_SET_MOVE(std::vector<std::unique_ptr<Parameter>>, Parameters, parameters)
			DEFINE_GETTER_SETTER(TextSpan, Semantic, semantic)
			DEFINE_GET_SET_MOVE(std::unique_ptr<BlockStatement>, Body, body)
	};

	class OperatorOverload : public FunctionOverload
	{
	private:
		OperatorFlags operatorFlags;
		Operator _operator;
	public:
		OperatorOverload()
			: FunctionOverload(TextSpan(), nullptr, NodeType_OperatorOverload, AccessModifier_Private, FunctionFlags_None, TextSpan()),
			ASTNode(TextSpan(), nullptr, NodeType_OperatorOverload, true),
			_operator(Operator_Unknown),
			operatorFlags(OperatorFlags_None)
		{
		}
		OperatorOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator, std::unique_ptr<SymbolRef> returnSymbol, std::vector<std::unique_ptr<Parameter>> parameters, TextSpan semantic)
			: FunctionOverload(span, parent, NodeType_OperatorOverload, accessModifiers, functionFlags, name, std::move(returnSymbol), std::move(parameters), semantic),
			ASTNode(span, parent, NodeType_OperatorOverload),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		OperatorOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator)
			: FunctionOverload(span, parent, NodeType_OperatorOverload, accessModifiers, functionFlags, name),
			ASTNode(span, parent, NodeType_OperatorOverload),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		OperatorOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator, std::unique_ptr<SymbolRef> returnSymbol)
			: FunctionOverload(span, parent, NodeType_OperatorOverload, accessModifiers, functionFlags, name, std::move(returnSymbol)),
			ASTNode(span, parent, NodeType_OperatorOverload),
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

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;
	};

	class Field : virtual public ASTNode, public SymbolDef, public IHasSymbolRef
	{
	private:
		AccessModifier accessModifiers;
		StorageClass storageClass;
		InterpolationModifier interpolationModifiers;
		std::unique_ptr<SymbolRef> symbol;
		TextSpan semantic;
	public:
		Field()
			: SymbolDef(TextSpan(), nullptr, NodeType_Field, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Field, true),
			accessModifiers(AccessModifier_Private),
			storageClass(StorageClass_None),
			interpolationModifiers(InterpolationModifier_Linear),
			semantic(TextSpan())
		{
		}
		Field(TextSpan span, ASTNode* parent, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, TextSpan name, std::unique_ptr<SymbolRef> symbol, TextSpan semantic)
			: SymbolDef(span, parent, NodeType_Field, name),
			ASTNode(span, parent, NodeType_Field),
			accessModifiers(access),
			storageClass(storageClass),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic)
		{
		}

		void SetSymbolRef(std::unique_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
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

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
	};

	class Struct : public Type, public Container
	{
	private:
		AccessModifier accessModifiers;
	public:
		Struct()
			: Type(TextSpan(), nullptr, NodeType_Struct, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Struct, true),
			Container(TextSpan(), nullptr, NodeType_Struct, true),
			accessModifiers(AccessModifier_Private)
		{
		}
		Struct(TextSpan span, ASTNode* parent, AccessModifier access, TextSpan name)
			: Type(span, parent, NodeType_Struct, name),
			ASTNode(span, parent, NodeType_Struct),
			Container(span, parent, NodeType_Struct),
			accessModifiers(access)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Name: " + name.toString();
			return oss.str();
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Struct;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
	};

	class Class : public Type, public Container
	{
	private:
		AccessModifier accessModifiers;
	public:
		Class()
			: Type(TextSpan(), nullptr, NodeType_Class, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Class, true),
			Container(TextSpan(), nullptr, NodeType_Class, true),
			accessModifiers(AccessModifier_Private)
		{
		}
		Class(TextSpan span, ASTNode* parent, AccessModifier access, TextSpan name)
			: Type(span, parent, NodeType_Class, name),
			ASTNode(span, parent, NodeType_Class),
			Container(span, parent, NodeType_Class),
			accessModifiers(access)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Name: " + name.toString();
			return oss.str();
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Struct;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
	};
}

#endif