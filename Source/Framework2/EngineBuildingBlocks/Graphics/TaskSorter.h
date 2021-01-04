// EngineBuildingBlocks/Graphics/TaskSorter.h

#ifndef _ENGINEBUILDINGBLOCKS_TASKSORTER_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_TASKSORTER_H_INCLUDED_

#include <Core/System/ThreadPool.h>
#include <EngineBuildingBlocks/SceneNode.h>
#include <EngineBuildingBlocks/Graphics/Camera/Camera.h>
#include <EngineBuildingBlocks/Math/AABoundingBox.h>

namespace EngineBuildingBlocks
{
	namespace Graphics
	{
		enum class TaskSortType : unsigned char
		{
			FrontToBack,
			BackToFront,
			MaximumOcclusion
		};

		class TaskSorter
		{
		public:

			struct SortData
			{
				unsigned Index;
				float Value;
				bool operator<(const SortData& other) const { return Value < other.Value; }
				bool operator>(const SortData& other) const { return Value > other.Value; }
			};

		private:

			Core::SimpleTypeVectorU<SortData> m_SortData;

		public:

			// This function sorts the tasks. It assumes, that the scene nodes' scaled
			// model transformation is up-to-date.
			template <typename TaskType>
			void Sort(Camera& camera,
				Core::ThreadPool& threadPool,
				const EngineBuildingBlocks::SceneNodeHandler& sceneNodeHandler,
				const TaskType* taskData,
				unsigned* taskIndices, unsigned countTasks, TaskSortType sortType)
			{
				m_SortData.Resize(countTasks);

				// Parallel load.
				switch (sortType)
				{
				case TaskSortType::FrontToBack:
					LoadKeys(&TaskSorter::LoadFrontToBackKeys<TaskType>, m_SortData, camera, threadPool,
						sceneNodeHandler, taskData, taskIndices, countTasks); break;
				case TaskSortType::BackToFront:
					LoadKeys(&TaskSorter::LoadBackToFrontKeys<TaskType>, m_SortData, camera, threadPool,
						sceneNodeHandler, taskData, taskIndices, countTasks); break;
				case TaskSortType::MaximumOcclusion:
					LoadKeys(&TaskSorter::LoadMaximumOcclusionKeys<TaskType>, m_SortData, camera, threadPool,
						sceneNodeHandler, taskData, taskIndices, countTasks); break;
				}

				// Single-threaded sort.
				std::sort(m_SortData.GetArray(), m_SortData.GetEndPointer());

				for (unsigned i = 0; i < countTasks; i++)
				{
					taskIndices[i] = m_SortData[i].Index;
				}
			}

		private:

			template <typename TaskType, typename Function>
			inline void LoadKeys(Function&& function, Core::SimpleTypeVectorU<TaskSorter::SortData>& sortData,
				Camera& camera, Core::ThreadPool& threadPool, const SceneNodeHandler& sceneNodeHandler,
				const TaskType* taskData, const unsigned* taskIndices, unsigned countTasks)
			{
				auto cameraPosition = camera.GetPosition();

				threadPool.ExecuteWithStaticScheduling(countTasks, std::forward<Function>(function), this,
					sortData.GetArray(), cameraPosition, &sceneNodeHandler, taskData, taskIndices);
			}

			template <typename TaskType>
			inline void LoadMaximumOcclusionKeys(unsigned threadId, unsigned startIndex, unsigned endIndex,
				SortData* sortData, const glm::vec3& cameraPosition,
				const SceneNodeHandler* sceneNodeHandler, const TaskType* taskData, const unsigned* taskIndices)
			{
				for (unsigned i = startIndex; i < endIndex; i++)
				{
					auto taskIndex = taskIndices[i];
					auto& task = taskData[taskIndex];
					auto& transformation = sceneNodeHandler->UnsafeGetScaledWorldTransformation(task.SceneNodeIndex);
					auto box = task.BoundingBox.Transform(transformation.AsMatrix4x3());
					auto distance = box.GetBoundaryDistance(cameraPosition);
					sortData[i] = { taskIndex, distance };
				}
			}

			template <typename TaskType>
			inline void LoadFrontToBackKeys(unsigned threadId, unsigned startIndex, unsigned endIndex,
				SortData* sortData, const glm::vec3& cameraPosition,
				const SceneNodeHandler* sceneNodeHandler, const TaskType* taskData, const unsigned* taskIndices)
			{
				for (unsigned i = startIndex; i < endIndex; i++)
				{
					auto taskIndex = taskIndices[i];
					auto& task = taskData[taskIndex];
					auto& transformation = sceneNodeHandler->UnsafeGetScaledWorldTransformation(task.SceneNodeIndex);
					auto box = task.BoundingBox.Transform(transformation.AsMatrix4x3());
					auto distance = box.GetDistance(cameraPosition);
					sortData[i] = { taskIndex, distance };
				}
			}

			template <typename TaskType>
			inline void LoadBackToFrontKeys(unsigned threadId, unsigned startIndex, unsigned endIndex,
				SortData* sortData, const glm::vec3& cameraPosition,
				const SceneNodeHandler* sceneNodeHandler, const TaskType* taskData, const unsigned* taskIndices)
			{
				for (unsigned i = startIndex; i < endIndex; i++)
				{
					auto taskIndex = taskIndices[i];
					auto& task = taskData[taskIndex];
					auto& transformation = sceneNodeHandler->UnsafeGetScaledWorldTransformation(task.SceneNodeIndex);
					auto box = task.BoundingBox.Transform(transformation.AsMatrix4x3());
					auto distance = -box.GetDistance(cameraPosition);
					sortData[i] = { taskIndex, distance };
				}
			}
		};
	}
}

#endif