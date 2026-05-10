---
title: Shadow Mapping in raylt
header-includes:
  - |
    <style>
    body {
        margin: 0;
        padding: 24px 28px 40px;
        max-width: 920px;
        line-height: 1.55;
    }

    pre, code {
        font-size: 0.95em;
    }

    img, table {
        max-width: 100%;
    }
    </style>
---

# Shadow Mapping in `raylt`

This repo uses classic shadow mapping for a directional light. The implementation is intentionally simple, but the math underneath is the same math used by larger engines:

1. Render the scene from the light's point of view into a depth texture.
2. Render the scene from the player camera.
3. For each shaded fragment, transform its world position into light space.
4. Compare the fragment's light-space depth against the stored depth from pass 1.

The relevant code currently lives in:

- [src/main.cpp](/Users/subwave/dev/game/raylt/src/main.cpp)
- [src/engine/shadow.hpp](/Users/subwave/dev/game/raylt/src/engine/shadow.hpp)
- [src/engine/shadow.cpp](/Users/subwave/dev/game/raylt/src/engine/shadow.cpp)
- [src/engine/model_shader_scope.hpp](/Users/subwave/dev/game/raylt/src/engine/model_shader_scope.hpp)
- [src/engine/model_shader_scope.cpp](/Users/subwave/dev/game/raylt/src/engine/model_shader_scope.cpp)
- [src/engine/shader.cpp](/Users/subwave/dev/game/raylt/src/engine/shader.cpp)
- [assets/shaders/depth.vert](/Users/subwave/dev/game/raylt/assets/shaders/depth.vert)
- [assets/shaders/depth.frag](/Users/subwave/dev/game/raylt/assets/shaders/depth.frag)
- [assets/shaders/lambert.vert](/Users/subwave/dev/game/raylt/assets/shaders/lambert.vert)
- [assets/shaders/lambert.frag](/Users/subwave/dev/game/raylt/assets/shaders/lambert.frag)

The current pipeline is:

```text
world geometry -> light camera -> shadow map
world geometry -> player camera -> lighting shader -> compare against shadow map
```

## 1. The Core Test

At the heart of shadow mapping is a depth comparison.

For a world-space point $\mathbf{p}_{\mathrm{world}}$, we compute its coordinates in the light's clip space:

$$
\mathbf{p}_{\mathrm{clip}}^{L} = M_{\mathrm{lightVP}}\,\mathbf{p}_{\mathrm{world}}
$$

Then we divide by $w$ to get normalized device coordinates:

$$
\mathbf{p}_{\mathrm{ndc}}^{L} = \frac{\mathbf{p}_{\mathrm{clip}}^{L}}{w_{\mathrm{clip}}^{L}}
$$

Then we remap from $[-1,1]$ into $[0,1]$:

$$
\mathbf{p}_{\mathrm{uvz}}^{L} = \frac{\mathbf{p}_{\mathrm{ndc}}^{L}}{2} + \frac{1}{2}
$$

The `x` and `y` components become texture coordinates into the shadow map. The `z` component is the depth of the current fragment from the light.

Let:

- $z_{\mathrm{current}}$ be the current fragment depth in light space
- $z_{\mathrm{stored}}$ be the depth stored in the shadow map
- $\beta$ be the depth bias

Then the visibility test is:

$$
z_{\mathrm{current}} - \beta > z_{\mathrm{stored}}
$$

If that inequality is true, the fragment is behind something the light already saw, so it is shadowed.

## 2. Coordinate Spaces

The implementation moves geometry through several spaces:

1. object space
2. world space
3. light view space
4. light clip space
5. light NDC
6. shadow-map UV space

For a vertex position $\mathbf{p}_{\mathrm{obj}}$:

$$
\mathbf{p}_{\mathrm{world}} = M_{\mathrm{model}}\,\mathbf{p}_{\mathrm{obj}}
$$

$$
\mathbf{p}_{\mathrm{view}}^{L} = M_{\mathrm{view}}^{L}\,\mathbf{p}_{\mathrm{world}}
$$

$$
\mathbf{p}_{\mathrm{clip}}^{L} = M_{\mathrm{proj}}^{L}\,\mathbf{p}_{\mathrm{view}}^{L}
$$

So the composition is:

$$
\mathbf{p}_{\mathrm{clip}}^{L} = M_{\mathrm{proj}}^{L} M_{\mathrm{view}}^{L} M_{\mathrm{model}} \mathbf{p}_{\mathrm{obj}}
$$

