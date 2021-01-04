// GLSL/BackgroundCubemap_ps.glsl

uniform samplerCube BackgroundTexture;

in vec3 vs_Direction;

out vec4 ResultColor;

void main()
{
	ResultColor = vec4(texture(BackgroundTexture, vs_Direction).xyz, 1.0f);
}