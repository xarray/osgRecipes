#include <osg/io_utils>
#include <osg/Depth>
#include <osg/LightModel>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgUtil/CullVisitor>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ViewDependentShadowMap>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <iostream>

#include <SilverLining.h>

class SilverLiningNode : public osg::Geode
{
    class SkyDrawable : public osg::Drawable
    {
    public:
        virtual void drawImplementation( osg::RenderInfo& renderInfo ) const
        {
            renderInfo.getState()->disableAllVertexArrays();
            _silverLining->initializeSilverLining( renderInfo );
            _silverLining->atmosphere()->DrawSky( true, false, 0.0, true, false );
            renderInfo.getState()->dirtyAllVertexArrays();
        }
        
        virtual osg::BoundingBox computeBound() const
        {
            osg::BoundingBox skyBoundBox;
            if ( !_silverLining->isAtmosphereValid() ) return skyBoundBox;
            
            SilverLining::Atmosphere* atmosphere = _silverLining->atmosphere();
            double skyboxSize = atmosphere->GetConfigOptionDouble("sky-box-size");
            if ( skyboxSize==0.0 ) skyboxSize = 1000.0;
            
            osg::Vec3d radiusVec = osg::Vec3d(skyboxSize, skyboxSize, skyboxSize) * 0.5;
            const osg::Vec3d& camPos = _silverLining->getCameraPosition();
            skyBoundBox.set( camPos-radiusVec, camPos+radiusVec );
            
            bool hasLimb = atmosphere->GetConfigOptionBoolean("enable-atmosphere-from-space");
            if ( hasLimb )
            {
                // Compute bounds of atmospheric limb centered at (0,0,0)
                double earthRadius = atmosphere->GetConfigOptionDouble("earth-radius-meters");
                double atmosphereHeight = earthRadius + atmosphere->GetConfigOptionDouble("atmosphere-height");
                double atmosphereThickness = atmosphere->GetConfigOptionDouble("atmosphere-scale-height-meters") + earthRadius;
                
                osg::BoundingBox atmosphereBox;
                osg::Vec3d atmMin(-atmosphereThickness, -atmosphereThickness, -atmosphereThickness);
                osg::Vec3d atmMax(atmosphereThickness, atmosphereThickness, atmosphereThickness);
                atmosphereBox.set( atmMin, atmMax );
                skyBoundBox.expandBy( atmosphereBox );
            }
            return skyBoundBox;
        }
        
        SkyDrawable( SilverLiningNode* s=NULL ) { _silverLining = s; }
        SkyDrawable( const SkyDrawable& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
        : osg::Drawable(copy, copyop), _silverLining(copy._silverLining) {}
        META_Object( osg, SkyDrawable )
        
    protected:
        SilverLiningNode* _silverLining;
    };
    
    class CloudDrawable : public osg::Drawable
    {
    public:
        virtual void drawImplementation( osg::RenderInfo& renderInfo ) const
        {
            renderInfo.getState()->disableAllVertexArrays();
            _silverLining->initializeSilverLining( renderInfo );
            _silverLining->atmosphere()->DrawObjects( true, true, true );
            renderInfo.getState()->dirtyAllVertexArrays();
        }
        
        virtual osg::BoundingBox computeBound() const
        {
            osg::BoundingBox cloudBoundBox;
            if ( !_silverLining->isAtmosphereValid() ) return cloudBoundBox;
            
            double minX, minY, minZ, maxX, maxY, maxZ;
            _silverLining->atmosphere()->GetCloudBounds( minX, minY, minZ, maxX, maxY, maxZ );
            cloudBoundBox.set( osg::Vec3d(minX, minY, minZ), osg::Vec3d(maxX, maxY, maxZ) );
            return cloudBoundBox;
        }
        
        CloudDrawable( SilverLiningNode* s=NULL ) { _silverLining = s; }
        CloudDrawable( const CloudDrawable& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
        : osg::Drawable(copy, copyop), _silverLining(copy._silverLining) {}
        META_Object( osg, CloudDrawable )
        
    protected:
        SilverLiningNode* _silverLining;
    };
    
