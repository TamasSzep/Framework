// EngineBuildingBlocks/Graphics/Primitives/ModelLoader.cpp

#include <EngineBuildingBlocks/Graphics/Primitives/ModelLoader.h>

#include <Core/Comparison.h>
#include <Core/Constants.h>
#include <Core/String.hpp>
#include <Core/AlgorithmExtensions.hpp>
#include <Core/System/SimpleIO.h>
#include <Core/System/Filesystem.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/ResourceDatabase.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <EngineBuildingBlocks/SceneNode.h>
#include <EngineBuildingBlocks/Animation/AnimationHelper.h>
#include <EngineBuildingBlocks/Graphics/Primitives/AssimpExtensions/SXMLSerialization.h>

#include <assimp/config.h>
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <queue>
#include <cassert>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace EngineBuildingBlocks::Animation;

MeshSplitOptionsType::MeshSplitOptionsType(bool isMeshSplittingAllowed)
	: IsSplittingLargeMeshes(isMeshSplittingAllowed)
	, VertexLimit(AI_SLM_DEFAULT_MAX_VERTICES)
	, TriangleLimit(AI_SLM_DEFAULT_MAX_TRIANGLES)
{
}

MeshSplitOptionsType::MeshSplitOptionsType(unsigned vertexLimit, unsigned triangleLimit)
	: IsSplittingLargeMeshes(true)
	, VertexLimit(vertexLimit)
	, TriangleLimit(triangleLimit)
{
}

bool MeshSplitOptionsType::operator==(const MeshSplitOptionsType& other)const
{
	NumericalEqualCompareBlock(IsSplittingLargeMeshes);
	if (IsSplittingLargeMeshes)
	{
		NumericalEqualCompareBlock(VertexLimit);
		NumericalEqualCompareBlock(TriangleLimit);
	}
	return true;
}

bool MeshSplitOptionsType::operator!=(const MeshSplitOptionsType& other)const
{
	return !(*this == other);
}

bool MeshSplitOptionsType::operator<(const MeshSplitOptionsType& other)const
{
	NumericalLessCompareBlock(IsSplittingLargeMeshes);
	if (IsSplittingLargeMeshes)
	{
		NumericalLessCompareBlock(VertexLimit);
		NumericalLessCompareBlock(TriangleLimit);
	}
	return false;
}

void MeshSplitOptionsType::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, IsSplittingLargeMeshes);
	Core::SerializeSB(bytes, VertexLimit);
	Core::SerializeSB(bytes, TriangleLimit);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

GeometryBuildOptions::GeometryBuildOptions()
	: IsCalculatingTangentSpaceVectors(false)
	, AreVertexColorsAllowed(true)
	, IsOptimizingMeshes(true)
	, IsOptimizingGraph(true)
	, IsFindingInstances(false)
	, IsImprovingCacheLocality(true)
	, IsFindingInvalidData(true)
	, BoneWeightEpsilon(0.0f)
	, IsForcingTextureCoordinates(true)
{
}

bool GeometryBuildOptions::operator==(const GeometryBuildOptions& other)const
{
	BoolEqualCompareBlock(IsCalculatingTangentSpaceVectors);
	BoolEqualCompareBlock(AreVertexColorsAllowed);
	BoolEqualCompareBlock(IsOptimizingMeshes);
	BoolEqualCompareBlock(IsOptimizingGraph);
	BoolEqualCompareBlock(IsFindingInstances);
	BoolEqualCompareBlock(IsImprovingCacheLocality);
	BoolEqualCompareBlock(IsFindingInvalidData);
	NumericalEqualCompareBlock(BoneWeightEpsilon);
	BoolEqualCompareBlock(IsForcingTextureCoordinates);
	StructureEqualCompareBlock(MeshSplitOptions);
	return true;
}

bool GeometryBuildOptions::operator!=(const GeometryBuildOptions& other)const
{
	return !(*this == other);
}

bool GeometryBuildOptions::operator<(const GeometryBuildOptions& other)const
{
	BoolLessCompareBlock(IsCalculatingTangentSpaceVectors);
	BoolLessCompareBlock(AreVertexColorsAllowed);
	BoolLessCompareBlock(IsOptimizingMeshes);
	BoolLessCompareBlock(IsOptimizingGraph);
	BoolLessCompareBlock(IsFindingInstances);
	BoolLessCompareBlock(IsImprovingCacheLocality);
	BoolLessCompareBlock(IsFindingInvalidData);
	NumericalLessCompareBlock(BoneWeightEpsilon);
	BoolLessCompareBlock(IsForcingTextureCoordinates);
	StructureLessCompareBlock(MeshSplitOptions);
	return false;
}

void GeometryBuildOptions::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, IsCalculatingTangentSpaceVectors);
	Core::SerializeSB(bytes, AreVertexColorsAllowed);
	Core::SerializeSB(bytes, IsOptimizingMeshes);
	Core::SerializeSB(bytes, IsOptimizingGraph);
	Core::SerializeSB(bytes, IsFindingInstances);
	Core::SerializeSB(bytes, IsImprovingCacheLocality);
	Core::SerializeSB(bytes, IsFindingInvalidData);
	Core::SerializeSB(bytes, BoneWeightEpsilon);
	Core::SerializeSB(bytes, IsForcingTextureCoordinates);
	Core::SerializeSB(bytes, MeshSplitOptions);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool AnimationBuildOptions::operator==(const AnimationBuildOptions& other) const
{
	NumericalEqualCompareBlock(FrameDuplicateReductionEpsilon);
	return true;
}

bool AnimationBuildOptions::operator!=(const AnimationBuildOptions& other) const
{
	return !(*this == other);
}

bool AnimationBuildOptions::operator<(const AnimationBuildOptions& other) const
{
	NumericalLessCompareBlock(FrameDuplicateReductionEpsilon);
	return false;
}

void AnimationBuildOptions::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, FrameDuplicateReductionEpsilon);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ModelBuildingDescription::ModelBuildingDescription()
	: IsBuiltModel(false)
{
}

std::string ModelBuildingDescription::GetModelBasePath() const
{
	return Core::GetParentPath(FilePath);
}

bool ModelBuildingDescription::operator==(const ModelBuildingDescription& other) const
{
	NumericalEqualCompareBlock(IsBuiltModel);
	StringEqualCompareBlock(FilePath);
	StructureEqualCompareBlock(GeometryOptions);
	return true;
}

bool ModelBuildingDescription::operator!=(const ModelBuildingDescription& other) const
{
	return !(*this == other);
}

bool ModelBuildingDescription::operator<(const ModelBuildingDescription& other) const
{
	NumericalLessCompareBlock(IsBuiltModel);
	StringLessCompareBlock(FilePath);
	StructureLessCompareBlock(GeometryOptions);
	return false;
}

void ModelBuildingDescription::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, IsBuiltModel);
	Core::SerializeSB(bytes, FilePath);
	Core::SerializeSB(bytes, GeometryOptions);
	Core::SerializeSB(bytes, AnimationOptions);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

PartialModelLoadingOptionsType::PartialModelLoadingOptionsType()
	: IsPartialModelLoadingAllowed(false)
{
}

