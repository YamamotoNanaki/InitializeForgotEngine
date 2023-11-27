#include "Particle.hlsli"

SamplerState smp : register(s0);

PSOutput main(GSOutput input) : SV_TARGET
{
    PSOutput o;
    o.target0 = tex.Sample(smp, input.uv) * colorSB[input.instanceID].color;
    float4 col = o.target0;
    float grayScale = col.r * 0.299 + col.g * 0.587 * col.b * 0.114;
    //float extract = smoothstep(0.2, 0.5, grayScale);
    col *= grayScale;
    o.target1 = col;
    o.target2 = float4(0, 0, 0, 0);
    return o;
}