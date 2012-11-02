uniform sampler2D sceneTex;
varying vec4 offset1;
varying vec4 offset2;
varying vec4 offset3;
#define SMAA_THRESHOLD 0.05

/**
 * Luma Edge Detection
 *
 * IMPORTANT NOTICE: luma edge detection requires gamma-corrected colors, and
 * thus 'sceneTex' should be a non-sRGB texture.
 */
vec4 SMAALumaEdgeDetectionPS(vec2 texcoord, vec4 offset1, vec4 offset2, vec4 offset3)
{
    // Calculate the threshold:
    vec2 threshold = vec2(SMAA_THRESHOLD, SMAA_THRESHOLD);
    
    // Calculate lumas:
    vec3 weights = vec3(0.2126, 0.7152, 0.0722);
    float L = dot(texture2D(sceneTex, texcoord).rgb, weights);
    float Lleft = dot(texture2D(sceneTex, offset1.xy).rgb, weights);
    float Ltop  = dot(texture2D(sceneTex, offset1.zw).rgb, weights);
    
    // We do the usual threshold:
    vec4 delta;
    delta.xy = abs(L - vec2(Lleft, Ltop));
    vec2 edges = step(threshold, delta.xy);
    
    // Then discard if there is no edge:
    if (dot(edges, vec2(1.0, 1.0)) == 0.0) discard;
    
    // Calculate right and bottom deltas:
    float Lright = dot(texture2D(sceneTex, offset2.xy).rgb, weights);
    float Lbottom  = dot(texture2D(sceneTex, offset2.zw).rgb, weights);
    delta.zw = abs(L - vec2(Lright, Lbottom));
    
    // Calculate the maximum delta in the direct neighborhood:
    vec2 maxDelta = max(delta.xy, delta.zw);
    maxDelta = max(maxDelta.xx, maxDelta.yy);
    
    // Calculate left-left and top-top deltas:
    float Lleftleft = dot(texture2D(sceneTex, offset3.xy).rgb, weights);
    float Ltoptop = dot(texture2D(sceneTex, offset3.zw).rgb, weights);
    delta.zw = abs(vec2(Lleft, Ltop) - vec2(Lleftleft, Ltoptop));
    
    // Calculate the final maximum delta:
    maxDelta = max(maxDelta.xy, delta.zw);
    
    /**
     * Each edge with a delta in luma of less than 50% of the maximum luma
     * surrounding this pixel is discarded. This allows to eliminate spurious
     * crossing edges, and is based on the fact that, if there is too much
     * contrast in a direction, that will hide contrast in the other
     * neighbors.
     * This is done after the discard intentionally as this situation doesn't
     * happen too frequently (but it's important to do as it prevents some 
     * edges from going undetected).
     */
    edges.xy *= step(0.5 * maxDelta, delta.xy);
    return vec4(edges, 0.0, 0.0);
}

void main()
{
    gl_FragColor = SMAALumaEdgeDetectionPS(gl_TexCoord[0].st, offset1, offset2, offset3);
}