bool PartialModelLoadingOptionsType::operator==(const PartialModelLoadingOptionsType& other)const
{
	NumericalEqualCompareBlock(IsPartialModelLoadingAllowed);
	StructureEqualCompareBlock(AllowedMeshIndices);
	StructureEqualCompareBlock(AllowedFaces);
	return true;
}

bool PartialModelLoadingOptionsType::operator!=(const PartialModelLoadingOptionsType& other)const
{
	return !(*this == other);
}

bool PartialModelLoadingOptionsType::operator<(const PartialModelLoadingOptionsType& other)const
{
	NumericalLessCompareBlock(IsPartialModelLoadingAllowed);
	StructureLessCompareBlock(AllowedMeshIndices);
	StructureLessCompareBlock(AllowedFaces);
	return false;
}

void PartialModelLoadingOptionsType::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, IsPartialModelLoadingAllowed);
	Core::SerializeSB(bytes, AllowedMeshIndices);
	Core::SerializeSB(bytes, AllowedFaces);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ModelLoadingDescription::operator==(const ModelLoadingDescription& other) const
{
	StructureEqualCompareBlock(BuildingDescription);
	StructureEqualCompareBlock(PartialModelLoadingOptions);
	return true;
}

bool ModelLoadingDescription::operator!=(const ModelLoadingDescription& other) const
{
	return !(*this == other);
}

bool ModelLoadingDescription::operator<(const ModelLoadingDescription& other) const
{
	StructureEqualCompareBlock(BuildingDescription);
	StructureLessCompareBlock(PartialModelLoadingOptions);
	return false;
}

void ModelLoadingDescription::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, BuildingDescription);
	Core::SerializeSB(bytes, PartialModelLoadingOptions);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void MeshGeometryData::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, CountVertices);
	Core::SerializeSB(bytes, CountIndices);
	Core::SerializeSB(bytes, BaseVertex);
	Core::SerializeSB(bytes, BaseIndex);
}

void MeshGeometryData::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, CountVertices);
	Core::DeserializeSB(bytes, CountIndices);
	Core::DeserializeSB(bytes, BaseVertex);
	Core::DeserializeSB(bytes, BaseIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void MaterialData::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, Name);
	Core::SerializeSB(bytes, Core::ToPlaceHolder(AmbientColor));
	Core::SerializeSB(bytes, Core::ToPlaceHolder(DiffuseColor));
	Core::SerializeSB(bytes, Core::ToPlaceHolder(SpecularColor));
	Core::SerializeSB(bytes, Opacity);
	Core::SerializeSB(bytes, DiffuseTextureName);
	Core::SerializeSB(bytes, OpacityTextureName);
	Core::SerializeSB(bytes, NormalTextureName);
}

void MaterialData::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, Name);
	Core::DeserializeSB(bytes, Core::ToPlaceHolder(AmbientColor));
	Core::DeserializeSB(bytes, Core::ToPlaceHolder(DiffuseColor));
	Core::DeserializeSB(bytes, Core::ToPlaceHolder(SpecularColor));
	Core::DeserializeSB(bytes, Opacity);
	Core::DeserializeSB(bytes, DiffuseTextureName);
	Core::DeserializeSB(bytes, OpacityTextureName);
	Core::DeserializeSB(bytes, NormalTextureName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void ObjectData::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, SceneNodeIndex);
	Core::SerializeSB(bytes, MeshIndex);
	Core::SerializeSB(bytes, MaterialIndex);
}

void ObjectData::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, SceneNodeIndex);
	Core::DeserializeSB(bytes, MeshIndex);
	Core::DeserializeSB(bytes, MaterialIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneNodeData::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, ParentIndex);
	Core::SerializeSB(bytes, Core::ToPlaceHolder(LocalTransformation));
	Core::SerializeSB(bytes, UpdateLevel);
}

void SceneNodeData::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, ParentIndex);
	Core::DeserializeSB(bytes, Core::ToPlaceHolder(LocalTransformation));
	Core::DeserializeSB(bytes, UpdateLevel);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Core::IndexVectorU BuiltModel::GetGlobalIndices() const
{
	auto& indices = Indices.Data;

	Core::IndexVectorU result;
	result.Reserve(indices.GetSize());

	unsigned countMeshes = Meshes.GetSize();
	for (unsigned i = 0; i < countMeshes; i++)
	{
		unsigned baseVertex = Meshes[i].BaseVertex;
		unsigned baseIndex = Meshes[i].BaseIndex;
		unsigned countIndices = Meshes[i].CountIndices;
		unsigned indexLimit = baseIndex + countIndices;
		for (unsigned j = baseIndex; j < indexLimit; j++)
		{
			result.UnsafePushBack(baseVertex + indices[j]);
		}
	}

	return result;
}

void BuiltModel::SerializeSB(Core::ByteVector& bytes) const
{
	Core::SerializeSB(bytes, SceneNodes);
	Core::SerializeSB(bytes, Objects);
	Core::SerializeSB(bytes, Meshes);
	Core::SerializeSB(bytes, Materials);
	Core::SerializeSB(bytes, SkeletalAnimations);
	Core::SerializeSB(bytes, BoneData);
	Core::SerializeSB(bytes, SceneNodeNames);
	Core::SerializeSB(bytes, Vertices);
	Core::SerializeSB(bytes, Indices);
	Core::SerializeSB(bytes, NonAnimatedBox);
	Core::SerializeSB(bytes, Textures);
}

void BuiltModel::DeserializeSB(const unsigned char*& bytes)
{
	Core::DeserializeSB(bytes, SceneNodes);
	Core::DeserializeSB(bytes, Objects);
	Core::DeserializeSB(bytes, Meshes);
	Core::DeserializeSB(bytes, Materials);
	Core::DeserializeSB(bytes, SkeletalAnimations);
	Core::DeserializeSB(bytes, BoneData);
	Core::DeserializeSB(bytes, SceneNodeNames);
	Core::DeserializeSB(bytes, Vertices);
	Core::DeserializeSB(bytes, Indices);
	Core::DeserializeSB(bytes, NonAnimatedBox);
	Core::DeserializeSB(bytes, Textures);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

inline void AssimpCheck(aiReturn expression)
{
	if (expression != AI_SUCCESS)
	{
		RaiseException("An assimp API call error has been occured.");
	}
}

const aiScene* ImportScene(Assimp::Importer* importer,
	const ModelBuildingDescription& buildingDescription)
{
	// If it's my own export, reading it directly.
	if (Core::ToLower(Core::GetFileExtension(buildingDescription.FilePath)) == ".sxml")
	{
		return ImportAssimpSceneSXML(buildingDescription.FilePath.c_str());
	}

	// Creating import flags.
	unsigned importFlags =
		aiProcess_JoinIdenticalVertices			// Joining identical vertices.
		| aiProcess_Triangulate					// Requesting triangles.
		| aiProcess_GenSmoothNormals			// If normals generated: generating smooth normals.
		| aiProcess_RemoveRedundantMaterials	// Removing redundant materials.
		| (buildingDescription.GeometryOptions.IsFindingInvalidData ? aiProcess_FindInvalidData : 0)
		| (buildingDescription.GeometryOptions.IsImprovingCacheLocality ? aiProcess_ImproveCacheLocality : 0)
		| (buildingDescription.GeometryOptions.IsOptimizingMeshes ? aiProcess_OptimizeMeshes : 0)
		| (buildingDescription.GeometryOptions.IsOptimizingGraph ? aiProcess_OptimizeGraph : 0)
		| (buildingDescription.GeometryOptions.IsFindingInstances ? aiProcess_FindInstances : 0)
		| (buildingDescription.GeometryOptions.MeshSplitOptions.IsSplittingLargeMeshes ? aiProcess_SplitLargeMeshes : 0)
		| (buildingDescription.GeometryOptions.IsCalculatingTangentSpaceVectors ? aiProcess_CalcTangentSpace : 0);

	// Avoiding aiProcess_FixInfacingNormals, since it sometimes messes up the normals
	// (eg. for Crytek Sponza).

	if (buildingDescription.GeometryOptions.MeshSplitOptions.IsSplittingLargeMeshes)
	{
		auto& options = buildingDescription.GeometryOptions.MeshSplitOptions;
		importer->SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, options.VertexLimit);
		importer->SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, options.TriangleLimit);
	}

	// Importing the scene.
	auto scene = importer->ReadFile(buildingDescription.FilePath.c_str(), importFlags);
	if (scene == nullptr)
	{
		std::stringstream ss;
		ss << "The model loader (Assimp) couldn't load model file '" << buildingDescription.FilePath
			<< "'. Error: " << importer->GetErrorString();
		RaiseException(ss);
	}

	// Taking ownership of the scene.
	importer->GetOrphanedScene();

	return scene;
}

