// GLSL/TextRender_ps.glsl

in vec2 vs_TexCoord;

out vec4 ResultColor;

// Uniform buffer.
layout(shared, binding = 0) uniform UB
{
	vec4 Color;
	float Depth;
};

uniform sampler2D DistanceTexture;

const float Smoothing = 1.0f / 16.0f;

void main()
{
	float distance = texture(DistanceTexture, vs_TexCoord).x;
	float alpha = smoothstep(0.5f - Smoothing, 0.5f + Smoothing, distance) * Color.a;
	ResultColor = vec4(Color.xyz, alpha);
}