#include <osg/io_utils>
#include <osg/LightModel>
#include <osg/Depth>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/TextureCubeMap>
#include <osg/ShapeDrawable>
#include <osg/ClearNode>
#include <osg/LightSource>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgUtil/CullVisitor>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ViewDependentShadowMap>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "EffectCompositor"

extern void configureViewerForMode( osgViewer::Viewer& viewer, osgFX::EffectCompositor* compositor,
                                    osg::Node* model, int displayMode );

#ifdef HAVE_SILVERLINING

/* The SilverLining skybox */
#include "SilverLiningNode.h"

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

osg::Node* createSkyBox()
{
    return new MySilverLiningNode( "Your user name", "Your license code" );
}

#else

osg::Node* createSkyBox()
{
    return new osg::Node;
}

#endif

/* Shadowed scene */

#define SHADOW_RECEIVE_MASK 0x1
#define SHADOW_CAST_MASK 0x2
osg::Group* createShadowedScene( osg::Node* scene )
{
    osg::ref_ptr<osgShadow::ShadowSettings> settings = new osgShadow::ShadowSettings;
    //settings->setShadowMapTechniqueHints( osgShadow::ShadowSettings::PARALLEL_SPLIT|osgShadow::ShadowSettings::PERSPECTIVE );
    settings->setShaderHint( osgShadow::ShadowSettings::PROVIDE_FRAGMENT_SHADER );
	settings->setTextureSize( osg::Vec2s(2048, 2048) );
    //settings->setNumShadowMapsPerLight( 3 );
    
    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
	shadowedScene->setReceivesShadowTraversalMask( SHADOW_RECEIVE_MASK );
	shadowedScene->setCastsShadowTraversalMask( SHADOW_CAST_MASK );
	shadowedScene->setShadowTechnique( new osgShadow::ViewDependentShadowMap );
	shadowedScene->setShadowSettings( settings.get() );
    shadowedScene->addChild( scene );
    
    osg::ref_ptr<osg::LightModel> lm = new osg::LightModel;
    lm->setAmbientIntensity( osg::Vec4(0.55f, 0.6f, 0.71f, 1.0f) );
    shadowedScene->getOrCreateStateSet()->setAttributeAndModes( lm.get() );
    
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
    ls->getLight()->setLightNum( 0 );
	ls->getLight()->setPosition( osg::Vec4(0.5f, 0.5f, 0.5f, 0.0f) );
	ls->getLight()->setAmbient( osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f) );
	ls->getLight()->setDiffuse( osg::Vec4(0.49f, 0.465f, 0.494f, 1.0f) );
    ls->getLight()->setSpecular( osg::Vec4(1.0f, 0.98f, 0.95f, 1.0f) );
    shadowedScene->addChild( ls.get() );
    return shadowedScene.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    int displayMode = 1;
    if ( arguments.read("--simple-mode") ) displayMode = 0;
    else if ( arguments.read("--analysis-mode") ) displayMode = 1;
    else if ( arguments.read("--compare-mode") ) displayMode = 2;
    else if ( arguments.read("--multiview-mode") ) displayMode = 3;
    
    std::string effectFile("test.xml");
    arguments.read( "--effect", effectFile );
    
    bool shadowed = false;
    if ( arguments.read("--shadowed") ) shadowed = true;
    
    float lodscale = 1.0f;
    arguments.read( "--lod-scale", lodscale );
    
    // Create the scene
    osg::Node* model = osgDB::readNodeFiles( arguments );
    if ( !model ) model = osgDB::readNodeFile( "lz.osg" );
    
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    scene->addChild( createSkyBox() );
    scene->addChild( shadowed ? createShadowedScene(model) : model );
    
    // Create the effect compositor from XML file
#ifdef HAVE_SILVERLINING
    osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlFile( effectFile );
    if ( !xmlRoot )
    {
        OSG_WARN << "Effect file " << effectFile << " can't be loaded!" << std::endl;
        return 1;
    }
    
    // FIXME: here we use PIXEL_BUFFER instead, because SilverLining is uncomfortable with
    // default FBO settings. I'm not sure if this is a SilverLining bug or mine
    osgFX::EffectCompositor::XmlTemplateMap templateMap;
    osgFX::EffectCompositor* compositor = new osgFX::EffectCompositor;
    compositor->setRenderTargetImplementation( osg::Camera::PIXEL_BUFFER );
    compositor->loadFromXML( xmlRoot.get(), templateMap, NULL );
#else
    osgFX::EffectCompositor* compositor = osgFX::readEffectFile( effectFile );
    if ( !compositor )
    {
        OSG_WARN << "Effect file " << effectFile << " can't be loaded!" << std::endl;
        return 1;
    }
#endif
    
    // For the fastest and simplest effect use, this is enough!
    compositor->addChild( scene.get() );
    
    // Add all to the root node of the viewer
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( compositor );
    
    osgViewer::Viewer viewer;
    viewer.getCamera()->setLODScale( lodscale );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
    
    if ( displayMode>0 )
        configureViewerForMode( viewer, compositor, scene.get(), displayMode );
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}