SceneNodeData& CreateSceneNodeData(Core::SimpleTypeVectorU<SceneNodeData>& sceneNodes,
	unsigned parentSceneNodeIndex, unsigned& sceneNodeIndex, const char* name,
	std::vector<std::string>& sceneNodeNames)
{
	sceneNodeIndex = sceneNodes.GetSize();
	auto& sceneNode = sceneNodes.PushBackPlaceHolder();
	sceneNode.ParentIndex = parentSceneNodeIndex;
	assert(sceneNodeIndex == (unsigned)sceneNodeNames.size());
	sceneNodeNames.push_back(name);
	return sceneNode;
}

bool MeshesHaveSameAttributes(aiMesh* mesh1, aiMesh* mesh2, const GeometryBuildOptions& geometryOptions)
{
	return (mesh1->HasPositions() == mesh2->HasPositions()
		&& mesh1->HasTextureCoords(0) == mesh2->HasTextureCoords(0)
		&& mesh1->HasNormals() == mesh2->HasNormals()
		&& (!geometryOptions.IsCalculatingTangentSpaceVectors || mesh1->HasTangentsAndBitangents() == mesh2->HasTangentsAndBitangents())
		&& (!geometryOptions.AreVertexColorsAllowed || mesh1->HasVertexColors(0) == mesh2->HasVertexColors(0))
		&& mesh1->HasFaces() == mesh2->HasFaces());
}

inline void GenerateTexCoords(Core::ByteVectorU& target, unsigned countCurrentVertices)
{
	target.PushBack(unsigned char(0), countCurrentVertices * static_cast<unsigned>(sizeof(glm::vec2)));
}

template <typename T>
inline const unsigned char* GetBytes(const T* ptr) { return reinterpret_cast<const unsigned char*>(ptr); }

inline void CreateGeometry(const aiScene* scene, BuiltModel& builtModel,
	const GeometryBuildOptions& geometryOptions)
{
	if (scene->mNumMeshes == 0) return;

	auto& inputLayout = builtModel.Vertices.InputLayout;
	auto& vertices = builtModel.Vertices;
	auto& indices = builtModel.Indices.Data;
	auto& meshes = builtModel.Meshes;

	builtModel.Indices.Topology = PrimitiveTopology::TriangleList;

	// Creating vertex input layout.
	auto assimpMesh = scene->mMeshes[0];
	bool hasPositions = assimpMesh->HasPositions();
	bool hasTextureCoords = assimpMesh->HasTextureCoords(0) || geometryOptions.IsForcingTextureCoordinates;
	bool hasNormals = assimpMesh->HasNormals();
	bool hasTB = geometryOptions.IsCalculatingTangentSpaceVectors && assimpMesh->HasTangentsAndBitangents();
	bool hasVertexColor = geometryOptions.AreVertexColorsAllowed && assimpMesh->HasVertexColors(0);

	bool isGeneratingTexCoords = !assimpMesh->HasTextureCoords(0) && geometryOptions.IsForcingTextureCoordinates;

	if (hasPositions) inputLayout.Elements.push_back(c_PositionVertexElement);
	if (hasTextureCoords) inputLayout.Elements.push_back(c_TextureCoordinateVertexElement);
	if (hasNormals) inputLayout.Elements.push_back(c_NormalVertexElement);
	if (hasTB)
	{
		inputLayout.Elements.push_back(c_TangentVertexElement);
		inputLayout.Elements.push_back(c_BitangentVertexElement);
	}
	if (hasVertexColor) inputLayout.Elements.push_back(c_VertexColorVertexElement);

	unsigned positionIndex = inputLayout.GetVertexElementIndex(c_PositionVertexElement.Name.c_str());
	unsigned texCoordIndex = inputLayout.GetVertexElementIndex(c_TextureCoordinateVertexElement.Name.c_str());
	unsigned normalIndex = inputLayout.GetVertexElementIndex(c_NormalVertexElement.Name.c_str());
	unsigned tangentIndex = inputLayout.GetVertexElementIndex(c_TangentVertexElement.Name.c_str());
	unsigned bitangentIndex = inputLayout.GetVertexElementIndex(c_BitangentVertexElement.Name.c_str());
	unsigned vertexColorIndex = inputLayout.GetVertexElementIndex(c_VertexColorVertexElement.Name.c_str());

	if (!assimpMesh->HasFaces()) RaiseException("Mesh doesn't have indices.");

	unsigned countVertices = 0;
	unsigned countIndices = 0;

	unsigned countMeshes = scene->mNumMeshes;

	meshes.Resize(countMeshes);

	for (unsigned i = 0; i < countMeshes; i++)
	{
		auto assimpMesh = scene->mMeshes[i];
		auto& mesh = meshes[i];

		unsigned countCurrentVertices = assimpMesh->mNumVertices;
		unsigned countCurrentIndices = assimpMesh->mNumFaces * 3;

		mesh.BaseVertex = countVertices;
		mesh.BaseIndex = countIndices;
		mesh.CountVertices = countCurrentVertices;
		mesh.CountIndices = countCurrentIndices;

		countVertices += countCurrentVertices;
		countIndices += countCurrentIndices;
	}

	vertices.Data.resize(inputLayout.Elements.size());
	indices.Resize(countIndices);

	const unsigned vec2Size = sizeof(glm::vec2);
	const unsigned vec3Size = sizeof(glm::vec3);
	const unsigned vec4Size = sizeof(glm::vec4);
	const unsigned faceSize = 3 * sizeof(unsigned);

	for (unsigned meshIndex = 0; meshIndex < countMeshes; meshIndex++)
	{
		auto assimpMesh = scene->mMeshes[meshIndex];
		unsigned countCurrentVertices = assimpMesh->mNumVertices;
		unsigned countCurrentFaces = assimpMesh->mNumFaces;

		// Checking vertex input layout.
		if (!MeshesHaveSameAttributes(scene->mMeshes[0], assimpMesh, geometryOptions))
		{
			RaiseWarning("Meshes in the model have different attributes. Skipping current mesh.");
			continue;
		}

		if (hasPositions)
			vertices.Data[positionIndex].PushBack(GetBytes(assimpMesh->mVertices), countCurrentVertices * vec3Size);
		if (hasTextureCoords)
		{
			if (isGeneratingTexCoords)
			{
				GenerateTexCoords(vertices.Data[texCoordIndex], countCurrentVertices);
			}
			else
			{
				auto& target = vertices.Data[texCoordIndex];
				auto& source = assimpMesh->mTextureCoords[0];
				target.ReserveAdditionalWithGrowing(countCurrentVertices * vec2Size);
				for (unsigned vertexIndex = 0; vertexIndex < countCurrentVertices; vertexIndex++)
					target.UnsafePushBack(GetBytes(source + vertexIndex), vec2Size);
			}
		}
		if (hasNormals)
			vertices.Data[normalIndex].PushBack(GetBytes(assimpMesh->mNormals), countCurrentVertices * vec3Size);
		if (hasTB)
		{
			vertices.Data[tangentIndex].PushBack(GetBytes(assimpMesh->mTangents), countCurrentVertices * vec3Size);
			vertices.Data[bitangentIndex].PushBack(GetBytes(assimpMesh->mBitangents), countCurrentVertices * vec3Size);
		}
		if (hasVertexColor)
			vertices.Data[vertexColorIndex].PushBack(GetBytes(assimpMesh->mColors[0]), countCurrentVertices * vec4Size);

		auto pIndices = indices.GetArray() + meshes[meshIndex].BaseIndex;
		for (unsigned i = 0; i < countCurrentFaces; i++)
		{
			auto& face = assimpMesh->mFaces[i];
			assert(face.mNumIndices == 3);
			memcpy(pIndices, face.mIndices, faceSize);
			pIndices += 3;
		}
	}
}

