// Remember to copy Arial.ttf from your system directory (or OSG data directory) to the executable folder

#include <osg/Image>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "Drawer2D.h"

osg::Node* createImageQuad( osg::Image* image, const osg::Vec3& corner )
{
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( image );
    texture->setResizeNonPowerOfTwoHint( false );
    
    osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
        corner, osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f) );
    quad->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    if ( image->isImageTranslucent() )
    {
        quad->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
        quad->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    }
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( quad.get() );
    return geode.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osgViewer::Viewer viewer;
    
    osg::ref_ptr<AggDrawer> image = new AggDrawer;
    image->allocateImage( 1024, 1024, 1, GL_RGBA, GL_UNSIGNED_BYTE );
    image->clear( 0, 0, 100, 80 );
    
    image->setFontFile( "Arial.ttf", false );
    image->setFontSize( 64.0f, 64.0f );
    image->drawText( osg::Vec2(100.0f, 800.0f), L"Hello world!" );
    
    osg::Image* subImage = osgDB::readImageFile( "Images/osg128.png" );
    image->drawImage( osg::Vec2(100.0f, 400.0f), 250.0f, 200.0f, subImage );
    
    image->setPenWidth( 5.0f );
    image->setPenColor( 250, 250, 0, 255 );
    image->setBrushColor( 0, 150, 0, 255 );
    
    image->setPenStyle( AggDrawer::DASH_DOT_LINE );
    image->drawLine( osg::Vec2(0.0f, 0.0f), osg::Vec2(1024.0f, 1024.0f) );
    image->drawArc( osg::Vec2(200.0f, 100.0f), 40.0f, 0.0f, osg::PI, true );
    
    image->setPenStyle( AggDrawer::SOLID_LINE );
    image->drawChord( osg::Vec2(300.0f, 100.0f), 50.0f, 0.0f, osg::PI, false );
    image->drawPie( osg::Vec2(400.0f, 100.0f), 80.0f, osg::PI*0.25, osg::PI*0.75 );
    image->drawCircle( osg::Vec2(500.0f, 100.0f), 50.0f, false );
    image->drawEllipse( osg::Vec2(600.0f, 100.0f), 30.0f, 80.0f, true );
    
    image->translate( 0.0f, -50.0f );
    image->drawRectangle( osg::Vec2(700.0f, 100.0f), 50.0f, 70.0f, true );
    image->drawRoundedRectangle( osg::Vec2(700.0f, 200.0f), 50.0f, 50.0f, 10.0f, false );
    image->resetTransform();
    
    std::vector<osg::Vec2> bspline;
    bspline.push_back( osg::Vec2(800.0f, 50.0f) );
    bspline.push_back( osg::Vec2(780.0f, 70.0f) );
    bspline.push_back( osg::Vec2(810.0f, 100.0f) );
    bspline.push_back( osg::Vec2(840.0f, 130.0f) );
    bspline.push_back( osg::Vec2(820.0f, 150.0f) );
    image->drawSpline( bspline );
    
    std::vector<osg::Vec2> polygon;
    polygon.push_back( osg::Vec2(860.0f, 50.0f) );
    polygon.push_back( osg::Vec2(890.0f, 150.0f) );
    polygon.push_back( osg::Vec2(900.0f, 100.0f) );
    polygon.push_back( osg::Vec2(930.0f, 100.0f) );
    polygon.push_back( osg::Vec2(860.0f, 50.0f) );
    image->drawPolygon( polygon, 0.5f, false );
    
    image->setPenWidth( 3.0f );
    image->setPenColor( 250, 0, 0, 255 );
    image->drawContour( polygon, 25.0f, AggDrawer::ROUND, 0.0f, false );
    
    image->setBrushColor( 150, 150, 0, 255 );
    image->drawArrowHead( osg::Vec2(700.0f, 300.0f), osg::Vec4(10.0f, 6.0f, 20.0f, 4.0f),
                          osg::Vec2(800.0f, 350.0f), osg::Vec4(15.0f, 10.0f, 30.0f, 5.0f) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( createImageQuad(image.get(), osg::Vec3()) );
    root->addChild( osgDB::readNodeFiles(arguments) );
    
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
