// EngineBuildingBlocks/Graphics/Primitives/AssimpExtensions/SXMLSerialization.h

#include <EngineBuildingBlocks/Graphics/Primitives/AssimpExtensions/SXMLSerialization.h>

#include <Core/SimpleXMLSerialization.hpp>
#include <Core/System/SimpleIO.h>
#include <Core/String.hpp>
#include <Core/Constants.h>

#include <assimp/scene.h>
#include <assimp/../../code/ScenePrivate.h>

namespace EngineBuildingBlocks
{
	namespace Graphics
	{
		template <typename NumberType, typename ElementType>
		inline void SerializeSXML(Core::SerializationSXMLData& data, const ElementType* array,
			unsigned elementSize, unsigned elementCount, const char* objectName)
		{
			if (array != nullptr)
			{
				auto values = reinterpret_cast<const NumberType*>(array);
				unsigned elementLimit = elementCount - 1;
				unsigned valueLimit = elementSize - 1;
				unsigned index = 0;
				unsigned elementStride = sizeof(ElementType) / sizeof(NumberType);
				for (unsigned i = 0; i < elementCount; i++, index += elementStride)
				{
					for (unsigned j = 0; j < elementSize; j++)
					{
						*data.SS << values[index + j];
						if (j < valueLimit) *data.SS << ",";
					}
					if (i < elementLimit) *data.SS << ";";
				}
				Core::CreateXMLNode(data, objectName, data.SS->str().c_str());
				data.SS->str("");
			}
		}

		template <typename NumberType, typename ElementType>
		inline void SerializeSXML(Core::SerializationSXMLData& data, const ElementType* array,
			unsigned elementCount, const char* objectName)
		{
			SerializeSXML<NumberType>(data, array, sizeof(ElementType) / sizeof(NumberType), elementCount, objectName);
		}

		template <typename NumberType, typename ElementType>
		inline void DeserializeSXML(Core::DeserializationSXMLData& data, ElementType*& array,
			unsigned elementSize, unsigned& elementCount, const char* objectName)
		{
			auto newData = Core::VisitChildXMLNode(data, objectName);
			if (newData.Node != nullptr)
			{
				newData.PlaceNodeValue();
				auto parts = Core::Split(newData.Chars->GetArray(), Core::IsCharacter(';'), true);
				std::vector<std::string> numberStrs;
				auto newElementCount = static_cast<unsigned>(parts.size());
				if (array == nullptr)
				{
					if (elementCount == 0) elementCount = newElementCount;
					else assert(elementCount == newElementCount);
					array = new ElementType[elementCount];
				}
				else assert(elementCount == newElementCount);
				auto values = reinterpret_cast<NumberType*>(array);
				unsigned elementLimit = elementCount - 1;
				unsigned valueLimit = elementSize - 1;
				unsigned index = 0;
				unsigned elementStride = sizeof(ElementType) / sizeof(NumberType);
				for (unsigned i = 0; i < elementCount; i++, index += elementStride)
				{
					for (unsigned j = 0; j < elementSize; j++)
					{
						Core::Split(parts[i], Core::IsCharacter(','), true, numberStrs);
						values[index + j] = Core::Parse<NumberType>(numberStrs[j].c_str());
					}
					for (unsigned j = elementSize; j < elementStride; ++j) values[index + j] = NumberType(0);
				}
			}
		}

		template <typename NumberType, typename ElementType>
		inline void DeserializeSXML(Core::DeserializationSXMLData& data, ElementType*& array,
			unsigned& elementCount, const char* objectName)
		{
			DeserializeSXML<NumberType>(data, array, sizeof(ElementType) / sizeof(NumberType), elementCount,
				objectName);
		}

		template <typename WrapperType, typename T>
		inline void SetFromWrapper(std::vector<WrapperType>& wrappers, T**& values, unsigned& countValues)
		{
			countValues = static_cast<unsigned>(wrappers.size());
			values = new T*[countValues];
			for (unsigned i = 0; i < countValues; i++)
			{
				values[i] = wrappers[i].GetWrappedPtr();
			}
		}

