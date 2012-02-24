/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 2
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ViewDependentShadowMap>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "CommonFunctions"

int main( int argc, char** argv )
{
    unsigned int rcvShadowMask = 0x1;
    unsigned int castShadowMask = 0x2;
    
    // Set the ground (only receives shadow)
    osg::ref_ptr<osg::MatrixTransform> groundNode = new osg::MatrixTransform;
    groundNode->addChild( osgDB::readNodeFile("lz.osg") );
    groundNode->setMatrix( osg::Matrix::translate(200.0f, 200.0f,-200.0f) );
    groundNode->setNodeMask( rcvShadowMask );
    
    // Set the cessna (only casts shadow)
    osg::ref_ptr<osg::MatrixTransform> cessnaNode = new osg::MatrixTransform;
    cessnaNode->addChild( osgDB::readNodeFile("cessna.osg.0,0,90.rot") );
    cessnaNode->addUpdateCallback( osgCookBook::createAnimationPathCallback(50.0f, 6.0f) );
    cessnaNode->setNodeMask( castShadowMask );
    
    // Set shadow node
    osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
    //vdsm->setShadowMapProjectionHint( osgShadow::ViewDependentShadowMap::ORTHOGRAPHIC_SHADOW_MAP );
    //vdsm->setBaseShadowTextureUnit( 1 );
    
    osg::ref_ptr<osgShadow::ShadowedScene> shadowRoot = new osgShadow::ShadowedScene;
    shadowRoot->setShadowTechnique( vdsm.get() );
    shadowRoot->setReceivesShadowTraversalMask( rcvShadowMask );
    shadowRoot->setCastsShadowTraversalMask( castShadowMask );
    
    shadowRoot->addChild( groundNode.get() );
    for ( unsigned int i=0; i<10; ++i )
    {
        for ( unsigned int j=0; j<10; ++j )
        {
            osg::ref_ptr<osg::MatrixTransform> cessnaInstance = new osg::MatrixTransform;
            cessnaInstance->setMatrix( osg::Matrix::translate((float)i*50.0f, (float)j*50.0f, 0.0f) );
            cessnaInstance->addChild( cessnaNode.get() );
            shadowRoot->addChild( cessnaInstance.get() );
        }
    }
    
    const osg::BoundingSphere& bs = groundNode->getBound();
    osg::ref_ptr<osgGA::TrackballManipulator> trackball = new osgGA::TrackballManipulator;
    trackball->setHomePosition( bs.center()+osg::Vec3(0.0f, 0.0f, bs.radius()*0.4f), bs.center(), osg::Y_AXIS );
    
    osgViewer::Viewer viewer;
    viewer.setCameraManipulator( trackball.get() );
    viewer.setSceneData( shadowRoot.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