The repo stores the light view-projection part in `lightVP`, and keeps the per-object model transform separate as `matModel`.

## 2.1 Model, View, and Projection

The acronym `MVP` is shorthand for:

$$
M_{\mathrm{MVP}} = M_{\mathrm{proj}} M_{\mathrm{view}} M_{\mathrm{model}}
$$

Each matrix has a different job:

- `M_{\mathrm{model}}` places a mesh instance into world space.
- `M_{\mathrm{view}}` expresses that world from the camera's frame of reference.
- `M_{\mathrm{proj}}` maps the camera frustum into clip space.

In the lit pass, raylib already supplies a per-draw `mvp` uniform that contains the player's projection, view, and the object's model transform. The shader uses:

$$
\mathbf{p}_{\mathrm{clip}}^{\mathrm{camera}} = M_{\mathrm{mvp}}\,\mathbf{p}_{\mathrm{obj}}
$$

and writes:

```glsl
gl_Position = mvp * vec4(vertexPosition, 1.0);
```

In the shadow pass, the game does not want the player's camera transform. It wants the light's transform. That is why the shadow code keeps the transforms split into:

$$
M_{\mathrm{lightVP}} = M_{\mathrm{proj}}^{L} M_{\mathrm{view}}^{L}
$$

and then multiplies by `matModel` separately per object:

$$
\mathbf{p}_{\mathrm{clip}}^{L} = M_{\mathrm{lightVP}} M_{\mathrm{model}} \mathbf{p}_{\mathrm{obj}}
$$

This split is important for two reasons:

1. `lightVP` is shared by every object in the shadow pass.
2. `matModel` changes per object instance, so it must stay separate.

If `matModel` were baked into `lightVP`, the matrix would no longer be reusable for other objects, and the shadow pass would stop being a clean "same light, different object" transform.

### Why this matters

Shadow mapping only works if the depth pass and the lighting pass use the same transform. If pass 1 stores depth using one light matrix and pass 2 compares using another, the numbers are not comparable and the result is either:

- everything shadowed
- nothing shadowed
- giant misaligned shadows

## 3. The Directional Light Camera

The shadow camera is set up in [src/engine/shadow.cpp]:

```cpp
void DirectionalShadow::init(int size, const Vector3& sunDir, const Vector3& target, float distance, float orthoSize) {
    map = LoadRenderTexture(size, size);
    SetTextureFilter(map.texture, TEXTURE_FILTER_POINT);
    SetTextureWrap(map.texture, TEXTURE_WRAP_CLAMP);

    Vector3 lightDir = Vector3Normalize(sunDir);
    Vector3 lightPos = Vector3Add(target, Vector3Scale(lightDir, distance));

    Matrix lightProj = MatrixOrtho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 200.0f);
    Matrix lightView = MatrixLookAt(lightPos, target, Vector3{0.0f, 1.0f, 0.0f});
    lightVP = MatrixMultiply(lightView, lightProj);

    camera.position = lightPos;
    camera.target = target;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = orthoSize * 2.0f;
    camera.projection = CAMERA_ORTHOGRAPHIC;
}
```

The game is using a directional light, not a point light. That means the light rays are parallel, so the correct projection is orthographic.

### Orthographic projection

In an orthographic projection, positions are mapped linearly into clip space. There is no perspective divide based on distance from the light. This is what you want for a sun:

$$
M_{\mathrm{proj}}^{L} =
\begin{bmatrix}
\frac{2}{r-l} & 0 & 0 & -\frac{r+l}{r-l} \\
0 & \frac{2}{t-b} & 0 & -\frac{t+b}{t-b} \\
0 & 0 & -\frac{2}{f-n} & -\frac{f+n}{f-n} \\
0 & 0 & 0 & 1
\end{bmatrix}
$$

where:

- $l, r$ are the left and right bounds
- $b, t$ are the bottom and top bounds
- $n, f$ are the near and far planes

This matrix defines the box of world space the shadow map can see.

### Why the box size matters

The shadow map has finite resolution. If the orthographic box is too large, each texel covers too much world space and the shadow gets blocky. If the box is too small, objects clip out of the shadow camera and disappear from the shadow map.

The current setup uses:

- `shadowMapSize = 512`
- `orthoSize = terrainWorldSize * 0.7f`

So the effective world-space shadow texel size is roughly:

$$
\Delta_{\mathrm{world}} \approx \frac{2 \cdot orthoSize}{shadowMapSize}
$$

