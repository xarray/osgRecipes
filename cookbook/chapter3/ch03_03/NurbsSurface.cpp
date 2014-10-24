/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 3 Recipe 3
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/GL>
#include <GL/glu.h>
#include "NurbsSurface"

NurbsSurface::NurbsSurface()
:   _sCount(0), _tCount(0), _sOrder(0), _tOrder(0), _nurbsObj(0)
{
}

NurbsSurface::NurbsSurface( const NurbsSurface& copy, osg::CopyOp copyop )
:   osg::Drawable(copy, copyop), _vertices(copy._vertices),
    _normals(copy._normals), _texcoords(copy._texcoords),
    _sKnots(copy._sKnots), _tKnots(copy._tKnots),
    _sOrder(copy._sOrder), _tOrder(copy._tOrder), _nurbsObj(copy._nurbsObj)
{
}

NurbsSurface::~NurbsSurface()
{
}

#if OSG_VERSION_GREATER_THAN(3,2,1)
osg::BoundingBox NurbsSurface::computeBoundingBox() const
#else
osg::BoundingBox NurbsSurface::computeBound() const
#endif
{
    osg::BoundingBox bb;
    if ( _vertices.valid() )
    {
        for ( unsigned int i=0; i<_vertices->size(); ++i )
            bb.expandBy( (*_vertices)[i] );
    }
    return bb;
}

void NurbsSurface::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    GLUnurbsObj* theNurbs = (GLUnurbsObj*)_nurbsObj;
    if ( !theNurbs )
    {
        theNurbs = gluNewNurbsRenderer();
        gluNurbsProperty( theNurbs, GLU_SAMPLING_TOLERANCE, 10 );
        gluNurbsProperty( theNurbs, GLU_DISPLAY_MODE, GLU_FILL );
        _nurbsObj = theNurbs;
    }
    
    if ( _vertices.valid() && _sKnots.valid() && _tKnots.valid() )
    {
        glEnable( GL_MAP2_NORMAL );
        glEnable( GL_MAP2_TEXTURE_COORD_2 );
        
        gluBeginCurve( theNurbs );
        if ( _texcoords.valid() )
        {
            gluNurbsSurface( theNurbs,
                _sKnots->size(), &((*_sKnots)[0]), _tKnots->size(), &((*_tKnots)[0]),
                _sCount*2, 2, &((*_texcoords)[0][0]), _sOrder, _tOrder, GL_MAP2_TEXTURE_COORD_2 );
        }
        if ( _normals.valid() )
        {
            gluNurbsSurface( theNurbs,
                _sKnots->size(), &((*_sKnots)[0]), _tKnots->size(), &((*_tKnots)[0]),
                _sCount*3, 3, &((*_normals)[0][0]), _sOrder, _tOrder, GL_MAP2_NORMAL );
        }
        gluNurbsSurface( theNurbs,
            _sKnots->size(), &((*_sKnots)[0]), _tKnots->size(), &((*_tKnots)[0]),
            _sCount*3, 3, &((*_vertices)[0][0]), _sOrder, _tOrder, GL_MAP2_VERTEX_3 );
        gluEndCurve( theNurbs );
        
        glDisable( GL_MAP2_NORMAL );
        glDisable( GL_MAP2_TEXTURE_COORD_2 );
    }
}
