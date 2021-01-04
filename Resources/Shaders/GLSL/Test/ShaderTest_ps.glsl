// GLSL/Test/ShaderTest_ps.glsl

in vec3 vs_Normal;
in vec2 vs_TextureCoordinate;
in vec3 vs_WorldPosition;

#if IS_USING_VERTEX_COLOR
in vec4 vs_VertexColor;
#endif

out vec4 ResultColor;

layout(shared, binding = 0) uniform RenderPass
{
	vec3 CameraPosition;
	vec3 CameraDirection;
	vec3 CameraUp;
	vec3 CameraRight;
	mat4 ViewMatrix;
	mat4 ViewMatrixInvserse;
	mat4 ProjectionMatrix;
	mat4 ProjectionMatrixInverse;
	mat4 ViewProjectionMatrix;
	mat4 ViewProjectionMatrixInverse;
};

layout(shared, binding = 2) uniform Material
{
	vec3 DiffuseColor;
	vec3 SpecularColor;
};

const vec3 c_SpecularColor = vec3(1.0f, 1.0f, 1.0f);

uniform sampler2D DiffuseTexture;

struct DirectionalLight
{
	vec3 Direction;
	vec3 DiffuseIntensity;
	vec3 SpecularIntensity;
	float SpecularExponent;
};

struct SpotLight
{
	vec3 Position;
	vec3 Direction;
	vec3 DiffuseIntensity;
	float SpotExponent;
	vec3 SpecularIntensity;
	float SpecularExponent;
};

const uint c_MaxCountLights = 16;

layout(shared, binding = 3) uniform Lighting
{
	DirectionalLight DirectionalLights[c_MaxCountLights];
	SpotLight SpotLights[c_MaxCountLights];
	uint CountDirectionalLights;
	uint CountSpotLights;
	vec3 AmbientIntensity;
};

vec3 EvaluatePhong(vec3 normal, vec3 worldPosition, vec3 cameraPosition, vec3 diffuseColor, vec3 specularColor)
{
	uint i;

	vec3 reflectedDir = normalize(reflect(worldPosition - cameraPosition, normal));

	// Ambient component.
	vec3 color = diffuseColor * AmbientIntensity;

	float unocclusionFactor = 1.0f;

	// Adding directional lights.
	uint countDirectionalLights = CountDirectionalLights;
	for (i = 0; i < countDirectionalLights; i++)
	{
		vec3 lightDirection = DirectionalLights[i].Direction;
		vec3 diffuseIntensity = DirectionalLights[i].DiffuseIntensity;
		vec3 specularIntensity = DirectionalLights[i].SpecularIntensity;
		float specularExponent = DirectionalLights[i].SpecularExponent;

		// Diffuse component.
		color += diffuseColor * max(-dot(lightDirection, normal), 0.0f) * diffuseIntensity * unocclusionFactor;

		// Specular component.
		color += specularColor
			* pow(max(-dot(reflectedDir, lightDirection), 0.0f), specularExponent)
			* specularIntensity * unocclusionFactor;
	}

	// Adding spotlights.
	uint countSpotLights = CountSpotLights;
	for (i = 0; i < countSpotLights; i++)
	{
		vec3 lightPosition = SpotLights[i].Position;
		vec3 lightDirection = SpotLights[i].Direction;
		vec3 diffuseIntensity = SpotLights[i].DiffuseIntensity;
		vec3 specularIntensity = SpotLights[i].SpecularIntensity;
		float specularExponent = SpotLights[i].SpecularExponent;
		float spotExponent = SpotLights[i].SpotExponent;

		vec3 toLight = lightPosition - worldPosition;
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

void main()
{
	vec3 normal = normalize(vs_Normal);

	vec4 diffuseColor4 = texture(DiffuseTexture, vs_TextureCoordinate);
	vec3 diffuseColor = diffuseColor4.xyz;

	vec3 specularColor;
#if (IS_USING_MATERIAL_COLORS == 1)
	diffuseColor *= DiffuseColor;
	specularColor = SpecularColor;
#else
	specularColor = c_SpecularColor;
#endif

#if (IS_USING_VERTEX_COLOR == 1)
	diffuseColor *= input.VertexColor;
	specularColor *= input.VertexColor;
#endif

	float opacity = diffuseColor4.w;

	vec3 color = EvaluatePhong(normal, vs_WorldPosition, CameraPosition, diffuseColor, specularColor);

	ResultColor = vec4(color, opacity);
}