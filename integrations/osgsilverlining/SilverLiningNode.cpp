#include <osg/Depth>
#include <osgDB/FileNameUtils>
#include <osgUtil/CullVisitor>
#include "SilverLiningNode.h"

/* SilverLiningNode::SkyDrawable */

SilverLiningNode::SkyDrawable::SkyDrawable( SilverLiningNode* s )
:   _silverLining(s)
{}

SilverLiningNode::SkyDrawable::SkyDrawable( const SkyDrawable& copy, const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop), _silverLining(copy._silverLining)
{}

void SilverLiningNode::SkyDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    renderInfo.getState()->disableAllVertexArrays();
    _silverLining->initializeSilverLining( renderInfo );
    _silverLining->atmosphere()->DrawSky( true, false, 0.0, true, false );
    renderInfo.getState()->dirtyAllVertexArrays();
}

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
osg::BoundingBox SilverLiningNode::SkyDrawable::computeBoundingBox() const
#else
osg::BoundingBox SilverLiningNode::SkyDrawable::computeBound() const
#endif
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

/* SilverLiningNode::CloudDrawable */

SilverLiningNode::CloudDrawable::CloudDrawable( SilverLiningNode* s )
:   _silverLining(s)
{}

SilverLiningNode::CloudDrawable::CloudDrawable( const CloudDrawable& copy, const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop), _silverLining(copy._silverLining)
{}

SilverLiningNode::~SilverLiningNode()
{ delete _atmosphere; }

void SilverLiningNode::CloudDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    renderInfo.getState()->disableAllVertexArrays();
    _silverLining->atmosphere()->DrawObjects( true, true, true, 1.0f );
    renderInfo.getState()->dirtyAllVertexArrays();
    
    /*void* shadowTexID = 0;
    SilverLining::Matrix4 lightMatrix, shadowMatrix;
    if ( _silverLining->atmosphere()->GetShadowMap(shadowTexID, &lightMatrix, &shadowMatrix) )
    {
    }*/
}

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
osg::BoundingBox SilverLiningNode::CloudDrawable::computeBoundingBox() const
#else
osg::BoundingBox SilverLiningNode::CloudDrawable::computeBound() const
#endif
{
    osg::BoundingBox cloudBoundBox;
    if ( !_silverLining->isAtmosphereValid() ) return cloudBoundBox;
    
    double minX, minY, minZ, maxX, maxY, maxZ;
    _silverLining->atmosphere()->GetCloudBounds( minX, minY, minZ, maxX, maxY, maxZ );
    cloudBoundBox.set( osg::Vec3d(minX, minY, minZ), osg::Vec3d(maxX, maxY, maxZ) );
    return cloudBoundBox;
}

/* SilverLiningNode::AtmosphereUpdater */

void SilverLiningNode::AtmosphereUpdater::operator()( osg::Node* node, osg::NodeVisitor* nv )
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

/* SilverLiningNode */

SilverLiningNode::SilverLiningNode( const char* licenseUser, const char* licenseKey )
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
    
    _atmosphere = new SilverLining::Atmosphere( licenseUser, licenseKey );
    _atmosphere->DisableFarCulling( true );
    _atmosphere->EnableLensFlare( true );
    
    const char* slPath = getenv( "SILVERLINING_PATH" );
    if ( slPath )
        _resourcePath = osgDB::convertFileNameToNativeStyle(std::string(slPath) + "/resources/");
}

SilverLiningNode::SilverLiningNode( const SilverLiningNode& copy, const osg::CopyOp& copyop )
:   osg::Geode(copy, copyop), _sky(copy._sky), _cloud(copy._cloud), _light(copy._light),
    _atmosphere(copy._atmosphere), _resourcePath(copy._resourcePath),
    _cameraPos(copy._cameraPos), _initialized(copy._initialized)
{}

bool SilverLiningNode::initializeSilverLining( osg::RenderInfo& renderInfo )
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

void SilverLiningNode::updateGlobalLight()
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
