// EngineBuildingBlocks/Animation/SkeletalAnimation.h

#ifndef _ENGINEBUILDINGBLOCKS_SKELETALANIMATION_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_SKELETALANIMATION_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <Core/DataStructures/SimpleTypeUnorderedVector.hpp>
#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <EngineBuildingBlocks/SceneNode.h>
#include <EngineBuildingBlocks/SystemTime.h>

namespace EngineBuildingBlocks
{
	namespace Animation
	{
		struct KeyFrame
		{
			EngineBuildingBlocks::RigidTransformation Location;
			glm::vec3 Scaler;
			double Time;

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		enum class AnimationWrapMode : unsigned char
		{
			Default,		// The default transformation is used.
			Clamp,			// Clamped to the first/last key frame.
			Extrapolate,	// Extrapolate from the first/last two key frames.
			Loop			// Repeating the animation endlessly.
		};

		struct BoneAnimationChannel
		{
			std::string BoneName;
			Core::SimpleTypeVectorU<KeyFrame> Keyframes;
			AnimationWrapMode BeginWrap;
			AnimationWrapMode EndWrap;

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		struct SkeletalAnimation
		{
			std::string Name;
			double Duration;
			double TicksPerSecond;

			std::vector<BoneAnimationChannel> Channels;

			void LoopAnimation();

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		struct BoneInfluence
		{
			unsigned BoneIndex;
			float Weight;

			bool operator==(const BoneInfluence& other) const;
			bool operator!=(const BoneInfluence& other) const;
			bool operator<(const BoneInfluence& other) const;

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		struct BoneInfluence_IndexLessComparator
		{
			bool operator()(const BoneInfluence& a, const BoneInfluence& b)
			{
				return a.BoneIndex < b.BoneIndex;
			}
		};

		struct BoneInfluence_WeightLessComparator
		{
			bool operator()(const BoneInfluence& a, const BoneInfluence& b)
			{
				return a.Weight < b.Weight;
			}
		};

		struct BoneInfluence_WeightGreaterComparator
		{
			bool operator()(const BoneInfluence& a, const BoneInfluence& b)
			{
				return a.Weight > b.Weight;
			}
		};

		// This is the easy-to-edit representation of the bone influences.
		// BoneData provides method to get and set (from) this representation
		// and static methods to edit the data in this representation.
		using BoneInfluenceVector = std::vector<Core::SimpleTypeVectorU<BoneInfluence>>;

		struct BoneData
		{
			// Indexed by the asset-local bone index.
			std::vector<std::string> BoneNames;

			// Contains vertex local -> bone space transformation for each bone.
			// Indexed by asset-local bone node indices.
			Core::SimpleTypeVectorU<glm::mat4> BoneOffsetTransformations;

			// Flattened array of vertex influences.
			Core::SimpleTypeVectorU<BoneInfluence> Influences;

			// Start index for the vertex influences in the 'Influence' array.
			// Indexed by the vertex.
			// Contains #Vertex + 1 elements for fast bone count computation.
			Core::IndexVectorU VertexInfluenceStartIndices;
			unsigned MaxInfluenceCountPerVertex;

			unsigned GetCountVertices() const;
			unsigned GetCountBones() const;

			const BoneInfluence* GetInfluences(unsigned vertexIndex) const;
			unsigned GetCountBones(unsigned vertexIndex) const;

			BoneInfluenceVector GetInfluenceVector() const;
			void SetInfluenceVector(const BoneInfluenceVector& influences);

			static void SortByIndices(BoneInfluenceVector& influences);
			static void SortByWeights(BoneInfluenceVector& influences);
			static void LimitMaximumBoneCountOnSorted(BoneInfluenceVector& influences, unsigned boneCountLimit);
			static void NormalizeWeights(BoneInfluenceVector& influences);
			static void ClampWeights(BoneInfluenceVector& influences);
			static void RemoveEpsilonWeights(BoneInfluenceVector& influences, float epsilon);

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};
	}
}

#endif