That quantity is a useful mental model. It tells you how much world-space area one shadow texel represents.

## 4. Pass 1: Writing the Shadow Map

The depth pass is orchestrated from [src/main.cpp]:

```cpp
void drawDepthPass() {
    shadow.beginDepthPass();

    [[maybe_unused]] engine::ScopedModelShader terrainDepth(terrainModel, depth->raw());
    [[maybe_unused]] engine::ScopedModelShader fishDepth(*fish, depth->raw());
    depth->send("lightVP", shadow.lightVP);

    DrawModel(terrainModel, ORIGIN.v(), 1.0f, WHITE);

    Vector3 fishPos{
        terrainWorldSize / 2.0f,
        terrainMaxHeight + 4.0f,
        terrainWorldSize / 2.0f + 3.0f};
    DrawModelEx(*fish, fishPos, UP.v(), total_time * 80, fishScale, WHITE);

    shadow.endDepthPass();
}
```

The helper [src/engine/model_shader_scope.cpp] temporarily overrides every material shader in a model:

```cpp
ScopedModelShader::ScopedModelShader(Model& model, const ::Shader& shader) : m_model(&model) {
    m_oldShaders.resize(model.materialCount);
    for (int i = 0; i < model.materialCount; i++) {
        m_oldShaders[i] = model.materials[i].shader;
        model.materials[i].shader = shader;
    }
}
```

This keeps the depth pass isolated from the normal lighting shader without having to permanently mutate the model materials.

### Depth pass state

The depth pass turns on three important GL-state changes in [src/engine/shadow.cpp]:

```cpp
BeginTextureMode(map);
ClearBackground(WHITE);
rlDisableColorBlend();
rlEnableBackfaceCulling();
rlSetCullFace(RL_CULL_FACE_FRONT);
BeginMode3D(camera);
```

Why each one matters:

- `BeginTextureMode(map)` renders into the shadow texture instead of the screen.
- `ClearBackground(WHITE)` initializes the depth texture to far depth.
- `rlDisableColorBlend()` prevents the packed RGBA depth from being blended with the clear color.
- `rlSetCullFace(RL_CULL_FACE_FRONT)` reduces self-shadowing on closed meshes by storing the back side of the surface shell instead of the exact front face.

### Shadow camera view transform

The light view matrix transforms a world point into the light's camera coordinates:

$$
\mathbf{p}_{\mathrm{view}}^{L} = M_{\mathrm{view}}^{L}\,\mathbf{p}_{\mathrm{world}}
$$

Geometrically, this is just a coordinate frame change. The light camera has:

- an origin at `lightPos`
- a forward axis toward `target`
- an up vector of `(0,1,0)`

This is the same idea as a player camera; the only difference is that the camera belongs to the light.

## 5. Pass 1: What Gets Stored

The depth vertex shader is [assets/shaders/depth.vert]:

```glsl
in vec3 vertexPosition;

out float fragDepth;

uniform mat4 lightVP;
uniform mat4 matModel;

void main() {
    gl_Position = lightVP * matModel * vec4(vertexPosition, 1.0);
    fragDepth = gl_Position.z / gl_Position.w * 0.5 + 0.5;
}
```

The fragment shader is [assets/shaders/depth.frag]:

```glsl
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
```

### Why `fragDepth` is normalized

The clip-space `z` value is first divided by `w`:

$$
z_{\mathrm{ndc}} = \frac{z_{\mathrm{clip}}}{w_{\mathrm{clip}}}
$$

Then remapped into texture depth space:

$$
z_{\mathrm{depth}} = \frac{z_{\mathrm{ndc}}}{2} + \frac{1}{2}
$$

This is necessary because the stored value must live in the same normalized range as the shadow lookup in the lighting shader.

### Why pack depth into RGBA

The shadow texture is a color render texture, not a hardware depth texture. A single 8-bit channel is usually too coarse. Packing into four channels gives much better precision.

The packing is base-256 positional encoding. If we think of the normalized depth as a fixed-point value:

$$
d \in [0,1]
$$

then packing converts it into four byte-sized digits. In abstract terms:

$$
d \mapsto (d_0, d_1, d_2, d_3)
$$

where each $d_i$ occupies one 8-bit channel.

The unpacking is the reverse linear combination:

$$
d = \sum_{i=0}^{3} d_i\,b_i
$$

with weights:

$$
b = \left(\frac{1}{256^3}, \frac{1}{256^2}, \frac{1}{256}, 1\right)
$$