inline void CreateSceneNodesAndObjects(aiNode* assimpNode, const aiScene* scene,
	Core::SimpleTypeVectorU<SceneNodeData>& sceneNodes,
	Core::SimpleTypeVectorU<ObjectData>& objects,
	std::vector<std::string>& sceneNodeNames,
	unsigned parentSceneNodeIndex)
{
	glm::mat4 transposedMatrix;
	memcpy(&transposedMatrix, &assimpNode->mTransformation, sizeof(glm::mat4));

	unsigned sceneNodeIndex;
	auto& sceneNode = CreateSceneNodeData(sceneNodes, parentSceneNodeIndex, sceneNodeIndex,
		assimpNode->mName.C_Str(), sceneNodeNames);
	sceneNode.LocalTransformation = reinterpret_cast<ScaledTransformation&>(
		glm::mat4x3(glm::transpose(transposedMatrix)));

	unsigned countObjects = assimpNode->mNumMeshes;
	for (unsigned i = 0; i < countObjects; i++)
	{
		unsigned meshIndex = assimpNode->mMeshes[i];
		auto assimpMesh = scene->mMeshes[meshIndex];
		auto& object = objects.PushBackPlaceHolder();
		object.MeshIndex = meshIndex;
		object.MaterialIndex = assimpMesh->mMaterialIndex;
		object.SceneNodeIndex = sceneNodeIndex;
	}

	unsigned countChildren = assimpNode->mNumChildren;
	for (unsigned i = 0; i < countChildren; i++)
	{
		CreateSceneNodesAndObjects(assimpNode->mChildren[i], scene, sceneNodes, objects, sceneNodeNames, sceneNodeIndex);
	}
}

inline void CreateSceneNodeUpdateData(BuiltModel& builtModel, const ModelLoadingDescription& loadingDescription)
{
	auto& sceneNodes = builtModel.SceneNodes;
	unsigned countInputSceneNodes = sceneNodes.GetSize();

	unsigned rootInputSNIndex = Core::c_InvalidIndexU;
	unsigned countRoots = 0;
	std::vector<Core::IndexVectorU> childrenIndices(countInputSceneNodes);
	for (unsigned i = 0; i < countInputSceneNodes; i++)
	{
		auto parentIndex = sceneNodes[i].ParentIndex;
		if (parentIndex == Core::c_InvalidIndexU) { rootInputSNIndex = i; ++countRoots; }
		else childrenIndices[parentIndex].PushBack(i);
	}
	if (countRoots > 1)
	{
		std::stringstream ss;
		ss << "More than one roots are present in the model: " << loadingDescription.BuildingDescription.FilePath;
		RaiseException(ss);
	}
	assert(rootInputSNIndex == 0);

	std::queue<unsigned> indexQueue1, indexQueue2;
	auto pSourceQueue = &indexQueue1;
	auto pTargetQueue = &indexQueue2;
	pSourceQueue->push(rootInputSNIndex);
	unsigned level = 0;
	while (true)
	{
		unsigned queueSize = static_cast<unsigned>(pSourceQueue->size());
		if (queueSize == 0) break;
		while (queueSize > 0)
		{
			auto index = pSourceQueue->front(); pSourceQueue->pop();
			sceneNodes[index].UpdateLevel = level;
			auto& children = childrenIndices[index];
			auto countChildren = children.GetSize();
			for (unsigned i = 0; i < countChildren; i++)
			{
				pTargetQueue->push(children[i]);
			}
			--queueSize;
		}
		std::swap(pSourceQueue, pTargetQueue);
		++level;
	}
}

inline void CreateSceneNodesAndObjects(const aiScene* scene, BuiltModel& builtModel,
	const ModelLoadingDescription& loadingDescription)
{
	CreateSceneNodesAndObjects(scene->mRootNode, scene, builtModel.SceneNodes, builtModel.Objects,
		builtModel.SceneNodeNames, Core::c_InvalidIndexU);
	CreateSceneNodeUpdateData(builtModel, loadingDescription);
}

inline ScaledTransformation GetSceneNodeTransformation(const BuiltModel& builtModel, unsigned sceneNodeIndex)
{
	// Note that if there are many scene nodes this recursive algorithm is not efficient.
	auto& node = builtModel.SceneNodes[sceneNodeIndex];
	if (node.ParentIndex == Core::c_InvalidIndexU) return node.LocalTransformation;
	auto parentTr = GetSceneNodeTransformation(builtModel, node.ParentIndex);
	auto& localTr = node.LocalTransformation;
	return { parentTr.A * localTr.A, parentTr.A * localTr.Position + parentTr.Position };
}