		struct BoneWrapper
		{
			aiBone* Bone;
			BoneWrapper() : Bone(nullptr) {}
			BoneWrapper(const aiBone* bone) : Bone(const_cast<aiBone*>(bone)) {}
			aiBone* GetWrappedPtr() { return Bone; }

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				auto Name = std::string(Bone->mName.C_Str());

				Core_SerializeSXML(data, Name);
				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, &Bone->mOffsetMatrix, 1, "OffsetMatrix");

				std::vector<unsigned> VertexIndices(Bone->mNumWeights);
				std::vector<float> VertexWeights(Bone->mNumWeights);
				for (unsigned i = 0; i < Bone->mNumWeights; ++i)
				{
					VertexIndices[i] = Bone->mWeights[i].mVertexId;
					VertexWeights[i] = Bone->mWeights[i].mWeight;
				}

				Core_SerializeSXML(data, VertexIndices);
				Core_SerializeSXML(data, VertexWeights);
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				assert(Bone == nullptr);
				Bone = new aiBone();

				std::string Name;

				Core_DeserializeSXML(data, Name);

				Bone->mName = Name.c_str();

				std::vector<unsigned> VertexIndices;
				std::vector<float> VertexWeights;

				Core_DeserializeSXML(data, VertexIndices);
				Core_DeserializeSXML(data, VertexWeights);

				Bone->mNumWeights = static_cast<unsigned>(VertexIndices.size());
				Bone->mWeights = new aiVertexWeight[Bone->mNumWeights];
				for (unsigned i = 0; i < Bone->mNumWeights; ++i)
				{
					Bone->mWeights[i].mVertexId = VertexIndices[i];
					Bone->mWeights[i].mWeight = VertexWeights[i];
				}

