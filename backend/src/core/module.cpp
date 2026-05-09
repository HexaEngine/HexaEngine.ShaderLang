#include "core/module.hpp"
#include "pch/il.hpp"
#include "core/layout_builder.hpp"
#include "utils/endianness.hpp"
#include "il/il_encoding.hpp"

namespace HXSL
{
	namespace Backend
	{
		AccessModifier HXSL::Backend::Layout::GetAccessModifier() const
		{
			switch (typeId)
			{
			case LayoutType::StructLayoutType:
				return cast<StructLayout>(this)->GetAccess();
			case LayoutType::EnumLayoutType:
				return cast<EnumLayout>(this)->GetAccess();
			case LayoutType::PrimitiveLayoutType:
				return cast<PrimitiveLayout>(this)->GetAccess();
			case LayoutType::PointerLayoutType:
				return cast<PointerLayout>(this)->GetAccess();
			case LayoutType::FunctionLayoutType:
				return cast<FunctionLayout>(this)->GetAccess();
			case LayoutType::OperatorLayoutType:
				return cast<OperatorLayout>(this)->GetAccess();
			case LayoutType::ConstructorLayoutType:
				return cast<ConstructorLayout>(this)->GetAccess();
			case LayoutType::FieldLayoutType:
				return cast<FieldLayout>(this)->GetAccess();
			case LayoutType::ModuleLayoutType:
			case LayoutType::NamespaceLayoutType:
			case LayoutType::ParameterLayoutType:
			case LayoutType::EnumItemLayoutType:
				return AccessModifier_Public;
			default:
				HXSL_ASSERT(false, "Unhandled LayoutType in GetAccessModifier");
				return AccessModifier_None;
			}
		}

		std::string FunctionLayout::ToString() const
		{
			std::ostringstream ss;
			ss << returnType->GetName() << " " << name << "(";
			for (size_t i = 0; i < parameters.size(); ++i)
			{
				if (i > 0)
				{
					ss << ", ";
				}
				auto& param = parameters[i];
				ss << param->GetType()->GetName() << " " << param->GetName();
			}
			ss << ")";

			return ss.str();
		}
	}
}