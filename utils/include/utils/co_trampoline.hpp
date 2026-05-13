#pragma once

#include "common.hpp"
#include "memory.hpp"
#include "macros.hpp"

#include <coroutine>

namespace HEXA_UTILS_NAMESPACE
{
	enum class TrampolineTaskStatus : uint8_t
	{
		WaitingForActivation = 0,
		Scheduled = 1 << 0,
		Running = 1 << 1,
		RanToCompletion = 1 << 2,
		Failed = 1 << 3,
		Completed = RanToCompletion | Failed,
	};

	DEFINE_FLAGS_OPERATORS(TrampolineTaskStatus, uint8_t);

	struct TrampolineTaskStateBase : SharedObject
	{
	private:
		virtual void Destroy() = 0;
		void* coroutinePtr;
		std::exception_ptr exception;
		TrampolineTaskStatus status = TrampolineTaskStatus::WaitingForActivation;
		bool hasValue = false;
	protected:
		bool HasValue() const { return hasValue; }
		void SetHasValue(bool val) { hasValue = val; }

		TrampolineTaskStateBase(void* coroutinePtr) : coroutinePtr(coroutinePtr) {}

	public:

		void SetState(TrampolineTaskStatus value) { status = value; }

		bool TransitionState(TrampolineTaskStatus from, TrampolineTaskStatus to)
		{
			if (from != status) return false;
			status = to;
			return true;
		}

		bool IsCompleted() const { return (status & TrampolineTaskStatus::Completed) != TrampolineTaskStatus::WaitingForActivation; }
		void SetCompleted() { status = TrampolineTaskStatus::RanToCompletion; }

		TrampolineTaskStatus GetStatus() const { return status; }

		void Execute()
		{
			HEXA_UTILS_ASSERT(TransitionState(TrampolineTaskStatus::Scheduled, TrampolineTaskStatus::Running), "Failed to transition state");
			auto h = std::coroutine_handle<>::from_address(coroutinePtr);
			h.resume();
			if (!h.done())
			{
				HEXA_UTILS_ASSERT(TransitionState(TrampolineTaskStatus::Running, TrampolineTaskStatus::Scheduled), "Failed to transition state");
			}
		}

		std::suspend_never initial_suspend() noexcept
		{
			return {};
		}

		std::suspend_always final_suspend() noexcept
		{
			if (exception == nullptr)
			{
				SetState(TrampolineTaskStatus::RanToCompletion);
			}

			return {};
		}

		std::exception_ptr GetException() const { return exception; }
		void ThrowIfFailed()
		{
			if (exception)
			{
				std::rethrow_exception(exception);
			}
		}

		void unhandled_exception()
		{
			exception = std::current_exception();
			status = TrampolineTaskStatus::Failed;
		}
	};

	// A trampoline task scheduler (single threaded)
	// Used to turn a recursive DFS to an iterative DFS,
	// without rewriting it to a stack machine via a shadow stack in the scheduler.
	class TrampolineTaskScheduler
	{
		struct ExecutionFrame
		{
			size_t start;
		};

		std::vector<ObjPtr<TrampolineTaskStateBase>> queue;
		std::vector<ExecutionFrame> stack;
		TrampolineTaskScheduler* last;

	public:
		TrampolineTaskScheduler()
		{
			last = GetCurrent();
			GetCurrent() = this;
		}

		~TrampolineTaskScheduler()
		{
			if (GetCurrent() == this)
			{
				GetCurrent() = last;
			}
		}

		void push(const ObjPtr<TrampolineTaskStateBase>& handle)
		{
			if (handle->TransitionState(TrampolineTaskStatus::WaitingForActivation, TrampolineTaskStatus::Scheduled))
				queue.push_back(handle);
		}

		void push(ObjPtr<TrampolineTaskStateBase>&& handle)
		{
			if (handle->TransitionState(TrampolineTaskStatus::WaitingForActivation, TrampolineTaskStatus::Scheduled))
				queue.emplace_back(std::move(handle));
		}

		void insert_frame() { stack.push_back({ queue.size() }); }

