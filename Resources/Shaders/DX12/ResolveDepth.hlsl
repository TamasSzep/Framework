// ResolveDepth.h

// This is a very simple implementation on the depth resolving, which computes the average of the
// projected depth values. However it seems to provide nice results. The next step would be to compute
// the linear depth aggrate them linearly, and compute the projected depth from the aggrated linear depth.
// The weights of the aggregation could be influenced by the colors as well.

Texture2DMS<float> DepthIn : register(t0);
RWTexture2D<float> DepthOut : register(u0);

struct ConstantsType
{
	uint CountSamples;
	float InvCountSamples;
};

ConstantBuffer<ConstantsType> Constants : register(b0);

[numthreads(8, 4, 1)]
void Main( uint3 globalID : SV_DispatchThreadID )
{
	float depth = 0.0f;
	for (uint i = 0; i < Constants.CountSamples; i++)
	{
		depth += DepthIn.Load(int2(globalID.xy), i);
	}
	DepthOut[globalID.xy] = depth * Constants.InvCountSamples;
}