    struct AtmosphereUpdater : public osg::NodeCallback
    {
    public:
        virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
        {
            SilverLiningNode* silverLining = static_cast<SilverLiningNode*>( node );
            if ( silverLining ) 
            {
                if ( nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR )
                {
                    if ( silverLining->isAtmosphereValid() )
                    {
                        silverLining->atmosphere()->UpdateSkyAndClouds();
                        silverLining->updateGlobalLight();
                        silverLining->skyDrawable()->dirtyBound();
                        silverLining->cloudDrawable()->dirtyBound();
                    }
                }
                else if ( nv->getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
                {
                    silverLining->setCameraPosition( nv->getEyePoint() );
                    if ( silverLining->isAtmosphereValid() )
                    {
                        osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>( nv );
                        silverLining->atmosphere()->SetCameraMatrix( cv->getModelViewMatrix()->ptr() );
                        silverLining->atmosphere()->SetProjectionMatrix( cv->getProjectionMatrix()->ptr() );
                        silverLining->atmosphere()->CullObjects();
                    }
                }
            }
            traverse( node, nv );
        }
    };
    
    struct AlwaysKeepCallback : public osg::Drawable::CullCallback
    {
        virtual bool cull( osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo ) const
        { return false; }
    };
    
public:
    SilverLiningNode()
    :   _initialized(false)
    {
        _sky = new SkyDrawable(this);
        _sky->setUseVertexBufferObjects( false );
        _sky->setUseDisplayList( false );
        _sky->getOrCreateStateSet()->setRenderBinDetails( 98, "RenderBin" );
        _sky->getOrCreateStateSet()->setAttributeAndModes(
            new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false) );
        addDrawable( _sky.get() );
        
        _cloud = new CloudDrawable(this);
        _cloud->setUseVertexBufferObjects( false );
        _cloud->setUseDisplayList( false );
        _cloud->getOrCreateStateSet()->setRenderBinDetails( 99, "RenderBin" );
        _cloud->setCullCallback( new AlwaysKeepCallback );  // This seems to avoid cloud to twinkle sometimes
        addDrawable( _cloud.get() );
        
        AtmosphereUpdater* updater = new AtmosphereUpdater;
        setUpdateCallback( updater );
        setCullCallback( updater );
        setCullingActive( false );
        getOrCreateStateSet()->setRenderBinDetails( 100, "RenderBin" );
        
        _atmosphere = new SilverLining::Atmosphere( "Your user name", "Your license code" );
        _atmosphere->DisableFarCulling( true );
        _atmosphere->EnableLensFlare( true );
        
        const char* slPath = getenv( "SILVERLINING_PATH" );
        if ( slPath )
            _resourcePath = osgDB::convertFileNameToNativeStyle(std::string(slPath) + "/resources/");
    }
    
    SilverLiningNode( const SilverLiningNode& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
    :   osg::Geode(copy, copyop), _sky(copy._sky), _cloud(copy._cloud), _light(copy._light),
        _atmosphere(copy._atmosphere), _resourcePath(copy._resourcePath),
        _cameraPos(copy._cameraPos), _initialized(copy._initialized)
    {}
    
    META_Node( osg, SilverLiningNode )
    
    osg::Drawable* skyDrawable() { return _sky.get(); }
    const osg::Drawable* skyDrawable() const { return _sky.get(); }
    osg::Drawable* cloudDrawable() { return _cloud.get(); }
    const osg::Drawable* cloudDrawable() const { return _cloud.get(); }
    
    unsigned int getNumCloudLayers() const { return _managedCloudLayers.size(); }
    SilverLining::CloudLayer* getCloudLayer( unsigned int id )
    {
        SilverLining::CloudLayer* layer = NULL;
        _atmosphere->GetConditions()->GetCloudLayer( _managedCloudLayers[id], &layer );
        return layer;
    }
    
    SilverLining::Atmosphere* atmosphere() { return _atmosphere; }
    const SilverLining::Atmosphere* atmosphere() const { return _atmosphere; }
    bool isAtmosphereValid() const { return _initialized; }
    
    void setResourcePath( const std::string& path ) { _resourcePath = path; }
    const std::string& getResourcePath() const { return _resourcePath; }
    
    void setGlobalLight( osg::Light* l ) { _light = l; }
    osg::Light* getGlobalLight() { return _light.get(); }
    const osg::Light* getGlobalLight() const { return _light.get(); }
    
    void setCameraPosition( const osg::Vec3d& pos ) { _cameraPos = pos; }
    const osg::Vec3d& getCameraPosition() const { return _cameraPos; }
    
    // Derive this method to create your atmosphere data at creation
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
            SilverLining::CloudLayerFactory::Create( CUMULUS_CONGESTUS/*CUMULONIMBUS_CAPPILATUS*/ );
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
        _managedCloudLayers.push_back( _atmosphere->GetConditions()->AddCloudLayer(cumulusCongestusLayer) );
    }
    
