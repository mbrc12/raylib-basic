in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;
in vec4 fragLightSpacePos;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightDir;
uniform vec4 ambient;
uniform vec4 diffuse;

uniform vec3 viewPos;
uniform vec4 fogColor;
uniform float fogDensity;

uniform mat4 lightVP;
uniform sampler2D shadowMap;

float unpackDepth(vec4 packed) {
    const vec4 bitShift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
    return dot(packed, bitShift);
}

float shadowFactor(vec3 worldPos, vec3 normal, float NdotL) {
    vec3 biasedWorldPos = worldPos + normal * 0.18;
    vec4 lightSpacePos = lightVP * vec4(biasedWorldPos, 1.0);
    vec3 proj = lightSpacePos.xyz / lightSpacePos.w;
    proj = proj * 0.5 + 0.5;

    if (proj.x < 0.0 || proj.x > 1.0 ||
        proj.y < 0.0 || proj.y > 1.0 ||
        proj.z < 0.0 || proj.z > 1.0) {
        return 1.0;
    }

    float slope = sqrt(max(1.0 - NdotL * NdotL, 0.0)) / max(NdotL, 0.1);
    float bias = max(0.003 + 0.01 * slope, 0.0015);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    vec2 shadowUv = proj.xy;
    float visibility = 0.0;

    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            float closest = unpackDepth(texture(shadowMap, shadowUv + vec2(x, y) * texelSize));
            visibility += (proj.z - bias > closest) ? 0.35 : 1.0;
        }
    }

    return visibility / 9.0;
}

void main() {
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    float NdotL = max(dot(N, L), 0.0);

    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 baseColor = texelColor * colDiffuse;
    float shadow = shadowFactor(fragPosition, N, NdotL);

    vec4 litColor = baseColor * ambient + baseColor * diffuse * NdotL * shadow;

    float dist = length(viewPos - fragPosition);
    float fogFactor = 1.0 / exp(dist * fogDensity);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    finalColor = mix(fogColor, litColor, fogFactor);
}
