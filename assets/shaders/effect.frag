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
    vec4 canvas = texture(texture0, fragTexCoord);

    float t = xy.x * 3.0 + xy.y * 9.0;
    t = mod(t + time, 1.0);
    vec4 pattern = (t < 0.5) ? colorA : colorB;

    finalColor = mix(canvas, pattern * colDiffuse * fragColor, 0.3);
}
