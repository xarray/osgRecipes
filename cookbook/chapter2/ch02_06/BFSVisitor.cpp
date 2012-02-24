/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 2 Recipe 6
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Group>
#include "BFSVisitor"

BFSVisitor::BFSVisitor()
:   osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
}

BFSVisitor::~BFSVisitor()
{
}

void BFSVisitor::traverseBFS( osg::Node& node )
{
    osg::Group* group = node.asGroup();
    if ( !group ) return;
    
    for ( unsigned int i=0; i<group->getNumChildren(); ++i )
    {
        _pendingNodes.push_back( group->getChild(i) );
    }
    
    while ( _pendingNodes.size()>0 )
    {
        osg::Node* node = _pendingNodes.front();
        _pendingNodes.pop_front();
        node->accept(*this);
    }
}
