/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 11
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/TextureRectangle>
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

osg::TextureRectangle* createFloatTexture()
{
    osg::ref_ptr<osg::TextureRectangle> tex2D = new osg::TextureRectangle;
    tex2D->setTextureSize( 1024, 1024 );
    tex2D->setInternalFormat( GL_RGBA16F_ARB );
    tex2D->setSourceFormat( GL_RGBA );
    tex2D->setSourceType( GL_FLOAT );
    return tex2D.release();
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
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, mrtVertSource) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, mrtFragSource) );
    rttCamera->getOrCreateStateSet()->setAttributeAndModes(
        program.get(), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
    rttCamera->getOrCreateStateSet()->addUniform( new osg::Uniform("defaultTex", 0) );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0.0, 1.0, 0.0, 1.0);
    hudCamera->addChild( osgCookBook::createScreenQuad(0.5f, 1.0f, 1024.0f) );
    hudCamera->getOrCreateStateSet()->setTextureAttributeAndModes( 0, normalTex );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( rttCamera.get() );
    root->addChild( hudCamera.get() );
    root->addChild( scene.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
