/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 2 Recipe 6
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgDB/ReadFile>
#include <osgUtil/PrintVisitor>
#include <iostream>

#include "BFSVisitor"

class BFSPrintVisitor : public BFSVisitor
{
public:
    virtual void apply( osg::Node& node )
    {
        std::cout << node.libraryName() << "::" << node.className() << std::endl;
        traverseBFS(node);
    }
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles( arguments );
    if ( !root ) root = osgDB::readNodeFile("osgcool.osg");
    
    std::cout << "DFS Visitor traversal: " << std::endl;
    
    osgUtil::PrintVisitor pv( std::cout );
    root->accept( pv );
    
    std::cout << std::endl;
    std::cout << "BFS Visitor traversal: " << std::endl;
    
    BFSPrintVisitor bpv;
    root->accept( bpv );
    return 0;
}
