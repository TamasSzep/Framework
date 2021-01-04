// Core/DataStructures/BitVector.cpp

#include <Core/DataStructures/BitVector.h>

#include <stdexcept>

using namespace Core;

bool BitVector::s_IsStaticInitialized = false;
std::vector<std::vector<unsigned char>> BitVector::s_Masks;

void BitVector::StaticConstructor()
{
	if (!s_IsStaticInitialized)
	{
		s_Masks.resize(8);
		for (size_t i = 0; i < 8; i++)
		{
			s_Masks[i].resize(9);
			for (size_t j = 0; j <= i; j++)
			{
				s_Masks[i][j] = 0;
			}
			unsigned mask = 1 << i;
			for (size_t j = i + 1; j <= 8; j++)
			{
				s_Masks[i][j] = (unsigned char)mask;
				if (i > 0)
				{
					mask >>= i - 1;
				}
				else
				{
					mask <<= 1;
				}
				mask++;
				mask <<= i;
			}
		}
		s_IsStaticInitialized = true;
	}
}

BitVector::BitVector()
	: m_CountElements(0)
	, m_CountBitsPerElement(0)
	, m_ReservedCountElements(0)
	, m_UsingOwnVector(true)
{
	StaticConstructor();
}

BitVector::~BitVector()
{
}

size_t BitVector::GetCountTotalBits() const
{
	return (m_CountElements * m_CountBitsPerElement);
}

size_t BitVector::GetStoredByteCount() const
{
	size_t countTotalBits = GetCountTotalBits();
	size_t countTotalBytes = countTotalBits / 8;
	if (countTotalBits % 8 != 0)
	{
		countTotalBytes++;
	}
	return countTotalBytes;
}

const unsigned char* BitVector::GetBytes() const
{
	return m_Vector;
}

void BitVector::Add(size_t value)
{
	m_CountElements++;
	if (m_ReservedCountElements < m_CountElements)
	{
		m_ReservedCountElements++;
	}
	UpdateVector();
	Set(m_CountElements - 1, value);
}

void BitVector::Resize(size_t size)
{
	m_ReservedCountElements = size;
	if (m_ReservedCountElements < m_CountElements)
	{
		m_CountElements = m_ReservedCountElements;
	}
	UpdateVector();
}

void BitVector::AcquireBuffer(unsigned char* buffer)
{
	m_Vector = buffer;
	m_UsingOwnVector = false;
}

BitVector BitVector::CreateFromCountBitsPerElement(unsigned char countBits)
{
	if (countBits < 1)
	{
		throw std::runtime_error("At least one bit information must be stored in the bit vector.");
	}
	else if (countBits > 32)
	{
		throw std::runtime_error("A maximum of 32 bit information can be stored in the bit vector.");
	}

	BitVector bitVector;
	bitVector.m_CountBitsPerElement = countBits;
	return bitVector;
}

BitVector BitVector::CreateFromCountValues(size_t countValues)
{
	unsigned char countBits = 0;
	for (size_t i = countValues - 1; i > 0; i >>= 1, countBits++);
	return CreateFromCountBitsPerElement(countBits);
}

void BitVector::UpdateVector()
{
	if (m_UsingOwnVector)
	{
		size_t totalBits = m_ReservedCountElements * m_CountBitsPerElement;
		size_t totalBytes = totalBits / 8;
		if (totalBits % 8 != 0)
		{
			totalBytes++;
		}
		if (totalBytes > m_ContainedVector.size())
		{
			m_ContainedVector.resize(totalBytes);
			m_ReservedCountElements = totalBytes;
			m_Vector = &m_ContainedVector[0];
		}
	}
}

size_t BitVector::Get(size_t index) const
{
	size_t startBit = index * m_CountBitsPerElement;
	size_t startByte = startBit / 8;
	int readBits = 0;
	size_t value = 0;
	unsigned char startByteBit = startBit % 8;
	for (size_t i = startByte; readBits < m_CountBitsPerElement; i++)
	{
		int requiredBits = (unsigned char)(m_CountBitsPerElement - readBits);
		int endByteBit = (unsigned char)(8 - startByteBit > requiredBits ? startByteBit + requiredBits : 8);
		size_t cValue = ((m_Vector[i] & s_Masks[startByteBit][endByteBit]) >> startByteBit);
		value |= cValue << readBits;
		readBits += endByteBit - startByteBit;
		startByteBit = 0;
	}
	return value;
}

void BitVector::Set(size_t index, size_t value)
{
	size_t startBit = index * m_CountBitsPerElement;
	size_t startByte = startBit / 8;
	int writtenBits = 0;
	int startByteBit = startBit % 8;
	for (size_t i = startByte; writtenBits < m_CountBitsPerElement; i++)
	{
		int requiredBits = m_CountBitsPerElement - writtenBits;
		int endByteBit = (8 - startByteBit > requiredBits ? startByteBit + requiredBits : 8);
		unsigned char cValue = (unsigned char)(value >> writtenBits);
		unsigned char uc = (unsigned char)(~s_Masks[startByteBit][endByteBit]);
		m_Vector[i] &= uc;
		m_Vector[i] |= (unsigned char)(cValue << startByteBit);
		writtenBits += endByteBit - startByteBit;
		startByteBit = 0;
	}
}

BitVector::BitVectorAccessor BitVector::operator[](size_t index)
{
	return { this, index };
}

const BitVector::ConstBitVectorAccessor BitVector::operator[](size_t index) const
{
	return{ this, index };
}

size_t BitVector::BitVectorAccessor::operator=(size_t value)
{
	Owner->Set(Index, value);
	return value;
}

BitVector::BitVectorAccessor::operator size_t() const
{
	return Owner->Get(Index);
}

BitVector::ConstBitVectorAccessor::operator size_t() const
{
	return Owner->Get(Index);
}