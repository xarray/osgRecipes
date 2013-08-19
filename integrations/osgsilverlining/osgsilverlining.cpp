#include <osg/io_utils>
#include <osg/Depth>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/LightModel>
#include <osg/LightSource>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ViewDependentShadowMap>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <iostream>
#include "SilverLiningNode.h"

// Create your own SilverLining envrionment class
class MySilverLiningNode : public SilverLiningNode
{
public:
    MySilverLiningNode( const char* licenseUser, const char* licenseKey )
    : SilverLiningNode(licenseUser, licenseKey) {}
    
    virtual void createAtmosphereData( osg::RenderInfo& renderInfo )
    {
        // Set our location (change this to your own latitude and longitude)
        SilverLining::Location loc;
        loc.SetAltitude( 0.0 );
        loc.SetLatitude( 45.0 );
        loc.SetLongitude( -122.0 );
        _atmosphere->GetConditions()->SetLocation( loc );
        
        // Set the time to noon in PST
        SilverLining::LocalTime t;
        t.SetFromSystemTime();
        t.SetHour( 15.0 );
        t.SetTimeZone( PST );
        _atmosphere->GetConditions()->SetTime( t );
        
        // Create cloud layers
        SilverLining::CloudLayer* cumulusCongestusLayer =
            SilverLining::CloudLayerFactory::Create( CUMULUS_CONGESTUS );
        //cumulusCongestusLayer->SetIsInfinite( true );
        cumulusCongestusLayer->SetBaseAltitude( 1500 );
        cumulusCongestusLayer->SetThickness( 200 );
        cumulusCongestusLayer->SetBaseLength( 40000 );
        cumulusCongestusLayer->SetBaseWidth( 40000 );
        cumulusCongestusLayer->SetDensity( 0.3 );
        
        // Note, we pass in X and -Y since this accepts "east" and "south" coordinates.
        cumulusCongestusLayer->SetLayerPosition( _cameraPos.x(), -_cameraPos.y() );
        cumulusCongestusLayer->SeedClouds( *_atmosphere );
        cumulusCongestusLayer->GenerateShadowMaps( false );
        atmosphere()->GetConditions()->AddCloudLayer( cumulusCongestusLayer );
    }
};

class SilverLiningTester : public osgGA::GUIEventHandler
{
public:
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( ea.getEventType()==osgGA::GUIEventAdapter::KEYUP )
        {
            SilverLining::Atmosphere* atmosphere = _silverLining->atmosphere();
            switch ( ea.getKey() )
            {
            case '0':  // clear
                atmosphere->GetConditions()->SetPrecipitation( SilverLining::CloudLayer::NONE, 0 );
                break;
            case '1':  // rain
                atmosphere->GetConditions()->SetPrecipitation( SilverLining::CloudLayer::RAIN, 10 );
                break;
            case '2':  // wet snow
                atmosphere->GetConditions()->SetPrecipitation( SilverLining::CloudLayer::WET_SNOW, 10 );
                break;
            case '3':  // dry snow
                atmosphere->GetConditions()->SetPrecipitation( SilverLining::CloudLayer::DRY_SNOW, 10 );
                break;
            case '4':  // sleet
                atmosphere->GetConditions()->SetPrecipitation( SilverLining::CloudLayer::SLEET, 10 );
                break;
            default: break;
            }
        }
        return false;
    }
    
    SilverLiningTester( SilverLiningNode* sn ) { _silverLining = sn; }
    SilverLiningNode* _silverLining;
};

#define SHADOW_RECEIVE_MASK 0x1
#define SHADOW_CAST_MASK 0x2
osg::Group* createShadowedScene( osg::Node* scene, osg::LightSource* ls )
{
    osg::ref_ptr<osgShadow::ShadowSettings> settings = new osgShadow::ShadowSettings;
    settings->setBaseShadowTextureUnit( 2 );
    settings->setReceivesShadowTraversalMask( SHADOW_RECEIVE_MASK );
	settings->setCastsShadowTraversalMask( SHADOW_CAST_MASK );
    settings->setShaderHint( osgShadow::ShadowSettings::PROVIDE_FRAGMENT_SHADER );
    settings->setTextureSize( osg::Vec2s(2048, 2048) );
#if 0
    settings->setShadowMapTechniqueHints( osgShadow::ShadowSettings::PARALLEL_SPLIT|osgShadow::ShadowSettings::PERSPECTIVE );
    settings->setNumShadowMapsPerLight( 3 );
#endif
    
    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
	shadowedScene->setShadowTechnique( new osgShadow::ViewDependentShadowMap );
	shadowedScene->setShadowSettings( settings.get() );
    shadowedScene->addChild( scene );
    
    osg::ref_ptr<osg::LightModel> lm = new osg::LightModel;
    lm->setAmbientIntensity( osg::Vec4(0.55f, 0.6f, 0.71f, 1.0f) );
    shadowedScene->getOrCreateStateSet()->setAttributeAndModes( lm.get() );
    shadowedScene->addChild( ls );
    return shadowedScene.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    bool shadowed = false;
    if ( arguments.read("--shadowed") ) shadowed = true;
    
    osg::Node* model = osgDB::readNodeFiles( arguments );
    if ( !model ) model = osgDB::readNodeFile( "lz.osg" );
    
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
    ls->getLight()->setLightNum( 0 );
	ls->getLight()->setPosition( osg::Vec4(0.5f, 0.5f, 0.5f, 0.0f) );
	ls->getLight()->setAmbient( osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f) );
	ls->getLight()->setDiffuse( osg::Vec4(0.49f, 0.465f, 0.494f, 1.0f) );
    ls->getLight()->setSpecular( osg::Vec4(1.0f, 0.98f, 0.95f, 1.0f) );
    
    osg::ref_ptr<SilverLiningNode> silverLining = new MySilverLiningNode( "Your user name", "Your license code" );
    silverLining->setGlobalLight( ls->getLight() );  // The light will be changed on the fly by SilverLining
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( shadowed ? createShadowedScene(model, ls.get()) : model );
    scene->addChild( silverLining.get() );
    
    osgViewer::Viewer viewer;
#if 0
    // With correct sky/cloud bound, we don't need to disable near/far computation now
    viewer.getCamera()->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );
#endif
    viewer.getCamera()->setClearMask( GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT );  // No need for OSG to clear the color buffer now
    viewer.addEventHandler( new SilverLiningTester(silverLining.get()) );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}
