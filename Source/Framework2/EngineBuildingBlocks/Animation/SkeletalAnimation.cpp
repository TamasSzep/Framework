// EngineBuildingBlocks/Animation/SkeletalAnimation.cpp

#include <EngineBuildingBlocks/Animation/SkeletalAnimation.h>

#include <Core/SimpleBinarySerialization.hpp>

using namespace EngineBuildingBlocks::Animation;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void KeyFrame::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, Location);
	Core::SerializeSB(bytes, Core::ToPlaceHolder(Scaler));
	Core::SerializeSB(bytes, Time);
}

void KeyFrame::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, Location);
	Core::DeserializeSB(bytes, Core::ToPlaceHolder(Scaler));
	Core::DeserializeSB(bytes, Time);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void BoneAnimationChannel::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, BoneName);
	Core::SerializeSB(bytes, Keyframes);
	Core::SerializeSB(bytes, BeginWrap);
	Core::SerializeSB(bytes, EndWrap);
}

void BoneAnimationChannel::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, BoneName);
	Core::DeserializeSB(bytes, Keyframes);
	Core::DeserializeSB(bytes, BeginWrap);
	Core::DeserializeSB(bytes, EndWrap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SkeletalAnimation::LoopAnimation()
{
	auto countNodes = static_cast<unsigned>(Channels.size());
	for (unsigned i = 0; i < countNodes; i++)
	{
		Channels[i].BeginWrap = AnimationWrapMode::Loop;
		Channels[i].EndWrap = AnimationWrapMode::Loop;
	}
}

void SkeletalAnimation::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, Name);
	Core::SerializeSB(bytes, Duration);
	Core::SerializeSB(bytes, TicksPerSecond);
	Core::SerializeSB(bytes, Channels);
}

void SkeletalAnimation::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, Name);
	Core::DeserializeSB(bytes, Duration);
	Core::DeserializeSB(bytes, TicksPerSecond);
	Core::DeserializeSB(bytes, Channels);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool BoneInfluence::operator==(const BoneInfluence& other) const
{
	NumericalEqualCompareBlock(BoneIndex);
	NumericalEqualCompareBlock(Weight);
	return true;
}

bool BoneInfluence::operator!=(const BoneInfluence& other) const
{
	return !(*this == other);
}

bool BoneInfluence::operator<(const BoneInfluence& other) const
{
	NumericalLessCompareBlock(BoneIndex);
	NumericalLessCompareBlock(Weight);
	return false;
}

void BoneInfluence::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, BoneIndex);
	Core::SerializeSB(bytes, Weight);
}

void BoneInfluence::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, BoneIndex);
	Core::DeserializeSB(bytes, Weight);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned BoneData::GetCountVertices() const
{
	return VertexInfluenceStartIndices.GetSize() - 1;
}

unsigned BoneData::GetCountBones() const
{
	return static_cast<unsigned>(BoneNames.size());
}

const BoneInfluence* BoneData::GetInfluences(unsigned vertexIndex) const
{
	return Influences.GetArray() + VertexInfluenceStartIndices[vertexIndex];
}

unsigned BoneData::GetCountBones(unsigned vertexIndex) const
{
	return VertexInfluenceStartIndices[vertexIndex + 1] - VertexInfluenceStartIndices[vertexIndex];
}

BoneInfluenceVector BoneData::GetInfluenceVector() const
{
	auto countVertices = GetCountVertices();
	BoneInfluenceVector result;
	result.resize(countVertices);
	for (unsigned i = 0; i < countVertices; i++)
	{
		auto& influences = result[i];
		auto cCountBones = GetCountBones(i);
		auto cBones = GetInfluences(i);
		for (unsigned j = 0; j < cCountBones; j++) influences.PushBack(cBones[j]);
	}
	return result;
}

void BoneData::SetInfluenceVector(const BoneInfluenceVector& influences)
{
	auto countVertices = static_cast<unsigned>(influences.size());

	unsigned totalInfluences = 0;
	for (unsigned i = 0; i < countVertices; i++) totalInfluences += influences[i].GetSize();

	unsigned vertexBoneDataStartIndex = 0;
	unsigned maxInfluenceCount = 0;
	Influences.ClearAndReserve(totalInfluences);
	VertexInfluenceStartIndices.ClearAndReserve(countVertices + 1);
	for (unsigned i = 0; i < countVertices; i++)
	{
		auto& vertexInfluences = influences[i];
		unsigned countInfluences = vertexInfluences.GetSize();
		VertexInfluenceStartIndices.UnsafePushBack(vertexBoneDataStartIndex);
		maxInfluenceCount = std::max(maxInfluenceCount, countInfluences);
		vertexBoneDataStartIndex += countInfluences;
		Influences.UnsafePushBack(vertexInfluences);
	}
	VertexInfluenceStartIndices.UnsafePushBack(vertexBoneDataStartIndex);	// Writing end index for simple bount
																			// count computation.
	MaxInfluenceCountPerVertex = maxInfluenceCount;
}

