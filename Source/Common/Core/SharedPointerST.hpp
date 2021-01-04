// SharedPointerST.hpp

#ifndef _CORE_SHAREDPOINTERST_H_
#define _CORE_SHAREDPOINTERST_H_

#include <Core/Platform.h>

#include <set>
#include <map>
#include <utility>
#include <type_traits>

namespace Core
{
	struct FusedObjectBase
	{
		virtual ~FusedObjectBase() CORE_NOEXCEPT // Ensuring proper destruction.
		{
		}
	};

	struct ControlBlock
	{
		size_t References;
		FusedObjectBase* FusedObject;

		ControlBlock(FusedObjectBase* fusedObject) CORE_NOEXCEPT
			: References(1)
			, FusedObject(fusedObject)
		{
		}
	};

	template <typename T>
	struct FusedObject : public FusedObjectBase
	{
		T Storage;
		ControlBlock ControlBlock;

		template<class... Types>
		FusedObject(Types&&... args)
			: Storage(std::forward<Types>(args)...)
			, ControlBlock(this)
		{
		}

		~FusedObject() override {}
	};

	template <typename T>
	class SharedPointerST
	{
		T* m_PObject;
		ControlBlock* m_ControlBlock;

		void Decrease()
		{
			if (m_ControlBlock != nullptr && --(m_ControlBlock->References) == 0)
			{
				if (m_ControlBlock->FusedObject == nullptr)
				{
					delete m_PObject;
					delete m_ControlBlock;
				}
				else
				{
					delete m_ControlBlock->FusedObject;
				}
			}
		}

		void Increase() CORE_NOEXCEPT
		{
			if (m_ControlBlock != nullptr)
			{
				++(m_ControlBlock->References);
			}
		}

		SharedPointerST(T* pObject, ControlBlock* controlBlock) CORE_NOEXCEPT
			: m_PObject(pObject)
			, m_ControlBlock(controlBlock)
		{
		}

		template <typename OtherType>
		friend class SharedPointerST;

		template <typename T, typename... Types>
		friend SharedPointerST<T> MakeSharedPointerST(Types&&...);

		template <typename NewType, typename OldType>
		friend SharedPointerST<NewType> DynamicPointerCast(const SharedPointerST<OldType>&);

		template <typename NewType, typename OldType>
		friend SharedPointerST<NewType> DynamicPointerCast(SharedPointerST<OldType>&&);

	public:

		SharedPointerST() CORE_NOEXCEPT
			: m_PObject(nullptr)
			, m_ControlBlock(nullptr)
		{
		}

		explicit SharedPointerST(T* pObject) CORE_NOEXCEPT
			: m_PObject(pObject)
			, m_ControlBlock(new ControlBlock(nullptr))
		{
		}

		SharedPointerST(const SharedPointerST& other) CORE_NOEXCEPT
			: m_PObject(other.m_PObject)
			, m_ControlBlock(other.m_ControlBlock)
		{
			this->Increase();
		}

		template <typename OtherType, class = typename std::enable_if_t<std::is_convertible<OtherType*, T*>::value>>
		SharedPointerST(const SharedPointerST<OtherType>& other)
			: m_PObject(static_cast<T*>(other.m_PObject))
			, m_ControlBlock(other.m_ControlBlock)
		{
			this->Increase();
		}

		SharedPointerST(SharedPointerST&& other) CORE_NOEXCEPT
			: m_PObject(other.m_PObject)
			, m_ControlBlock(other.m_ControlBlock)
		{
			other.m_PObject = nullptr;
			other.m_ControlBlock = nullptr;
		}

		template <typename OtherType, class = typename std::enable_if_t<std::is_convertible<OtherType*, T*>::value>>
		SharedPointerST(SharedPointerST<T>&& other) CORE_NOEXCEPT
			: m_PObject(static_cast<T*>(other.m_PObject))
			, m_ControlBlock(other.m_ControlBlock)
		{
			other.m_PObject = nullptr;
			other.m_ControlBlock = nullptr;
		}

		CORE_CONSTEXPR SharedPointerST(std::nullptr_t) CORE_NOEXCEPT
			: m_PObject(nullptr)
			, m_ControlBlock(nullptr)
		{
		}

		~SharedPointerST()
		{
			Decrease();
		}

		SharedPointerST& operator=(const SharedPointerST& other)
		{
			this->Decrease();
			this->m_PObject = other.m_PObject;
			this->m_ControlBlock = other.m_ControlBlock;
			this->Increase();
			return *this;
		}

