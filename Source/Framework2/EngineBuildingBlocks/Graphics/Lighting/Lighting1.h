// EngineBuildingBlocks/Graphics/Lighting/Lighting1.h

#ifndef _ENGINEBUILDINGBLOCKS_LIGHTING1_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_LIGHTING1_H_INCLUDED_

#include <EngineBuildingBlocks/Math/GLM.h>

#include <cassert>

// As it's name suggests this code is not intended to provide a general solution for lighting,
// it just implements a bunch of lighting structures that can be directly used for constant (uniform) buffers.
// We always pad our data to 16 byte, this should be compatible with all of the usual alignment specifications in graphics APIs.

namespace EngineBuildingBlocks
{
	namespace Graphics
	{
		struct DirectionalLightCBData1
		{
			glm::vec3 Direction;
			float _Padding0;
			glm::vec3 DiffuseIntensity;
			float _Padding1;
			glm::vec3 SpecularIntensity;
			float SpecularExponent;
		};

		struct SpotLightCBData1
		{
			glm::vec3 Position;
			float _Padding0;
			glm::vec3 Direction;
			float _Padding1;
			glm::vec3 DiffuseIntensity;
			float SpotExponent;
			glm::vec3 SpecularIntensity;
			float SpecularExponent;
		};

		template <unsigned MaxCountLights>
		struct Lighting_Dir_Spot_CBType1
		{
			DirectionalLightCBData1 DirectionalLights[MaxCountLights];
			SpotLightCBData1 SpotLights[MaxCountLights];
			unsigned CountDirectionalLights;
			unsigned CountSpotLights;
			glm::vec2 _Padding0;
			glm::vec3 AmbientIntensity;
			float _Padding1;
		};

		template <unsigned MaxCountLights>
		void CreateDefaultLighting1(Lighting_Dir_Spot_CBType1<MaxCountLights>& lighting)
		{
			assert(MaxCountLights >= 1);

			lighting.AmbientIntensity = glm::vec3(0.3f);
			lighting.CountDirectionalLights = 1;
			lighting.DirectionalLights[0].Direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
			lighting.DirectionalLights[0].DiffuseIntensity = glm::vec3(1.0f);
			lighting.DirectionalLights[0].SpecularIntensity = glm::vec3(0.5f);
			lighting.DirectionalLights[0].SpecularExponent = 10.0f;
			lighting.CountSpotLights = 0;
		}
	}
}

#endif