In the lighting shader:

```glsl
float unpackDepth(vec4 packed) {
    const vec4 bitShift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
    return dot(packed, bitShift);
}
```

## 6. Pass 2: Lighting and Shadow Lookup

The lit vertex shader is [assets/shaders/lambert.vert]:

```glsl
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
```

The important outputs are:

- `fragPosition`, the world-space position
- `fragNormal`, the world-space normal
- `fragLightSpacePos`, which is currently redundant but still emitted

The fragment shader does the actual comparison.

### World-space receiver bias

The current lighting shader offsets the receiver position along its normal before projecting into light space:

```glsl
vec3 biasedWorldPos = worldPos + normal * 0.18;
vec4 lightSpacePos = lightVP * vec4(biasedWorldPos, 1.0);
```

This is a normal-offset bias. It shifts the receiver slightly away from the surface so the comparison is not made exactly on the geometric boundary.

Geometrically, this means the test point is:

$$
\mathbf{p}_{\mathrm{biased}} = \mathbf{p}_{\mathrm{world}} + \alpha \mathbf{n}
$$

where $\alpha = 0.18$ in the current shader.

That is different from depth bias. Depth bias changes the comparison threshold. Normal bias changes the point being projected.

### NdotL and slope-scaled bias

The shader computes:

$$
N \cdot L = \max(\hat{\mathbf{n}} \cdot \hat{\mathbf{l}}, 0)
$$

where:

- $\hat{\mathbf{n}}$ is the normalized normal
- $\hat{\mathbf{l}}$ is the normalized light direction

Then it derives a slope term:

$$
s = \frac{\sqrt{\max(1 - (N \cdot L)^2, 0)}}{\max(N \cdot L, 0.1)}
$$

This quantity grows rapidly as the surface becomes more parallel to the light direction. That is exactly where shadow acne is worst.

The current bias is:

$$
\beta = \max(0.003 + 0.01\,s, 0.0015)
$$

So the final comparison becomes:

$$
z_{\mathrm{current}} - \beta > z_{\mathrm{stored}}
$$

This is the core acne suppression mechanism in the current build.

## 7. Percentage-Closer Filtering

Raw shadow comparisons produce hard aliasing. The repo uses a 3x3 PCF kernel:

```glsl
vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
float visibility = 0.0;

for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
        float closest = unpackDepth(texture(shadowMap, shadowUv + vec2(x, y) * texelSize));
        visibility += (proj.z - bias > closest) ? 0.35 : 1.0;
    }
}

return visibility / 9.0;
```

Mathematically, PCF is a local average of binary visibility tests:

$$
V(\mathbf{u}) = \frac{1}{9} \sum_{i=-1}^{1} \sum_{j=-1}^{1} v(\mathbf{u} + (i,j)\Delta)
$$

where:

$$
v(\mathbf{u}) =
\begin{cases}
1.0 & \mathrm{if\ lit} \\
0.35 & \mathrm{if\ shadowed}
\end{cases}
$$

The value `0.35` is not physically special. It is an artistic choice that keeps fully shadowed areas from going completely black.

## 8. Why the Shadow Test Must Match the Depth Pass

The depth pass and the main pass have to agree on all of these:

1. the light transform
2. the light projection bounds
3. the depth normalization
4. the storage format
5. the comparison bias

If any one of those differs, then the inequality is comparing values from different coordinate systems.

The failure mode looks like this:

$$
\mathbf{p}_{\mathrm{current}}^{L,pass2} \neq \mathbf{p}_{\mathrm{stored}}^{L,pass1}
$$

Even a small mismatch can make the whole scene appear shadowed or unshadowed.

## 9. Raylib Material Binding

The shadow map is bound through raylib's material texture machinery.

In [src/engine/shader.cpp]:

```cpp
void Shader::bindLocation(int locIndex, const char* name) {
    m_shader.locs[locIndex] = getLocation(name);
}
```

In [src/main.cpp]:

```cpp
lambert->bindLocation(SHADER_LOC_MAP_METALNESS, "shadowMap");
terrainModel.materials[0].maps[MATERIAL_MAP_METALNESS].texture = shadow.map.texture;
fish->materials[i].maps[MATERIAL_MAP_METALNESS].texture = shadow.map.texture;
```

This is a raylib-specific bridge. The engine tells raylib, "when you bind the metalness slot, route that to the custom `shadowMap` sampler in the shader."

The important consequence is that the shadow texture is bound through model/material drawing, not via a separate manual sampler path.

