/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 4 Recipe 10
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgDB/ReadFile>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include "CommonFunctions"
#include "JoystickManipulator"

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( osgDB::readNodeFile("lz.osg") );
    
    osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keySwitch = new osgGA::KeySwitchMatrixManipulator;
    keySwitch->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator );
    keySwitch->addMatrixManipulator( '2', "TwoDim", new TwoDimManipulator );
    
    const osg::BoundingSphere& bs = root->getBound();
    keySwitch->setHomePosition( bs.center()+osg::Vec3(0.0f, 0.0f, bs.radius()), bs.center(), osg::Y_AXIS );
    
    osgViewer::Viewer viewer;
    viewer.setCameraManipulator( keySwitch.get() );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
