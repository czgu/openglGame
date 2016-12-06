#version 330

// uniform vec3 colour;
uniform sampler2D tex;
uniform sampler2D texShadow;
uniform vec3 ambientIntensity;

struct LightSource {
    vec3 position;
    vec3 rgbIntensity;
};

in VsOutFsIn {
    vec4 texcoord;
	vec3 position_ES; // Eye-space position
	vec3 normal_ES;   // Eye-space normal
	LightSource light;
    vec4 shadowCoord;
} fs_in;

out vec4 fragColor;

//const vec4 fogcolor = vec4(0.2, 0.2, 0.2, 1.0);
const float fogdensity = .0003;
const float bias = 0.0005;

vec3 phongModel(vec3 fragPosition, vec3 fragNormal, vec3 textureColor, float visibility) {
	LightSource light = fs_in.light;

    // Direction from fragment to light source.
    // Since the light is the sun, the direction is same for any pixel
    vec3 l = light.position;

    // Direction from fragment to viewer (origin - fragPosition).
    vec3 v = normalize(-fragPosition.xyz);

    float n_dot_l = max(dot(fragNormal, l), 0.0);

	vec3 diffuse;
	diffuse = textureColor * n_dot_l;

    vec3 specular = vec3(0.0);

    if (n_dot_l > 0.0) {
		// Halfway vector.
		vec3 h = normalize(v + l);
        float n_dot_h = max(dot(fragNormal, h), 0.0);

        specular = textureColor * pow(n_dot_h, 10);
    }

    return visibility * (ambientIntensity * textureColor + light.rgbIntensity * (diffuse + specular));
}

void main() {
    vec4 texcoord = fs_in.texcoord;

    int texNum = int(texcoord.w);
    bool sides = texNum >= 0;
    texNum = abs(texNum);
    int texCol = texNum % 16;
    int texRow = texNum / 16;

    vec4 color;

    if (texNum == 39) {
        if (sides) {
        } else {
            texcoord.z = 0;
            sides = true;
        }
    }

    if (sides) {
        color = texture(tex, vec2((fract(texcoord.x + texcoord.z) + texCol) / 16.0, ((1 - fract(texcoord.y)) + texRow) / 16.0)) * vec4(0.85, 0.85, 0.85, 1.0);
    } else {
        color = texture(tex, vec2((fract(texcoord.y + texcoord.x) + texCol) / 16.0, ((1 - fract(texcoord.z)) + texRow) / 16.0));
    }


    // Check Shadow
    float visibility = 1.0;
    if (fs_in.shadowCoord.x >= 0 && fs_in.shadowCoord.x <= 1 && fs_in.shadowCoord.y >= 0 && fs_in.shadowCoord.y <= 1) {
        if ( texture( texShadow, fs_in.shadowCoord.xy ).r  <  fs_in.shadowCoord.z - bias) {
            visibility = 0.5;
        }
    }

    if(color.a < 0.5)
        discard;
    color = vec4(phongModel(fs_in.position_ES, fs_in.normal_ES, color.xyz, visibility), color.w);
    // Homogenous depth value
    vec4 fogcolor = vec4(fs_in.light.rgbIntensity, 1.0f);
    float z = gl_FragCoord.z / gl_FragCoord.w;
    float fog = clamp(exp(-fogdensity * z * z), 0.0, 1);


    fragColor = mix(fogcolor, color, fog);

    if (fs_in.shadowCoord.x >= 0 && fs_in.shadowCoord.x <= 1 && fs_in.shadowCoord.y >= 0 && fs_in.shadowCoord.y <= 1) {
        if (false) {
            float r = texture(texShadow, fs_in.shadowCoord.xy).r;
            //r = fs_in.shadowCoord.z;
            color = vec4(1,1,1,1);
            if (r < fs_in.shadowCoord.z - bias) {
                color = vec4(0,0,0,1);
            }
            fragColor = color;
        }
    }
}
