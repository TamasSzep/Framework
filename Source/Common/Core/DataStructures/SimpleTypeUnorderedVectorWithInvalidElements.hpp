// Core/DataStructures/SimpleTypeUnorderedVectorWithInvalidElements.hpp

#ifndef _CORE_SIMPLETYPEUNORDEREDVECTORWITHINVALIDELEMENTS_HPP_
#define _CORE_SIMPLETYPEUNORDEREDVECTORWITHINVALIDELEMENTS_HPP_

#include <Core/DataStructures/SimpleTypeVector.hpp>

#include <map>

namespace Core
{
	template <typename T, const T InvalidElement_Invalid, const T InvalidElement_End, typename SizeType = size_t>
	class SimpleTypeUnorderedVectorWithInvalidElements
	{
		// InvalidElement1: 'Invalid' state
		// InvalidElement2: 'End' state

		SimpleTypeVector<T, SizeType> m_Data;
		SimpleTypeVector<SizeType, SizeType> m_UnusedIndices;
		SizeType m_CountElements;

		inline SizeType AddToVector(const T& element)
		{
			assert(element != InvalidElement_Invalid && element != InvalidElement_End);

			if (m_UnusedIndices.IsEmpty())
			{
				assert(!m_Data.IsEmpty());

				auto index = m_Data.GetSize() - 1;
				m_Data[index] = element;
				m_Data.PushBack(InvalidElement_End);
				return index;
			}
			else
			{
				auto index = m_UnusedIndices.PopBackReturn();
				m_Data[index] = element;
				return index;
			}
		}

	public:

		SimpleTypeUnorderedVectorWithInvalidElements()
			: m_CountElements(0)
		{
			m_Data.PushBack(InvalidElement_End);
		}

		// Initializes vector using a begin and an end iterator.
		// Indices are the integer numbers of the range [0, N-1], if the iterator encounter N elements.
		// If the number of the elements is unknown use the GetSize function
		// to figure out the index interval.
		template <typename Iterator>
		SimpleTypeUnorderedVectorWithInvalidElements(Iterator& begin, Iterator& end)
			: m_CountElements(0)
		{
			for (; begin != end; ++begin)
			{
				m_Data.PushBack(*begin);
				m_CountElements++;
			}
			m_Data.PushBack(InvalidElement_End);
		}

		inline SizeType GetSize() const
		{
			return m_CountElements;
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

		inline SizeType Add(const T& element)
		{
			++m_CountElements;
			return AddToVector(element);
		}

		inline void Remove(SizeType index)
		{
			assert(m_CountElements > 0 && index < m_Data.GetSize() - 1
				&& m_Data[index] != InvalidElement_Invalid && m_Data[index] != InvalidElement_End);

			--m_CountElements;
			m_Data[index] = InvalidElement_Invalid;
			m_UnusedIndices.PushBack(index);
		}

		inline void Clear()
		{
			m_Data.Clear();
			m_Data.PushBack(InvalidElement_End);
			m_UnusedIndices.Clear();
			m_CountElements = 0;
		}

		inline SizeType GetCountUnusedElements() const
		{
			return m_UnusedIndices.GetSize();
		}

		inline std::map<SizeType, SizeType> ShrinkToFit(bool isShrinkingUnderlyingVectors = true)
		{
			std::map<SizeType, SizeType> indexMap;

			SizeType arraySize = static_cast<SizeType>(m_Data.GetSize());
			SizeType targetIndex = 0;
			for (SizeType sourceIndex = 0; sourceIndex < arraySize; ++sourceIndex)
			{
				if (m_Data[sourceIndex] != InvalidElement_Invalid && m_Data[sourceIndex] != InvalidElement_End)
				{
					m_Data[targetIndex] = std::move(m_Data[sourceIndex]);
					indexMap[sourceIndex] = targetIndex;
					targetIndex++;
				}
			}
			m_Data[targetIndex] = InvalidElement_End;

			m_UnusedIndices.Clear();

			if (isShrinkingUnderlyingVectors)
			{
				m_State.ShrinkToFit();
				m_Data.ShrinkToFit();
			}

			return indexMap;
		}

		class Iterator
		{
			T* m_PData;

		public:

			Iterator(T* pData)
				: m_PData(pData)
			{
				while (*m_PData == InvalidElement_Invalid)  // State should be either Valid or End.
				{
					++m_PData;
				}
			}

			inline bool operator==(Iterator& other)
			{
				return (m_PData == other.m_PData);
			}

			inline bool operator!=(Iterator& other)
			{
				return (m_PData != other.m_PData);
			}

			inline bool operator<(Iterator& other)
			{
				return (m_PData < other.m_PData);
			}

			inline Iterator& operator++()
			{
				assert(*m_PData != InvalidElement_End);

				do
				{
					++m_PData;
				} while (*m_PData == InvalidElement_Invalid); // State should be either Valid or End.

				return *this;
			}

			inline T& operator*()
			{
				return *m_PData;
			}

			inline T* operator->()
			{
				return m_PData;
			}

			inline T* GetPtr()
			{
				return m_PData;
			}

			inline const T* GetPtr() const
			{
				return m_PData;
			}
		};

		inline Iterator GetBeginIterator()
		{
			return Iterator(m_Data.GetArray());
		}

		inline Iterator GetEndIterator()
		{
			return Iterator(m_Data.GetArray() + m_Data.GetSize() - 1);
		}

		inline SizeType ToIndex(const Iterator& it) const
		{
			return static_cast<SizeType>(it.GetPtr() - m_Data.GetArray());
		}
	};

	template <typename SizeType = size_t>
	using UnsignedSimpleTypeUnorderedVectorWithInvalidElements
		= SimpleTypeUnorderedVectorWithInvalidElements<unsigned, 0xfffffffe, 0xffffffff, SizeType>;
}

#endif