    virtual bool initializeSilverLining( osg::RenderInfo& renderInfo )
    {
        if ( _initialized ) return true;
        srand( 1234 ); // constant random seed to ensure consistent clouds across windows
        
        int result = _atmosphere->Initialize(
            SilverLining::Atmosphere::OPENGL, _resourcePath.c_str(), true, 0 );
        if ( result!=SilverLining::Atmosphere::E_NOERROR )
        {
            std::cout << "SilverLining failed to initialize: " << result << std::endl;
            return false;
        }
        
        _initialized = true;
        _atmosphere->SetUpVector( 0.0, 0.0, 1.0 );
        _atmosphere->SetRightVector( 1.0, 0.0, 0.0 );
        createAtmosphereData( renderInfo );
        return true;
    }
    
    virtual void updateGlobalLight()
    {
        if ( _initialized && _light.valid() )
        {
            float ra, ga, ba, rd, gd, bd, x, y, z;
            _atmosphere->GetAmbientColor( &ra, &ga, &ba );
            _atmosphere->GetSunOrMoonColor( &rd, &gd, &bd );
            _atmosphere->GetSunOrMoonPosition( &x, &y, &z );
            
            osg::Vec3 direction(x, y, z);
            direction.normalize();
            _light->setAmbient( osg::Vec4(ra, ga, ba, 1.0f) );
            _light->setDiffuse( osg::Vec4(rd, gd, bd, 1.0f) );
            _light->setPosition( osg::Vec4(direction, 0.0f) );
        }
    }
    
protected:
    virtual ~SilverLiningNode()
    { delete _atmosphere; }
    
    std::vector<int> _managedCloudLayers;
    osg::observer_ptr<SkyDrawable> _sky;
    osg::observer_ptr<CloudDrawable> _cloud;
    osg::observer_ptr<osg::Light> _light;
    SilverLining::Atmosphere* _atmosphere;
    std::string _resourcePath;
    osg::Vec3d _cameraPos;
    bool _initialized;
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
            case '5':  // Force a lightning
                {
                    // Note: you must use cloud of the type CUMULONIMBUS_CAPPILATUS to enable lightning
                    SilverLining::CloudLayer* layer = _silverLining->getCloudLayer(0);
                    if ( layer ) layer->ForceLightning();
                }
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
    settings->setShadowMapTechniqueHints( osgShadow::ShadowSettings::PARALLEL_SPLIT|osgShadow::ShadowSettings::PERSPECTIVE );
    settings->setShaderHint( osgShadow::ShadowSettings::PROVIDE_FRAGMENT_SHADER );
	settings->setTextureSize( osg::Vec2s(2048, 2048) );
    settings->setNumShadowMapsPerLight( 3 );
    settings->setMaximumShadowMapNearFarDistance( 5000.0 );
    
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
    
    osg::ref_ptr<SilverLiningNode> silverLining = new SilverLiningNode;
    silverLining->setGlobalLight( ls->getLight() );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( shadowed ? createShadowedScene(model, ls.get()) : model );
    scene->addChild( silverLining.get() );
    
    osgViewer::Viewer viewer;
#if 0
    // FIXME: In fact it performs better with fixed near/far at present.
    // The sky/cloud may flicker if we move the camera quickly, don't know the reason till now...
    viewer.getCamera()->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );
#endif
    viewer.addEventHandler( new SilverLiningTester(silverLining.get()) );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}
