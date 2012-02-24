/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 5
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Texture2D>
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

static const char* vertSource = {
    "void main(void)\n"
    "{\n"
    "   gl_Position = ftransform();\n"
    "   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "}\n"
};

static const char* fragSource = {
    "uniform sampler2D sceneTex;\n"
    "uniform sampler2D noiseTex;\n"
    "uniform float osg_FrameTime;\n"
    
    "void main(void)\n"
    "{\n"
    "   float factor = osg_FrameTime * 100.0;\n"
    "   vec2 uv = vec2(0.4*sin(factor), 0.4*cos(factor));\n"
    "   vec3 n = texture2D(noiseTex, (gl_TexCoord[0].st*3.5) + uv).rgb;\n"
    
    "   vec3 c = texture2D(sceneTex, gl_TexCoord[0].st + (n.xy*0.005)).rgb;\n"
    "   float lum = dot(vec3(0.30, 0.59, 0.11), c);\n"
    "   if (lum < 0.2) c *= 4.0;\n"
    
    "   vec3 finalColor = (c + (n*0.2)) * vec3(0.1, 0.95, 0.2);\n"
    "   gl_FragColor = vec4(finalColor, 1.0);\n"
    "}\n"
};

osg::Texture* createTexture2D( const std::string& fileName )
{
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( osgDB::readImageFile(fileName) );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
    texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    return texture.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("cessna.osg");
    
    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
    tex2D->setTextureSize( 1024, 1024 );
    tex2D->setInternalFormat( GL_RGBA );
    
    osg::ref_ptr<osg::Camera> rttCamera = osgCookBook::createRTTCamera(osg::Camera::COLOR_BUFFER, tex2D.get());
    rttCamera->addChild( scene.get() );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0.0, 1.0, 0.0, 1.0);
    hudCamera->addChild( osgCookBook::createScreenQuad(0.5f, 1.0f) );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, vertSource) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, fragSource) );
    
    osg::StateSet* stateset = hudCamera->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes( 0, tex2D.get() );
    stateset->setTextureAttributeAndModes( 1, createTexture2D("noise_tex.jpg") );
    stateset->setAttributeAndModes( program.get() );
    stateset->addUniform( new osg::Uniform("sceneTex", 0) );
    stateset->addUniform( new osg::Uniform("noiseTex", 1) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( rttCamera.get() );
    root->addChild( hudCamera.get() );
    root->addChild( scene.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
