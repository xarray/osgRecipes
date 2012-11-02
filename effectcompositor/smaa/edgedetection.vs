varying vec4 offset1;
varying vec4 offset2;
varying vec4 offset3;
#define SMAA_PIXEL_SIZE vec2(1.0/1920.0, 1.0/1080.0)

void main()
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_MultiTexCoord0;
    offset1 = gl_TexCoord[0].xyxy + SMAA_PIXEL_SIZE.xyxy * vec4(-1.0, 0.0, 0.0,-1.0);
    offset2 = gl_TexCoord[0].xyxy + SMAA_PIXEL_SIZE.xyxy * vec4( 1.0, 0.0, 0.0, 1.0);
    offset3 = gl_TexCoord[0].xyxy + SMAA_PIXEL_SIZE.xyxy * vec4(-2.0, 0.0, 0.0,-2.0);
}
