/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 7
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Depth>
#include <osgUtil/CullVisitor>
#include "SkyBox"

SkyBox::SkyBox()
{
    setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    setCullingActive( false );
    
    osg::StateSet* ss = getOrCreateStateSet();
    ss->setAttributeAndModes( new osg::Depth(osg::Depth::LEQUAL, 1.0f, 1.0f) );
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    ss->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
    ss->setRenderBinDetails( 5, "RenderBin" );
}

void SkyBox::setEnvironmentMap( unsigned int unit, osg::Image* posX, osg::Image* negX,
                                osg::Image* posY, osg::Image* negY, osg::Image* posZ, osg::Image* negZ )
{
    if ( posX && posY && posZ && negX && negY && negZ )
    {
        osg::ref_ptr<osg::TextureCubeMap> cubemap = new osg::TextureCubeMap;
        cubemap->setImage( osg::TextureCubeMap::POSITIVE_X, posX );
        cubemap->setImage( osg::TextureCubeMap::NEGATIVE_X, negX );
        cubemap->setImage( osg::TextureCubeMap::POSITIVE_Y, posY );
        cubemap->setImage( osg::TextureCubeMap::NEGATIVE_Y, negY );
        cubemap->setImage( osg::TextureCubeMap::POSITIVE_Z, posZ );
        cubemap->setImage( osg::TextureCubeMap::NEGATIVE_Z, negZ );

        cubemap->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        cubemap->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
        cubemap->setWrap( osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE );
		cubemap->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
		cubemap->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
        cubemap->setResizeNonPowerOfTwoHint( false );
        getOrCreateStateSet()->setTextureAttributeAndModes( unit, cubemap.get() );
    }
}

bool SkyBox::computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const
{
    if ( nv && nv->getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
    {
        osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>( nv );
        matrix.preMult( osg::Matrix::translate(cv->getEyeLocal()) );
        return true;
    }
    else
        return osg::Transform::computeLocalToWorldMatrix( matrix, nv );
}

bool SkyBox::computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const
{
    if ( nv && nv->getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
    {
        osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>( nv );
        matrix.postMult( osg::Matrix::translate(-cv->getEyeLocal()) );
        return true;
    }
    else
        return osg::Transform::computeWorldToLocalMatrix( matrix, nv );
}
