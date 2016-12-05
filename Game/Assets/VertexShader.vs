#version 330

uniform mat4 P;
uniform mat4 VM;
uniform mat3 NormalMatrix;
uniform mat4 depthBiasMVP;

in vec4 position;
in vec3 normal;

// The light source right now is the Sun
struct LightSource {
    vec3 position;
    vec3 rgbIntensity;
};
uniform LightSource light;

out VsOutFsIn {
    vec4 texcoord;
	vec3 position_ES; // Eye-space position
	vec3 normal_ES;   // Eye-space normal
	LightSource light;
    vec4 shadowCoord;
} vs_out;

void main() {
    vs_out.texcoord = position;
    vs_out.normal_ES = normalize(NormalMatrix * normal);
    vs_out.light = light;

    vec4 shadowCoord = depthBiasMVP * vec4(position.xyz, 1.0);
    vs_out.shadowCoord = vec4(shadowCoord.xyz / shadowCoord.w, 1.0);

    vec4 pos4 = VM * vec4(position.xyz, 1.0);
    vs_out.position_ES = pos4.xyz;
	gl_Position = P * pos4;
}
