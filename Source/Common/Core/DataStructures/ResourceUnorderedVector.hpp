// Core/DataStructures/ResourceUnorderedVector.hpp

#ifndef _CORE_RESOURCEUNORDEREDVECTOR_HPP_
#define _CORE_RESOURCEUNORDEREDVECTOR_HPP_

#include <Core/Platform.h>
#include <Core/DataStructures/SimpleTypeVector.hpp>

#include <map>
#include <vector>

namespace Core
{
	template <typename T, typename SizeType = size_t>
	class ResourceUnorderedVector
	{
		enum class State : unsigned char
		{
			Valid, Invalid, End
		};

		std::vector<T> m_Data;
		SimpleTypeVector<State, SizeType> m_State;
		SimpleTypeVector<SizeType, SizeType> m_UnusedIndices;
		SizeType m_CountElements;

		template <typename... _T>
		CORE_FORCEINLINE SizeType InitializeNewObject(_T&& ... element)
		{
			assert(!m_State.IsEmpty());

			auto index = static_cast<SizeType>(m_Data.size());
			m_Data.emplace_back(std::forward<_T>(element)...);
			m_State.GetLastElement() = State::Valid;
			m_State.PushBack(State::End);
			return index;
		}

	public:

		ResourceUnorderedVector()
			: m_CountElements(0)
		{
			m_State.PushBack(State::End);
		}

		// Initializes vector using a begin and an end iterator.
		// Indices are the integer numbers of the range [0, N-1], if the iterator encounter N elements.
		// If the number of the elements is unknown use the GetSize function
		// to figure out the index interval.
		template <typename Iterator>
		ResourceUnorderedVector(Iterator& begin, Iterator& end)
			: m_CountElements(0)
		{
			for (; begin != end; ++begin)
			{
				m_Data.push_back(*begin);
				m_State.PushBack(State::Valid);
				m_CountElements++;
			}
			m_State.PushBack(State::End);
		}

		inline SizeType GetSize() const
		{
			return m_CountElements;
		}

		inline T* GetArray()
		{
			return m_Data.data();
		}

		inline const T* GetArray() const
		{
			return m_Data.data();
		}

		inline bool IsEmpty() const
		{
			return (m_CountElements == 0);
		}

		inline const T& operator[](SizeType index) const
		{
			assert(m_State[index] == State::Valid);

			return m_Data[index];
		}

		inline T& operator[](SizeType index)
		{
			assert(m_State[index] == State::Valid);

			return m_Data[index];
		}

		template <typename... _T>
		inline SizeType Add(_T&& ... element)
		{
			++m_CountElements;
			if (m_UnusedIndices.IsEmpty())
			{
				return InitializeNewObject(std::forward<_T>(element)...);
			}
			else
			{
				auto index = m_UnusedIndices.PopBackReturn();
				m_Data[index] = T(std::forward<_T>(element)...);
				m_State[index] = State::Valid;
				return index;
			}
		}

		// Note that the parameters are ONLY used if the reused object
		// is constructed and NOT when it's reused.
		template <typename... _T>
		inline SizeType AddNoReinit(_T&& ... element)
		{
			++m_CountElements;
			if (m_UnusedIndices.IsEmpty())
			{
				return InitializeNewObject(std::forward<_T>(element)...);
			}
			else
			{
				auto index = m_UnusedIndices.PopBackReturn();
				m_State[index] = State::Valid;
				return index;
			}
		}

		inline void Remove(SizeType index)
		{
			RemoveNoReinit(index);

			// Setting the data to default state.
			m_Data[index] = T();
		}

		CORE_FORCEINLINE void RemoveNoReinit(SizeType index)
		{
			assert(m_CountElements > 0 && m_State[index] == State::Valid);

			--m_CountElements;
			m_State[index] = State::Invalid;
			m_UnusedIndices.PushBack(index);
		}

		inline void Clear()
		{
			m_Data.clear();
			m_State.Clear();
			m_UnusedIndices.Clear();
			m_State.PushBack(State::End);
			m_CountElements = 0;
		}

		inline SizeType GetCountUnusedElements() const
		{
			return m_UnusedIndices.GetSize();
		}

