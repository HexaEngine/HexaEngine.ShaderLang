#include "core/name_mangler.hpp"

namespace HXSL
{
	namespace Backend
	{
		static void EncodeLength(size_t size, std::string& out)
		{
			do
			{
				uint8_t byte = static_cast<uint8_t>(size & 0x7F);
				size >>= 7;

				if (size != 0)
				{
					byte |= 0x80;
				}

				out.push_back(static_cast<char>(byte));
			} while (size != 0);
		}

		static void EncodeString(const StringSpan& str, std::string& out)
		{
			EncodeLength(str.size(), out);
			out.append(str.data(), str.size());
		}

		NameMangler::NameMangler(Module& module, ModuleReferenceTableBuilder& referenceTableBuilder) : module(module), referenceTableBuilder(referenceTableBuilder)
		{
			referenceTableBuilder.Append(&module, module.GetName());
			moduleMap.insert({ &module, 0 });
		}

		NameMangler::ModuleId NameMangler::GetModuleId(const Module* module)
		{
			auto it = moduleMap.find(module);
			if (it != moduleMap.end())
			{
				return it->second;
			}
			else
			{
				auto& name = module->GetName();
				auto& allocator = this->module.GetAllocator();
				auto id = referenceTableBuilder.Append(module, allocator.CopyString(name)); // important we need to copy the name to the referencer allocator to avoid ownership issues.

				moduleMap.insert({ module, id });
				modules.push_back(module);
				return id;
			}
		}

		void NameMangler::Mangle(const FunctionLayout* func, std::string& out)
		{
			AppendRoot(func->GetModule(), out);
			ManglePath(func->GetParent(), out);
			EncodeString(func->GetName(), out);
			out.append("#");
			EncodeLength(func->GetParameters().size(), out);

			for (auto* param : func->GetParameters())
			{
				AppendRoot(param->GetType()->GetModule(), out);
				ManglePath(param->GetType(), out);
			}
		}

		void NameMangler::Mangle(const OperatorLayout* op, std::string& out)
		{
			AppendRoot(op->GetModule(), out);
			ManglePath(op->GetParent(), out);
			std::format_to(std::back_inserter(out), "op{}#", static_cast<std::underlying_type_t<Operator>>(op->GetOperator()));
			EncodeLength(op->GetParameters().size(), out);

			for (auto* param : op->GetParameters())
			{
				AppendRoot(param->GetType()->GetModule(), out);
				ManglePath(param->GetType(), out);
			}
		}

		void NameMangler::Mangle(const ConstructorLayout* ctor, std::string& out)
		{
			AppendRoot(ctor->GetModule(), out);
			ManglePath(ctor->GetParent(), out);
			out.append(".ctor#");
			EncodeLength(ctor->GetParameters().size(), out);

			for (auto* param : ctor->GetParameters())
			{
				AppendRoot(param->GetType()->GetModule(), out);
				ManglePath(param->GetType(), out);
			}
		}

		void NameMangler::Mangle(const ParameterLayout* param, std::string& out)
		{
			AppendRoot(param->GetModule(), out);
			ManglePath(param->GetParent(), out);
			out.append("+");
			EncodeString(param->GetName(), out);
		}

		void NameMangler::Mangle(const FieldLayout* field, std::string& out)
		{
			AppendRoot(field->GetModule(), out);
			ManglePath(field->GetParent(), out);
			EncodeString(field->GetName(), out);
		}

		void NameMangler::Mangle(const EnumItemLayout* item, std::string& out)
		{
			AppendRoot(item->GetModule(), out);
			ManglePath(item->GetParent(), out);
			EncodeString(item->GetName(), out);
		}

		void NameMangler::Mangle(const Module* mod, std::string& out)
		{
			AppendRoot(mod, out);
		}

		void NameMangler::Mangle(const NamespaceLayout* ns, std::string& out)
		{
			AppendRoot(ns->GetModule(), out);
			ManglePath(ns, out);
		}

		void NameMangler::Mangle(const TypeLayout* type, std::string& out)
		{
			AppendRoot(type->GetModule(), out);
			ManglePath(type, out);
		}

