uniform sampler2D edgeTex;
uniform sampler2D areaTex;
uniform sampler2D searchTex;
varying vec4 offset1;
varying vec4 offset2;
varying vec4 offset3;
#define SMAA_PIXEL_SIZE vec2(1.0/1920.0, 1.0/1080.0)
#define SMAA_AREATEX_PIXEL_SIZE (1.0 / vec2(160.0, 560.0))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20

#define SMAA_AREATEX_MAX_DISTANCE 32
#define SMAA_CORNER_ROUNDING 25
#define SMAA_MAX_SEARCH_STEPS_DIAG 16

#define SMAAMad(a, b, c) (a * b + c)
#define SMAASaturate(a) clamp(a, 0.0, 1.0)

//-----------------------------------------------------------------------------
// Diagonal Search Functions

#if SMAA_MAX_SEARCH_STEPS_DIAG > 0 || SMAA_FORCE_DIAGONAL_DETECTION == 1

/**
 * These functions allows to perform diagonal pattern searches.
 */
float SMAASearchDiag1(sampler2D edgeTex, vec2 texcoord, vec2 dir, float c)
{
    texcoord += dir * SMAA_PIXEL_SIZE;
    vec2 e = vec2(0.0, 0.0);
    
    float i;
    for (i = 0.0; i < float(SMAA_MAX_SEARCH_STEPS_DIAG); i++)
    {
        e.rg = texture2DLod(edgeTex, texcoord, 0.0).rg;
        if (dot(e, vec2(1.0, 1.0)) < 1.9) break;
        texcoord += dir * SMAA_PIXEL_SIZE;
    }
    return i + float(e.g > 0.9) * c;
}

float SMAASearchDiag2(sampler2D edgeTex, vec2 texcoord, vec2 dir, float c)
{
    texcoord += dir * SMAA_PIXEL_SIZE;
    vec2 e = vec2(0.0, 0.0);
    
    float i;
    for (i = 0.0; i < float(SMAA_MAX_SEARCH_STEPS_DIAG); i++)
    {
        e.g = texture2DLod(edgeTex, texcoord, 0.0).g;
        e.r = texture2DLodOffset(edgeTex, texcoord, 0.0, ivec2(1, 0)).r;
        if (dot(e, vec2(1.0, 1.0)) < 1.9) break;
        texcoord += dir * SMAA_PIXEL_SIZE;
    }
    return i + float(e.g > 0.9) * c;
}

/** 
 * Similar to SMAAArea, this calculates the area corresponding to a certain
 * diagonal distance and crossing edges 'e'.
 */
vec2 SMAAAreaDiag(sampler2D areaTex, vec2 dist, vec2 e, float offset)
{
    vec2 texcoord = float(SMAA_AREATEX_MAX_DISTANCE_DIAG) * e + dist;
    
    // We do a scale and bias for mapping to texel space:
    texcoord = SMAA_AREATEX_PIXEL_SIZE * texcoord + (0.5 * SMAA_AREATEX_PIXEL_SIZE);
    
    // Diagonal areas are on the second half of the texture:
    texcoord.x += 0.5;
    
    // Move to proper place, according to the subpixel offset:
    texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;
    return texture2DLod(areaTex, texcoord, 0.0).ra;
}

/**
 * This searches for diagonal patterns and returns the corresponding weights.
 */
vec2 SMAACalculateDiagWeights(sampler2D edgeTex, sampler2D areaTex, vec2 texcoord, vec2 e, ivec4 subsampleIndices)
{
    vec2 weights = vec2(0.0, 0.0);
    
    vec2 d;
    d.x = e.r > 0.0 ? SMAASearchDiag1(edgeTex, texcoord, vec2(-1.0,  1.0), 1.0) : 0.0;
    d.y = SMAASearchDiag1(edgeTex, texcoord, vec2(1.0, -1.0), 0.0);
    
    if (d.r + d.g > 2.0)  // d.r + d.g + 1 > 3
    {
        vec4 coords = SMAAMad(vec4(-d.r, d.r, d.g, -d.g), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);

        vec4 c;
        c.x = texture2DLodOffset(edgeTex, coords.xy, 0.0, ivec2(-1,  0)).g;
        c.y = texture2DLodOffset(edgeTex, coords.xy, 0.0, ivec2( 0,  0)).r;
        c.z = texture2DLodOffset(edgeTex, coords.zw, 0.0, ivec2( 1,  0)).g;
        c.w = texture2DLodOffset(edgeTex, coords.zw, 0.0, ivec2( 1, -1)).r;
        
        vec2 e = 2.0 * c.xz + c.yw;
        float t = float(SMAA_MAX_SEARCH_STEPS_DIAG) - 1.0;
        e *= step(d.rg, vec2(t, t));
        
        weights += SMAAAreaDiag(areaTex, d, e, float(subsampleIndices.z));
    }
    
    d.x = SMAASearchDiag2(edgeTex, texcoord, vec2(-1.0, -1.0), 0.0);
    float right = texture2DLodOffset(edgeTex, texcoord, 0.0, ivec2(1, 0)).r;
    d.y = right > 0.0? SMAASearchDiag2(edgeTex, texcoord, vec2(1.0, 1.0), 1.0) : 0.0;
    
    if (d.r + d.g > 2.0)  // d.r + d.g + 1 > 3
    {
        vec4 coords = SMAAMad(vec4(-d.r, -d.r, d.g, d.g), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);
        
        vec4 c;
        c.x  = texture2DLodOffset(edgeTex, coords.xy, 0.0, ivec2(-1,  0)).g;
        c.y  = texture2DLodOffset(edgeTex, coords.xy, 0.0, ivec2( 0, -1)).r;
        c.zw = texture2DLodOffset(edgeTex, coords.zw, 0.0, ivec2( 1,  0)).gr;
        
        vec2 e = 2.0 * c.xz + c.yw;
        float t = float(SMAA_MAX_SEARCH_STEPS_DIAG) - 1.0;
        e *= step(d.rg, vec2(t, t));
        
        weights += SMAAAreaDiag(areaTex, d, e, float(subsampleIndices.w)).gr;
    }
    return weights;
}
#endif

