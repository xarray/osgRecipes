/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 8 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Texture>
#include <osg/Node>
#include <osgDB/DatabasePager>
#include <osgDB/ReadFile>
#include <osgUtil/PrintVisitor>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "CommonFunctions"

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(arguments);
    osg::Texture::getTextureObjectManager(0)->setMaxTexturePoolSize( 64000 );
    
    osgViewer::Viewer viewer;
    osgDB::DatabasePager* pager = viewer.getDatabasePager();
    pager->setDoPreCompile( true) ;
    pager->setTargetMaximumNumberOfPageLOD( 10 );
    
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
