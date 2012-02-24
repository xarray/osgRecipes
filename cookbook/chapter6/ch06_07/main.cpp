/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 7
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/TexGen>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"
#include "SkyBox"

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("lz.osg.90,0,0.rot");
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( new osg::ShapeDrawable(
        new osg::Sphere(osg::Vec3(), scene->getBound().radius())) );
    geode->setCullingActive( false );
    
    osg::ref_ptr<SkyBox> skybox = new SkyBox;
    skybox->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::TexGen );
    skybox->setEnvironmentMap( 0,
        osgDB::readImageFile("Cubemap_snow/posx.jpg"), osgDB::readImageFile("Cubemap_snow/negx.jpg"),
        osgDB::readImageFile("Cubemap_snow/posy.jpg"), osgDB::readImageFile("Cubemap_snow/negy.jpg"),
        osgDB::readImageFile("Cubemap_snow/posz.jpg"), osgDB::readImageFile("Cubemap_snow/negz.jpg") );
    skybox->addChild( geode.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( scene.get() );
    root->addChild( skybox.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
