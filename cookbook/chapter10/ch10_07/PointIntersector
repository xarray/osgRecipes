/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 10 Recipe 7
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#ifndef H_COOKBOOK_CH10_POINTINTERSECTOR
#define H_COOKBOOK_CH10_POINTINTERSECTOR

#include <osgUtil/LineSegmentIntersector>

class PointIntersector : public osgUtil::LineSegmentIntersector
{
public:
    PointIntersector();
    PointIntersector( const osg::Vec3& start, const osg::Vec3& end );
    PointIntersector( CoordinateFrame cf, double x, double y );
    
    void setPickBias( float bias ) { _pickBias = bias; }
    float getPickBias() const { return _pickBias; }
    
    virtual Intersector* clone( osgUtil::IntersectionVisitor& iv );
    virtual void intersect( osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable );
    
protected:
    virtual ~PointIntersector() {}
    float _pickBias;
};

#endif