				unsigned countMatrices = 1;
				auto pMatrix = &Bone->mOffsetMatrix;
				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, pMatrix, countMatrices, "OffsetMatrix");
			}
		};

		struct MeshWrapper
		{
			aiMesh* Mesh;
			MeshWrapper() : Mesh(nullptr) {}
			MeshWrapper(const aiMesh* mesh) : Mesh(const_cast<aiMesh*>(mesh)) {}
			aiMesh* GetWrappedPtr() { return Mesh; }

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				auto PrimitiveTypes = Mesh->mPrimitiveTypes;
				auto MaterialIndex = Mesh->mMaterialIndex;
				auto Name = std::string(Mesh->mName.C_Str());
				auto NumAnimMeshes = Mesh->mNumAnimMeshes;

				std::vector<unsigned> NumUVComponents(AI_MAX_NUMBER_OF_TEXTURECOORDS);
				for (unsigned i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
					NumUVComponents[i] = Mesh->mNumUVComponents[i];

				Core_SerializeSXML(data, PrimitiveTypes);
				Core_SerializeSXML(data, MaterialIndex);
				Core_SerializeSXML(data, Name);
				Core_SerializeSXML(data, NumUVComponents);
				Core_SerializeSXML(data, NumAnimMeshes); // TODO: Remove this when adding the animation meshes!

				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Mesh->mVertices, Mesh->mNumVertices, "Positions");
				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Mesh->mNormals, Mesh->mNumVertices, "Normals");
				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Mesh->mTangents, Mesh->mNumVertices, "Tangents");
				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Mesh->mBitangents, Mesh->mNumVertices, "Bitangents");
				char buffer[32];
				for (unsigned i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
				{
					snprintf(buffer, sizeof(buffer), "TexCoord%d", i);
					EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Mesh->mTextureCoords[i],
						NumUVComponents[i], Mesh->mNumVertices, buffer);
				}
				for (unsigned i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; i++)
				{
					snprintf(buffer, sizeof(buffer), "VertexColor%d", i);
					EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Mesh->mColors[i], Mesh->mNumVertices, buffer);
				}

				std::vector<unsigned> FaceVertexCounts(Mesh->mNumFaces);
				unsigned indexCount = 0;
				for (unsigned i = 0; i < Mesh->mNumFaces; i++)
				{
					unsigned vertexCount = Mesh->mFaces[i].mNumIndices;
					FaceVertexCounts[i] = vertexCount;
					indexCount += vertexCount;
				}
				std::vector<unsigned> Indices(indexCount);
				unsigned index = 0;
				for (unsigned i = 0; i < Mesh->mNumFaces; i++)
				{
					unsigned vertexCount = FaceVertexCounts[i];
					memcpy(&Indices[index], Mesh->mFaces[i].mIndices, vertexCount * sizeof(unsigned));
					index += vertexCount;
				}

				Core_SerializeSXML(data, FaceVertexCounts);
				Core_SerializeSXML(data, Indices);

				std::vector<BoneWrapper> Bones;
				for (unsigned i = 0; i < Mesh->mNumBones; i++) Bones.emplace_back(Mesh->mBones[i]);

				Core_SerializeSXML(data, Bones);

				// TODO: missing: animation meshes!
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				assert(Mesh == nullptr);
				Mesh = new aiMesh();

				auto& PrimitiveTypes = Mesh->mPrimitiveTypes;
				auto& MaterialIndex = Mesh->mMaterialIndex;
				std::string Name;
				auto& NumAnimMeshes = Mesh->mNumAnimMeshes;

				Core_DeserializeSXML(data, PrimitiveTypes);
				Core_DeserializeSXML(data, MaterialIndex);
				Core_DeserializeSXML(data, Name);
				Core_DeserializeSXML(data, NumAnimMeshes); // TODO: Remove this when adding the animation meshes!

				Mesh->mName = Name.c_str();

				std::vector<unsigned> NumUVComponents;
				Core_DeserializeSXML(data, NumUVComponents);
				for (unsigned i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
					Mesh->mNumUVComponents[i] = NumUVComponents[i];

				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Mesh->mVertices, Mesh->mNumVertices, "Positions");
				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Mesh->mNormals, Mesh->mNumVertices, "Normals");
				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Mesh->mTangents, Mesh->mNumVertices, "Tangents");
				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Mesh->mBitangents, Mesh->mNumVertices, "Bitangents");
				char buffer[32];
				for (unsigned i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
				{
					snprintf(buffer, sizeof(buffer), "TexCoord%d", i);
					EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Mesh->mTextureCoords[i],
						NumUVComponents[i], Mesh->mNumVertices, buffer);
				}
				for (unsigned i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; i++)
				{
					snprintf(buffer, sizeof(buffer), "VertexColor%d", i);
					EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Mesh->mColors[i], Mesh->mNumVertices, buffer);
				}

				std::vector<unsigned> FaceVertexCounts;
				std::vector<unsigned> Indices;
				Core_DeserializeSXML(data, FaceVertexCounts);
				Core_DeserializeSXML(data, Indices);
				Mesh->mNumFaces = static_cast<unsigned>(FaceVertexCounts.size());
				Mesh->mFaces = new aiFace[Mesh->mNumFaces];
				unsigned index = 0;
				for (unsigned i = 0; i < Mesh->mNumFaces; i++)
				{
					auto& vertexCount = Mesh->mFaces[i].mNumIndices;
					auto& indices = Mesh->mFaces[i].mIndices;
					vertexCount = FaceVertexCounts[i];
					indices = new unsigned[vertexCount];
					memcpy(indices, &Indices[index], vertexCount * sizeof(unsigned));
					index += vertexCount;
				}

				std::vector<BoneWrapper> Bones;
				Core_DeserializeSXML(data, Bones);
				SetFromWrapper(Bones, Mesh->mBones, Mesh->mNumBones);

				// TODO: missing: animation meshes!
			}
		};

		struct MaterialPropertyWrapper
		{
			aiMaterialProperty* Property;
			MaterialPropertyWrapper() : Property(nullptr) {}
			MaterialPropertyWrapper(aiMaterialProperty* property)
				: Property(const_cast<aiMaterialProperty*>(property)) {}
			aiMaterialProperty* GetWrappedPtr() { return Property; }

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				auto Key = std::string(Property->mKey.C_Str());
				auto Sematic = Property->mSemantic;
				auto Index = Property->mIndex;
				auto Type = Property->mType;
				auto Data = Core::ToBinaryText(reinterpret_cast<const unsigned char*>(Property->mData),
					Property->mDataLength);

				Core_SerializeSXML(data, Key);
				Core_SerializeSXML(data, Sematic);
				Core_SerializeSXML(data, Index);
				Core_SerializeSXML(data, Type);
				Core_SerializeSXML(data, Data);
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				assert(Property == nullptr);
				Property = new aiMaterialProperty();

				std::string Key;
				auto& Sematic = Property->mSemantic;
				auto& Index = Property->mIndex;
				auto& Type = Property->mType;
				std::string Data;

				Core_DeserializeSXML(data, Key);
				Core_DeserializeSXML(data, Sematic);
				Core_DeserializeSXML(data, Index);
				Core_DeserializeSXML(data, Type);
				Core_DeserializeSXML(data, Data);

				Property->mKey = Key.c_str();
				Core::ToBinaryData(Data, reinterpret_cast<unsigned char*&>(Property->mData),
					Property->mDataLength);
			}
		};

		struct MaterialWrapper
		{
			aiMaterial* Material;
			MaterialWrapper() : Material(nullptr) {}
			MaterialWrapper(const aiMaterial* material) : Material(const_cast<aiMaterial*>(material)) {}
			aiMaterial* GetWrappedPtr() { return Material; }

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				std::vector<MaterialPropertyWrapper> Properties;
				for (unsigned i = 0; i < Material->mNumProperties; i++)
					Properties.emplace_back(Material->mProperties[i]);
				Core_SerializeSXML(data, Properties);
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				assert(Material == nullptr);
				Material = new aiMaterial();

				std::vector<MaterialPropertyWrapper> Properties;
				Core_DeserializeSXML(data, Properties);
				SetFromWrapper(Properties, Material->mProperties, Material->mNumProperties);
				Material->mNumAllocated = Material->mNumProperties;
			}
		};

		struct AnimationChannelWrapper
		{
			aiNodeAnim* Channel;
			AnimationChannelWrapper() : Channel(nullptr) {}
			AnimationChannelWrapper(const aiNodeAnim* channel) : Channel(const_cast<aiNodeAnim*>(channel)) {}
			aiNodeAnim* GetWrappedPtr() { return Channel; }

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				auto BoneName = std::string(Channel->mNodeName.C_Str());
				auto PreState = Channel->mPreState;
				auto PostState = Channel->mPostState;

				std::vector<double> OrientationTimes(Channel->mNumRotationKeys),
					PositionTimes(Channel->mNumPositionKeys), ScalerTimes(Channel->mNumScalingKeys);
				std::vector<aiQuaternion> Orientations(Channel->mNumRotationKeys);
				std::vector<aiVector3D> Positions(Channel->mNumPositionKeys), Scalers(Channel->mNumScalingKeys);
				for (unsigned i = 0; i < Channel->mNumRotationKeys; i++)
				{
					Orientations[i] = Channel->mRotationKeys[i].mValue;
					OrientationTimes[i] = Channel->mRotationKeys[i].mTime;
				}
				for (unsigned i = 0; i < Channel->mNumPositionKeys; i++)
				{
					Positions[i] = Channel->mPositionKeys[i].mValue;
					PositionTimes[i] = Channel->mPositionKeys[i].mTime;
				}
				for (unsigned i = 0; i < Channel->mNumScalingKeys; i++)
				{
					Scalers[i] = Channel->mScalingKeys[i].mValue;
					ScalerTimes[i] = Channel->mScalingKeys[i].mTime;
				}

				Core_SerializeSXML(data, BoneName);
				Core_SerializeSXML(data, PreState);
				Core_SerializeSXML(data, PostState);
				Core_SerializeSXML(data, OrientationTimes);
				Core_SerializeSXML(data, PositionTimes);
				Core_SerializeSXML(data, ScalerTimes);

				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Orientations.data(), Channel->mNumRotationKeys,
					"Orientations");
				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Positions.data(), Channel->mNumPositionKeys,
					"Position");
				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, Scalers.data(), Channel->mNumScalingKeys,
					"Scaler");
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				assert(Channel == nullptr);
				Channel = new aiNodeAnim();

				std::string BoneName;
				auto& PreState = Channel->mPreState;
				auto& PostState = Channel->mPostState;

				Core_DeserializeSXML(data, BoneName);
				Core_DeserializeSXML(data, PreState);
				Core_DeserializeSXML(data, PostState);

				Channel->mNodeName = BoneName.c_str();

				std::vector<double> OrientationTimes, PositionTimes, ScalerTimes;
				aiQuaternion* Orientations = nullptr;
				aiVector3D* Positions = nullptr;
				aiVector3D* Scalers = nullptr;

				Core_DeserializeSXML(data, OrientationTimes);
				Core_DeserializeSXML(data, PositionTimes);
				Core_DeserializeSXML(data, ScalerTimes);
				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Orientations, Channel->mNumRotationKeys, "Orientations");
				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Positions, Channel->mNumPositionKeys, "Position");
				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, Scalers, Channel->mNumScalingKeys, "Scaler");

				Channel->mRotationKeys = new aiQuatKey[Channel->mNumRotationKeys];
				Channel->mPositionKeys = new aiVectorKey[Channel->mNumPositionKeys];
				Channel->mScalingKeys = new aiVectorKey[Channel->mNumScalingKeys];

				for (unsigned i = 0; i < Channel->mNumRotationKeys; i++)
				{
					Channel->mRotationKeys[i].mValue = Orientations[i];
					Channel->mRotationKeys[i].mTime = OrientationTimes[i];
				}
				for (unsigned i = 0; i < Channel->mNumPositionKeys; i++)
				{
					Channel->mPositionKeys[i].mValue = Positions[i];
					Channel->mPositionKeys[i].mTime = PositionTimes[i];
				}
				for (unsigned i = 0; i < Channel->mNumScalingKeys; i++)
				{
					Channel->mScalingKeys[i].mValue = Scalers[i];
					Channel->mScalingKeys[i].mTime = ScalerTimes[i];
				}

				delete[] Orientations; delete[] Positions; delete[] Scalers;
			}
		};

		struct AnimationWrapper
		{
			aiAnimation* Animation;
			AnimationWrapper() : Animation(nullptr) {}
			AnimationWrapper(const aiAnimation* animation) : Animation(const_cast<aiAnimation*>(animation)) {}
			aiAnimation* GetWrappedPtr() { return Animation; }

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				auto Name = std::string(Animation->mName.C_Str());
				auto Duration = Animation->mDuration;
				auto TicksPerSecond = Animation->mTicksPerSecond;
				auto NumMeshChannels = Animation->mNumMeshChannels;

				std::vector<AnimationChannelWrapper> Channels;
				for (unsigned i = 0; i < Animation->mNumChannels; i++)
					Channels.emplace_back(Animation->mChannels[i]);

				Core_SerializeSXML(data, Name);
				Core_SerializeSXML(data, Duration);
				Core_SerializeSXML(data, TicksPerSecond);
				Core_SerializeSXML(data, NumMeshChannels); // TODO: Remove this when adding the mesh channels!
				Core_SerializeSXML(data, Channels);

				// TODO: missing: mesh animations.
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				assert(Animation == nullptr);
				Animation = new aiAnimation();

				std::string Name;
				auto& Duration = Animation->mDuration;
				auto& TicksPerSecond = Animation->mTicksPerSecond;
				auto& NumMeshChannels = Animation->mNumMeshChannels;

				Core_DeserializeSXML(data, Name);
				Core_DeserializeSXML(data, Duration);
				Core_DeserializeSXML(data, TicksPerSecond);
				Core_DeserializeSXML(data, NumMeshChannels); // TODO: Remove this when adding the mesh channels!

				Animation->mName = Name.c_str();

				std::vector<AnimationChannelWrapper> Channels;
				Core_DeserializeSXML(data, Channels);
				SetFromWrapper(Channels, Animation->mChannels, Animation->mNumChannels);

				// TODO: missing: mesh animations.
			}
		};

		struct PrivateDataWrapper
		{
			Assimp::ScenePrivateData* Private;
			PrivateDataWrapper(const Assimp::ScenePrivateData* privateData)
				: Private(const_cast<Assimp::ScenePrivateData*>(privateData)) {}

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				auto PPStepsApplied = Private->mPPStepsApplied;
				Core_SerializeSXML(data, PPStepsApplied);
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				Private->mOrigImporter = nullptr;
				Private->mIsCopy = true;
				auto& PPStepsApplied = Private->mPPStepsApplied;
				Core_DeserializeSXML(data, PPStepsApplied);
			}
		};

		struct MetadataWrapper
		{
			aiMetadata* Metadata;
			MetadataWrapper() : Metadata(nullptr) {}
			MetadataWrapper(const aiMetadata* metadata) : Metadata(const_cast<aiMetadata*>(metadata)) {}
			aiMetadata* GetWrappedPtr() { return Metadata; }

			inline size_t GetEntrySize(const aiMetadataEntry& entry) const
			{
				switch (entry.mType)
				{
				case AI_BOOL: return sizeof(bool);
				case AI_INT32: return sizeof(int);
				case AI_UINT64: return sizeof(uint64_t);
				case AI_FLOAT: return sizeof(float);
				case AI_DOUBLE: return sizeof(double);
				case AI_AISTRING: return strlen(reinterpret_cast<const char*>(entry.mData)) + 1;
				case AI_AIVECTOR3D: return sizeof(aiVector3D);
				}
				assert(false); return 0;
			}

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				if (Metadata == nullptr) return;

				std::vector<std::string> Keys;
				std::vector<aiMetadataType> Types;
				std::vector<std::string> Data;
				for (unsigned i = 0; i < Metadata->mNumProperties; i++)
				{
					Keys.emplace_back(Metadata->mKeys[i].C_Str());
					Types.push_back(Metadata->mValues[i].mType);
					Data.push_back(Core::ToBinaryText(
						reinterpret_cast<const unsigned char*>(Metadata->mValues[i].mData),
						GetEntrySize(Metadata->mValues[i])));
				}
				Core_SerializeSXML(data, Keys);
				Core_SerializeSXML(data, Types);
				Core_SerializeSXML(data, Data);
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				assert(Metadata == nullptr);
				if (data.Node->first_node() != nullptr)
				{
					Metadata = new aiMetadata();

					std::vector<std::string> Keys;
					std::vector<aiMetadataType> Types;
					std::vector<std::string> Data;
					Core_DeserializeSXML(data, Keys);
					Core_DeserializeSXML(data, Types);
					Core_DeserializeSXML(data, Data);
					assert(Keys.size() == Types.size() && Keys.size() == Data.size());

					Metadata->mNumProperties = static_cast<unsigned>(Keys.size());
					Metadata->mKeys = new aiString[Metadata->mNumProperties];
					Metadata->mValues = new aiMetadataEntry[Metadata->mNumProperties];
					for (unsigned i = 0; i < Metadata->mNumProperties; i++)
					{
						Metadata->mKeys[i] = Keys[i].c_str();
						Metadata->mValues[i].mType = Types[i];
						unsigned dataLength;
						Core::ToBinaryData(Data[i], reinterpret_cast<unsigned char*&>(Metadata->mValues[i].mData),
							dataLength);
					}
				}
			}
		};

		struct NodeWrapper
		{
			aiNode* Node;
			std::map<aiNode*, unsigned>* PNodeIndices;
			NodeWrapper() : Node(nullptr) {}
			NodeWrapper(const aiNode* node, std::map<aiNode*, unsigned>* pNodeIndices)
				: Node(const_cast<aiNode*>(node)), PNodeIndices(pNodeIndices) {}
			aiNode* GetWrappedPtr() { return Node; }

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				auto Name = std::string(Node->mName.C_Str());
				auto Parent = (Node->mParent == nullptr
					? Core::c_InvalidIndexU
					: PNodeIndices->find(Node->mParent)->second);

				std::vector<unsigned> Children(Node->mNumChildren);
				for (unsigned i = 0; i < Node->mNumChildren; i++)
					Children[i] = PNodeIndices->find(Node->mChildren[i])->second;

				std::vector<unsigned> Meshes(Node->mNumMeshes);
				for (unsigned i = 0; i < Node->mNumMeshes; i++)
					Meshes[i] = Node->mMeshes[i];

				MetadataWrapper Metadata(Node->mMetaData);

				Core_SerializeSXML(data, Name);
				Core_SerializeSXML(data, Parent);
				Core_SerializeSXML(data, Children);
				Core_SerializeSXML(data, Meshes);
				Core_SerializeSXML(data, Metadata);

				EngineBuildingBlocks::Graphics::SerializeSXML<float>(data, &Node->mTransformation, 1, "Transformation");
			}

			unsigned DParent;
			std::vector<unsigned> DChildren;

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				assert(Node == nullptr);
				Node = new aiNode();

				std::string Name;
				auto& Parent = DParent;
				auto& Children = DChildren;
				std::vector<unsigned> Meshes;
				MetadataWrapper Metadata;

				Core_DeserializeSXML(data, Name);
				Core_DeserializeSXML(data, Parent);
				Core_DeserializeSXML(data, Children);
				Core_DeserializeSXML(data, Meshes);
				Core_DeserializeSXML(data, Metadata);

				Node->mName = Name.c_str();

				Node->mNumChildren = static_cast<unsigned>(Children.size());
				Node->mChildren = new aiNode*[Node->mNumChildren];
				Node->mNumMeshes = static_cast<unsigned>(Meshes.size());
				Node->mMeshes = new unsigned[Node->mNumMeshes];
				memcpy(Node->mMeshes, Meshes.data(), Node->mNumMeshes * sizeof(unsigned));
				Node->mMetaData = Metadata.Metadata;

				unsigned countMatrices = 1;
				auto pMatrix = &Node->mTransformation;
				EngineBuildingBlocks::Graphics::DeserializeSXML<float>(data, pMatrix, countMatrices, "Transformation");
			}

			inline void DeserializeSXML_SetHierarchy(const std::vector<NodeWrapper>& wrappers)
			{
				Node->mParent = (DParent == Core::c_InvalidIndexU ? nullptr : wrappers[DParent].Node);
				for (unsigned i = 0; i < Node->mNumChildren; i++) Node->mChildren[i] = wrappers[DChildren[i]].Node;
			}
		};

		struct SceneWrapper
		{
			aiScene* Scene;
			SceneWrapper() : Scene(nullptr) {}
			SceneWrapper(const aiScene* scene) : Scene(const_cast<aiScene*>(scene)) {}

			inline void CreateNodeMap(std::vector<NodeWrapper>& nodes, std::map<aiNode*, unsigned>& nodeMap,
				aiNode* node) const
			{
				auto index = static_cast<unsigned>(nodes.size());
				nodes.emplace_back(node, &nodeMap);
				nodeMap[node] = index;
				for (unsigned i = 0; i < node->mNumChildren; ++i) CreateNodeMap(nodes, nodeMap, node->mChildren[i]);
			}

			inline void SerializeSXML(Core::SerializationSXMLData& data) const
			{
				auto Flags = Scene->mFlags;
				auto NumTextures = Scene->mNumTextures;
				auto NumLights = Scene->mNumLights;
				auto NumCameras = Scene->mNumCameras;

				std::vector<MeshWrapper> Meshes;
				for (unsigned i = 0; i < Scene->mNumMeshes; i++) Meshes.emplace_back(Scene->mMeshes[i]);

				std::vector<MaterialWrapper> Materials;
				for (unsigned i = 0; i < Scene->mNumMaterials; i++) Materials.emplace_back(Scene->mMaterials[i]);

				std::vector<AnimationWrapper> Animations;
				for (unsigned i = 0; i < Scene->mNumAnimations; i++) Animations.emplace_back(Scene->mAnimations[i]);

				Core_SerializeSXML(data, Flags);
				Core_SerializeSXML(data, NumTextures); // TODO: Remove this when adding the textures!
				Core_SerializeSXML(data, NumLights);  // TODO: Remove this when adding the lights!
				Core_SerializeSXML(data, NumCameras); // TODO: Remove this when adding the cameras!	
				Core_SerializeSXML(data, Meshes);
				Core_SerializeSXML(data, Materials);
				Core_SerializeSXML(data, Animations);

				std::vector<NodeWrapper> Nodes;
				std::map<aiNode*, unsigned> NodeMap;
				CreateNodeMap(Nodes, NodeMap, Scene->mRootNode);
				Core_SerializeSXML(data, Nodes);

				auto pPrivate = reinterpret_cast<const Assimp::ScenePrivateData*>(Scene->mPrivate);
				assert(pPrivate != nullptr);
				auto Private = PrivateDataWrapper(pPrivate);
				Core_SerializeSXML(data, Private);

				// TODO: missing: textures, lights, cameras.
			}

			inline void DeserializeSXML(Core::DeserializationSXMLData& data)
			{
				auto& Flags = Scene->mFlags;
				auto& NumTextures = Scene->mNumTextures;
				auto& NumLights = Scene->mNumLights;
				auto& NumCameras = Scene->mNumCameras;

				std::vector<MeshWrapper> Meshes;
				std::vector<MaterialWrapper> Materials;
				std::vector<AnimationWrapper> Animations;

				Core_DeserializeSXML(data, Flags);
				Core_DeserializeSXML(data, NumTextures); // TODO: Remove this when adding the textures!
				Core_DeserializeSXML(data, NumLights);  // TODO: Remove this when adding the lights!
				Core_DeserializeSXML(data, NumCameras); // TODO: Remove this when adding the cameras!
				Core_DeserializeSXML(data, Meshes);
				Core_DeserializeSXML(data, Materials);
				Core_DeserializeSXML(data, Animations);

				SetFromWrapper(Meshes, Scene->mMeshes, Scene->mNumMeshes);
				SetFromWrapper(Materials, Scene->mMaterials, Scene->mNumMaterials);
				SetFromWrapper(Animations, Scene->mAnimations, Scene->mNumAnimations);

				std::vector<NodeWrapper> Nodes;
				Core_DeserializeSXML(data, Nodes);
				unsigned countNodes = static_cast<unsigned>(Nodes.size());
				for (unsigned i = 0; i < countNodes; i++) Nodes[i].DeserializeSXML_SetHierarchy(Nodes);
				Scene->mRootNode = Nodes[0].Node;

				auto pPrivate = reinterpret_cast<Assimp::ScenePrivateData*>(Scene->mPrivate);
				assert(pPrivate != nullptr);
				auto Private = PrivateDataWrapper(pPrivate);
				Core_DeserializeSXML(data, Private);

				// TODO: missing: textures, lights, cameras.
			}
		};

		aiScene* ImportAssimpSceneSXML(const char* path)
		{
			auto scene = new aiScene();
			std::string text;
			Core::ReadAllText(path, text);
			Core::StartDeserializeSXML(text, SceneWrapper(scene), "Scene");
			return scene;
		}

		void ExportAssimpSceneSXML(const aiScene* scene, const char* path)
		{
			std::string text;
			Core::StartSerializeSXML(text, SceneWrapper(scene), "Scene");
			Core::WriteAllText(path, text);
		}
	}
}