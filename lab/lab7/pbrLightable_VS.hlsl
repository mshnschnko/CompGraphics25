#include "PBRBuffers.h"

PS_INPUT main(VS_INPUT input) {
  PS_INPUT output;

  output.worldPos = mul(worldMatrix, float4(input.position, 1.0f));
  output.position = mul(viewProjectionMatrix, output.worldPos);
  output.normal = mul(worldMatrix, input.normal);
  output.tangent = mul(worldMatrix, input.tangent);
  output.texUV = input.texUV;
  
  return output;
}