## 10. Why the Scene Uses a Small Main Render Texture

The main scene is rendered to a lower-resolution canvas:

```cpp
constexpr int gameRenderWidth = 480;
constexpr int gameRenderHeight = 270;
```

The final image is then scaled up to the window.

This does not change the shadow math, but it does affect perception. Lower internal resolution makes shadow edges look more stylized and less expensive to compute, which fits the intended pixelated style.

## 11. What Each Piece Does

### `src/engine/shadow.*`

Owns the shadow render target and the light camera state.

- creates the shadow texture
- sets filtering and wrapping
- computes `lightVP`
- manages render-state changes for the shadow pass

### `src/engine/model_shader_scope.*`

Temporarily replaces a model's material shaders with the depth shader, then restores them automatically.

This is a local cleanup layer over a raylib limitation: model materials are stateful, but the depth pass wants a different shader than the lit pass.

### `assets/shaders/depth.*`

Writes normalized light-space depth into the shadow texture.

### `assets/shaders/lambert.*`

Performs lighting, shadow lookup, PCF, and biasing.

### `src/main.cpp`

Orchestrates:

- terrain generation
- camera control
- depth pass
- main pass
- presentation to the screen

## 12. The Full Math in One Line

If we compress the whole shadow test into one expression, it is this:

$$
\mathrm{shadowed}(\mathbf{p}) =
\left[
\frac{(M_{\mathrm{lightVP}}\,[\mathbf{p} + \alpha \mathbf{n}], 1)_z}
{(M_{\mathrm{lightVP}}\,[\mathbf{p} + \alpha \mathbf{n}], 1)_w}
\cdot \frac{1}{2} + \frac{1}{2}
- \beta
>
\mathrm{shadowMap}\left(
\frac{(M_{\mathrm{lightVP}}\,[\mathbf{p} + \alpha \mathbf{n}], 1)_{xy}}
{(M_{\mathrm{lightVP}}\,[\mathbf{p} + \alpha \mathbf{n}], 1)_w}
\cdot \frac{1}{2} + \frac{1}{2}
\right)
\right]
$$

where:

- $\alpha$ is the normal offset
- $\beta$ is the slope-scaled bias
- the shadow map lookup is filtered with a 3x3 PCF kernel

That is the entire algorithm.

## 13. Practical Failure Modes

When shadow mapping breaks, the failure usually falls into one of these buckets:

### A. Light-space mismatch

The depth pass and the lighting pass are not using the same `lightVP`.

### B. Bad depth storage

The shadow map is too low precision or gets blended/corrupted.

### C. Bad projection bounds

The orthographic box is too large or too small.

### D. Bias too low

You get acne and stripes.

### E. Bias too high

You get detached shadows.

### F. Sampling outside the map

UVs fall outside `[0,1]`, so the fragment samples cleared texels.

### G. Too little filtering

Edges alias and crawl.

This repo currently mitigates those with:

- packed RGBA depth
- front-face culling in the depth pass
- normal offset bias
- slope-scaled comparison bias
- 3x3 PCF
- a relatively small 512x512 shadow map for the pixelated style

## 14. Current Shadow Parameters

From [src/main.cpp]:

```cpp
constexpr int shadowMapSize = 512;
constexpr Vector3 sunDirection{-0.4f, 1.0f, 0.6f};
```

From [src/engine/shadow.cpp]:

```cpp
Matrix lightProj = MatrixOrtho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 200.0f);
```

From [assets/shaders/lambert.frag]:

```glsl
vec3 biasedWorldPos = worldPos + normal * 0.18;
float bias = max(0.003 + 0.01 * slope, 0.0015);
```

These are the numbers that currently define the visual character of the shadows.

## 15. One Small Note

`fragLightSpacePos` is still written in [assets/shaders/lambert.vert], but the current fragment shader recomputes the biased light-space position from `fragPosition`, `fragNormal`, and `lightVP`. That is deliberate in the current implementation because the receiver bias is applied in world space before projection.

That means the vertex output is slightly more general than the current fragment path needs.

## 16. Bottom Line

Shadow mapping here is a comparison between two projections of the same world point:

- one projection from the light
- one projection from the camera

If those projections match, the comparison is meaningful:

$$
z_{\mathrm{current}} - \beta > z_{\mathrm{stored}}
$$

If they do not match, the shadow is wrong.

The rest of the code is just machinery for making that comparison stable enough to survive real geometry, finite precision, and a deliberately low-resolution presentation style.
