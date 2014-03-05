#include <osg/ImageStream>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "VideoPlayer.h"

osg::Node* createVideoQuad( const osg::Vec3& corner, const std::string& file )
{
    osg::ref_ptr<VideoPlayer> imageStream = new VideoPlayer;
    if ( imageStream )
    {
        imageStream->open( file );
        imageStream->play();
    }
    
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( imageStream.get() );
    texture->setResizeNonPowerOfTwoHint( false );
    
    osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
        corner, osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f, 1.0f, 0.0f );
    quad->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( quad.get() );
    return geode.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    std::string file("plush1_720p_10s.m2v");  // Borrowed from CUDA (an H.264 segment)
    if ( arguments.argc()>1 ) file = arguments[1];
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( createVideoQuad(osg::Vec3(), file) );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