inline void ComputeNonAnimatedBox(BuiltModel& builtModel)
{
	auto box = EngineBuildingBlocks::Math::c_InvalidAABB;
	unsigned countObjects = builtModel.Objects.GetSize();
	for (unsigned i = 0; i < countObjects; i++)
	{
		auto& obj = builtModel.Objects[i];
		auto& mesh = builtModel.Meshes[obj.MeshIndex];

		auto transformation = GetSceneNodeTransformation(builtModel, obj.SceneNodeIndex);

		auto positions = builtModel.Vertices.GetPositions();
		unsigned limitVertex = mesh.BaseVertex + mesh.CountVertices;
		for (unsigned i = mesh.BaseVertex; i < limitVertex; i++)
		{
			auto transformedPosition = transformation.A * positions[i] + transformation.Position;
			box.Minimum = glm::min(box.Minimum, transformedPosition);
			box.Maximum = glm::max(box.Maximum, transformedPosition);
		}
	}
	builtModel.NonAnimatedBox = box;
}

const std::map<aiTextureType, std::string> c_TextureTypeMap =
{
	{ aiTextureType_DIFFUSE, "diffuse" },
	{ aiTextureType_OPACITY, "opacity" },
	{ aiTextureType_HEIGHT, "normal" }
};

inline std::string GetTextureName(const std::string& basePath, aiMaterial* assimpMaterial,
	aiTextureType textureType, bool isOptional)
{
	unsigned countTextures = assimpMaterial->GetTextureCount(textureType);
	for (unsigned i = 0; i < countTextures; i++)
	{
		aiString path;
		if (assimpMaterial->GetTexture(textureType, i, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			auto sPath = std::string(path.C_Str());
			auto fullPath = basePath + "/" + sPath;
			if (sPath.size() > 0 && sPath[0] == '*')
				return sPath; // It's an embedded texture.
			else if(Core::FileExists(fullPath))
				return fullPath;
			else
				RaiseWarning("Texture doesn't exist: " + sPath);
		}
		else if (!isOptional)
		{
			std::stringstream ss;
			ss << "The " << c_TextureTypeMap.find(textureType)->second << " model texture '"
				<< path.C_Str() << "' at index " << i << " couldn't be found.";
			RaiseException(ss);
		}
	}
	if (!isOptional)
	{
		// We use the replacement texture.
		return "white_pixel.png";
	}
	return std::string();
}

inline void CreateTexturesAndMaterials(const std::string& basePath, const aiScene* scene,
	std::vector<ImageRawData>& textures, std::vector<MaterialData>& materials)
{
	unsigned countTextures = scene->mNumTextures;
	textures.resize(countTextures);
	for (unsigned i = 0; i < countTextures; i++)
	{
		auto assimpTexture = scene->mTextures[i];
		auto& texture = textures[i];

		unsigned width, dataSize;
		if (assimpTexture->mHeight == 0)
		{
			width = 0;
			dataSize = assimpTexture->mWidth;
		}
		else
		{
			width = assimpTexture->mWidth;
			dataSize = width * assimpTexture->mHeight * static_cast<unsigned>(sizeof(aiTexel));
		}

		texture.Width = width;
		texture.Height = assimpTexture->mHeight;
		texture.Extension = assimpTexture->achFormatHint;
		texture.Data.PushBack(reinterpret_cast<unsigned char*>(assimpTexture->pcData), dataSize);
	}

	aiString name;
	unsigned countMaterials = scene->mNumMaterials;
	auto assimpMaterials = scene->mMaterials;
	materials.resize(countMaterials);
	for (unsigned i = 0; i < countMaterials; i++)
	{
		auto assimpMaterial = assimpMaterials[i];
		auto& material = materials[i];
		
		if(assimpMaterial->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) material.Name = name.C_Str();
		else material.Name = Core::Format("_unnamed_material_%x_", i);
		if (assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, reinterpret_cast<aiColor3D&>(material.AmbientColor)) != AI_SUCCESS) { material.AmbientColor = glm::vec3(0.0f); }
		if (assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(material.DiffuseColor)) != AI_SUCCESS) { material.DiffuseColor = glm::vec3(0.0f); }
		if (assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(material.SpecularColor)) != AI_SUCCESS) { material.SpecularColor = glm::vec3(0.0f); }
		if (assimpMaterial->Get(AI_MATKEY_OPACITY, material.Opacity) != AI_SUCCESS) { material.Opacity = 1.0f; }
		material.DiffuseTextureName = GetTextureName(basePath, assimpMaterial, aiTextureType_DIFFUSE, false);
		material.OpacityTextureName = GetTextureName(basePath, assimpMaterial, aiTextureType_OPACITY, true);
		material.NormalTextureName = GetTextureName(basePath, assimpMaterial, aiTextureType_HEIGHT, true);
	}
}

inline Core::SimpleTypeVectorU<KeyFrame> MergeKeyFrames(const Core::SimpleTypeVectorU<KeyFrame>& k1,
	const Core::SimpleTypeVectorU<KeyFrame>& k2)
{
	auto count1 = k1.GetSize();
	auto count2 = k2.GetSize();
	auto p1 = k1.GetArray();
	auto p2 = k2.GetArray();
	auto p1End = p1 + count1;
	auto p2End = p2 + count2;
	auto copySize = sizeof(KeyFrame);

	Core::SimpleTypeVectorU<KeyFrame> result;
	result.Resize(count1 + count2);
	auto pt = result.GetArray();

	while (p1 != p1End && p2 != p2End)
	{
		assert(p1->Time != p2->Time);
		if (p1->Time < p2->Time) { memcpy(pt, p1, copySize); ++p1; }
		else { memcpy(pt, p2, copySize); ++p2; }
		++pt;
	}
	if (p1 == p1End) memcpy(pt, p2, (p2End - p2) * copySize);
	else memcpy(pt, p1, (p1End - p1) * copySize);

	return result;
}

inline void CreateBoneData(const aiScene* scene, BuiltModel& builtModel,
	const GeometryBuildOptions& geometryOptions)
{
	std::map<std::string, unsigned> boneIndexMap;

	auto& boneNames = builtModel.BoneData.BoneNames;
	auto& boneOffsetTransformations = builtModel.BoneData.BoneOffsetTransformations;

	BoneInfluenceVector influenceVector;
	auto countMeshes = scene->mNumMeshes;
	unsigned countVertices = 0;
	for (unsigned i = 0; i < countMeshes; i++)
	{
		countVertices += scene->mMeshes[i]->mNumVertices;
	}
	influenceVector.resize(countVertices);
	unsigned baseVertex = 0;
	for (unsigned i = 0; i < countMeshes; i++)
	{
		auto assimpMesh = scene->mMeshes[i];
		unsigned countCurrentBones = assimpMesh->mNumBones;
		auto assimpBones = assimpMesh->mBones;
		for (unsigned j = 0; j < countCurrentBones; j++)
		{
			auto assimpBone = assimpBones[j];
			auto bIt = boneIndexMap.find(assimpBone->mName.C_Str());
			if (bIt == boneIndexMap.end())
			{
				auto boneName = assimpBone->mName.C_Str();
				boneNames.push_back(boneName);
				bIt = boneIndexMap.insert({ boneName, (unsigned)boneIndexMap.size() }).first;

				glm::mat4 transposedMatrix;
				memcpy(&transposedMatrix, &assimpBone->mOffsetMatrix, sizeof(glm::mat4));
				boneOffsetTransformations.PushBack(glm::transpose(transposedMatrix));
				
			}
			unsigned boneIndex = bIt->second;
			unsigned countWeights = assimpBone->mNumWeights;
			auto& weightDatas = assimpBone->mWeights;
			for (unsigned k = 0; k < countWeights; k++)
			{
				auto& weightData = weightDatas[k];
				auto& vertexInfluences = influenceVector[baseVertex + weightData.mVertexId];
				vertexInfluences.PushBack({ boneIndex, weightData.mWeight });
			}
		}
		baseVertex += assimpMesh->mNumVertices;
	}

	// Normalizing and removing epsilons.
	BoneData::NormalizeWeights(influenceVector);
	BoneData::RemoveEpsilonWeights(influenceVector, geometryOptions.BoneWeightEpsilon);
	BoneData::NormalizeWeights(influenceVector);

	builtModel.BoneData.SetInfluenceVector(influenceVector);
}