		SimpleTypeVector<SizeType, SizeType> GetIndices() const
		{
			SimpleTypeVector<SizeType, SizeType> indices;
			indices.Reserve(m_CountElements);
			SizeType limit = m_State.GetSize() - 1;
			for (SizeType i = 0; i < limit; i++) if (m_State[i] == State::Valid) indices.UnsafePushBack(i);
			return indices;
		}

		inline std::map<SizeType, SizeType> ShrinkToFit(bool isShrinkingUnderlyingVectors = true)
		{
			std::map<SizeType, SizeType> indexMap;

			SizeType arraySize = static_cast<SizeType>(m_Data.size());
			SizeType targetIndex = 0;
			for (SizeType sourceIndex = 0; sourceIndex < arraySize; ++sourceIndex)
			{
				if (m_State[sourceIndex] == State::Valid)
				{
					m_Data[targetIndex] = std::move(m_Data[sourceIndex]);
					indexMap[sourceIndex] = targetIndex;
					targetIndex++;
				}
			}
			m_State.Clear();
			m_State.PushBack(State::Valid, targetIndex);
			m_State.PushBack(State::End);
			m_UnusedIndices.Clear();

			if (isShrinkingUnderlyingVectors)
			{
				m_State.ShrinkToFit();
				m_Data.shrink_to_fit();
			}

			return indexMap;
		}

		template <typename U>
		class TIterator
		{
			U* m_PData;
			const State* m_PState;

		public:

			TIterator(U* pData, const State* pState)
				: m_PData(pData)
				, m_PState(pState)
			{
				while (*m_PState == State::Invalid)  // State should be either Valid or End.
				{
					++m_PData;
					++m_PState;
				}
			}

			inline bool operator==(TIterator& other) const
			{
				return (m_PData == other.m_PData);
			}

			inline bool operator!=(TIterator& other) const
			{
				return (m_PData != other.m_PData);
			}

			inline bool operator<(TIterator& other) const
			{
				return (m_PData < other.m_PData);
			}

			inline TIterator& operator++()
			{
				assert(*m_PState != State::End);

				do
				{
					++m_PData;
					++m_PState;
				} while (*m_PState == State::Invalid); // State should be either Valid or End.

				return *this;
			}

			inline U& operator*() const
			{
				return *m_PData;
			}

			inline U* operator->() const
			{
				return m_PData;
			}

			inline U* GetDataPtr() const
			{
				return m_PData;
			}
		};

		using Iterator = TIterator<T>;
		using ConstIterator = TIterator<const T>;

	private:

		template <typename TIt, typename TData>
		static inline TIt _GetBeginIterator(TData& data, const SimpleTypeVector<State, SizeType>& state)
		{
			return TIt(data.data(), state.GetArray());
		}

		template <typename TIt, typename TData>
		static inline TIt _GetEndIterator(TData& data, const SimpleTypeVector<State, SizeType>& state)
		{
			SizeType arraySize = static_cast<SizeType>(data.size());
			return TIt(data.data() + arraySize, state.GetArray() + arraySize);
		}

		template <typename TIt>
		inline SizeType _ToIndex(const TIt& it) const
		{
			return static_cast<SizeType>(it.GetDataPtr() - m_Data.data());
		}

	public:

		inline Iterator GetBeginIterator()
		{
			return _GetBeginIterator<Iterator>(m_Data, m_State);
		}

		inline ConstIterator GetBeginConstIterator() const
		{
			return _GetBeginIterator<ConstIterator>(m_Data, m_State);
		}

		inline Iterator GetEndIterator()
		{
			return _GetEndIterator<Iterator>(m_Data, m_State);
		}

		inline ConstIterator GetEndConstIterator() const
		{
			return _GetEndIterator<ConstIterator>(m_Data, m_State);
		}

		inline SizeType ToIndex(const Iterator& it) const
		{
			return _ToIndex(it);
		}

		inline SizeType ToIndex(const ConstIterator& it) const
		{
			return _ToIndex(it);
		}
	};

	template <typename T>
	using ResourceUnorderedVectorU = ResourceUnorderedVector<T, unsigned>;
}

#endif