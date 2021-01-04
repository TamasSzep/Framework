// EngineBuildingBlocks/Graphics/Primitives/ModelLoader.h

#ifndef _ENGINEBUILDINGBLOCKS_MODELLOADER_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_MODELLOADER_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <Core/IntervalData.hpp>
#include <Core/SimpleBinarySerialization.hpp>
#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>
#include <EngineBuildingBlocks/SceneNode.h>
#include <EngineBuildingBlocks/Animation/SkeletalAnimation.h>
#include <EngineBuildingBlocks/Graphics/Resources/ImageHelper.h>
#include <EngineBuildingBlocks/Math/AABoundingBox.h>

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace Assimp
{
	class Importer;
	class Exporter;
}

namespace EngineBuildingBlocks
{
	class PathHandler;
	class ResourceDatabase;
	class SceneNodeHandler;

	namespace Graphics
	{
		struct MeshSplitOptionsType
		{
			bool IsSplittingLargeMeshes;
			unsigned VertexLimit;
			unsigned TriangleLimit;

			MeshSplitOptionsType(bool isMeshSplittingAllowed = false);
			MeshSplitOptionsType(unsigned vertexLimit, unsigned triangleLimit);

			bool operator==(const MeshSplitOptionsType& other)const;
			bool operator!=(const MeshSplitOptionsType& other)const;
			bool operator<(const MeshSplitOptionsType& other)const;

			void SerializeSB(Core::ByteVector& bytes) const;
		};

		struct GeometryBuildOptions
		{
			bool IsCalculatingTangentSpaceVectors;
			bool AreVertexColorsAllowed;
			bool IsOptimizingMeshes;
			bool IsOptimizingGraph;
			bool IsFindingInstances;
			bool IsImprovingCacheLocality;
			bool IsFindingInvalidData;
			float BoneWeightEpsilon;
			
			bool IsForcingTextureCoordinates;

			MeshSplitOptionsType MeshSplitOptions;

			GeometryBuildOptions();

			bool operator==(const GeometryBuildOptions& other)const;
			bool operator!=(const GeometryBuildOptions& other)const;
			bool operator<(const GeometryBuildOptions& other)const;

			void SerializeSB(Core::ByteVector& bytes) const;
		};

		struct AnimationBuildOptions
		{
			float FrameDuplicateReductionEpsilon = 0.0f;

			bool operator==(const AnimationBuildOptions& other) const;
			bool operator!=(const AnimationBuildOptions& other) const;
			bool operator<(const AnimationBuildOptions& other) const;

			void SerializeSB(Core::ByteVector& bytes) const;
		};

		struct ModelBuildingDescription
		{
			bool IsBuiltModel;
			std::string FilePath;
			GeometryBuildOptions GeometryOptions;
			AnimationBuildOptions AnimationOptions;

			ModelBuildingDescription();

			std::string GetModelBasePath() const;

			bool operator==(const ModelBuildingDescription& other) const;
			bool operator!=(const ModelBuildingDescription& other) const;
			bool operator<(const ModelBuildingDescription& other) const;

			void SerializeSB(Core::ByteVector& bytes) const;
		};

		struct PartialModelLoadingOptionsType
		{
			bool IsPartialModelLoadingAllowed;
			Core::IndexVectorU AllowedMeshIndices;
			std::map<unsigned, Core::IntervalData<unsigned>> AllowedFaces;

			PartialModelLoadingOptionsType();

			bool operator==(const PartialModelLoadingOptionsType& other)const;
			bool operator!=(const PartialModelLoadingOptionsType& other)const;
			bool operator<(const PartialModelLoadingOptionsType& other)const;

			void SerializeSB(Core::ByteVector& bytes) const;
		};

		struct ModelLoadingDescription
		{
			ModelBuildingDescription BuildingDescription;
			PartialModelLoadingOptionsType PartialModelLoadingOptions;

			bool operator==(const ModelLoadingDescription& other) const;
			bool operator!=(const ModelLoadingDescription& other) const;
			bool operator<(const ModelLoadingDescription& other) const;

			void SerializeSB(Core::ByteVector& bytes) const;
		};

		struct ModelInstantiationDescription
		{
			bool IsStatic = false;
		};

