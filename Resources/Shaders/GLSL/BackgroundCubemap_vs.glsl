// GLSL/BackgroundCubemap_vs.glsl

in vec3 Position;

out vec3 vs_Direction;

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

const float LastZ = 0.99999f;

void main()
{
	gl_Position = vec4(Position.xy, LastZ, 1.0f);
	vec4 worldPosition4 = ViewProjectionMatrixInverse * vec4(Position, 1.0f);
	vs_Direction = worldPosition4.xyz / worldPosition4.w - CameraPosition;
}