		void NameMangler::Mangle(const Layout* lay, std::string& out)
		{
			auto type = lay->GetTypeId();

			switch (type)
			{
			case LayoutType::ModuleLayoutType:
				Mangle(cast<Module>(lay), out);
				break;
			case LayoutType::ParameterLayoutType:
				Mangle(cast<ParameterLayout>(lay), out);
				break;
			case LayoutType::FieldLayoutType:
				Mangle(cast<FieldLayout>(lay), out);
				break;
			case LayoutType::EnumItemLayoutType:
				Mangle(cast<EnumItemLayout>(lay), out);
				break;
			case LayoutType::FunctionLayoutType:
				Mangle(cast<FunctionLayout>(lay), out);
				break;
			case LayoutType::OperatorLayoutType:
				Mangle(cast<OperatorLayout>(lay), out);
				break;
			case LayoutType::ConstructorLayoutType:
				Mangle(cast<ConstructorLayout>(lay), out);
				break;
			case LayoutType::StructLayoutType:
			case LayoutType::EnumLayoutType:
			case LayoutType::PrimitiveLayoutType:
			case LayoutType::PointerLayoutType:
				Mangle(cast<TypeLayout>(lay), out);
				break;
			case LayoutType::NamespaceLayoutType:
				Mangle(cast<NamespaceLayout>(lay), out);
				break;
			default:
				HXSL_ASSERT(false, "Unsupported layout type for mangling");
				break;
			}
		}

		void NameMangler::MangleExtern(const Layout* layout, std::string& out)
		{
			auto extMod = layout->GetModule();
			auto extId = layout->GetExportId();
			auto& extExportTable = extMod->GetExportTable();
			auto& extEntry = extExportTable[extId];
			auto& name = extEntry.name;
			auto idx = name.IndexOf('@');
			auto path = name[{idx + 1, 0_rr}];
			AppendRoot(extMod, out);
			out.append(path.data(), path.size());
		}

		void NameMangler::AppendRoot(const Module* module, std::string& out)
		{
			auto id = GetModuleId(module);
			std::format_to(std::back_inserter(out), "{:x}@", id);
		}

		void NameMangler::ManglePath(const Layout* layout, std::string& out) const
		{
			std::vector<StringSpan> parts;
			const Layout* current = layout;
			while (current)
			{
				switch (current->GetTypeId())
				{
				case LayoutType::ParameterLayoutType:
				{
					auto param = cast<ParameterLayout>(current);
					parts.push_back(param->GetName());
					current = param->GetParent();
				}
				break;
				case LayoutType::FieldLayoutType:
				{
					auto field = cast<FieldLayout>(current);
					parts.push_back(field->GetName());
					current = field->GetParent();
				}
				break;
				case LayoutType::EnumItemLayoutType:
				{
					auto item = cast<EnumItemLayout>(current);
					parts.push_back(item->GetName());
					current = item->GetParent();
				}
				break;
				case LayoutType::ModuleLayoutType:
				{
					current = nullptr;
				}
				break;
				case LayoutType::NamespaceLayoutType:
				{
					auto ns = cast<NamespaceLayout>(current);
					parts.push_back(ns->GetName());
					current = ns->GetParent();
				}
				break;
				case LayoutType::StructLayoutType:
				{
					auto strct = cast<StructLayout>(current);
					parts.push_back(strct->GetName());
					current = strct->GetParent();
				}
				break;
				case LayoutType::EnumLayoutType:
				{
					auto enm = cast<EnumLayout>(current);
					parts.push_back(enm->GetName());
					current = enm->GetParent();
				}
				break;
				case LayoutType::PrimitiveLayoutType:
				{
					auto prim = cast<PrimitiveLayout>(current);
					parts.push_back(prim->GetName());
					current = nullptr;
				}
				break;
				case LayoutType::PointerLayoutType:
				{
					auto ptr = cast<PointerLayout>(current);
					parts.push_back(ptr->GetName());
					current = nullptr;
				}
				break;
				case LayoutType::FunctionLayoutType:
				case LayoutType::OperatorLayoutType:
				case LayoutType::ConstructorLayoutType:
				{
					auto func = cast<FunctionLayout>(current);
					parts.push_back(func->GetName());
					current = func->GetParent();
				}
				break;
				default:
					HXSL_ASSERT(false, "Invalid parent type for type layout");
					break;
				}
			}

			for (auto it = parts.rbegin(); it != parts.rend(); ++it)
			{
				EncodeString(*it, out);
			}
		}
	}
}