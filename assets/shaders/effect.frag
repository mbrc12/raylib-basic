in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;
uniform float time;
uniform vec4 colorA;
uniform vec4 colorB;

out vec4 finalColor;

void main() {
    vec2 xy = gl_FragCoord.xy / resolution;
    float t = xy.x * 3.0 + xy.y * 9.0;
    t = mod(t + time, 1.0);
    if (t < 0.5) {
        finalColor = colorA * colDiffuse * fragColor;
    } else {
        finalColor = colorB * colDiffuse * fragColor;
    }
}
