// GLSL/TextRender_vs.glsl

// Per vertex.
in vec3 Position;
in vec2 TextureCoordinate;

// Per instance.
in vec2 PositionMultiplier;
in vec2 PositionOffset;

in vec2 PositionInTex;
in vec2 SizeInTex;

// Output.
out vec2 vs_TexCoord;

// Uniform buffer.

layout(shared, binding = 0) uniform UB
{
	vec4 Color;
	float Depth;
};

void main()
{
	gl_Position = vec4(Position.xy * PositionMultiplier + PositionOffset, Depth, 1.0f);
	vs_TexCoord = TextureCoordinate * SizeInTex + PositionInTex;
}