inline bool RemoveAnimationFrameDuplicates(BuiltModel& builtModel, const AnimationBuildOptions& animationOptions)
{
	bool hasDuplicate = false;

	auto& skeletalAnimations = builtModel.SkeletalAnimations;
	auto countAnimations = static_cast<unsigned>(skeletalAnimations.size());
	double epsilon = static_cast<double>(animationOptions.FrameDuplicateReductionEpsilon);
	
	Core::SimpleTypeVectorU<KeyFrame> newKeyframes;

	for (unsigned i = 0; i < countAnimations; i++)
	{
		auto& animation = skeletalAnimations[i];
		auto& channels = animation.Channels;
		auto countChannels = static_cast<unsigned>(channels.size());

		for (unsigned j = 0; j < countChannels; j++) assert(channels[i].Keyframes[0].Time == 0.0);

		double lastTime = 0.0;
		for (unsigned j = 0; j < countChannels; j++)
			lastTime = std::max(lastTime, channels[i].Keyframes.GetLastElement().Time);

		double diffToClosestLimit = lastTime * epsilon;
		for (unsigned j = 0; j < countChannels; j++)
		{
			auto& keyframes = channels[j].Keyframes;
			auto countKeyframes = keyframes.GetSize();
			newKeyframes.ClearAndReserve(keyframes.GetSize());
			newKeyframes.UnsafePushBack(keyframes[0]);
			for (unsigned k = 1; k < countKeyframes; k++)
			{
				double time0 = keyframes[k - 1].Time;
				double time1 = keyframes[k].Time;
				if (time1 - time0 < diffToClosestLimit)
				{
					// Simple heuristic: trying to keep frames with an integer time.
					if (abs(time0 - round(time0)) >= abs(time1 - round(time1)))
					{
						// The already saved time (time0) is the duplicate: rewrite it with time1.
						newKeyframes.GetLastElement() = keyframes[k];
					}
					// Th new time (time1) is the duplicate: we simple don't copy it.
					hasDuplicate = true;
				}
				else
				{
					newKeyframes.UnsafePushBack(keyframes[k]);
				}
			}
			keyframes = newKeyframes;
		}
	}
	return hasDuplicate;
}

inline void CreateAnimations(const aiScene* scene, BuiltModel& builtModel, const std::string& filePath,
	const AnimationBuildOptions& animationOptions)
{
	auto& skeletalAnimations = builtModel.SkeletalAnimations;

	bool usedReinterpolation = false;
	Core::SimpleTypeVectorU<glm::dualquat> dualQuats;
	Core::SimpleTypeVectorU<KeyFrame> keyframes1, keyframes2;

	unsigned countAnimations = scene->mNumAnimations;
	auto assimpAnimations = scene->mAnimations;
	skeletalAnimations.resize(countAnimations);
	for (unsigned i = 0; i < countAnimations; i++)
	{
		auto assimpAnimation = assimpAnimations[i];
		auto& animation = skeletalAnimations[i];

		assert(assimpAnimation->mNumMeshChannels == 0); // Mesh-based animation is not supported.

		animation.Name = assimpAnimation->mName.C_Str();
		animation.Duration = assimpAnimation->mDuration;
		animation.TicksPerSecond = assimpAnimation->mTicksPerSecond;

		unsigned countChannels = assimpAnimation->mNumChannels;
		auto assimpNodesAnimations = assimpAnimation->mChannels;
		for (unsigned j = 0; j < countChannels; j++)
		{
			auto assimpNodeAnimation = assimpNodesAnimations[j];

			animation.Channels.emplace_back();
			auto& boneAnimation = animation.Channels.back();

			boneAnimation.BoneName = assimpNodeAnimation->mNodeName.C_Str();
			boneAnimation.BeginWrap = static_cast<AnimationWrapMode>(assimpNodeAnimation->mPreState);
			boneAnimation.EndWrap = static_cast<AnimationWrapMode>(assimpNodeAnimation->mPostState);

			assert((boneAnimation.BeginWrap == AnimationWrapMode::Loop)
				== (boneAnimation.EndWrap == AnimationWrapMode::Loop));

			unsigned countPositions = assimpNodeAnimation->mNumPositionKeys;
			unsigned countOrientations = assimpNodeAnimation->mNumRotationKeys;
			unsigned countScalers = assimpNodeAnimation->mNumScalingKeys;

			auto orientations = assimpNodeAnimation->mRotationKeys;
			auto positions = assimpNodeAnimation->mPositionKeys;
			auto scalers = assimpNodeAnimation->mScalingKeys;

			auto& keyframes = boneAnimation.Keyframes;

			if (AreKeyframesSynchronized<ATGF, ATGF>(positions, countPositions, orientations, countOrientations))
			{
				keyframes.Resize(countPositions);
				for (unsigned k = 0; k < countPositions; ++k)
				{
					auto& keyFrame = keyframes[k];
					auto& q = orientations[k].mValue;
					auto& p = positions[k].mValue;
					keyFrame.Location.Orientation = glm::mat3(glm::quat(q.w, q.x, q.y, q.z));
					keyFrame.Location.Position = glm::vec3(p.x, p.y, p.z);
					keyFrame.Time = positions[k].mTime;
				}
			}
			else
			{
				usedReinterpolation = true;

				// Interpolating orientations for the position keyframes.
				keyframes1.Resize(countPositions);
				dualQuats.Resize(countPositions);
				for (unsigned k = 0; k < countPositions; k++)
				{
					auto time = positions[k].mTime;
					auto& p = positions[k].mValue;
					unsigned index0, index1; float weight;
					GetInterpolationDataForKeyframes<ATGF>(orientations, countOrientations,
						time, index0, index1, weight);
					auto& q0 = orientations[index0].mValue;
					auto& q1 = orientations[index1].mValue;
					auto q = glm::normalize(glm::slerp(glm::quat(q0.w, q0.x, q0.y, q0.z),
						glm::quat(q1.w, q1.x, q1.y, q1.z), weight));
					auto p2 = glm::vec3(p.x, p.y, p.z);
					auto& keyFrame = keyframes1[k];
					keyFrame.Location.Orientation = glm::mat3(q);
					keyFrame.Location.Position = p2;
					keyFrame.Time = time;
					dualQuats[i] = glm::dualquat(q, p2);
				}

				// Interpolating positions for the orientation keyframes using the dual quaternions.
				keyframes2.Clear();
				for (unsigned k = 0; k < countOrientations; k++)
				{
					auto time = orientations[k].mTime;
					if (GetTimeIndex<ATGF>(positions, countPositions, time) == Core::c_InvalidIndexU)
					{
						auto q = orientations[k].mValue;
						unsigned index0, index1; float weight;
						GetInterpolationDataForKeyframes<ATGF>(positions, countPositions, time,
							index0, index1, weight);
						auto& dq0 = dualQuats[index0];
						auto& dq1 = dualQuats[index1];
						auto dq = glm::lerp(dq0, dq1, weight);
						auto tq = (dq.dual * 2.0f) * glm::conjugate(dq.real);
						auto& keyFrame = keyframes2.PushBackPlaceHolder();
						keyFrame.Location.Orientation = glm::mat3(glm::quat(q.w, q.x, q.y, q.z));
						keyFrame.Location.Position = glm::vec3(tq.x, tq.y, tq.z);
						keyFrame.Time = time;
					}
				}

				keyframes = MergeKeyFrames(keyframes1, keyframes2);
			}

			auto opKeyframeArray = keyframes.GetArray();
			auto countOPKeyframes = keyframes.GetSize();
			if (AreKeyframesSynchronized<ETGF, ATGF>(opKeyframeArray, countOPKeyframes, scalers, countScalers))
			{
				for (unsigned k = 0; k < countOPKeyframes; ++k)
				{
					auto& s = scalers[k].mValue;
					keyframes[k].Scaler = glm::vec3(s.x, s.y, s.z);
				}
			}
			else
			{
				usedReinterpolation = true;

				// Computing scalers for the OP keyframes.
				for (unsigned k = 0; k < countOPKeyframes; k++)
				{
					unsigned index0, index1; float weight;
					GetInterpolationDataForKeyframes<ATGF>(scalers, countScalers, keyframes[k].Time,
						index0, index1, weight);
					auto& s0 = scalers[index0].mValue;
					auto& s1 = scalers[index1].mValue;
					keyframes[k].Scaler = glm::mix(glm::vec3(s0.x, s0.y, s0.z),
						glm::vec3(s1.x, s1.y, s1.z), weight);
				}

				// Checking that there is no scaler-only keyframe.
				for (unsigned k = 0; k < countScalers; k++)
				{
					if (GetTimeIndex<ETGF>(opKeyframeArray, countOPKeyframes, scalers[k].mTime)
						== Core::c_InvalidIndexU)
					{
						RaiseException("Scaler-only key frames are not yet supported.");
					}
				}
			}
		}
	}

	if (usedReinterpolation)
	{
		RaiseWarning("Used reinterpolation for animation key frames. File: " + filePath);
	}

	if (RemoveAnimationFrameDuplicates(builtModel, animationOptions))
	{
		RaiseWarning("Removed duplicates from animation keyframes. File: " + filePath);
	}
}

