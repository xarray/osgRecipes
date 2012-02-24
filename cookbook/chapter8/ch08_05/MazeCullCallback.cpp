/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 8 Recipe 5
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Transform>
#include "MazeCullCallback"

void MazeCullCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    osg::Vec3 eye = nv->getEyePoint();
    osg::Vec3 center = node->getBound().center();
    
    osg::Matrix l2w = osg::computeLocalToWorld( node->getParentalNodePaths()[0] );
    eye = eye * l2w; center = center * l2w;
    
    CellIndex indexEye, indexNode;
    if ( getCellIndex(indexEye, eye) && getCellIndex(indexNode, center) )
    {
        traverse( node, nv );
    }
    // We don't traverse if the node is not visible in maze
}

bool MazeCullCallback::getCellIndex( CellIndex& index, const osg::Vec3& pos )
{
    index.first = int(pos[0] + 0.5f);
    index.second = int(pos[1] + 0.5f);
    CellMap::iterator itr = g_mazeMap.find(index);
    if ( itr!=g_mazeMap.end() && itr->second==0 )
        return true;
    return false;
}
