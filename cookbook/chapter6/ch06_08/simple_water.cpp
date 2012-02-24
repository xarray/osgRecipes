/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 8
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/TexGen>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/ClipNode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

static const char* waterVert = {
    "uniform float osg_FrameTime;\n"
    "varying vec4 projCoords;\n"
    "varying vec3 lightDir, eyeDir;\n"
    "varying vec2 flowCoords, rippleCoords;\n"
    
    "void main()\n"
    "{\n"
    "   vec3 T = vec3(0.0, 1.0, 0.0);\n"
    "   vec3 N = vec3(0.0, 0.0, 1.0);\n"
    "   vec3 B = vec3(1.0, 0.0, 0.0);\n"
    "   T = normalize(gl_NormalMatrix * T);\n"
    "   B = normalize(gl_NormalMatrix * B);\n"
    "   N = normalize(gl_NormalMatrix * N);\n"
    
    "   mat3 TBNmat;\n"
    "   TBNmat[0][0] = T[0]; TBNmat[1][0] = T[1]; TBNmat[2][0] = T[2];\n"
    "   TBNmat[0][1] = B[0]; TBNmat[1][1] = B[1]; TBNmat[2][1] = B[2];\n"
    "   TBNmat[0][2] = N[0]; TBNmat[1][2] = N[1]; TBNmat[2][2] = N[2];\n"
    
    "   vec3 vertexInEye = vec3(gl_ModelViewMatrix * gl_Vertex);\n"
    "   lightDir =  gl_LightSource[0].position.xyz - vertexInEye;\n"
    "   lightDir = normalize(TBNmat * lightDir);\n"
    "   eyeDir = normalize(TBNmat * (-vertexInEye));\n"
    
    "   vec2 t1 = vec2(osg_FrameTime*0.02, osg_FrameTime*0.02);\n"
    "   vec2 t2 = vec2(osg_FrameTime*0.05, osg_FrameTime*0.05);\n"
    "   flowCoords = gl_MultiTexCoord0.xy * 5.0 + t1;\n"
    "   rippleCoords = gl_MultiTexCoord0.xy * 10.0 + t2;\n"
    
    "   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "   gl_Position = ftransform();\n"
    "   projCoords = gl_Position;\n"
    "}\n"
};

static const char* waterFrag = {
    "uniform sampler2D defaultTex;\n"
    "uniform sampler2D reflection;\n"
    "uniform sampler2D refraction;\n"
    "uniform sampler2D normalTex;\n"
    "varying vec4 projCoords;\n"
    "varying vec3 lightDir, eyeDir;\n"
    "varying vec2 flowCoords, rippleCoords;\n"
    
    "void main()\n"
    "{\n"
    "   vec2 rippleEffect = 0.02 * texture2D(refraction, rippleCoords * 0.1).xy;\n"
    "   vec4 N = texture2D(normalTex, flowCoords + rippleEffect);\n"
    "   N = N * 2.0 - vec4(1.0);\n"
    "   N.a = 1.0; N = normalize(N);\n"
    
    "   vec3 refVec = normalize(reflect(-lightDir, vec3(N) * 0.6));\n"
    "   float refAngle = clamp(dot(eyeDir, refVec), 0.0, 1.0);\n"
    "   vec4 specular = vec4(pow(refAngle, 40.0));\n"
    
    "   vec2 dist = texture2D(refraction, flowCoords + rippleEffect).xy;\n"
    "   dist = (dist * 2.0 - vec2(1.0)) * 0.1;\n"
    "   vec2 uv = projCoords.xy / projCoords.w;\n"
    "   uv = clamp((uv + 1.0) * 0.5 + dist, 0.0, 1.0);\n"
    
    "   vec4 base = texture2D(defaultTex, uv);\n"
    "   vec4 refl = texture2D(reflection, uv);\n"
    "   gl_FragColor = mix(base, refl + specular, 0.6);\n"
    "}\n"
};

osg::Texture2D* createTexture( const std::string& filename )
{
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( osgDB::readImageFile(filename) );
    texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
    texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    return texture.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("cessna.osg");
    
    // The reversed node
    float z = -20.0f;
    osg::ref_ptr<osg::MatrixTransform> reverse = new osg::MatrixTransform;
    reverse->preMult(osg::Matrix::translate(0.0f, 0.0f, -z) *
                     osg::Matrix::scale(1.0f, 1.0f, -1.0f) *
                     osg::Matrix::translate(0.0f, 0.0f, z) );
    reverse->addChild( scene.get() );
    
    osg::ref_ptr<osg::ClipPlane> clipPlane = new osg::ClipPlane;
    clipPlane->setClipPlane( 0.0, 0.0, -1.0, z );
    clipPlane->setClipPlaneNum( 0 );
    
    osg::ref_ptr<osg::ClipNode> clipNode = new osg::ClipNode;
    clipNode->addClipPlane( clipPlane.get() );
    clipNode->addChild( reverse.get() );
    
    // The RTT camera
    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
    tex2D->setTextureSize( 1024, 1024 );
    tex2D->setInternalFormat( GL_RGBA );
    
    osg::ref_ptr<osg::Camera> rttCamera = osgCookBook::createRTTCamera(osg::Camera::COLOR_BUFFER, tex2D.get());
    rttCamera->addChild( clipNode.get() );
    
    // The water plane
    const osg::Vec3& center = scene->getBound().center();
    float planeSize = 20.0f * scene->getBound().radius();
    osg::Vec3 planeCorner( center.x()-0.5f*planeSize, center.y()-0.5f*planeSize, z );
    osg::ref_ptr<osg::Geometry> quad = osg::createTexturedQuadGeometry(
        planeCorner, osg::Vec3(planeSize, 0.0f, 0.0f), osg::Vec3(0.0f, planeSize, 0.0f) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( quad.get() );
    
    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->setTextureAttributeAndModes( 0, tex2D.get() );
    ss->setTextureAttributeAndModes( 1, createTexture("Images/skymap.jpg") );
    ss->setTextureAttributeAndModes( 2, createTexture("water_DUDV.jpg") );
    ss->setTextureAttributeAndModes( 3, createTexture("water_NM.jpg") );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, waterVert) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, waterFrag) );
    geode->getOrCreateStateSet()->setAttributeAndModes( program.get() );
    geode->getOrCreateStateSet()->addUniform( new osg::Uniform("reflection", 0) );
    geode->getOrCreateStateSet()->addUniform( new osg::Uniform("defaultTex", 1) );
    geode->getOrCreateStateSet()->addUniform( new osg::Uniform("refraction", 2) );
    geode->getOrCreateStateSet()->addUniform( new osg::Uniform("normalTex", 3) );
    
    // Build the scene graph
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( rttCamera.get() );
    root->addChild( geode.get() );
    root->addChild( scene.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
