// Core/DataStructures/BitVector.h

#ifndef _CORE_BITVECTOR_H_
#define _CORE_BITVECTOR_H_

#include <cstddef>
#include <vector>

namespace Core
{
	class BitVector
	{
		static bool s_IsStaticInitialized;
		static std::vector<std::vector<unsigned char>> s_Masks;

		unsigned char* m_Vector;
		size_t m_CountElements;
		unsigned char m_CountBitsPerElement;
		size_t m_ReservedCountElements;

		std::vector<unsigned char> m_ContainedVector;
		bool m_UsingOwnVector;

		void UpdateVector();

		void Set(size_t index, size_t value);

		struct BitVectorAccessor
		{
			BitVector* Owner;
			size_t Index;

			size_t operator=(size_t value);
			operator size_t() const;
		};

		struct ConstBitVectorAccessor
		{
			const BitVector* Owner;
			size_t Index;

			operator size_t() const;
		};

		static void StaticConstructor();

	public:

		BitVector();
		~BitVector();

		size_t GetCountTotalBits() const;
		size_t GetStoredByteCount() const;

		const unsigned char* GetBytes() const;

		size_t Get(size_t index) const;

		void Add(size_t value);

		void Resize(size_t size);
		void AcquireBuffer(unsigned char* buffer);
		
		static BitVector CreateFromCountBitsPerElement(unsigned char countBits);
		static BitVector CreateFromCountValues(size_t countValues);
	
		BitVectorAccessor operator[](size_t index);
		const ConstBitVectorAccessor operator[](size_t index) const;
	};
}

#endif