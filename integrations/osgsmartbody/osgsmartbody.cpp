#include <osg/io_utils>
#include <osg/Depth>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/LightModel>
#include <osg/LightSource>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ViewDependentShadowMap>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <iostream>

#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPython.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBSceneListener.h>

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    scene->setMediaPath( "./smartbody/data/" );
    scene->addAssetPath( "motion", "ChrBrad" );
    scene->loadAssets();
    
    int numMotions = scene->getNumMotions();
    std::cout << "Loaded motions: " << numMotions << std::endl;
    
    SmartBody::SBCharacter* character = scene->createCharacter( "mycharacter", "" );
    SmartBody::SBSkeleton* skeleton = scene->createSkeleton( "ChrBrad.sk" );
    character->setSkeleton( skeleton );
    character->createStandardControllers();
    
    SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
    sim->setupTimer();
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    
    osgViewer::Viewer viewer;
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
    viewer.setUpViewOnSingleScreen( 0 );
    
    scene->getBmlProcessor()->execBML( "mycharacter", "<body posture=\"ChrBrad@Idle01\"/>" );
    sim->start();
    while ( !viewer.done() )
    {
        scene->update();
        sim->updateTimer();
        viewer.frame();
    }
    sim->stop();
    return 0;
}
