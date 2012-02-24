/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 4 Recipe 8
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/NodeTrackerManipulator>
#include <osgViewer/Viewer>

#include "CommonFunctions"

int main( int argc, char** argv )
{
    osg::Node* model = osgDB::readNodeFile("cessna.osg.0,0,90.rot");
    if ( !model ) return 1;

    osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
    trans->addUpdateCallback( osgCookBook::createAnimationPathCallback(100.0f, 20.0) );
    trans->addChild( model );
    
    osg::ref_ptr<osg::MatrixTransform> terrain = new osg::MatrixTransform;
    terrain->addChild( osgDB::readNodeFile("lz.osg") );
    terrain->setMatrix( osg::Matrix::translate(0.0f, 0.0f,-200.0f) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( trans.get() );
    root->addChild( terrain.get() );
    
    osg::ref_ptr<osgGA::NodeTrackerManipulator> nodeTracker = new osgGA::NodeTrackerManipulator;
    nodeTracker->setHomePosition( osg::Vec3(0, -10.0, 0), osg::Vec3(), osg::Z_AXIS );
    nodeTracker->setTrackerMode( osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION );
    nodeTracker->setRotationMode( osgGA::NodeTrackerManipulator::TRACKBALL );
    nodeTracker->setTrackNode( model );
    
    osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keySwitch = new osgGA::KeySwitchMatrixManipulator;
    keySwitch->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator );
    keySwitch->addMatrixManipulator( '2', "NodeTracker", nodeTracker.get() );
    
    osgViewer::Viewer viewer;
    viewer.setCameraManipulator( keySwitch.get() );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