//-----------------------------------------------------------------------------
// Horizontal/Vertical Search Functions
//-----------------------------------------------------------------------------
/**
 * This allows to determine how much length should we add in the last step
 * of the searches. It takes the bilinearly interpolated edge (see 
 * @PSEUDO_GATHER4), and adds 0, 1 or 2, depending on which edges and
 * crossing edges are active.
 */
float SMAASearchLength(sampler2D searchTex, vec2 e, float bias, float scale)
{
    // Not required if searchTex accesses are set to point:
    // vec2 SEARCH_TEX_PIXEL_SIZE = 1.0 / vec2(66.0, 33.0);
    // e = vec2(bias, 0.0) + 0.5 * SEARCH_TEX_PIXEL_SIZE + 
    //     e * vec2(scale, 1.0) * vec2(64.0, 32.0) * SEARCH_TEX_PIXEL_SIZE;
    e.r = bias + e.r * scale;
	e.g = -e.g;
    return 255.0 * texture2DLod(searchTex, e, 0.0).r;
}

/**
 * Horizontal/vertical search functions for the 2nd pass.
 */
float SMAASearchXLeft(sampler2D edgeTex, sampler2D searchTex, vec2 texcoord, float end)
{
    /**
     * @PSEUDO_GATHER4
     * This texcoord has been offset by (-0.25, -0.125) in the vertex shader to
     * sample between edge, thus fetching four edges in a row.
     * Sampling with different offsets in each direction allows to disambiguate
     * which edges are active from the four fetched ones.
     */
    vec2 e = vec2(0.0, 1.0);
    while (texcoord.x > end && 
           e.g > 0.8281 && // Is there some edge not activated?
           e.r == 0.0)     // Or is there a crossing edge that breaks the line?
    {
        e = texture2DLod(edgeTex, texcoord, 0.0).rg;
        texcoord -= vec2(2.0, 0.0) * SMAA_PIXEL_SIZE;
    }
    
    // We correct the previous (-0.25, -0.125) offset we applied:
    texcoord.x += 0.25 * SMAA_PIXEL_SIZE.x;
    
    // The searches are bias by 1, so adjust the coords accordingly:
    texcoord.x += SMAA_PIXEL_SIZE.x;
    
    // Disambiguate the length added by the last step:
    texcoord.x += 2.0 * SMAA_PIXEL_SIZE.x; // Undo last step
    texcoord.x -= SMAA_PIXEL_SIZE.x * SMAASearchLength(searchTex, e, 0.0, 0.5);
    return texcoord.x;
}

float SMAASearchXRight(sampler2D edgeTex, sampler2D searchTex, vec2 texcoord, float end)
{
    vec2 e = vec2(0.0, 1.0);
    while (texcoord.x < end && 
           e.g > 0.8281 && // Is there some edge not activated?
           e.r == 0.0)     // Or is there a crossing edge that breaks the line?
    {
        e = texture2DLod(edgeTex, texcoord, 0.0).rg;
        texcoord += vec2(2.0, 0.0) * SMAA_PIXEL_SIZE;
    }
    
    texcoord.x -= 0.25 * SMAA_PIXEL_SIZE.x;
    texcoord.x -= SMAA_PIXEL_SIZE.x;
    texcoord.x -= 2.0 * SMAA_PIXEL_SIZE.x;
    texcoord.x += SMAA_PIXEL_SIZE.x * SMAASearchLength(searchTex, e, 0.5, 0.5);
    return texcoord.x;
}

