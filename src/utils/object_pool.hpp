#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

#include "ast.hpp"

#include <memory>
#include <stack>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <mutex>

namespace HXSL
{
	struct BasePool
	{
		virtual ~BasePool() = default;
	};

	template <typename T, typename D> class ObjectPool;

	template <typename T, typename D = std::default_delete<T>>
	struct ReturnToPool_Deleter {
		explicit ReturnToPool_Deleter(std::weak_ptr<ObjectPool<T, D>*> pool)
			: pool_(std::move(pool)) {
		}

		void operator()(T* ptr) {
			if (auto pool_ptr = pool_.lock())
				(*pool_ptr.get())->add(std::unique_ptr<T, D>{ptr});
			else
				D{}(ptr);
		}

	private:
		std::weak_ptr<ObjectPool<T, D>*> pool_;
	};

	template <typename T, typename D = std::default_delete<T>>
	using pool_ptr = std::unique_ptr<T, ReturnToPool_Deleter<T, D>>;

	template <class T, class D = std::default_delete<T>>
	class ObjectPool : public BasePool
	{
	private:

	public:
		ObjectPool() : this_ptr_(new ObjectPool<T, D>* (this)) {}
		virtual ~ObjectPool() {}

		void add(std::unique_ptr<T, D> t)
		{
			pool_.push(std::move(t));
		}

		template<class... Args>
		pool_ptr<T> acquire(Args&&... args)
		{
			if (pool_.empty())
			{
				return pool_ptr<T>(std::make_unique<T>(std::forward<Args>(args)...).release(), ReturnToPool_Deleter{ std::weak_ptr<ObjectPool<T, D>*>{this_ptr_} });
			}
			else
			{
				auto obj = pool_.top().release();
				pool_.pop();
				new(obj) T(std::forward<Args>(args)...);
				return pool_ptr<T>(obj, ReturnToPool_Deleter{ std::weak_ptr<ObjectPool<T, D>*>{this_ptr_} });
			}
		}

		std::unique_ptr<T> detach(pool_ptr<T>&& obj)
		{
			T* rawPtr = obj.release();
			return std::unique_ptr<T>(rawPtr);
		}

		bool empty() const
		{
			return pool_.empty();
		}

		size_t size() const
		{
			return pool_.size();
		}

	private:
		std::shared_ptr<ObjectPool<T, D>* > this_ptr_;
		std::stack<std::unique_ptr<T, D> > pool_;
	};

	template<NodeType Key>
	struct NodeTypeToType;

#define DEFINE_POOL_KEY(enumVal, type) \
	template<> struct NodeTypeToType<enumVal> { using Type = type; };

