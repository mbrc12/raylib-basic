in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightDir;
uniform vec4 ambient;
uniform vec4 diffuse;

uniform vec3 viewPos;
uniform vec4 fogColor;
uniform float fogDensity;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    float NdotL = max(dot(N, L), 0.0);

    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 baseColor = texelColor * colDiffuse;
    vec4 litColor = baseColor * ambient + baseColor * diffuse * NdotL;

    float dist = length(viewPos - fragPosition);
    float fogFactor = 1.0 / exp(dist * fogDensity);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    finalColor = mix(fogColor, litColor, fogFactor);
}