float SMAASearchYUp(sampler2D edgeTex, sampler2D searchTex, vec2 texcoord, float end)
{
    vec2 e = vec2(1.0, 0.0);
    while (texcoord.y > end && 
           e.r > 0.8281 && // Is there some edge not activated?
           e.g == 0.0)     // Or is there a crossing edge that breaks the line?
    {
        e = texture2DLod(edgeTex, texcoord, 0.0).rg;
        texcoord -= vec2(0.0, 2.0) * SMAA_PIXEL_SIZE;
    }
    
    texcoord.y += 0.25 * SMAA_PIXEL_SIZE.y;
    texcoord.y += SMAA_PIXEL_SIZE.y;
    texcoord.y += 2.0 * SMAA_PIXEL_SIZE.y;
    texcoord.y -= SMAA_PIXEL_SIZE.y * SMAASearchLength(searchTex, e.gr, 0.0, 0.5);
    return texcoord.y;
}

float SMAASearchYDown(sampler2D edgeTex, sampler2D searchTex, vec2 texcoord, float end)
{
    vec2 e = vec2(1.0, 0.0);
    while (texcoord.y < end && 
           e.r > 0.8281 && // Is there some edge not activated?
           e.g == 0.0)     // Or is there a crossing edge that breaks the line?
    {
        e = texture2DLod(edgeTex, texcoord, 0.0).rg;
        texcoord += vec2(0.0, 2.0) * SMAA_PIXEL_SIZE;
    }
    
    texcoord.y -= 0.25 * SMAA_PIXEL_SIZE.y;
    texcoord.y -= SMAA_PIXEL_SIZE.y;
    texcoord.y -= 2.0 * SMAA_PIXEL_SIZE.y;
    texcoord.y += SMAA_PIXEL_SIZE.y * SMAASearchLength(searchTex, e.gr, 0.5, 0.5);
    return texcoord.y;
}

/** 
 * Ok, we have the distance and both crossing edges. So, what are the areas
 * at each side of current edge?
 */
vec2 SMAAArea(sampler2D areaTex, vec2 dist, float e1, float e2, float offset)
{
    // Rounding prevents precision errors of bilinear filtering:
    vec2 texcoord = float(SMAA_AREATEX_MAX_DISTANCE) * round(4.0 * vec2(e1, e2)) + dist;
    
    // We do a scale and bias for mapping to texel space:
    texcoord = SMAA_AREATEX_PIXEL_SIZE * texcoord + (0.5 * SMAA_AREATEX_PIXEL_SIZE);
    
    // Move to proper place, according to the subpixel offset:
    texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;
    return texture2DLod(areaTex, texcoord, 0.0).ra;
}

//-----------------------------------------------------------------------------
// Corner Detection Functions
//-----------------------------------------------------------------------------
void SMAADetectHorizontalCornerPattern(sampler2D edgeTex, inout vec2 weights, vec2 texcoord, vec2 d)
{
#if SMAA_CORNER_ROUNDING < 100 || SMAA_FORCE_CORNER_DETECTION == 1
    vec4 coords = SMAAMad(vec4(d.x, 0.0, d.y, 0.0), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);
    
    vec2 e;
    e.r = texture2DLodOffset(edgeTex, coords.xy, 0.0, ivec2(0.0,  1.0)).r;
    bool left = abs(d.x) < abs(d.y);
    e.g = texture2DLodOffset(edgeTex, coords.xy, 0.0, ivec2(0.0, -2.0)).r;
    if (left) weights *= SMAASaturate(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e);

    e.r = texture2DLodOffset(edgeTex, coords.zw, 0.0, ivec2(1.0,  1.0)).r;
    e.g = texture2DLodOffset(edgeTex, coords.zw, 0.0, ivec2(1.0, -2.0)).r;
    if (!left) weights *= SMAASaturate(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e);
#endif
}

void SMAADetectVerticalCornerPattern(sampler2D edgeTex, inout vec2 weights, vec2 texcoord, vec2 d)
{
#if SMAA_CORNER_ROUNDING < 100 || SMAA_FORCE_CORNER_DETECTION == 1
    vec4 coords = SMAAMad(vec4(0.0, d.x, 0.0, d.y), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);
    
    vec2 e;
    e.r = texture2DLodOffset(edgeTex, coords.xy, 0.0, ivec2( 1.0, 0.0)).g;
    bool left = abs(d.x) < abs(d.y);
    e.g = texture2DLodOffset(edgeTex, coords.xy, 0.0, ivec2(-2.0, 0.0)).g;
    if (left) weights *= SMAASaturate(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e);

    e.r = texture2DLodOffset(edgeTex, coords.zw, 0.0, ivec2( 1.0, 1.0)).g;
    e.g = texture2DLodOffset(edgeTex, coords.zw, 0.0, ivec2(-2.0, 1.0)).g;
    if (!left) weights *= SMAASaturate(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e);
#endif
}

