// EngineBuildingBlocks/Animation/AnimationHelper.h

#ifndef _ENGINEBUILDINGBLOCKS_ANIMATIONHELPER_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_ANIMATIONHELPER_H_INCLUDED_

namespace EngineBuildingBlocks
{
	namespace Animation
	{
		template <typename F1, typename F2, typename T1, typename T2>
		inline bool AreKeyframesSynchronized(const T1* keyframes1, unsigned size1,
			const T2* keyframes2, unsigned size2)
		{
			if (size1 != size2) return false;
			for (unsigned k = 0; k < size1; ++k)
			{
				if (F1::GetTime(keyframes1[k]) != F2::GetTime(keyframes2[k])) return false;
			}
			return true;
		}

		template <typename F, typename T>
		inline unsigned GetTimeIndex(const T* keyframes, unsigned size, double time)
		{
			assert(size > 0);
			unsigned a = 0;
			unsigned b = size - 1;
			if (time < F::GetTime(keyframes[0])) return Core::c_InvalidIndexU;
			if (time > F::GetTime(keyframes[b])) return Core::c_InvalidIndexU;
			do
			{
				unsigned c = (a + b) >> 1;
				if (F::GetTime(keyframes[c]) <= time) a = c;
				else b = c;
			} while (b - a > 1);
			assert((b == a + 1 || size == 1)
				&& F::GetTime(keyframes[a]) <= time && F::GetTime(keyframes[b]) >= time);
			if (time == F::GetTime(keyframes[a])) return a;
			if (time == F::GetTime(keyframes[b])) return b;
			return Core::c_InvalidIndexU;
		}

		template <typename F, typename T>
		inline void GetInterpolationDataForKeyframes(const T* keyframes, unsigned size, double time,
			unsigned& index0, unsigned& index1, float& weight)
		{
			assert(size > 0);
			unsigned a = 0;
			unsigned b = size - 1;
			if (time <= F::GetTime(keyframes[0])) { index0 = 0; index1 = 0; weight = 1.0f; return; }
			if (time >= F::GetTime(keyframes[b])) { index0 = b; index1 = b; weight = 1.0f; return; }
			do
			{
				unsigned c = (a + b) >> 1;
				if (F::GetTime(keyframes[c]) <= time) a = c;
				else b = c;
			} while (b - a > 1);
			assert(b == a + 1);
			index0 = a;
			index1 = b;
			auto t0 = F::GetTime(keyframes[a]);
			auto t1 = F::GetTime(keyframes[b]);
			assert(time >= t0 && time <= t1 && t1 != t0);
			weight = static_cast<float>((time - t0) / (t1 - t0));
		}

		struct AssimpTimeGettingFunction
		{
			public: template <typename T> inline static double GetTime(const T& t) { return t.mTime; }
		};

		struct EngineBuildingBlocksTimeGettingFunction
		{
			public: template <typename T> inline static double GetTime(const T& t) { return t.Time; }
		};

		using ATGF = AssimpTimeGettingFunction;
		using ETGF = EngineBuildingBlocksTimeGettingFunction;
	}
}

#endif