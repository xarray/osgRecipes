#include <osg/io_utils>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgUtil/CullVisitor>
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
                        silverLining->dirtyBound();
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
        addDrawable( _cloud.get() );
        
        AtmosphereUpdater* updater = new AtmosphereUpdater;
        setUpdateCallback( updater );
        setCullCallback( updater );
        setCullingActive( false );
        getOrCreateStateSet()->setRenderBinDetails( 100, "RenderBin" );
        _atmosphere = new SilverLining::Atmosphere( "Your user name", "Your license code" );
        
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
    
    SilverLining::Atmosphere* atmosphere() { return _atmosphere; }
    const SilverLining::Atmosphere* atmosphere() const { return _atmosphere; }
    bool isAtmosphereValid() const { return _initialized; }
    
    void setResourcePath( const std::string& path ) { _resourcePath = path; }
    const std::string& getResourcePath() const { return _resourcePath; }
    
    void setAtmosphereLight( osg::Light* l ) { _light = l; }
    osg::Light* getAtmosphereLight() { return _light.get(); }
    const osg::Light* getAtmosphereLight() const { return _light.get(); }
    
    void setCameraPosition( const osg::Vec3d& pos ) { _cameraPos = pos; }
    const osg::Vec3d& getCameraPosition() const { return _cameraPos; }
    
    virtual osg::BoundingSphere computeBound() const
    {
        osg::BoundingSphere bs;
        if ( !_initialized ) return bs;
        
        // Check sky bound box
        osg::BoundingBox skyBoundBox;
        {
            double skyboxSize = _atmosphere->GetConfigOptionDouble("sky-box-size");
            if ( skyboxSize==0.0 ) skyboxSize = 1000.0;
            
            osg::Vec3d radiusVec = osg::Vec3d(skyboxSize, skyboxSize, skyboxSize) * 0.5;
            skyBoundBox.set( _cameraPos-radiusVec, _cameraPos+radiusVec );
            
            bool hasLimb = _atmosphere->GetConfigOptionBoolean("enable-atmosphere-from-space");
            if ( hasLimb )
            {
                // Compute bounds of atmospheric limb centered at (0,0,0)
                double earthRadius = _atmosphere->GetConfigOptionDouble("earth-radius-meters");
                double atmosphereHeight = earthRadius + _atmosphere->GetConfigOptionDouble("atmosphere-height");
                double atmosphereThickness = _atmosphere->GetConfigOptionDouble("atmosphere-scale-height-meters") + earthRadius;
                
                osg::BoundingBox atmosphereBox;
                osg::Vec3d atmMin(-atmosphereThickness, -atmosphereThickness, -atmosphereThickness);
                osg::Vec3d atmMax(atmosphereThickness, atmosphereThickness, atmosphereThickness);
                atmosphereBox.set( atmMin, atmMax );
                skyBoundBox.expandBy( atmosphereBox );
            }
            std::cout << "Sky: " << skyBoundBox.center() << ": " << skyBoundBox.radius() << std::endl;
        }
        
        // Check cloud bound box
        osg::BoundingBox cloudBoundBox;
        {
            double minX, minY, minZ, maxX, maxY, maxZ;
            _atmosphere->GetCloudBounds( minX, minY, minZ, maxX, maxY, maxZ );
            cloudBoundBox.set( osg::Vec3d(minX, minY, minZ), osg::Vec3d(maxX, maxY, maxZ) );
            std::cout << "Cloud: " << cloudBoundBox.center() << ": " << cloudBoundBox.radius() << std::endl;
        }
        
        bs.expandBy( skyBoundBox );
        bs.expandBy( cloudBoundBox );
        return bs;
    }
    
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
        t.SetHour( 12.0 );
        t.SetTimeZone( PST );
        _atmosphere->GetConditions()->SetTime( t );
        
        // Create cloud layers
        SilverLining::CloudLayer* cumulusCongestusLayer;
        cumulusCongestusLayer = SilverLining::CloudLayerFactory::Create(CUMULUS_CONGESTUS);
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
        _atmosphere->GetConditions()->AddCloudLayer( cumulusCongestusLayer );
    }
    
public:
    bool initializeSilverLining( osg::RenderInfo& renderInfo )
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
    
    void updateGlobalLight()
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
    
    osg::observer_ptr<SkyDrawable> _sky;
    osg::observer_ptr<CloudDrawable> _cloud;
    osg::observer_ptr<osg::Light> _light;
    SilverLining::Atmosphere* _atmosphere;
    std::string _resourcePath;
    osg::Vec3d _cameraPos;
    bool _initialized;
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    osg::Node* model = osgDB::readNodeFiles( arguments );
    if ( !model ) model = osgDB::readNodeFile( "lz.osg" );
    
    osg::ref_ptr<SilverLiningNode> silverLining = new SilverLiningNode;
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( model );
    scene->addChild( silverLining.get() );
    
    osgViewer::Viewer viewer;
    viewer.getCamera()->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );  // FIXME: want auto compute
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}
