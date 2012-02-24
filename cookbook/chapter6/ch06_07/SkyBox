/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 7
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#ifndef H_COOKBOOK_CH6_SKYBOX
#define H_COOKBOOK_CH6_SKYBOX

#include <osg/TextureCubeMap>
#include <osg/Transform>

class SkyBox : public osg::Transform
{
public:
    SkyBox();
    
    SkyBox( const SkyBox& copy, osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY )
    : osg::Transform(copy, copyop) {}
    
    META_Node( osg, SkyBox );
    
    void setEnvironmentMap( unsigned int unit, osg::Image* posX, osg::Image* negX,
                            osg::Image* posY, osg::Image* negY, osg::Image* posZ, osg::Image* negZ );
    
    virtual bool computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const;
    virtual bool computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const;
    
protected:
    virtual ~SkyBox() {}
};

#endif
