static const float PI = 3.14159265359;

SamplerState pointWrap  : register(s0);
SamplerState pointClamp  : register(s1);
SamplerState linearWrap  : register(s2);
SamplerState linearClamp  : register(s3);
SamplerState anisotropicWrap  : register(s4);
SamplerState anisotropicClamp  : register(s5);
SamplerState depthMapSam  : register(s6);
SamplerState depthMap2Sam : register(s7);