		struct MeshGeometryData
		{
			unsigned CountVertices;
			unsigned CountIndices;
			unsigned BaseVertex;
			unsigned BaseIndex;

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		struct MaterialData
		{
			std::string Name;

			glm::vec3 AmbientColor;
			glm::vec3 DiffuseColor;
			glm::vec3 SpecularColor;
			float Opacity;

			std::string DiffuseTextureName;
			std::string OpacityTextureName;
			std::string NormalTextureName;

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		struct ObjectData
		{
			// Asset-local scene node index.
			unsigned SceneNodeIndex;
			unsigned MeshIndex;
			unsigned MaterialIndex;

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		struct SceneNodeData
		{
			unsigned ParentIndex;
			ScaledTransformation LocalTransformation;
			unsigned UpdateLevel;

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		struct BuiltModel
		{
			Core::SimpleTypeVectorU<SceneNodeData> SceneNodes;
			Core::SimpleTypeVectorU<ObjectData> Objects;
			Core::SimpleTypeVectorU<MeshGeometryData> Meshes;
			std::vector<MaterialData> Materials;
			std::vector<EngineBuildingBlocks::Animation::SkeletalAnimation> SkeletalAnimations;
			EngineBuildingBlocks::Animation::BoneData BoneData;
			std::vector<std::string> SceneNodeNames;
			std::vector<ImageRawData> Textures;

			Vertex_SOA_Data Vertices;
			IndexData Indices;

			EngineBuildingBlocks::Math::AABoundingBox NonAnimatedBox;

			// Returns an index vector where the base vertex values
			// are added to the vertex indices.
			Core::IndexVectorU GetGlobalIndices() const;

			void SerializeSB(Core::ByteVector& bytes) const;
			void DeserializeSB(const unsigned char*& bytes);
		};

		struct ModelLoadingResult
		{
			unsigned ModelIndex;
			Core::SimpleTypeVectorU<MeshGeometryData> Meshes;
		};

		struct ModelInstantiationResult
		{
			// Maps the asset-local scene node indices to the global scene node
			// indices, where they are stored in the scene node handler.
			Core::IndexVectorU GlobalSceneNodeMapping;

			// The global root scene node index,
			// where the root scene node is stored in the scene node handler.
			unsigned GlobalRootSceneNodeIndex;
		};

		struct RawAnimationApplyDescription
		{
			ModelBuildingDescription BaseDescription;
			ModelBuildingDescription AnimationDescription;
			std::string ResultFile;
		};

		class ModelLoader
		{
			EngineBuildingBlocks::PathHandler* m_PathHandler;
			EngineBuildingBlocks::ResourceDatabase* m_ResourceDatabase;

			Core::ResourceUnorderedVectorU<BuiltModel> m_BuiltModels;
			std::map<std::string, unsigned> m_BuiltModelMap;

			Core::ByteVector m_Buffer;

			std::unique_ptr<Assimp::Importer> m_Importer;
			std::unique_ptr<Assimp::Exporter> m_Exporter;

			void BuildResource(const ModelLoadingDescription& description,
				const std::string& builtResourceFilePath);

			ModelLoadingResult LoadFromBuiltResource(
				const ModelLoadingDescription& loadingDescription,
				const std::string& builtResourceFilePath,
				Vertex_SOA_Data& vertexData, IndexData& indexData);

		public:

			ModelLoader(EngineBuildingBlocks::PathHandler* pathHandler,
				EngineBuildingBlocks::ResourceDatabase* resourceDatabase);
			~ModelLoader();

			ModelLoadingResult Load(const ModelLoadingDescription& description,
				Vertex_SOA_Data& vertexData, IndexData& indexData);

			ModelInstantiationResult Instatiate(unsigned modelIndex,
				EngineBuildingBlocks::SceneNodeHandler& sceneNodeHandler,
				const ModelInstantiationDescription& description = ModelInstantiationDescription());

			const BuiltModel& GetModel(unsigned index) const;
			BuiltModel& GetModel(unsigned index);

			void RawApplyAnimation(const RawAnimationApplyDescription& description);
		};

		ModelInstantiationResult InstantiateSceneNodes(
			const BuiltModel& builtModel, SceneNodeHandler& sceneNodeHandler,
			const ModelInstantiationDescription& description = ModelInstantiationDescription());
	}
}

#endif