		template <typename OtherType, class = typename std::enable_if_t<std::is_convertible<OtherType*, T*>::value>>
		SharedPointerST& operator=(const SharedPointerST<OtherType>& other)
		{
			this->Decrease();
			this->m_PObject = static_cast<T*>(other.m_PObject);
			this->m_ControlBlock = other.m_ControlBlock;
			this->Increase();
			return *this;
		}

		SharedPointerST& operator=(SharedPointerST&& other)
		{
			this->Decrease();
			this->m_PObject = other.m_PObject;
			this->m_ControlBlock = other.m_ControlBlock;
			other.m_PObject = nullptr;
			other.m_ControlBlock = nullptr;
			return *this;
		}

		template <typename OtherType, class = typename std::enable_if_t<std::is_convertible<OtherType*, T*>::value>>
		SharedPointerST& operator=(SharedPointerST<OtherType>&& other)
		{
			this->Decrease();
			this->m_PObject = static_cast<T*>(other.m_PObject);
			this->m_ControlBlock = other.m_ControlBlock;
			other.m_PObject = nullptr;
			other.m_ControlBlock = nullptr;
			return *this;
		}

		void Reset()
		{
			Decrease();
			m_PObject = nullptr;
			m_ControlBlock = nullptr;
		}

		void Swap(SharedPointerST& other) CORE_NOEXCEPT
		{
			std::swap(m_PObject, other.m_PObject);
			std::swap(m_ControlBlock, other.m_ControlBlock);
		}

		size_t GetReferenceCount() const CORE_NOEXCEPT
		{
			if (m_ControlBlock != nullptr)
			{
				return m_ControlBlock->References;
			}
			return 0;
		}

		bool IsValid() const CORE_NOEXCEPT
		{
			return (m_PObject != nullptr);
		}

		T* Get() CORE_NOEXCEPT
		{
			return m_PObject;
		}

		const T* Get() const CORE_NOEXCEPT
		{
			return m_PObject;
		}

		T& operator*()
		{
			return *m_PObject;
		}

		const T& operator*() const
		{
			return *m_PObject;
		}

		T* operator->() CORE_NOEXCEPT
		{
			return m_PObject;
		}

		const T* operator->() const CORE_NOEXCEPT
		{
			return m_PObject;
		}

		bool operator==(const SharedPointerST& other) const CORE_NOEXCEPT
		{
			return (m_PObject == other.m_PObject);
		}

		bool operator!=(const SharedPointerST& other) const CORE_NOEXCEPT
		{
			return (m_PObject != other.m_PObject);
		}

		bool operator<(const SharedPointerST& other) const CORE_NOEXCEPT
		{
			return (m_PObject < other.m_PObject);
		}

		explicit operator bool() const CORE_NOEXCEPT
		{
			return IsValid();
		}
	};

	template <typename T, typename... Types>
	inline SharedPointerST<T> MakeSharedPointerST(Types&&... args)
	{
		auto* fusedObject = new FusedObject<T>(std::forward<Types>(args)...);
		return SharedPointerST<T>(&fusedObject->Storage, &fusedObject->ControlBlock);
	}

	template <typename NewType, typename OldType>
	inline SharedPointerST<NewType> DynamicPointerCast(const SharedPointerST<OldType>& sharedPointer)
	{
		auto newSharedPointer = SharedPointerST<NewType>(
			static_cast<NewType*>(sharedPointer.m_PObject), sharedPointer.m_ControlBlock);
		newSharedPointer.Increase();
		return newSharedPointer;
	}

	template <typename NewType, typename OldType>
	inline SharedPointerST<NewType> DynamicPointerCast(SharedPointerST<OldType>&& sharedPointer)
	{
		auto newSharedPointer = SharedPointerST<NewType>(
			static_cast<NewType*>(sharedPointer.m_PObject), sharedPointer.m_ControlBlock);
		sharedPointer.m_PObject = nullptr;
		sharedPointer.m_ControlBlock = nullptr;
		return newSharedPointer;
	}

	template <typename T>
	struct SharedPointerSTComparator : public std::binary_function<T, T, bool>
	{
		bool operator()(const Core::SharedPointerST<T>& lhs, const Core::SharedPointerST<T>& rhs) const
		{
			return *lhs.Get() < *rhs.Get();
		}
	};

	template <typename T> using SharedPointerSTSet
		= std::set<Core::SharedPointerST<T>, SharedPointerSTComparator<T>>;
	template <typename Key, typename Data> using SharedPointerSTMap
		= std::map<Core::SharedPointerST<Key>, Data, SharedPointerSTComparator<Key>>;
}

#endif