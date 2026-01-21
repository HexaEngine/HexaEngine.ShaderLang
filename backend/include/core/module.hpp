#ifndef MODULE_HPP
#define MODULE_HPP

#include "pch/std.hpp"
#include "language.hpp"

#include "utils/span.hpp"
#include "utils/static_vector.hpp"
#include "utils/bump_allocator.hpp"

#include "utils/rtti_helper.hpp"
#include "io/stream.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ILContext;
		class ILCodeBlob;
		class TypeLayout;
		class FunctionLayout;
		class OperatorLayout;
		class ConstructorLayout;
		class FieldLayout;
		class StructLayout;
		class PrimitiveLayout;
		class PointerLayout;
		class NamespaceLayout;

		class Layout
		{
		public:
			enum LayoutType : char
			{
				ModuleLayoutType,
				NamespaceLayoutType,
				ParameterLayoutType,
				FunctionLayoutType,
				OperatorLayoutType,
				ConstructorLayoutType,
				FieldLayoutType,
				StructLayoutType,
				PrimitiveLayoutType,
				PointerLayoutType,
			};
		protected:
			Layout(LayoutType typeId) : typeId(typeId) {}
			LayoutType typeId;
		public:
			LayoutType GetTypeId() const { return typeId; }
		};

		class TypeLayout : public Layout
		{
			StringSpan name;
			AccessModifier access = AccessModifier_None;
		protected:
			TypeLayout(LayoutType typeId) : Layout(typeId) {}
		public:
			const StringSpan& GetName() const { return name; }
			void SetName(const StringSpan& n) { name = n; }

			AccessModifier GetAccess() const { return access; }
			void SetAccess(AccessModifier a) { access = a; }
		};

		class ParameterLayout : public Layout
		{
			FunctionLayout* parent = nullptr;
			StringSpan name;
			StringSpan semantic;
			TypeLayout* type = nullptr;
			StorageClass storageClass = StorageClass_None;
			InterpolationModifier interpolMod = InterpolationModifier_None;
			ParameterFlags parameterFlags = ParameterFlags_None;
		public:
			static constexpr LayoutType ID = ParameterLayoutType;
			ParameterLayout() : Layout(ID) {}

			FunctionLayout* GetParent() const { return parent; }
			void SetParent(FunctionLayout* newParent) { parent = newParent; }

			const StringSpan& GetName() const { return name; }
			void SetName(const StringSpan& n) { name = n; }

			const StringSpan& GetSemantic() const { return semantic; }
			void SetSemantic(const StringSpan& s) { semantic = s; }

			TypeLayout* GetType() const { return type; }
			void SetType(TypeLayout* t) { type = t; }

			StorageClass GetStorageClass() const { return storageClass; }
			void SetStorageClass(StorageClass sc) { storageClass = sc; }

			InterpolationModifier GetInterpolationModifier() const { return interpolMod; }
			void SetInterpolationModifier(InterpolationModifier im) { interpolMod = im; }

			ParameterFlags GetParameterFlags() const { return parameterFlags; }
			void SetParameterFlags(ParameterFlags pf) { parameterFlags = pf; }
		};

		enum FunctionLayoutFlags
		{
			FunctionLayoutFlags_None,
			FunctionLayoutFlags_Member,
			FunctionLayoutFlags_Operator,
			FunctionLayoutFlags_Constructor,
		};

		class FunctionLayout : public Layout
		{
			Layout* parent = nullptr;
			StringSpan name;
			TypeLayout* returnType = nullptr;
			Span<ParameterLayout*> parameters;
			AccessModifier access = AccessModifier_None;
			StorageClass storageClass = StorageClass_None;
			FunctionFlags functionFlags = FunctionFlags_None;
			ILContext* context = nullptr;
			ILCodeBlob* codeBlob = nullptr;

		protected:
			FunctionLayout(LayoutType typeId) : Layout(typeId) {}
		public:
			static constexpr LayoutType ID = FunctionLayoutType;
			FunctionLayout() : Layout(ID) {}

			Layout* GetParent() const { return parent; }
			void SetParent(Layout* newParent) { parent = newParent; }

			const StringSpan& GetName() const { return name; }
			void SetName(const StringSpan& n) { name = n; }

			TypeLayout* GetReturnType() const { return returnType; }
			void SetReturnType(TypeLayout* rt) { returnType = rt; }

			const Span<ParameterLayout*>& GetParameters() const { return parameters; }
			void SetParameters(const Span<ParameterLayout*>& p) { parameters = p; }

			AccessModifier GetAccess() const { return access; }
			void SetAccess(AccessModifier a) { access = a; }

			StorageClass GetStorageClass() const { return storageClass; }
			void SetStorageClass(StorageClass sc) { storageClass = sc; }

			FunctionFlags GetFunctionFlags() const { return functionFlags; }
			void SetFunctionFlags(FunctionFlags ff) { functionFlags = ff; }

			ILContext* GetContext() const { return context; }
			void SetContext(ILContext* value) { context = value; }

			ILCodeBlob* GetCodeBlob() const { return codeBlob; }
			void SetCodeBlob(ILCodeBlob* blob) { codeBlob = blob; }
		};

		class OperatorLayout : public FunctionLayout
		{
			Operator op = Operator_Unknown;
			OperatorFlags operatorFlags = OperatorFlags_None;
		public:
			static constexpr LayoutType ID = OperatorLayoutType;
			OperatorLayout() : FunctionLayout(ID) {}

			Operator GetOperator() const { return op; }
			void SetOperator(Operator o) { op = o; }

			OperatorFlags GetOperatorFlags() const { return operatorFlags; }
			void SetOperatorFlags(OperatorFlags of) { operatorFlags = of; }
		};

		class ConstructorLayout : public FunctionLayout
		{
		public:
			ConstructorLayout() : FunctionLayout(ConstructorLayoutType) {}
		};

		class FieldLayout : public Layout
		{
			Layout* parent = nullptr;
			StringSpan name;
			StringSpan semantic;
			TypeLayout* type = nullptr;
			AccessModifier access = AccessModifier_None;
			StorageClass storageClass = StorageClass_None;
			InterpolationModifier interpolMod = InterpolationModifier_None;
		public:
			static constexpr LayoutType ID = FieldLayoutType;
			FieldLayout() : Layout(ID) {}

			Layout* GetParent() const { return parent; }
			void SetParent(Layout* newParent) { parent = newParent; }

			const StringSpan& GetName() const { return name; }
			void SetName(const StringSpan& n) { name = n; }

			const StringSpan& GetSemantic() const { return semantic; }
			void SetSemantic(const StringSpan& s) { semantic = s; }

			TypeLayout* GetType() const { return type; }
			void SetType(TypeLayout* t) { type = t; }

			AccessModifier GetAccess() const { return access; }
			void SetAccess(AccessModifier a) { access = a; }

			StorageClass GetStorageClass() const { return storageClass; }
			void SetStorageClass(StorageClass sc) { storageClass = sc; }

			InterpolationModifier GetInterpolationModifier() const { return interpolMod; }
			void SetInterpolationModifier(InterpolationModifier im) { interpolMod = im; }
		};

		enum StructLayoutFlags : char
		{
			StructLayoutFlags_None,
			StructLayoutFlags_Class,
		};

		class StructLayout : public TypeLayout
		{
			Layout* parent = nullptr;
			Span<FieldLayout*> fields;
			Span<FunctionLayout*> functions;
			Span<OperatorLayout*> operators;
			Span<ConstructorLayout*> constructors;
			Span<StructLayout*> structs;
			StructLayoutFlags structFlags = StructLayoutFlags_None;

		public:
			static constexpr LayoutType ID = StructLayoutType;
			StructLayout() : TypeLayout(ID) {}

			Layout* GetParent() const { return parent; }
			void SetParent(Layout* newParent) { parent = newParent; }

			const Span<FieldLayout*>& GetFields() const { return fields; }
			void SetFields(const Span<FieldLayout*>& f) { fields = f; }

			const Span<FunctionLayout*>& GetFunctions() const { return functions; }
			void SetFunctions(const Span<FunctionLayout*>& f) { functions = f; }

			const Span<OperatorLayout*>& GetOperators() const { return operators; }
			void SetOperators(const Span<OperatorLayout*>& o) { operators = o; }

			const Span<ConstructorLayout*>& GetConstructors() const { return constructors; }
			void SetConstructors(const Span<ConstructorLayout*>& o) { constructors = o; }

			const Span<StructLayout*>& GetStructs() const { return structs; }
			void SetStructs(const Span<StructLayout*>& t) { structs = t; }

			StructLayoutFlags GetStructFlags() const { return structFlags; }
			void SetStructFlags(StructLayoutFlags sf) { structFlags = sf; }

			size_t GetFieldOffset(FieldLayout* field)
			{
				return fields.find(field);
			}
		};

		class PrimitiveLayout : public TypeLayout
		{
			PrimitiveKind primKind = PrimitiveKind_Void;
			PrimitiveClass primClass = PrimitiveClass_Scalar;
			uint32_t rows = 0;
			uint32_t columns = 0;

		public:
			static constexpr LayoutType ID = PrimitiveLayoutType;
			PrimitiveLayout() : TypeLayout(ID) {}

			PrimitiveKind GetKind() const { return primKind; }
			void SetKind(PrimitiveKind a) { primKind = a; }

			PrimitiveClass GetClass() const { return primClass; }
			void SetClass(PrimitiveClass a) { primClass = a; }

			uint32_t GetRows() const { return rows; }
			void SetRows(uint32_t a) { rows = a; }

			uint32_t GetColumns() const { return columns; }
			void SetColumns(uint32_t a) { columns = a; }
		};

		class PointerLayout : public TypeLayout
		{
			TypeLayout* elementType;
		public:
			static constexpr LayoutType ID = PointerLayoutType;
			PointerLayout() : TypeLayout(ID), elementType(nullptr) {}

			TypeLayout* GetElementType() const { return elementType; }
			void SetElementType(TypeLayout* value) { elementType = value; }
		};

		class NamespaceLayout : public Layout
		{
			StringSpan name;
			NamespaceLayout* parent = nullptr;
			Span<StructLayout*> structs;
			Span<FunctionLayout*> functions;
			Span<FieldLayout*> globalFields;
			Span<NamespaceLayout*> nestedNamespaces;

		public:
			static constexpr LayoutType ID = NamespaceLayoutType;
			NamespaceLayout() : Layout(ID) {}

			const StringSpan& GetName() const { return name; }
			void SetName(const StringSpan& value) { name = value; }

			NamespaceLayout* GetParent() const { return parent; }
			void SetParent(NamespaceLayout* value) { parent = value; }

			const Span<StructLayout*>& GetStructs() const { return structs; }
			void SetStructs(const Span<StructLayout*>& value) { structs = value; }

			const Span<FunctionLayout*>& GetFunctions() const { return functions; }
			void SetFunctions(const Span<FunctionLayout*>& value) { functions = value; }

			const Span<FieldLayout*>& GetGlobalFields() const { return globalFields; }
			void SetGlobalFields(const Span<FieldLayout*>& value) { globalFields = value; }

			const Span<NamespaceLayout*>& GetNestedNamespaces() const { return nestedNamespaces; }
			void SetNestedNamespaces(const Span<NamespaceLayout*>& value) { nestedNamespaces = value; }
		};

		class Module : public Layout
		{
		
			BumpAllocator allocator;
			std::vector<NamespaceLayout*> namespaces;
			Span<FunctionLayout*> functions;

		public:	
			using RecordId = uint64_t;
			static constexpr LayoutType ID = ModuleLayoutType;
			Module() : Layout(ID) {}

			BumpAllocator& GetAllocator() { return allocator; }

			const std::vector<NamespaceLayout*>& GetNamespaces() const { return namespaces; }
			void AddNamespace(NamespaceLayout* ns) { namespaces.push_back(ns); }

			const Span<FunctionLayout*>& GetAllFunctions() const { return functions; }
			void SetAllFunctions(const Span<FunctionLayout*>& value) { functions = value; }
		};

		using type_layout_checker =
			rtti_type_equals_checker<
			Layout::StructLayoutType,
			Layout::PrimitiveLayoutType,
			Layout::PointerLayoutType>;

		template <typename T>
		inline static bool isa(const Layout* N) { return N && N->GetTypeId() == T::ID; }

		template<>
		inline static bool isa<TypeLayout>(const Layout* N)
		{
			if (!N) return false;
			auto id = N->GetTypeId();
			return type_layout_checker::check(id);
		}

		template <typename T>
		inline static T* cast(Layout* N) { assert(isa<T>(N) && "cast<T>() argument is not of type T!"); return static_cast<T*>(N); }

		template <typename T>
		inline static const T* cast(const Layout* N) { assert(isa<T>(N) && "cast<T>() argument is not of type T!"); return static_cast<const T*>(N); }

		template <typename T>
		inline static T* dyn_cast(Layout* N) { return isa<T>(N) ? static_cast<T*>(N) : nullptr; }

		template <typename T>
		inline static const T* dyn_cast(const Layout* N) { return isa<T>(N) ? static_cast<const T*>(N) : nullptr; }

		class ModuleWriter;
		class ModuleReader;

		struct ModuleWriterContext
		{
			using RecordId = Module::RecordId;
			ModuleWriter& writer;
			const dense_map<const Layout*, RecordId>& recordMap;
		};

		struct ModuleReaderContext
		{
			using RecordId = Module::RecordId;
			ModuleReader& reader;
			const dense_map<RecordId, Layout*>& recordMap;
		};

		class ModuleWriter
		{
			using RecordId = Module::RecordId;
			Stream* stream = nullptr;
			dense_map<const Layout*, RecordId> recordMap;
			dense_set<const Layout*> writtenRecords;
			RecordId recordCounter = 1;
			
			ModuleWriterContext context{ *this, recordMap };

		public:
			template<typename T>
			inline void WriteLittleEndian(T value)
			{
				stream->WriteValue(EndianUtils::ToLittleEndian(value));
			}

			inline void WriteString(const StringSpan& str)
			{
				uint32_t len = static_cast<uint32_t>(str.size());
				WriteLittleEndian(len);
				if (len == 0) return;
				stream->Write(str.data(), len);
			}

			RecordId GetRecordId(const Layout* layout);
			bool WriteRecordHeader(const Layout* layout);
			void WriteRecordRef(const Layout* layout);
			void WriteNamespace(const NamespaceLayout* ns);
			void WriteStruct(const StructLayout* strct);
			void WriteFunction(const FunctionLayout* func);
			void WriteOperator(const OperatorLayout* op);
			void WriteConstructor(const ConstructorLayout* ctor);
			void WriteParameter(const ParameterLayout* param);
			void WriteField(const FieldLayout* field);
			void WritePointerType(const PointerLayout* ptr);
			void WritePrimitiveType(const PrimitiveLayout* prim);
			void WriteType(const TypeLayout* type);
			void WriteModule(const Module* module);

			ModuleWriter(Stream* s) : stream(s) {}

			void Write(const Module* module);
		};

		struct ModuleReference
		{
			StringSpan name;
			uint32_t majorVersion;
			uint32_t minorVersion;
			uint32_t patchVersion;
			uint32_t buildVersion;
		};

		struct ExternalSymbol
		{
			uint32_t moduleRefId;
			StringSpan name;
		};

		class ModuleReader
		{
			using RecordId = Module::RecordId;
			Stream* stream = nullptr;
			Module* module = nullptr;
			dense_map<RecordId, Layout*> recordMap;
			ModuleReaderContext context{ *this, recordMap };


		public:
			template<typename T>
			inline T ReadLittleEndian()
			{
				return EndianUtils::FromLittleEndian(stream->ReadValue<T>());
			}

			RecordId ReadRecordRef();

			template<typename T>
			T* ReadRecordRef()
			{
				return cast<T>(recordMap[ReadRecordRef()]);
			}

			void ReadModule();
			NamespaceLayout* ReadNamespace();
			StructLayout* ReadStruct();
			FunctionLayout* ReadFunction();
			OperatorLayout* ReadOperator();
			ConstructorLayout* ReadConstructor();
			ParameterLayout* ReadParameter();
			FieldLayout* ReadField();
			PointerLayout* ReadPointerType();
			PrimitiveLayout* ReadPrimitiveType();
			StringSpan ReadStringSpan();
			ILCodeBlob* ReadILCodeBlob();

			ModuleReader(Stream* s) : stream(s) {}

			uptr<Module> Read();
		};
	}
}

#endif