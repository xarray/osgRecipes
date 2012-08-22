#include "DirectWriteImage.h"
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

std::wstring toWideString( const std::string& str, unsigned int codePage=CP_ACP )
{
    DWORD outSize = MultiByteToWideChar( codePage, 0, str.c_str(), -1, NULL, 0 );
    wchar_t* wtext = new wchar_t[outSize];
    wtext[outSize-1] = '\0';
    
    MultiByteToWideChar( codePage, 0, str.c_str(), str.size(), wtext, outSize );
    std::wstring result = wtext;
    delete[] wtext;
    return result;
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    std::string text("Hello osgRecipes");
    arguments.read( "--text", text );
    
    int w = 800, h = 600;
    arguments.read( "--width", w );
    arguments.read( "--height", h );
    
    osg::ref_ptr<DirectWriteImage> image = new DirectWriteImage;
    image->initialize( toWideString(text), L"Gabriola", L"en-us", 72.0f, w, h );
    image->setFontStyle( DWRITE_FONT_STYLE_ITALIC, 0, 5 );
    image->setFontWeight( DWRITE_FONT_WEIGHT_BOLD, 6, 3 );
    
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( image.get() );
    texture->setResizeNonPowerOfTwoHint( false );
    
    osg::ref_ptr<osg::Geode> textGeode = new osg::Geode;
    textGeode->addDrawable( osg::createTexturedQuadGeometry(osg::Vec3(-0.5f, -0.5f, 0.0f), osg::X_AXIS,-osg::Z_AXIS) );
    textGeode->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    textGeode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    textGeode->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc(GL_SRC_COLOR, GL_ONE, GL_SRC_COLOR, GL_ZERO) );
    
    osg::ref_ptr<osg::MatrixTransform> textTrans = new osg::MatrixTransform;
    textTrans->setMatrix( osg::Matrix::translate(0.0f, 0.0f, 0.5f) );
    textTrans->addChild( textGeode.get() );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("glider.osg") );
    scene->addChild( textTrans.get() );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
