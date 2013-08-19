#include <osg/io_utils>
#include <osg/Depth>
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
#include "TritonNode.h"

#ifdef USE_SILVERLINING_SKY

#include "../osgsilverlining/SilverLiningNode.h"

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
        cumulusCongestusLayer->SetIsInfinite( true );
        cumulusCongestusLayer->SetBaseAltitude( 500 );
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

#endif

class TritonTester : public osgGA::GUIEventHandler
{
public:
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( ea.getEventType()==osgGA::GUIEventAdapter::KEYUP )
        {
            switch ( ea.getKey() )
            {
            case '1':  // add wind
                {
                    Triton::WindFetch wf;
                    wf.SetWind( 4.0, 0.0 );
                    if ( _triton->environment() )
                        _triton->environment()->AddWindFetch( wf );
                }
                break;
            case '2':
                {
                    if ( _testImpact ) delete _testImpact;
                    _testImpact = new Triton::Impact(_triton->ocean(), 5.0, 200.0, true);
                    _testImpact->Trigger(Triton::Vector3(0.0f, 0.0f, 100.0f), Triton::Vector3(0.0f, 0.0f,-1.0f),
                                         500.0, 0.01);
                }
                break;
            case '0':  // clear winds
                if ( _triton->environment() )
                    _triton->environment()->ClearWindFetches();
                if ( _testImpact ) delete _testImpact;
                break;
            default: break;
            }
        }
        return false;
    }
    
    TritonTester( TritonNode* tn ) { _triton = tn; _testImpact = NULL; }
    TritonNode* _triton;
    Triton::Impact* _testImpact;
};

#define SHADOW_RECEIVE_MASK 0x1
#define SHADOW_CAST_MASK 0x2
osg::Group* createShadowedScene( osg::Node* scene, osg::LightSource* ls )
{
    osg::ref_ptr<osgShadow::ShadowSettings> settings = new osgShadow::ShadowSettings;
    settings->setShaderHint( osgShadow::ShadowSettings::PROVIDE_FRAGMENT_SHADER );
    settings->setTextureSize( osg::Vec2s(2048, 2048) );
#if 0
    settings->setShadowMapTechniqueHints( osgShadow::ShadowSettings::PARALLEL_SPLIT|osgShadow::ShadowSettings::PERSPECTIVE );
    settings->setNumShadowMapsPerLight( 3 );
#endif
    
    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
	shadowedScene->setReceivesShadowTraversalMask( SHADOW_RECEIVE_MASK );
	shadowedScene->setCastsShadowTraversalMask( SHADOW_CAST_MASK );
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
    
    bool shadowed = false, usePatch = false;
    if ( arguments.read("--shadowed") ) shadowed = true;
    if ( arguments.read("--ocean-patch") ) usePatch = true;
    
    osg::Node* model = osgDB::readNodeFiles( arguments );
    if ( !model ) model = osgDB::readNodeFile( "cessna.osg.0,0,50.trans" );
    
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
    ls->getLight()->setLightNum( 0 );
	ls->getLight()->setPosition( osg::Vec4(0.5f, 0.5f, 0.5f, 0.0f) );
	ls->getLight()->setAmbient( osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f) );
	ls->getLight()->setDiffuse( osg::Vec4(0.49f, 0.465f, 0.494f, 1.0f) );
    ls->getLight()->setSpecular( osg::Vec4(1.0f, 0.98f, 0.95f, 1.0f) );
    
    osg::ref_ptr<TritonNode> triton = new TritonNode( "Your user name", "Your license code" );
    triton->setGlobalLight( ls->getLight() );
    if ( usePatch )
    {
        osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array;
        va->push_back( osg::Vec3(-100.0f,-100.0f, 0.0f) );
        va->push_back( osg::Vec3( 100.0f,-100.0f, 0.0f) );
        va->push_back( osg::Vec3( 100.0f, 100.0f, 0.0f) );
        va->push_back( osg::Vec3(-100.0f, 100.0f, 0.0f) );
        
        osg::ref_ptr<osg::DrawElementsUInt> de = new osg::DrawElementsUInt(GL_QUADS);
        de->push_back( 0 );
        de->push_back( 1 );
        de->push_back( 2 );
        de->push_back( 3 );
        
        triton->addOceanMesh( va.get(), de.get() );
    }
    else
        triton->setCullingActive( false );  // Use default infinite ocean surface so disable culling
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( triton.get() );
    scene->addChild( shadowed ? createShadowedScene(model, ls.get()) : model );
    
#ifdef USE_SILVERLINING_SKY
    osg::ref_ptr<SilverLiningNode> silverLining = new MySilverLiningNode( "Your user name", "Your license code" );
    silverLining->setGlobalLight( ls->getLight() );  // The light will be changed on the fly by SilverLining
    scene->addChild( silverLining.get() );
    
    // Set the atmosphere to Triton so we can handle environment map
    triton->setAtmosphere( silverLining->atmosphere() );
#endif
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new TritonTester(triton.get()) );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}
