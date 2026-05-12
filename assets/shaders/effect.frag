in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec4 colDiffuse;
uniform vec2 size;
uniform float time;
uniform vec4 colorA;
uniform vec4 colorB;

out vec4 finalColor;

void main() {
    vec2 xy = gl_FragCoord.xy / size;

    float t = xy.x * 3.0 + xy.y * 9.0;
    t = mod(t + time, 1.0);
    vec4 pattern = (t < 0.5) ? colorA : colorB;

    finalColor = pattern * colDiffuse * fragColor;
}
