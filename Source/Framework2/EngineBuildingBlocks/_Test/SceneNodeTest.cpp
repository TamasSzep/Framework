// EngineBuildingBlocks/_Test/SceneNodeTest.cpp

#include "stdafx.h"

#include <EngineBuildingBlocks/_Test/SceneNodeTest.h>

#include <Framework/SystemTime.h>
#include <Framework/SceneNode.h>
#include <EngineBuildingBlocks/SceneNode.h>
#include <EngineBuildingBlocks/Graphics/Camera/Camera.h>
#include <EngineBuildingBlocks/Graphics/ViewFrustumCuller.h>

#include <random>

#include <glm-0.9.5.4/glm/gtx/euler_angles.hpp>

using namespace EngineBuildingBlocksTest;

const unsigned c_CountSceneNodes = 32 * 1024;
const unsigned c_CountStaticNodes = 0;
const unsigned c_CountConnections = 0;
const bool c_IsUpdatingFW1InThread = true;

std::mt19937 s_RandomGenerator;
std::uniform_real_distribution<float> s_UniformFloatDistribution(0.0f, 1.0f);
std::uniform_int_distribution<unsigned> s_UniformIntDistribution(0, c_CountSceneNodes - 1);

unsigned GetRandomSceneNodeIndex()
{
	return s_UniformIntDistribution(s_RandomGenerator);
}

void GetRandomConnection(unsigned& parentIndex, unsigned& childIndex)
{
	while (true)
	{
		parentIndex = GetRandomSceneNodeIndex();
		childIndex = GetRandomSceneNodeIndex();

		if (parentIndex != childIndex)
		{
			break;
		}
	}
	if (parentIndex > childIndex)
	{
		std::swap(parentIndex, childIndex);
	}
}

float GetRandomFloat()
{
	return s_UniformFloatDistribution(s_RandomGenerator);
}

float GetRandomAngle()
{
	return 2.0f * glm::pi<float>() * GetRandomFloat();
}

glm::vec3 GetRandomScaler()
{
	const float maxScaler = 2.0f;
	return{ GetRandomFloat() * maxScaler, GetRandomFloat() * maxScaler, GetRandomFloat() * maxScaler };
}

glm::vec3 GetRandomPosition()
{
	return{ GetRandomFloat(), GetRandomFloat(), GetRandomFloat() };
}

glm::mat3 GetRandomOrientation()
{
	return glm::mat3(glm::yawPitchRoll(GetRandomAngle(), GetRandomAngle(), GetRandomAngle()));
}

bool Equals(const glm::mat4x3& matrix1, const glm::mat4x3& matrix2)
{
	const float maxRelativeDifference = 2e-2f;
	const float maxLimit = 1e-3f;

	auto m1 = reinterpret_cast<const float*>(&matrix1);
	auto m2 = reinterpret_cast<const float*>(&matrix2);
	for (unsigned i = 0; i < 12; i++)
	{
		float diff = fabsf(m1[i] - m2[i]);
		float max = fabsf(std::max(m1[i], m2[i]));

		if (max > maxLimit && diff / max > maxRelativeDifference)
		{
			return false;
		}
	}
	return true;
}

struct SceneNodeUpdater
{
	void UpdateSceneNodesInThread(unsigned threadId, unsigned startIndex, unsigned endIndex,
		Framework::SceneNode* sceneNodes, glm::mat4x3* transformations, glm::mat4x3* invTransformations)
	{
		for (unsigned i = startIndex; i < endIndex; i++)
		{
			sceneNodes[i].GetScaledWorldAndInverseScaledWorldMatrices(
				transformations[i], invTransformations[i]);
		}
	}
};

