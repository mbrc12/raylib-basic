in vec3 vertexPosition;

out float fragDepth;

uniform mat4 lightVP;
uniform mat4 matModel;

void main() {
    gl_Position = lightVP * matModel * vec4(vertexPosition, 1.0);
    fragDepth = gl_Position.z / gl_Position.w * 0.5 + 0.5;
}
