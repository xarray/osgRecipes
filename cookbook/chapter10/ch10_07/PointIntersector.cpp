/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 10 Recipe 7
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Geometry>
#include <osg/TemplatePrimitiveFunctor>
#include "PointIntersector"

PointIntersector::PointIntersector()
:   osgUtil::LineSegmentIntersector(MODEL, 0.0, 0.0),
    _pickBias(2.0f)
{
}

PointIntersector::PointIntersector( const osg::Vec3& start, const osg::Vec3& end )
:   osgUtil::LineSegmentIntersector(start, end),
    _pickBias(2.0f)
{
}

PointIntersector::PointIntersector( CoordinateFrame cf, double x, double y )
:   osgUtil::LineSegmentIntersector(cf, x, y),
    _pickBias(2.0f)
{
}

osgUtil::Intersector* PointIntersector::clone( osgUtil::IntersectionVisitor& iv )
{
    if ( _coordinateFrame==MODEL && iv.getModelMatrix()==0 )
    {
        osg::ref_ptr<PointIntersector> cloned = new PointIntersector( _start, _end );
        cloned->_parent = this;
        cloned->_pickBias = _pickBias;
        return cloned.release();
    }
    
    osg::Matrix matrix;
    switch ( _coordinateFrame )
    {
        case WINDOW:
            if (iv.getWindowMatrix()) matrix.preMult( *iv.getWindowMatrix() );
            if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case PROJECTION:
            if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case VIEW:
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case MODEL:
            if (iv.getModelMatrix()) matrix = *iv.getModelMatrix();
            break;
    }
    
    osg::Matrix inverse = osg::Matrix::inverse(matrix);
    osg::ref_ptr<PointIntersector> cloned = new PointIntersector( _start*inverse, _end*inverse );
    cloned->_parent = this;
    cloned->_pickBias = _pickBias;
    return cloned.release();
}

void PointIntersector::intersect( osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable )
{
    osg::BoundingBox bb = drawable->getBound();
    bb.xMin() -= _pickBias; bb.xMax() += _pickBias;
    bb.yMin() -= _pickBias; bb.yMax() += _pickBias;
    bb.zMin() -= _pickBias; bb.zMax() += _pickBias;
    
    osg::Vec3d s(_start), e(_end);
    if ( !intersectAndClip(s, e, bb) ) return;
    if ( iv.getDoDummyTraversal() ) return;
    
    osg::Geometry* geometry = drawable->asGeometry();
    if ( geometry )
    {
        osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>( geometry->getVertexArray() );
        if ( !vertices ) return;
        
        osg::Vec3d dir = e - s;
        double invLength = 1.0 / dir.length();
        for ( unsigned int i=0; i<vertices->size(); ++i )
        {
            double distance =  fabs( (((*vertices)[i] - s)^dir).length() );
            distance *= invLength;
            if ( _pickBias<distance ) continue;
            
            Intersection hit;
            hit.ratio = distance;
            hit.nodePath = iv.getNodePath();
            hit.drawable = drawable;
            hit.matrix = iv.getModelMatrix();
            hit.localIntersectionPoint = (*vertices)[i];
            insertIntersection( hit );
        }
    }
}