//-----------------------------------------------------------------------------
// Blending Weight Calculation Pixel Shader (Second Pass)
//-----------------------------------------------------------------------------
vec4 SMAABlendingWeightCalculationPS(vec2 texcoord, vec4 offset1, vec4 offset2, vec4 offset3,
                                     ivec4 subsampleIndices)  // Just pass zero for SMAA 1x, see @SUBSAMPLE_INDICES.
{
    vec4 weights = vec4(0.0, 0.0, 0.0, 0.0);
    vec2 e = texture2D(edgeTex, texcoord).rg;
    if (e.g > 0.0)  // Edge at north
    {
#if SMAA_MAX_SEARCH_STEPS_DIAG > 0 || SMAA_FORCE_DIAGONAL_DETECTION == 1
        // Diagonals have both north and west edges, so searching for them in
        // one of the boundaries is enough.
        weights.rg = SMAACalculateDiagWeights(edgeTex, areaTex, texcoord, e, subsampleIndices);
        
        // We give priority to diagonals, so if we find a diagonal we skip 
        // horizontal/vertical processing.
        if (dot(weights.rg, vec2(1.0, 1.0)) == 0.0)
        {
#endif
        
        // Find the distance to the left:
        vec2 d, coords;
        coords.x = SMAASearchXLeft(edgeTex, searchTex, offset1.xy, offset3.x);
        coords.y = offset2.y; // offset2.y = texcoord.y - 0.25 * SMAA_PIXEL_SIZE.y (@CROSSING_OFFSET)
        d.x = coords.x;
        
        // Now fetch the left crossing edges, two at a time using bilinear
        // filtering. Sampling at -0.25 (see @CROSSING_OFFSET) enables to
        // discern what value each edge has:
        float e1 = texture2DLod(edgeTex, coords, 0.0).r;
        
        // Find the distance to the right:
        coords.x = SMAASearchXRight(edgeTex, searchTex, offset1.zw, offset3.y);
        d.y = coords.x;
        
        // We want the distances to be in pixel units (doing this here allow to
        // better interleave arithmetic and memory accesses):
        d = (d - texcoord.x) / SMAA_PIXEL_SIZE.x;
        
        // SMAAArea below needs a sqrt, as the areas texture is compressed 
        // quadratically:
        vec2 sqrt_d = sqrt(abs(d));
        
        // Fetch the right crossing edges:
        float e2 = texture2DLodOffset(edgeTex, coords, 0.0, ivec2(1, 0)).r;
        
        // Ok, we know how this pattern looks like, now it is time for getting
        // the actual area:
        weights.rg = SMAAArea(areaTex, sqrt_d, e1, e2, float(subsampleIndices.y));
        
        // Fix corners:
        SMAADetectHorizontalCornerPattern(edgeTex, weights.rg, texcoord, d);
        
#if SMAA_MAX_SEARCH_STEPS_DIAG > 0 || SMAA_FORCE_DIAGONAL_DETECTION == 1
        }
else e.r = 0.0; // Skip vertical processing.
        #endif
    }
    
    if (e.r > 0.0)  // Edge at west
    {
        // Find the distance to the top:
        vec2 d, coords;
        coords.y = SMAASearchYUp(edgeTex, searchTex, offset2.xy, offset3.z);
        coords.x = offset1.x; // offset2.x = texcoord.x - 0.25 * SMAA_PIXEL_SIZE.x;
        d.x = coords.y;
        
        // Fetch the top crossing edges:
        float e1 = texture2DLod(edgeTex, coords, 0.0).g;
        
        // Find the distance to the bottom:
        coords.y = SMAASearchYDown(edgeTex, searchTex, offset2.zw, offset3.w);
        d.y = coords.y;
        
        // We want the distances to be in pixel units:
        d = (d - texcoord.y) / SMAA_PIXEL_SIZE.y;
        
        // SMAAArea below needs a sqrt, as the areas texture is compressed 
        // quadratically:
        vec2 sqrt_d = sqrt(abs(d));
        
        // Fetch the bottom crossing edges:
        float e2 = texture2DLodOffset(edgeTex, coords, 0.0, ivec2(0, 1)).g;
        
        // Get the area for this direction:
        weights.ba = SMAAArea(areaTex, sqrt_d, e1, e2, float(subsampleIndices.x));
        
        // Fix corners:
        SMAADetectVerticalCornerPattern(edgeTex, weights.ba, texcoord, d);
    }
    return weights;
}

void main()
{
    gl_FragColor = SMAABlendingWeightCalculationPS(gl_TexCoord[0].st, offset1, offset2, offset3, ivec4(0));
}
