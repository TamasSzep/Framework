// Lighting.hlsl

#ifndef _LIGHTING_HLSL_
#define _LIGHTING_HLSL_

struct DirectionalLight
{
	float3 Direction;
	float3 DiffuseIntensity;
	float3 SpecularIntensity;
	float SpecularExponent;
};

struct SpotLight
{
	float3 Position;
	float3 Direction;
	float3 DiffuseIntensity;
	float SpotExponent;
	float3 SpecularIntensity;
	float SpecularExponent;
};

static const uint c_MaxCountLights = 16;

struct LightingCBType
{
	DirectionalLight DirectionalLights[c_MaxCountLights];
	SpotLight SpotLights[c_MaxCountLights];
	uint CountDirectionalLights;
	uint CountSpotLights;
	float3 AmbientIntensity;
};

float3 EvaluatePhong(LightingCBType lighting, float3 normal, float3 worldPosition, float3 cameraPosition,
	float3 diffuseColor, float3 specularColor)
{
	uint i;

	float3 reflectedDir = normalize(reflect(worldPosition - cameraPosition, normal));

	// Ambient component.
	float3 color = diffuseColor * lighting.AmbientIntensity;

	float unocclusionFactor = 1.0f;

	// Adding directional lights.
	uint countDirectionalLights = lighting.CountDirectionalLights;
	for (i = 0; i < countDirectionalLights; i++)
	{
		float3 lightDirection = lighting.DirectionalLights[i].Direction;
		float3 diffuseIntensity = lighting.DirectionalLights[i].DiffuseIntensity;
		float3 specularIntensity = lighting.DirectionalLights[i].SpecularIntensity;
		float specularExponent = lighting.DirectionalLights[i].SpecularExponent;

		// Diffuse component.
		color += diffuseColor * max(-dot(lightDirection, normal), 0.0f) * diffuseIntensity * unocclusionFactor;

		// Specular component.
		color += specularColor
			* pow(max(-dot(reflectedDir, lightDirection), 0.0f), specularExponent)
			* specularIntensity * unocclusionFactor;
	}

	// Adding spotlights.
	uint countSpotLights = lighting.CountSpotLights;
	for (i = 0; i < countSpotLights; i++)
	{
		float3 lightPosition = lighting.SpotLights[i].Position;
		float3 lightDirection = lighting.SpotLights[i].Direction;
		float3 diffuseIntensity = lighting.SpotLights[i].DiffuseIntensity;
		float3 specularIntensity = lighting.SpotLights[i].SpecularIntensity;
		float specularExponent = lighting.SpotLights[i].SpecularExponent;
		float spotExponent = lighting.SpotLights[i].SpotExponent;

		float3 toLight = lightPosition - worldPosition;
		float distanceSqr = dot(toLight, toLight);
		toLight = normalize(toLight);
		float spotEffect = pow(max(0.0f, -dot(toLight, lightDirection)), spotExponent) / distanceSqr;

		// Diffuse lighting.
		float diffuseLight = max(dot(normal, toLight), 0.0f);
		color += diffuseColor * diffuseIntensity * diffuseLight * spotEffect * unocclusionFactor;

		// Specular lighting.
		float specularLight = pow(max(0.0f, dot(reflectedDir, toLight)), specularExponent);
		color += specularIntensity * specularColor * specularLight * spotEffect * unocclusionFactor;
	}

	return color;
}

#endif