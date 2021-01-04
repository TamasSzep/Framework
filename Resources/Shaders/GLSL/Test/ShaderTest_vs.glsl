// GLSL/Test/ShaderTest_vs.glsl

layout(shared, binding = 1) uniform Object
{
	mat4 ModelMatrix;
	mat4 InverseModelMatrix;
	mat4 ModelViewProjectionMatrix;
};

in vec3 Position;
in vec3 Normal;
in vec2 TextureCoordinate;

#if IS_USING_VERTEX_COLOR
in vec4 VertexColor;
#endif

out vec3 vs_Normal;
out vec2 vs_TextureCoordinate;
out vec3 vs_WorldPosition;

#if IS_USING_VERTEX_COLOR
out vec4 vs_VertexColor;
#endif

void main()
{
	vec4 position4 = vec4(Position, 1.0f);
	gl_Position = ModelViewProjectionMatrix * position4;
	vs_Normal = normalize((vec4(Normal, 0.0f) * InverseModelMatrix).xyz);
	vs_TextureCoordinate = TextureCoordinate;
	vs_WorldPosition = (ModelMatrix * position4).xyz;

#if IS_USING_VERTEX_COLOR
	vs_VertexColor = VertexColor;
#endif
}