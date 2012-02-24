/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 12
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#if 0
#   include <osg/Texture2D>
#   define SL_SAMPLER "sampler2D"
#   define SL_TEXTURE "texture2D"
#   define CLASS_TEXTURE osg::Texture2D
#   define TEXCOORD_SCALE 1.0f
#else
#   include <osg/TextureRectangle>
#   define SL_SAMPLER "samplerRect"
#   define SL_TEXTURE "textureRect"
#   define CLASS_TEXTURE osg::TextureRectangle
#   define TEXCOORD_SCALE 1024.0f
#endif

#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

static const char* mrtVertSource = {
    "uniform mat4 osg_ViewMatrix;\n"
    "uniform mat4 osg_ViewMatrixInverse;\n"
    "varying vec3 worldNormal;\n"
    "varying vec3 worldView;\n"
    "void main(void)\n"
    "{\n"
    "   worldNormal = vec3(gl_ModelViewMatrixInverseTranspose * vec4(gl_Normal,0.0));\n"
    "   worldNormal = normalize(worldNormal);\n"
    "   worldView = vec3(osg_ViewMatrixInverse * gl_ModelViewMatrix * gl_Vertex);\n"
    "   worldView = (osg_ViewMatrix[3].xyz / osg_ViewMatrix[3].w) - worldView;\n"
    "   worldView = normalize(worldView);\n"
    "   gl_Position = ftransform();\n"
    "   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "}\n"
};

static const char* mrtFragSource = {
    "uniform sampler2D defaultTex;\n"
    "varying vec3 worldNormal;\n"
    "varying vec3 worldView;\n"
    "void main(void)\n"
    "{\n"
    "   gl_FragData[0] = texture2D(defaultTex, gl_TexCoord[0].xy);\n"
    "   gl_FragData[1] = vec4(worldNormal, 0.0);\n"
    "   gl_FragData[2] = vec4(worldView, 0.0);\n"
    "}\n"
};

static const char* finalVertSource = {
    "void main(void)\n"
    "{\n"
    "   gl_Position = ftransform();\n"
    "   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "}\n"
};

static const char* finalFragSource = {
    "uniform "SL_SAMPLER" colorTex;\n"
    "uniform "SL_SAMPLER" normalTex;\n"
    "uniform "SL_SAMPLER" viewTex;\n"
    "void main(void)\n"
    "{\n"
    "   vec2 uv = gl_TexCoord[0].xy;\n"
    "   vec3 color = "SL_TEXTURE"(colorTex, uv).xyz;\n"
    "   vec3 normal = "SL_TEXTURE"(normalTex, uv).xyz;\n"
    "   vec3 viewDir = "SL_TEXTURE"(viewTex, uv).xyz;\n"
    "   vec3 lightDir = vec3(0.7, -0.7, -0.7);\n"
    "   lightDir = normalize(-lightDir);\n"
    
    "   vec3 halfDir = normalize(viewDir + lightDir);\n"
    "   float LdotN = dot(lightDir, normal);\n"
    "   float HdotN = dot(halfDir, normal);\n"
    "   float diffuse = LdotN<0.0 ? 0.0 : LdotN;\n"
    "   float specular = (LdotN<0.0 || HdotN<0.0) ? 0.0 : pow(HdotN, 30.0);\n"
    
    "   vec3 finalColor = vec3(1.0) * 0.4 * diffuse * specular;\n"
    "   finalColor += color * (diffuse + 0.07);\n"
    "   gl_FragColor = vec4(finalColor, 1.0);\n"
    "}\n"
};

osg::Texture* createFloatTexture()
{
    osg::ref_ptr<CLASS_TEXTURE> texture = new CLASS_TEXTURE;
    texture->setTextureSize( 1024, 1024 );
    texture->setInternalFormat( GL_RGBA16F_ARB );
    texture->setSourceFormat( GL_RGBA );
    texture->setSourceType( GL_FLOAT );
    return texture.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("teapot.osg");
    
    osg::Texture* colorTex = createFloatTexture();
    osg::Texture* normalTex = createFloatTexture();
    osg::Texture* viewTex = createFloatTexture();
    
    osg::ref_ptr<osg::Camera> rttCamera = osgCookBook::createRTTCamera(osg::Camera::COLOR_BUFFER0, colorTex);
    rttCamera->attach( osg::Camera::COLOR_BUFFER1, normalTex );
    rttCamera->attach( osg::Camera::COLOR_BUFFER2, viewTex );
    rttCamera->addChild( scene.get() );
    
    osg::ref_ptr<osg::Program> mrtProg = new osg::Program;
    mrtProg->addShader( new osg::Shader(osg::Shader::VERTEX, mrtVertSource) );
    mrtProg->addShader( new osg::Shader(osg::Shader::FRAGMENT, mrtFragSource) );
    rttCamera->getOrCreateStateSet()->setAttributeAndModes(
        mrtProg.get(), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
    rttCamera->getOrCreateStateSet()->addUniform( new osg::Uniform("defaultTex", 0) );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0.0, 1.0, 0.0, 1.0);
    hudCamera->addChild( osgCookBook::createScreenQuad(0.5f, 1.0f, TEXCOORD_SCALE) );
    
    osg::ref_ptr<osg::Program> finalProg = new osg::Program;
    finalProg->addShader( new osg::Shader(osg::Shader::VERTEX, finalVertSource) );
    finalProg->addShader( new osg::Shader(osg::Shader::FRAGMENT, finalFragSource) );
    
    osg::StateSet* ss = hudCamera->getOrCreateStateSet();
    ss->setTextureAttributeAndModes( 0, colorTex );
    ss->setTextureAttributeAndModes( 1, normalTex );
    ss->setTextureAttributeAndModes( 2, viewTex );
    ss->setAttributeAndModes( finalProg.get(), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
    ss->addUniform( new osg::Uniform("colorTex", 0) );
    ss->addUniform( new osg::Uniform("normalTex", 1) );
    ss->addUniform( new osg::Uniform("viewTex", 2) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( rttCamera.get() );
    root->addChild( hudCamera.get() );
    root->addChild( scene.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