inline void InstantiateSceneNodes(const ModelInstantiationDescription& instantiationDescription,
	const BuiltModel& builtModel,
	SceneNodeHandler& sceneNodeHandler, ModelInstantiationResult& instantiationResult)
{
	auto& inputSceneNodes = builtModel.SceneNodes;
	auto countInputSceneNodes = inputSceneNodes.GetSize();

	// This is ensured by the creation of the scene nodes.
	unsigned rootInputSNIndex = 0;

	// Creating scene nodes with level hint.
	auto& sceneNodeIndices = instantiationResult.GlobalSceneNodeMapping;
	sceneNodeIndices.Resize(countInputSceneNodes);
	bool isStatic = instantiationDescription.IsStatic;
	for (unsigned i = 0; i < countInputSceneNodes; i++)
	{
		sceneNodeIndices[i] = sceneNodeHandler.CreateSceneNode(isStatic, inputSceneNodes[i].UpdateLevel);
	}
	instantiationResult.GlobalRootSceneNodeIndex = sceneNodeIndices[rootInputSNIndex];

	// Setting local transformations for and connections between scene nodes.
	for (unsigned i = 0; i < countInputSceneNodes; i++)
	{
		unsigned sceneNodeIndex = sceneNodeIndices[i];

		auto& inputSceneNodeData = inputSceneNodes[i];
		unsigned inputParentSNIndex = inputSceneNodeData.ParentIndex;

		if (inputParentSNIndex != Core::c_InvalidIndexU)
		{
			sceneNodeHandler.SetConnection(sceneNodeIndices[inputParentSNIndex], sceneNodeIndex);
		}

		sceneNodeHandler.SetLocalTransformation(sceneNodeIndex, inputSceneNodeData.LocalTransformation);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ModelLoader::ModelLoader(PathHandler* pathHandler, ResourceDatabase* resourceDatabase)
	: m_PathHandler(pathHandler)
	, m_ResourceDatabase(resourceDatabase)
	, m_Importer(std::make_unique<Assimp::Importer>())
	, m_Exporter(std::make_unique<Assimp::Exporter>())
{
	Core::PreparePath(m_PathHandler->GetPathFromBuiltResourcesDirectory(""));
}

ModelLoader::~ModelLoader()
{
}

inline void HandleModelPath(EngineBuildingBlocks::PathHandler* pathHandler, std::string& filePath)
{
	if (Core::IsRelativePath(filePath))
		filePath = pathHandler->GetPathFromResourcesDirectory("Models/" + filePath);
}

ModelLoadingResult ModelLoader::Load(const ModelLoadingDescription& description,
	Vertex_SOA_Data& vertexData, IndexData& indexData)
{
	// Copying and modifying description.
	auto descriptionCopy = description;
	HandleModelPath(m_PathHandler, descriptionCopy.BuildingDescription.FilePath);

	// Getting built resource description.
	BuiltResourceDescription builtResourceDescription;
	{
		ResourceDescription resourceDescription;
		resourceDescription.IsBuiltResource = descriptionCopy.BuildingDescription.IsBuiltModel;
		resourceDescription.ResourceFilePath = descriptionCopy.BuildingDescription.FilePath;

		// Serializing model building description.
		Core::StartSerializeSB(m_Buffer, descriptionCopy.BuildingDescription);
		resourceDescription.SerializedBuildOptions.PushBack(m_Buffer.GetArray(),
			static_cast<unsigned>(m_Buffer.GetSize()));

		// Currently dependencies are not set in the resource description.

		m_ResourceDatabase->GetBuiltResourceDescription(resourceDescription, builtResourceDescription);
	}

	// If the resource is not up-to-date, we are building it.
	auto& builtResourceFilePath = builtResourceDescription.BuiltResourceFilePath;
	if (!builtResourceDescription.IsUpToDate)
	{
		BuildResource(descriptionCopy, builtResourceFilePath);
	}

	// Loading from the built resource.
	return LoadFromBuiltResource(descriptionCopy, builtResourceFilePath, vertexData, indexData);
}

void ModelLoader::BuildResource(const ModelLoadingDescription& description,
	const std::string& builtResourceFilePath)
{
	// We store the model, so it can be directly accessed without loading.
	unsigned builtModelIndex = m_BuiltModels.Add();
	auto& builtModel = m_BuiltModels[builtModelIndex];
	m_BuiltModelMap[builtResourceFilePath] = builtModelIndex;

	auto& buildingDescription = description.BuildingDescription;

	// Loading resource.
	{
		auto& vertices = builtModel.Vertices;
		auto& indices = builtModel.Indices;
		auto& vertexInputLayout = builtModel.Vertices.InputLayout;

		// Importing scene.
		auto scene = ImportScene(m_Importer.get(), buildingDescription);

		// Creating components.
		CreateSceneNodesAndObjects(scene, builtModel, description);
		CreateGeometry(scene, builtModel, description.BuildingDescription.GeometryOptions);
		ComputeNonAnimatedBox(builtModel);
		CreateTexturesAndMaterials(description.BuildingDescription.GetModelBasePath(), scene,
			builtModel.Textures, builtModel.Materials);
		CreateBoneData(scene, builtModel, description.BuildingDescription.GeometryOptions);
		CreateAnimations(scene, builtModel, buildingDescription.FilePath,
			description.BuildingDescription.AnimationOptions);

		// Deleting scene.
		delete scene;
	}

	// Saving built resource.
	{
		Core::StartSerializeSB(m_Buffer, builtModel);
		Core::WriteAllBytes(builtResourceFilePath, m_Buffer.GetArray(), m_Buffer.GetSize());
	}
}

ModelLoadingResult ModelLoader::LoadFromBuiltResource(const ModelLoadingDescription& loadingDescription,
	const std::string& builtResourceFilePath, Vertex_SOA_Data& vertexData, IndexData& indexData)
{
	unsigned builtModelIndex;

	// Getting built resource from cache or file.
	auto rIt = m_BuiltModelMap.find(builtResourceFilePath);
	if (rIt == m_BuiltModelMap.end())
	{
		Core::ReadAllBytes(builtResourceFilePath, m_Buffer);

		builtModelIndex = m_BuiltModels.Add();
		Core::StartDeserializeSB(m_Buffer, m_BuiltModels[builtModelIndex]);

		m_BuiltModelMap[builtResourceFilePath] = builtModelIndex;
	}
	else
	{
		builtModelIndex = rIt->second;
	}
	auto& builtModel = m_BuiltModels[builtModelIndex];

	ModelLoadingResult result;
	result.ModelIndex = builtModelIndex;

	// Setting vertex and index buffers.
	assert((builtModel.Meshes.GetSize() > 0) == (builtModel.Objects.GetSize() > 0));
	if(builtModel.Meshes.GetSize() > 0)
	{
		auto& inputVertices = builtModel.Vertices;
		auto& outputVertices = vertexData;
		
		if (outputVertices.InputLayout.Elements.size() > 0)
		{
			if (!VertexInputLayout::IsSubset(inputVertices.InputLayout, outputVertices.InputLayout))
				RaiseException("Cannot propagate vertex buffer: the input layout of the output buffer is not a subset of the input layout of the input buffer.");
		}
		else
		{
			outputVertices.SetInputLayout(inputVertices.InputLayout);

			if (!inputVertices.InputLayout.HasPositions())
				RaiseException("Positions were not found in model.");
			if(!inputVertices.InputLayout.HasTextureCoordinates())
				RaiseWarning("Texture coordinates were not found in model.");
			if (!inputVertices.InputLayout.HasNormals())
				RaiseWarning("Normals were not found in model.");
		}
		if (indexData.GetCountIndices() == 0)
		{
			indexData.Topology = builtModel.Indices.Topology;
		}

		// MUADDIB_TODO: implement object and triangle selection.

		// Copying indices and vertices.
		indexData.Append(builtModel.Indices);
		vertexData.Append(builtModel.Vertices);

		// Copying geometry and meshes.
		auto& inputMeshes = builtModel.Meshes;
		auto& outputMeshes = result.Meshes;

		unsigned countInputMeshes = inputMeshes.GetSize();
		unsigned baseVertex = 0;
		unsigned baseIndex = 0;
		for (unsigned meshIndex = 0; meshIndex < countInputMeshes; meshIndex++)
		{
			auto& inputGeometryData = inputMeshes[meshIndex];
			unsigned countInputVertices = inputGeometryData.CountVertices;
			unsigned countInputIndices = inputGeometryData.CountIndices;

			auto& outputGeometryData = outputMeshes.PushBackPlaceHolder();
			outputGeometryData.CountVertices = countInputVertices;
			outputGeometryData.CountIndices = countInputIndices;
			outputGeometryData.BaseVertex = baseVertex;	// Setting resource-dependent base vertex.
			outputGeometryData.BaseIndex = baseIndex;	// Setting resource-dependent base index.
		
			baseVertex += countInputVertices;
			baseIndex += countInputIndices;
		}
	}

	return result;
}

ModelInstantiationResult ModelLoader::Instatiate(unsigned modelIndex,
	EngineBuildingBlocks::SceneNodeHandler& sceneNodeHandler, const ModelInstantiationDescription& description)
{
	auto& builtModel = m_BuiltModels[modelIndex];
	return InstantiateSceneNodes(builtModel, sceneNodeHandler, description);
}

const BuiltModel& ModelLoader::GetModel(unsigned index) const
{
	return m_BuiltModels[index];
}

BuiltModel& ModelLoader::GetModel(unsigned index)
{
	return m_BuiltModels[index];
}

namespace EngineBuildingBlocks
{
	namespace Graphics
	{
		ModelInstantiationResult InstantiateSceneNodes(
			const BuiltModel& builtModel, SceneNodeHandler& sceneNodeHandler,
			const ModelInstantiationDescription& description)
		{
			// MUADDIB_TODO: implement object selection.
			// Note that if objects are selected the update indices could be recomputed to be more dense.

			ModelInstantiationResult result;
			::InstantiateSceneNodes(description, builtModel, sceneNodeHandler, result);
			return result;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ModelLoader::RawApplyAnimation(const RawAnimationApplyDescription& description)
{
	assert(!description.BaseDescription.IsBuiltModel && !description.AnimationDescription.IsBuiltModel);
	auto baseDescCopy = description.BaseDescription;
	auto animationDescCopy = description.AnimationDescription;
	HandleModelPath(m_PathHandler, baseDescCopy.FilePath);
	HandleModelPath(m_PathHandler, animationDescCopy.FilePath);
	auto baseScene = const_cast<aiScene*>(ImportScene(m_Importer.get(), baseDescCopy));
	auto animationScene = const_cast<aiScene*>(ImportScene(m_Importer.get(), animationDescCopy));

	assert(baseScene->mNumAnimations == 0);
	baseScene->mNumAnimations = animationScene->mNumAnimations;
	animationScene->mNumAnimations = 0;
	baseScene->mAnimations = animationScene->mAnimations;
	animationScene->mAnimations = nullptr;

	auto fullPath = m_PathHandler->GetPathFromResourcesDirectory(
		"Models/_exported/" + description.ResultFile + ".sxml");
	ExportAssimpSceneSXML(baseScene, fullPath.c_str());

	delete baseScene;
	delete animationScene;
}