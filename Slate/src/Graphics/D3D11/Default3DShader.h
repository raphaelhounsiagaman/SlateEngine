#pragma once

namespace Slate
{

	inline constexpr char Default3DShaderSource[] = R"(
		cbuffer ObjectConstants : register(b0)
		{
			row_major float4x4 World;
			row_major float4x4 View;
			row_major float4x4 Projection;
			float4 Albedo;
		};

		struct VertexInput
		{
			float3 Position : POSITION;
			float3 Normal : NORMAL;
		};

		struct VertexOutput
		{
			float4 Position : SV_POSITION;
			float3 WorldNormal : NORMAL;
		};

		VertexOutput VSMain(VertexInput input)
		{
			VertexOutput output;

			float4 worldPosition = mul(float4(input.Position, 1.0f), World);
			float4 viewPosition = mul(worldPosition, View);

			output.Position = mul(viewPosition, Projection);
			output.WorldNormal = normalize(mul(float4(input.Normal, 0.0f), World).xyz);

			return output;
		}

		float4 PSMain(VertexOutput input) : SV_TARGET
		{
			const float3 lightDirection = normalize(float3(-0.5f, 0.8f, -0.6f));
			const float diffuse = saturate(dot(normalize(input.WorldNormal), lightDirection));
			const float brightness = 0.25f + diffuse * 0.75f;

			return float4(Albedo.rgb * brightness, Albedo.a);
		}
	)";

}
