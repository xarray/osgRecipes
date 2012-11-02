uniform sampler2D sceneTex;
uniform sampler2D smaaTex;

varying vec4 offset;
#define SMAA_PIXEL_SIZE vec2(1.0/1920.0, 1.0/1080.0)

//-----------------------------------------------------------------------------
// Neighborhood Blending Pixel Shader (Third Pass)
//-----------------------------------------------------------------------------
vec4 SMAANeighborhoodBlendingPS(vec2 texcoord, vec4 offset)
{
    // Fetch the blending weights for current pixel:
    vec4 a;
    a.xz = texture2D(smaaTex, texcoord).xz;
    a.y = texture2D(smaaTex, offset.zw).g;
    a.w = texture2D(smaaTex, offset.xy).a;
    
    // Is there any blending weight with a value greater than 0.0?
    if (dot(a, vec4(1.0)) < 1e-5)
        return texture2DLod(sceneTex, texcoord, 0.0);
    else
    {
        vec4 color = vec4(0.0);
        
        // Up to 4 lines can be crossing a pixel (one through each edge). We
        // favor blending by choosing the line with the maximum weight for each
        // direction:
        vec2 localOffset;
        localOffset.x = a.a > a.b? a.a : -a.b; // left vs. right 
        localOffset.y = a.g > a.r? a.g : -a.r; // top vs. bottom
        
        // Then we go in the direction that has the maximum weight:
        if (abs(localOffset.x) > abs(localOffset.y)) // horizontal vs. vertical
            localOffset.y = 0.0;
        else
            localOffset.x = 0.0;
        
#if SMAA_REPROJECTION == 1
        // Fetch the opposite color and lerp by hand:
        vec4 C = texture2DLod(sceneTex, texcoord, 0.0);
        texcoord += sign(localOffset) * SMAA_PIXEL_SIZE;
        vec4 Cop = texture2DLod(sceneTex, texcoord, 0.0);
        float s = abs(localOffset.x) > abs(localOffset.y)? abs(localOffset.x) : abs(localOffset.y);
        
        // Unpack the velocity values:
        C.a *= C.a;
        Cop.a *= Cop.a;
        
        // Lerp the colors:
        vec4 Caa = mix(C, Cop, s);
        
        // Unpack velocity and return the resulting value:
        Caa.a = sqrt(Caa.a);
        return Caa;
#else
        // Fetch the opposite color and lerp by hand:
        vec4 C = texture2DLod(sceneTex, texcoord, 0.0);
        texcoord += sign(offset) * SMAA_PIXEL_SIZE;
        vec4 Cop = texture2DLod(sceneTex, texcoord, 0.0);
        float s = abs(offset.x) > abs(offset.y)? abs(offset.x) : abs(offset.y);
        return mix(C, Cop, s);
#endif
    }
}

void main(void)
{
   gl_FragColor = SMAANeighborhoodBlendingPS(gl_TexCoord[0].st, offset);
}