		static TrampolineTaskScheduler*& GetCurrent()
		{
			static thread_local TrampolineTaskScheduler* scheduler = nullptr;
			return scheduler;
		}

		size_t scheduled_count() const { return queue.size(); }

		bool pump();
	};

	template<typename T>
	struct TrampolineTask;

	template<typename T>
	struct TrampolineTaskState;

	template<typename T>
	using TrampolineTaskHandle = std::coroutine_handle<TrampolineTaskState<T>>;

	template<typename T>
	struct TrampolineTaskStateT : public TrampolineTaskStateBase
	{
		TrampolineTaskStateT() : TrampolineTaskStateBase(TrampolineTaskHandle<T>::from_promise(*static_cast<TrampolineTaskState<T>*>(this)).address())
		{
		}

		TrampolineTaskHandle<T> GetHandle()
		{
			return TrampolineTaskHandle<T>::from_promise(*static_cast<TrampolineTaskState<T>*>(this));
		}

		void Destroy() override
		{
			GetHandle().destroy();
		}
	};

	template<typename T>
	struct TrampolineTaskState : public TrampolineTaskStateT<T>
	{
		Aligned<Array<uint8_t, sizeof(T)>, alignof(T)> result;

		void return_value(T value) { new(result.data()) T(std::move(value)); this->SetHasValue(true); }
		TrampolineTask<T> get_return_object();

		T GetResult()
		{
			auto p = reinterpret_cast<T*>(result.data());
			auto v = std::move(*p);
			std::destroy_at(p);
			this->SetHasValue(false);
			return v;
		}

		~TrampolineTaskState()
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
			{
				if (this->HasValue())
					std::destroy_at(reinterpret_cast<T*>(result.data()));
			}
		}
	};

	template<>
	struct TrampolineTaskState<void> : public TrampolineTaskStateT<void>
	{
		void return_void() {}
		TrampolineTask<void> get_return_object();
	};

	struct TrampolineBounceAwaiter
	{
		bool await_ready() const noexcept
		{
			return false;
		}

		template<typename U>
		void await_suspend(TrampolineTaskHandle<U> handle) const noexcept
		{
			auto scheduler = TrampolineTaskScheduler::GetCurrent();
			scheduler->push(ObjPtr<TrampolineTaskStateBase>(&handle.promise()));
		}

		void await_resume() const noexcept
		{
		}
	};

	static TrampolineBounceAwaiter TrampolineBounce() { return TrampolineBounceAwaiter(); }

	template<typename T>
	struct TrampolineTask
	{
		using TaskState = TrampolineTaskState<T>;
		using promise_type = TrampolineTaskState<T>;
		using handle_type = TrampolineTaskHandle<T>;

		ObjPtr<TaskState> state;

		explicit TrampolineTask(const ObjPtr<TaskState>& h) : state(h) {}
		explicit TrampolineTask(ObjPtr<TaskState>&& h) : state(std::move(h)) {}
		TrampolineTask() = default;

		bool IsCompleted() const
		{
			return state->IsCompleted();
		}

		T GetResult()
		{
			return std::move(state->GetResult());
		}

		bool await_ready()
		{
			return state->IsCompleted();
		}

		template<typename U>
		void await_suspend(TrampolineTaskHandle<U> handle) const noexcept
		{
			auto scheduler = TrampolineTaskScheduler::GetCurrent();
			scheduler->push(ObjPtr<TrampolineTaskStateBase>(&handle.promise()));
		}

		T await_resume()
		{
			state->ThrowIfFailed();

			if constexpr (!std::is_void_v<T>)
			{
				return std::move(state->GetResult());
			}
		}
	};

	template<typename T>
	inline TrampolineTask<T> TrampolineTaskState<T>::get_return_object()
	{
		return TrampolineTask<T>(ObjPtr<TrampolineTaskState<T>>::Attach(this));
	}

	inline TrampolineTask<void> TrampolineTaskState<void>::get_return_object() { return TrampolineTask<void>(ObjPtr<TrampolineTaskState<void>>::Attach(this)); }
}