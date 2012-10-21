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

/* The skybox */

struct TexMatCallback : public osg::NodeCallback
{
public:
    osg::TexMat& _texMat;
    TexMatCallback( osg::TexMat& tm ) : _texMat(tm) {}
    
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if ( cv )
        {
            const osg::Matrix& MV = *(cv->getModelViewMatrix());
            const osg::Matrix R = osg::Matrix::rotate(osg::DegreesToRadians(112.0f), 0.0f, 0.0f, 1.0f) *
                                  osg::Matrix::rotate(osg::DegreesToRadians(90.0f), 1.0f, 0.0f, 0.0f);
            const osg::Matrix C = osg::Matrix::rotate( MV.getRotate().inverse() );
            _texMat.setMatrix( C*R );
        }
        traverse(node,nv);
    }
};

class MoveEarthySkyWithEyePointTransform : public osg::Transform
{
public:
    virtual bool computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const 
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if ( cv )
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.preMultTranslate( eyePointLocal );
        }
        return true;
    }
    
    virtual bool computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if ( cv )
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.postMultTranslate( -eyePointLocal );
        }
        return true;
    }
};

#define CUBEMAP_FILENAME(face) "Cubemap_snow/" #face ".jpg"
osg::Node* createSkyBox()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(), 1.0f)) );
    geode->setCullingActive( false );
    
    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->setRenderBinDetails( -1,"RenderBin" );
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    ss->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
    ss->setAttributeAndModes( new osg::Depth(osg::Depth::ALWAYS, 1.0, 1.0) );
    ss->setTextureAttributeAndModes( 0, new osg::TexEnv(osg::TexEnv::REPLACE) );
    
    osg::ref_ptr<osg::TexGen> tg = new osg::TexGen;
    tg->setMode( osg::TexGen::NORMAL_MAP );
    ss->setTextureAttributeAndModes( 0, tg.get() );
    
    osg::ref_ptr<osg::TexMat> tm = new osg::TexMat;
    ss->setTextureAttribute( 0, tm.get() );
    
    osg::ref_ptr<osg::TextureCubeMap> cubemap = new osg::TextureCubeMap;
    cubemap->setImage( osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile(CUBEMAP_FILENAME(posx)) );
    cubemap->setImage( osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile(CUBEMAP_FILENAME(negx)) );
    cubemap->setImage( osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile(CUBEMAP_FILENAME(posy)) );
    cubemap->setImage( osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile(CUBEMAP_FILENAME(negy)) );
    cubemap->setImage( osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile(CUBEMAP_FILENAME(posz)) );
    cubemap->setImage( osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile(CUBEMAP_FILENAME(negz)) );
    cubemap->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    cubemap->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
    cubemap->setWrap( osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE );
    cubemap->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    cubemap->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    ss->setTextureAttributeAndModes( 0, cubemap.get() );
    
    osg::ref_ptr<osg::Transform> transform = new MoveEarthySkyWithEyePointTransform;
    transform->setCullingActive( false );
    transform->addChild( geode.get() );
    
    osg::ref_ptr<osg::ClearNode> clearNode = new osg::ClearNode;
    clearNode->setCullCallback( new TexMatCallback(*tm) );
    clearNode->addChild( transform.get() );
    return clearNode.release();
}

/* Shadowed scene */

#define SHADOW_RECEIVE_MASK 0x1
#define SHADOW_CAST_MASK 0x2
osg::Group* createShadowedScene( osg::Node* scene )
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
    osgFX::EffectCompositor* compositor = osgFX::readEffectFile( effectFile );
    if ( !compositor )
    {
        OSG_WARN << "Effect file " << effectFile << " can't be loaded!" << std::endl;
        return 1;
    }
    
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