void SceneNodeTest::Test()
{
	// Creating scene nodes for Framework1.
	std::vector<Framework::SceneNode> sceneNodes1;
	sceneNodes1.resize(c_CountSceneNodes);

	// Creating scene nodes for Framework2.
	EngineBuildingBlocks::SceneNodeHandler sceneNodeHandler;
	Core::SimpleTypeVector<unsigned, unsigned> sceneNode2Indices;
	for (unsigned i = 0; i < c_CountSceneNodes; i++)
	{
		sceneNode2Indices.PushBack(sceneNodeHandler.CreateSceneNode(i < c_CountStaticNodes));
	}

	// Creating random connections.
	unsigned parentIndex, childIndex;
	for (unsigned i = 0; i < c_CountConnections; i++)
	{
		GetRandomConnection(parentIndex, childIndex);

		sceneNodes1[parentIndex].AddChild(&sceneNodes1[childIndex]);
		sceneNodeHandler.SetConnection(parentIndex, childIndex);
	}

	// Testing connections.
	Core::SimpleTypeVector<unsigned> childrenIndices2;
	childrenIndices2.Resize(c_CountSceneNodes);
	for (unsigned i = 0; i < c_CountSceneNodes; i++)
	{
		auto parent1 = sceneNodes1[i].GetParent();
		unsigned parentIndex1 = (parent1 == nullptr
			? 0xffffffff
			: static_cast<unsigned>(parent1 - sceneNodes1.data()));
		unsigned countChildren1 = sceneNodes1[i].GetCountChildren();

		unsigned parentIndex2 = sceneNodeHandler.GetParent(sceneNode2Indices[i]);
		unsigned countChildren2 = 0xffffffff;
		sceneNodeHandler.GetChildren(sceneNode2Indices[i], childrenIndices2.GetArray(), countChildren2);

		assert(parentIndex1 == parentIndex2 && countChildren1 == countChildren2);

		for (unsigned j = 0; j < countChildren1; j++)
		{
			assert(static_cast<unsigned>(sceneNodes1[i].GetChild(j) - sceneNodes1.data())
				== childrenIndices2[j]);
		}
	}

	// Setting random orientation, position and scaler.
	for (unsigned i = 0; i < c_CountSceneNodes; i++)
	{
		auto orientation = GetRandomOrientation();
		auto position = GetRandomPosition();
		auto scaler = GetRandomScaler();

		auto& sceneNode1 = sceneNodes1[i];
		sceneNode1.SetLocalOrientation(orientation);
		sceneNode1.SetLocalPosition(position);
		sceneNode1.SetScaler(scaler);

		unsigned sceneNode2Index = sceneNode2Indices[i];
		sceneNodeHandler.SetLocalOrientation(sceneNode2Index, orientation);
		sceneNodeHandler.SetLocalPosition(sceneNode2Index, position);
		sceneNodeHandler.SetScaler(sceneNode2Index, scaler);
	}

	// Measuring computation times.
	Core::SimpleTypeVector<glm::mat4x3> transformations1, transformations2,
		invTransformations1, invTransformations2;
	transformations1.Resize(c_CountSceneNodes);
	transformations2.Resize(c_CountSceneNodes);
	invTransformations1.Resize(c_CountSceneNodes);
	invTransformations2.Resize(c_CountSceneNodes);

	Core::ThreadPool threadPool;
	SceneNodeUpdater sceneNodeUpdater;

	auto size1 = static_cast<unsigned>(sizeof(Framework::SceneNode));
	auto size2 = static_cast<unsigned>(sizeof(EngineBuildingBlocks::SceneNodeMainData));

	printf("Sizes: FW1: %d, FW2: %d\n", size1, size2);

	Framework::SystemTime time;
	time.Initialize();

	time.Update();
	if (c_IsUpdatingFW1InThread)
	{
		threadPool.ExecuteWithStaticScheduling(c_CountSceneNodes, &SceneNodeUpdater::UpdateSceneNodesInThread,
			&sceneNodeUpdater, sceneNodes1.data(), transformations1.GetArray(), invTransformations1.GetArray());
	}
	else
	{
		for (unsigned i = 0; i < c_CountSceneNodes; i++)
		{
			sceneNodes1[i].GetScaledWorldAndInverseScaledWorldMatrices(
				transformations1[i], invTransformations1[i]);
		}
	}
	time.Update();

	unsigned elapsedTime1 = static_cast<unsigned>(round(time.GetElapsedTime() * 1000000));

	time.Update();

	//for (size_t j = 0; j < 100; j++)
	{
		sceneNodeHandler.UpdateTransformations();
		for (unsigned i = 0; i < c_CountSceneNodes; i++)
		{
			unsigned sceneNode2Index = sceneNode2Indices[i];
			transformations2[i] = reinterpret_cast<const glm::mat4x3&>(
				sceneNodeHandler.UnsafeGetScaledWorldTransformation(sceneNode2Index));
			invTransformations2[i] = reinterpret_cast<const glm::mat4x3&>(
				sceneNodeHandler.UnsafeGetInverseScaledWorldTransformation(sceneNode2Index));
		}
		//auto nodeData = sceneNodeHandler.GetMainData();
		//for (unsigned i = 0; i < c_CountSceneNodes; i++)
		//{
		//	nodeData[sceneNode2Indices[i]].SetTransformationDirty();
		//}
	}

	time.Update();

	unsigned elapsedTime2 = static_cast<unsigned>(round(time.GetElapsedTime() * 1000000));

	printf("Time FW1: %d us\nTime FW2: %d us\nRatio: %f\n", elapsedTime1, elapsedTime2,
		elapsedTime2 / (float)elapsedTime1);

	// Checking computed result correctness.
	bool isResultCorrect = true;
	for (unsigned i = 0; i < c_CountSceneNodes; i++)
	{
		if (!Equals(transformations1[i], transformations2[i])
			|| !Equals(invTransformations1[i], invTransformations2[i]))
		{
			isResultCorrect = false;
			break;
		}
	}
	if (isResultCorrect)
	{
		printf("Result is correct!\n");
	}
	else
	{
		printf("Result is incorrect!\n");
	}

	// Creating camera.
	EngineBuildingBlocks::Graphics::Camera camera(&sceneNodeHandler);
	camera.SetPosition({ 0.0f, 0.0f, -3.0f });
	camera.LookAt({ 0.0f, 0.0f, 0.0f });

	// Creating bounding boxes.
	Core::SimpleTypeVector<EngineBuildingBlocks::Math::AABoundingBox> boundingBoxes;
	boundingBoxes.Resize(sceneNode2Indices.GetSize());
	glm::vec3 halfSize(0.5f);
	for (unsigned i = 0; i < c_CountSceneNodes; i++)
	{
		unsigned sceneNode2Index = sceneNode2Indices[i];
		auto position = sceneNodeHandler.GetPosition(sceneNode2Index);
		boundingBoxes[i].Minimum = position - halfSize;
		boundingBoxes[i].Maximum = position + halfSize;
	}

	// We also test view frustum culling.
	Core::SimpleTypeVector<unsigned, unsigned> outputIndices;
	EngineBuildingBlocks::Graphics::ViewFrustumCuller viewFrustumCuller;

	time.Update();

	viewFrustumCuller.ViewFrustumCull(camera, threadPool,
		sceneNodeHandler, boundingBoxes.GetArray(), sceneNode2Indices.GetArray(),
		sceneNode2Indices.GetArray(), outputIndices, sceneNode2Indices.GetSize());

	time.Update();

	unsigned elapsedTime3 = static_cast<unsigned>(round(time.GetElapsedTime() * 1000000));

	printf("Count visible scene nodes: %d\n", outputIndices.GetSize());
	printf("View frustum culling: %d us\n", elapsedTime3);
}