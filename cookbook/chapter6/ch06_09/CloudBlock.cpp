/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 8
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/io_utils>
#include <iostream>
#include <algorithm>
#include "CloudBlock"

CloudBlock::CloudBlock()
{
    setUseDisplayList( false );
    setSupportsDisplayList( false );
}

CloudBlock::CloudBlock( const CloudBlock& copy, const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop), _cells(copy._cells)
{
}

#if OSG_VERSION_GREATER_THAN(3,2,1)
osg::BoundingBox CloudBlock::computeBoundingBox() const
#else
osg::BoundingBox CloudBlock::computeBound() const
#endif
{
    osg::BoundingBox bb;
    for ( CloudCells::const_iterator itr=_cells.begin(); itr!=_cells.end(); ++itr )
    {
        bb.expandBy( itr->_pos );
    }
    return bb;
}

void CloudBlock::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    const osg::State* state = renderInfo.getState();
    if ( !state || !_cells.size() ) return;
    
    const osg::Matrix& modelview = state->getModelViewMatrix();
    std::sort( _cells.begin(), _cells.end(), LessDepthSortFunctor(modelview) );
    
    glPushMatrix();
    renderCells( modelview );
    glPopMatrix();
}

void CloudBlock::renderCells( const osg::Matrix& modelview ) const
{
    osg::Vec3d px = osg::Matrix::transform3x3( modelview, osg::X_AXIS );
    osg::Vec3d py = osg::Matrix::transform3x3( modelview, osg::Y_AXIS );
    px.normalize(); py.normalize();
    
    double size = 1.0f, scale = 1.0f;
    osg::Vec3d right, up;
    glBegin( GL_QUADS );
    
    unsigned int detail = 1;
    unsigned int numOfCells = _cells.size();
    for ( unsigned int i=0; i<numOfCells; i+=detail )
    {
        const CloudCell& cell = _cells[i];
        osg::Vec3d pos = cell._pos;
        unsigned char alpha = (unsigned char)( cell._density );
        unsigned char color = (unsigned char)( cell._brightness * cell._density / 255.0f );
        right.set( px * size * scale );
        up.set( py * size * scale );
        
        glColor4ub( color, color, color, alpha );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3dv( (pos-right+up).ptr() );
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3dv( (pos-right-up).ptr() );
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3dv( (pos+right-up).ptr() );
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3dv( (pos+right+up).ptr() );
    }
    glEnd();
}
