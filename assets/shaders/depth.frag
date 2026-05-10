in float fragDepth;

out vec4 finalColor;

vec4 packDepth(float depth) {
    const vec4 bitShift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    const vec4 bitMask = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    vec4 packed = fract(depth * bitShift);
    packed -= packed.xxyz * bitMask;
    return packed;
}

void main() {
    float depth = clamp(fragDepth, 0.0, 1.0);
    finalColor = packDepth(depth);
}
