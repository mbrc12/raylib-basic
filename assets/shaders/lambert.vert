in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragPosition;
out vec4 fragLightSpacePos;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform mat4 lightVP;

void main() {
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(mat3(matNormal) * vertexNormal);
    fragPosition = (matModel * vec4(vertexPosition, 1.0)).xyz;
    fragLightSpacePos = lightVP * matModel * vec4(vertexPosition, 1.0);
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
