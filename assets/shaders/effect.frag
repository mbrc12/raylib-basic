in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;
uniform float time;

out vec4 finalColor;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec4 previous = texelFetch(texture0, pixel, 0);
    vec2 uv = fragTexCoord;
    vec2 centered = (gl_FragCoord.xy - 0.5 * resolution) / resolution.y;
    float wave = sin(centered.x * 18.0 + time * 2.4) * cos(centered.y * 15.0 - time * 1.8);
    vec3 shaderColor = vec3(0.20 + 0.45 * wave, 0.55 + 0.35 * sin(time + uv.x * 6.0), 0.85);
    finalColor = vec4(mix(previous.rgb, shaderColor, 0.72), previous.a) * colDiffuse * fragColor;
}