void BoneData::SortByIndices(BoneInfluenceVector& influences)
{
	auto countVertices = static_cast<unsigned>(influences.size());
	for (unsigned vertexIndex = 0; vertexIndex < countVertices; vertexIndex++)
	{
		auto& vertexInfluences = influences[vertexIndex];
		std::sort(vertexInfluences.GetArray(), vertexInfluences.GetEndPointer(), BoneInfluence_IndexLessComparator());
	}
}

void BoneData::SortByWeights(BoneInfluenceVector& influences)
{
	auto countVertices = static_cast<unsigned>(influences.size());
	for (unsigned vertexIndex = 0; vertexIndex < countVertices; vertexIndex++)
	{
		auto& vertexInfluences = influences[vertexIndex];
		std::sort(vertexInfluences.GetArray(), vertexInfluences.GetEndPointer(), BoneInfluence_WeightGreaterComparator());
	}
}

void BoneData::LimitMaximumBoneCountOnSorted(BoneInfluenceVector& influences, unsigned boneCountLimit)
{
	auto countVertices = static_cast<unsigned>(influences.size());
	for (unsigned vertexIndex = 0; vertexIndex < countVertices; vertexIndex++)
	{
		auto& vertexInfluences = influences[vertexIndex];
		if (vertexInfluences.GetSize() > boneCountLimit)
			vertexInfluences.Remove(boneCountLimit, vertexInfluences.GetSize());
	}
}

void BoneData::NormalizeWeights(BoneInfluenceVector& influences)
{
	auto countVertices = static_cast<unsigned>(influences.size());
	for (unsigned vertexIndex = 0; vertexIndex < countVertices; vertexIndex++)
	{
		auto& vertexInfluences = influences[vertexIndex];
		auto countBones = vertexInfluences.GetSize();
		float weightSum = 0.0f;
		for (unsigned i = 0; i < countBones; i++) weightSum += vertexInfluences[i].Weight;
		if (weightSum == 0.0f) continue;
		auto multiplier = 1.0f / weightSum;
		for (unsigned i = 0; i < countBones; i++) vertexInfluences[i].Weight *= multiplier;
	}
}

void BoneData::ClampWeights(BoneInfluenceVector& influences)
{
	auto countVertices = static_cast<unsigned>(influences.size());
	for (unsigned vertexIndex = 0; vertexIndex < countVertices; vertexIndex++)
	{
		auto& vertexInfluences = influences[vertexIndex];
		auto countBones = vertexInfluences.GetSize();
		for (unsigned i = 0; i < countBones; i++)
		{
			auto& weight = vertexInfluences[i].Weight;
			weight = glm::clamp(weight, 0.0f, 1.0f);
		}
	}
}

void BoneData::RemoveEpsilonWeights(BoneInfluenceVector& influences, float epsilon)
{
	auto countVertices = static_cast<unsigned>(influences.size());
	for (unsigned vertexIndex = 0; vertexIndex < countVertices; vertexIndex++)
	{
		auto& vertexInfluences = influences[vertexIndex];
		for (unsigned i = 0; i < vertexInfluences.GetSize();)
		{
			// We must also remove the influences if the weight equals to the epsilon, since for epsilon = 0.0
			// we want to remove the zero weighted influences.
			if (vertexInfluences[i].Weight <= epsilon) vertexInfluences.Remove(i);
			else ++i;
		}
	}
}

void BoneData::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, BoneNames);
	Core::SerializeSB(bytes, BoneOffsetTransformations);
	Core::SerializeSB(bytes, Influences);
	Core::SerializeSB(bytes, VertexInfluenceStartIndices);
	Core::SerializeSB(bytes, MaxInfluenceCountPerVertex);
}

void BoneData::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, BoneNames);
	Core::DeserializeSB(bytes, BoneOffsetTransformations);
	Core::DeserializeSB(bytes, Influences);
	Core::DeserializeSB(bytes, VertexInfluenceStartIndices);
	Core::DeserializeSB(bytes, MaxInfluenceCountPerVertex);
}