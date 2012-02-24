/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 6
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

static const char* blurFragSource = {
    "uniform sampler2D inputTex;\n"
    "uniform vec2 blurDir;\n"
    "void main(void)\n"
    "{\n"
    "   vec2 uv = gl_TexCoord[0].st;\n"
    "   vec2 offset = vec2(0.004*blurDir.x, 0.003*blurDir.y);\n"
    "   vec4 color = texture2D(inputTex, uv) * 0.3;\n"
    "   color += texture2D(inputTex, uv - offset*3.0) * 0.05;\n"
    "   color += texture2D(inputTex, uv - offset*2.0) * 0.1;\n"
    "   color += texture2D(inputTex, uv - offset) * 0.2;\n"
    "   color += texture2D(inputTex, uv + offset) * 0.2;\n"
    "   color += texture2D(inputTex, uv + offset*2.0) * 0.1;\n"
    "   color += texture2D(inputTex, uv + offset*3.0) * 0.05;\n"
    "   gl_FragColor = color;\n"
    "}\n"
};

static const char* combineFragSource = {
    "uniform sampler2D sceneTex;\n"
    "uniform sampler2D blurTex;\n"
    "uniform sampler2D depthTex;\n"
    "uniform float focalDistance;\n"
    "uniform float focalRange;\n"
    
    "float getBlurFromLinearDepth(vec2 uv)\n"
    "{\n"
    "   float z = texture2D(depthTex, uv).x;\n"
    "   z = 2.0 * 10001.0 / (10001.0 - z * 9999.0) - 1.0;\n"  // Considering the default znear/zfar
    "   return clamp((z - focalDistance)/focalRange, 0.0, 1.0);\n"
    "}\n"
    
    "void main(void)\n"
    "{\n"
    "   vec2 uv = gl_TexCoord[0].st;\n"
    "   vec4 fullColor = texture2D(sceneTex, uv);\n"
    "   vec4 blurColor = texture2D(blurTex, uv);\n"
    "   float blurValue = getBlurFromLinearDepth(uv);\n"
    "   gl_FragColor = fullColor + blurValue * (blurColor - fullColor);\n"
    "}\n"
};

typedef std::pair<osg::Camera*, osg::Texture*> RTTPair;

RTTPair createColorInput( osg::Node* scene )
{
    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
    tex2D->setTextureSize( 1024, 1024 );
    tex2D->setInternalFormat( GL_RGBA );
    
    osg::ref_ptr<osg::Camera> camera = osgCookBook::createRTTCamera(osg::Camera::COLOR_BUFFER, tex2D.get());
    camera->addChild( scene );
    return RTTPair(camera.release(), tex2D.get());
}

RTTPair createDepthInput( osg::Node* scene )
{
    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
    tex2D->setTextureSize( 1024, 1024 );
    tex2D->setInternalFormat( GL_DEPTH_COMPONENT24 );
    tex2D->setSourceFormat( GL_DEPTH_COMPONENT );
    tex2D->setSourceType( GL_FLOAT );
    
    osg::ref_ptr<osg::Camera> camera = osgCookBook::createRTTCamera(osg::Camera::DEPTH_BUFFER, tex2D.get());
    camera->addChild( scene );
    return RTTPair(camera.release(), tex2D.get());
}

RTTPair createBlurPass( osg::Texture* inputTex, const osg::Vec2& dir )
{
    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
    tex2D->setTextureSize( 1024, 1024 );
    tex2D->setInternalFormat( GL_RGBA );
    osg::ref_ptr<osg::Camera> camera = osgCookBook::createRTTCamera(
        osg::Camera::COLOR_BUFFER, tex2D.get(), true);
    
    osg::ref_ptr<osg::Program> blurProg = new osg::Program;
    blurProg->addShader( new osg::Shader(osg::Shader::VERTEX, vertSource) );
    blurProg->addShader( new osg::Shader(osg::Shader::FRAGMENT, blurFragSource) );
    
    osg::StateSet* ss = camera->getOrCreateStateSet();
    ss->setTextureAttributeAndModes( 0, inputTex );
    ss->setAttributeAndModes( blurProg.get(), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
    ss->addUniform( new osg::Uniform("sceneTex", 0) );
    ss->addUniform( new osg::Uniform("blurDir", dir) );
    return RTTPair(camera.release(), tex2D.get());
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("lz.osg");
    
    // The first pass: color
    RTTPair pass0_color = createColorInput( scene.get() );
    
    // The first pass: depth
    RTTPair pass0_depth = createDepthInput( scene.get() );
    
    // The horizonal blur pass
    RTTPair pass1 = createBlurPass( pass0_color.second, osg::Vec2(1.0f, 0.0f) );
    
    // The vertical blur pass
    RTTPair pass2 = createBlurPass( pass1.second, osg::Vec2(0.0f, 1.0f) );
    
    // The final pass
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0.0, 1.0, 0.0, 1.0);
    hudCamera->addChild( osgCookBook::createScreenQuad(1.0f, 1.0f) );
    
    osg::ref_ptr<osg::Program> finalProg = new osg::Program;
    finalProg->addShader( new osg::Shader(osg::Shader::VERTEX, vertSource) );
    finalProg->addShader( new osg::Shader(osg::Shader::FRAGMENT, combineFragSource) );
    
    osg::StateSet* stateset = hudCamera->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes( 0, pass0_color.second );
    stateset->setTextureAttributeAndModes( 1, pass2.second );
    stateset->setTextureAttributeAndModes( 2, pass0_depth.second );
    stateset->setAttributeAndModes( finalProg.get() );
    stateset->addUniform( new osg::Uniform("sceneTex", 0) );
    stateset->addUniform( new osg::Uniform("blurTex", 1) );
    stateset->addUniform( new osg::Uniform("depthTex", 2) );
    stateset->addUniform( new osg::Uniform("focalDistance", 100.0f) );
    stateset->addUniform( new osg::Uniform("focalRange", 200.0f) );
    
    // Build the scene graph
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( pass0_color.first );
    root->addChild( pass0_depth.first );
    root->addChild( pass1.first );
    root->addChild( pass2.first );
    root->addChild( hudCamera.get() );
    
    osgViewer::Viewer viewer;
    viewer.getCamera()->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
