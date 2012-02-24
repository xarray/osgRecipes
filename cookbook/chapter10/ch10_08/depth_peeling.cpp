/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 10 Recipe 8
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Camera>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/TexGenNode>
#include <osg/TexMat>
#include <osg/TextureRectangle>
#include <osgDB/ReadFile>
#include <osgUtil/CullVisitor>
#include <osgViewer/Viewer>

#include "CommonFunctions"

unsigned int g_width = 800, g_height = 600;
unsigned int g_offset = 8;
int g_unit = 1;

class CullCallback : public osg::NodeCallback
{
public:
    CullCallback( unsigned int unit, unsigned int off )
    : _unit(unit), _offset(off) {}
    
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osgUtil::CullVisitor* cullVisitor = static_cast<osgUtil::CullVisitor*>(nv);
        osgUtil::RenderStage* renderStage = cullVisitor->getCurrentRenderStage();
        const osg::Viewport* vp = renderStage->getViewport();
        if ( !vp ) return;
        
        osg::Matrixd m( *cullVisitor->getProjectionMatrix() );
        m.postMultTranslate( osg::Vec3d(1.0, 1.0, 1.0) );
        m.postMultScale( osg::Vec3d(0.5, 0.5, 0.5) );
        m.postMultScale( osg::Vec3d(vp->width(), vp->height(), 1.0) );
        m.postMultTranslate( osg::Vec3d(0.0, 0.0, -ldexp(double(_offset), -24.0)) );
        
        _stateset = new osg::StateSet;
        _stateset->setTextureAttribute( _unit, new osg::TexMat(m) );
        cullVisitor->pushStateSet( _stateset.get() );
        traverse( node, nv );
        cullVisitor->popStateSet();
    }
    
protected:
    osg::ref_ptr<osg::StateSet> _stateset;
    unsigned int _unit, _offset;
};

osg::Texture* createTexture( GLenum format, bool asMidLayer )
{
    osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle;
    texture->setTextureSize( g_width, g_height );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER );
    texture->setInternalFormat( format );
    
    if ( asMidLayer )
    {
        texture->setSourceFormat( GL_DEPTH_STENCIL_EXT );
        texture->setSourceType( GL_UNSIGNED_INT_24_8_EXT );
        
        texture->setShadowComparison( true );
        texture->setShadowAmbient( 0 );
        texture->setShadowCompareFunc( osg::Texture::GREATER );
        texture->setShadowTextureMode( osg::Texture::INTENSITY );
    }
    return texture.release();
}

typedef std::pair<osg::Camera*, osg::Texture*> CameraAndTexture;
CameraAndTexture createProcessCamera( int order, osg::Node* scene,
                                      osg::Texture* depth, osg::Texture* prevDepth )
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setDataVariance( osg::Object::DYNAMIC );
    camera->setInheritanceMask( osg::Camera::ALL_VARIABLES );
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    camera->setRenderOrder( osg::Camera::PRE_RENDER, order );
    camera->setClearColor( osg::Vec4() );
    camera->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );
    
    osg::ref_ptr<osg::Texture> color = createTexture(GL_RGBA, false);
    camera->attach( osg::Camera::COLOR_BUFFER, color.get() );
    camera->attach( osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, depth );
    
    osg::StateSet* ss = camera->getOrCreateStateSet();
    ss->setMode( GL_BLEND, osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE );
    if ( order!=0 )  // not the first camera?
    {
        ss->setAttributeAndModes( new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.01),
            osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
        ss->setTextureMode( g_unit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON );
        ss->setTextureMode( g_unit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON );
        ss->setTextureMode( g_unit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON );
        ss->setTextureMode( g_unit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON );
        ss->setTextureAttributeAndModes( g_unit, prevDepth );
        
        osg::ref_ptr<osg::TexGenNode> texGenNode = new osg::TexGenNode;
        texGenNode->setReferenceFrame( osg::TexGenNode::ABSOLUTE_RF );
        texGenNode->setTextureUnit( g_unit );
        texGenNode->getTexGen()->setMode( osg::TexGen::EYE_LINEAR );
        texGenNode->addChild( scene );
        
        camera->addChild( texGenNode.get() );
        camera->addCullCallback( new CullCallback(g_unit, g_offset) );
    }
    else camera->addChild( scene );
    return CameraAndTexture(camera.release(), color.get());
}

osg::Camera* createCompositionCamera()
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setDataVariance( osg::Object::DYNAMIC );
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setInheritanceMask( osg::Camera::READ_BUFFER|osg::Camera::DRAW_BUFFER );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setComputeNearFarMode( osg::Camera::COMPUTE_NEAR_FAR_USING_PRIMITIVES );
    camera->setClearMask( 0 );
    
    camera->setViewMatrix( osg::Matrix() );
    camera->setProjectionMatrix( osg::Matrix::ortho2D(0, 1, 0, 1) );
    camera->addCullCallback( new CullCallback(0, 0) );
    
    osg::StateSet* ss = camera->getOrCreateStateSet();
    ss->setRenderBinDetails( 0, "TraversalOrderBin" );
    return camera.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Material> material = new osg::Material;
    material->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    material->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.5f) );
    
    osg::Node* loadedModel = osgDB::readNodeFile( "cessna.osg" );
    loadedModel->getOrCreateStateSet()->setAttributeAndModes(
        material.get(), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
    loadedModel->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc );
    loadedModel->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::Camera* compositeCamera = createCompositionCamera();
	osg::ref_ptr<osg::Texture> depth[2];
	depth[0] = createTexture(GL_DEPTH24_STENCIL8_EXT, true);
	depth[1] = createTexture(GL_DEPTH24_STENCIL8_EXT, true);
	
	osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( compositeCamera );
	
	unsigned int numPasses = 8;
	for ( unsigned int i=0; i<numPasses; ++i )
	{
	    CameraAndTexture cat = createProcessCamera(
	        i, loadedModel, depth[i%2].get(), depth[(i+1)%2].get() );
	    root->addChild( cat.first );
	    
	    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	    geode->addDrawable( osg::createTexturedQuadGeometry(
	        osg::Vec3(), osg::X_AXIS, osg::Y_AXIS) );
	    
        osg::StateSet* ss = geode->getOrCreateStateSet();
        ss->setTextureAttributeAndModes( 0, cat.second );
        ss->setAttributeAndModes( new osg::BlendFunc );
        ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        ss->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
        compositeCamera->insertChild( 0, geode.get() );
	}
	
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.setUpViewInWindow( 50, 50, g_width, g_height );
    return viewer.run();
}
