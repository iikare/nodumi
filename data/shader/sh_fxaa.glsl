#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 u_resolution;

out vec4 finalColor;

#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0

void main() {
  vec2 st = fragTexCoord;
  vec2 i_res = vec2(1.0)/u_resolution;
  vec4 texColor = texture(texture0, st);

  vec2 posOffset = vec2(1.0,1.0)*i_res;
  vec3 sampleTL = texture2D(texture0, st - posOffset).xyz;
  vec3 sampleTR = texture2D(texture0, st + vec2(posOffset.x, -posOffset.y)).xyz;
  vec3 sampleBL = texture2D(texture0, st + vec2(-posOffset.x, posOffset.y)).xyz;
  vec3 sampleBR = texture2D(texture0, st + posOffset).xyz;
  vec3 sampleH = texture2D(texture0, st).xyz;

  vec3 luma = vec3(0.299, 0.587, 0.114);

  float lumaTL = dot(sampleTL, luma);
  float lumaTR = dot(sampleTR, luma);
  float lumaBL = dot(sampleBL, luma);
  float lumaBR = dot(sampleBR, luma);
  float lumaH  = dot(sampleH,  luma);
  float lumaMin = min(lumaH, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
  float lumaMax = max(lumaH, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR))); 
  
  vec2 dir = vec2(-((lumaTL + lumaTR) - (lumaBL + lumaBR)), ((lumaTL + lumaBL) - (lumaTR + lumaBR)));
  //dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
  //dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));

  float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR)*(0.25*FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
  float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
  dir = min(vec2(FXAA_SPAN_MAX),max(vec2(-FXAA_SPAN_MAX), dir*rcpDirMin))*i_res;

  vec3 rgbA = 0.5*(texture2D(texture0, st + dir*(1.0/3.0 - 0.5)).xyz + texture2D(texture0, st + dir*(2.0/3.0 - 0.5)).xyz);
  vec3 rgbB = rgbA*0.5 + 0.25*(texture2D(texture0, st - dir*0.5).xyz + texture2D(texture0, st + dir*0.5).xyz);

  float lumaB = dot(rgbB, luma);

  finalColor = vec4(((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB, 1.0);
}