	//DEFINE_POOL_KEY(NodeType_Unknown, Unknown);
	DEFINE_POOL_KEY(NodeType_Compilation, Compilation);
	DEFINE_POOL_KEY(NodeType_Namespace, Namespace);
	//DEFINE_POOL_KEY(NodeType_Enum, Enum);
	DEFINE_POOL_KEY(NodeType_Primitive, Primitive);
	DEFINE_POOL_KEY(NodeType_Struct, Struct);
	DEFINE_POOL_KEY(NodeType_Class, Class);
	DEFINE_POOL_KEY(NodeType_Array, Array);
	DEFINE_POOL_KEY(NodeType_Field, Field);
	//DEFINE_POOL_KEY(NodeType_IntrinsicFunction, IntrinsicFunction);
	DEFINE_POOL_KEY(NodeType_FunctionOverload, FunctionOverload);
	DEFINE_POOL_KEY(NodeType_OperatorOverload, OperatorOverload);
	//DEFINE_POOL_KEY(NodeType_Constructor, Constructor);
	DEFINE_POOL_KEY(NodeType_Parameter, Parameter);
	DEFINE_POOL_KEY(NodeType_AttributeDeclaration, AttributeDeclaration);
	DEFINE_POOL_KEY(NodeType_BlockStatement, BlockStatement);
	DEFINE_POOL_KEY(NodeType_DeclarationStatement, DeclarationStatement);
	DEFINE_POOL_KEY(NodeType_AssignmentStatement, AssignmentStatement);
	DEFINE_POOL_KEY(NodeType_CompoundAssignmentStatement, CompoundAssignmentStatement);
	DEFINE_POOL_KEY(NodeType_FunctionCallStatement, FunctionCallStatement);
	DEFINE_POOL_KEY(NodeType_ReturnStatement, ReturnStatement);
	DEFINE_POOL_KEY(NodeType_IfStatement, IfStatement);
	DEFINE_POOL_KEY(NodeType_ElseStatement, ElseStatement);
	DEFINE_POOL_KEY(NodeType_ElseIfStatement, ElseIfStatement);
	DEFINE_POOL_KEY(NodeType_WhileStatement, WhileStatement);
	DEFINE_POOL_KEY(NodeType_ForStatement, ForStatement);
	DEFINE_POOL_KEY(NodeType_BreakStatement, BreakStatement);
	DEFINE_POOL_KEY(NodeType_ContinueStatement, ContinueStatement);
	DEFINE_POOL_KEY(NodeType_DiscardStatement, DiscardStatement);
	DEFINE_POOL_KEY(NodeType_SwitchStatement, SwitchStatement);
	DEFINE_POOL_KEY(NodeType_CaseStatement, CaseStatement);
	DEFINE_POOL_KEY(NodeType_DefaultCaseStatement, DefaultCaseStatement);
	DEFINE_POOL_KEY(NodeType_SwizzleDefinition, SwizzleDefinition);
	DEFINE_POOL_KEY(NodeType_BinaryExpression, BinaryExpression);
	DEFINE_POOL_KEY(NodeType_EmptyExpression, EmptyExpression);
	DEFINE_POOL_KEY(NodeType_LiteralExpression, LiteralExpression);
	DEFINE_POOL_KEY(NodeType_MemberReferenceExpression, MemberReferenceExpression);
	DEFINE_POOL_KEY(NodeType_FunctionCallExpression, FunctionCallExpression);
	DEFINE_POOL_KEY(NodeType_FunctionCallParameter, FunctionCallParameter);
	DEFINE_POOL_KEY(NodeType_MemberAccessExpression, MemberAccessExpression);
	DEFINE_POOL_KEY(NodeType_ComplexMemberAccessExpression, ComplexMemberAccessExpression);
	DEFINE_POOL_KEY(NodeType_IndexerAccessExpression, IndexerAccessExpression);
	DEFINE_POOL_KEY(NodeType_CastExpression, CastExpression);
	DEFINE_POOL_KEY(NodeType_TernaryExpression, TernaryExpression);
	DEFINE_POOL_KEY(NodeType_UnaryExpression, UnaryExpression);
	DEFINE_POOL_KEY(NodeType_PrefixExpression, PrefixExpression);
	DEFINE_POOL_KEY(NodeType_PostfixExpression, PostfixExpression);
	DEFINE_POOL_KEY(NodeType_AssignmentExpression, AssignmentExpression);
	DEFINE_POOL_KEY(NodeType_CompoundAssignmentExpression, CompoundAssignmentExpression);
	DEFINE_POOL_KEY(NodeType_InitializationExpression, InitializationExpression);

	class PoolRegistry
	{
	public:

		PoolRegistry()
		{
		}

		virtual ~PoolRegistry() = default;

		static PoolRegistry& getShared()
		{
			std::call_once(initFlag, []()
				{
					shared = std::make_unique<PoolRegistry>();
				});

			return *shared.get();
		}

		template<NodeType Key, typename T>
		ObjectPool<T>& getPool()
		{
			auto& pool = pools[Key];

			if (pool)
			{
				return *static_cast<ObjectPool<T>*>(pool.get());
			}
			else
			{
				auto newPool = std::make_unique<ObjectPool<T>>();
				ObjectPool<T>* rawPtr = newPool.get();
				pool = std::move(newPool);
				return *rawPtr;
			}
		}

		template<NodeType Key>
		ObjectPool<typename NodeTypeToType<Key>::Type>& getPool()
		{
			auto& pool = pools[Key];

			if (pool)
			{
				return *static_cast<ObjectPool<typename NodeTypeToType<Key>::Type>*>(pool.get());
			}
			else
			{
				auto newPool = std::make_unique<ObjectPool<typename NodeTypeToType<Key>::Type>>();
				ObjectPool<typename NodeTypeToType<Key>::Type>* rawPtr = newPool.get();
				pool = std::move(newPool);
				return *rawPtr;
			}
		}

	private:
		std::unique_ptr<BasePool> pools[NodeType_Count];
		static std::unique_ptr<PoolRegistry> shared;
		static std::once_flag initFlag;
	};

	template<NodeType Key, class... Args>
	static typename pool_ptr<typename NodeTypeToType<Key>::Type> make_pool_ptr(Args&& ... args)
	{
		return PoolRegistry::getShared().getPool<Key>().acquire(std::forward<Args>(args)...);
	}
}
#endif