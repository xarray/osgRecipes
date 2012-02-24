/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 2 Recipe 5
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/ClipNode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("cessna.osg");
    
    float z = -10.0f;
    osg::ref_ptr<osg::MatrixTransform> reverse = new osg::MatrixTransform;
    reverse->preMult(osg::Matrix::translate(0.0f, 0.0f, -z) *
                     osg::Matrix::scale(1.0f, 1.0f, -1.0f) *
                     osg::Matrix::translate(0.0f, 0.0f, z) );
    reverse->addChild( scene.get() );
    
    osg::ref_ptr<osg::ClipPlane> clipPlane = new osg::ClipPlane;
    clipPlane->setClipPlane( 0.0, 0.0, -1.0, z );
    clipPlane->setClipPlaneNum( 0 );
    
    osg::ref_ptr<osg::ClipNode> clipNode = new osg::ClipNode;
    clipNode->addClipPlane( clipPlane.get() );
    clipNode->addChild( reverse.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( scene.get() );
    root->addChild( clipNode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
