/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 2 Recipe 10
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "Compass"

osg::MatrixTransform* createCompassPart( const std::string& image, float radius, float height )
{
    osg::Vec3 center(-radius, -radius, height);
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(
        createTexturedQuadGeometry(center, osg::Vec3(radius*2.0f,0.0f,0.0f), osg::Vec3(0.0f,radius*2.0f,0.0f)) );
    
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( osgDB::readImageFile(image) );
    
    osg::ref_ptr<osg::MatrixTransform> part = new osg::MatrixTransform;
    part->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    part->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    part->addChild( geode.get() );
    return part.release();
}

osg::Geode* createEarth( const std::string& filename )
{
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( osgDB::readImageFile(filename) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(), osg::WGS_84_RADIUS_POLAR)) );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    return geode.release();
}

int main( int argc, char** argv )
{
    osgViewer::Viewer viewer;
    
    osg::ref_ptr<Compass> compass = new Compass;
    compass->setViewport( 0.0, 0.0, 200.0, 200.0 );
    compass->setProjectionMatrix( osg::Matrixd::ortho(-1.5, 1.5, -1.5, 1.5, -10.0, 10.0) );
    compass->setPlate( createCompassPart("compass_plate.png", 1.5f, -1.0f) );
    compass->setNeedle( createCompassPart("compass_needle.png", 1.5f, 0.0f) );
    compass->setMainCamera( viewer.getCamera() );
    
    compass->setRenderOrder( osg::Camera::POST_RENDER );
    compass->setClearMask( GL_DEPTH_BUFFER_BIT );
    compass->setAllowEventFocus( false );
    compass->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    compass->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    compass->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( createEarth("Images/land_shallow_topo_2048.jpg") );
    root->addChild( compass.get() );
    
    viewer.setSceneData( root.get() );
    return viewer.run();
}
