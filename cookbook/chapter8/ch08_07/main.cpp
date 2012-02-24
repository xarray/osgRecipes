/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 8 Recipe 7
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgUtil/PrintVisitor>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <iostream>
#include <fstream>
#include <sstream>

#include "CommonFunctions"
#include "OctreeBuilder"

class PrintNameVisitor : public osgUtil::PrintVisitor
{
public:
    PrintNameVisitor( std::ostream& out ) : osgUtil::PrintVisitor(out) {}
    
    void apply( osg::Node& node )
    {
        if ( !node.getName().empty() )
        {
            output() << node.getName() << std::endl;
            enter();
            traverse( node );
            leave();
        }
        else osgUtil::PrintVisitor::apply(node);
    }
};

int main( int argc, char** argv )
{
    osg::BoundingBox globalBound;
    std::vector<OctreeBuilder::ElementInfo> globalElements;
    for ( unsigned int i=0; i<5000; ++i )
    {
        osg::Vec3 pos = osgCookBook::randomVector( -500.0f, 500.0f );
        float radius = osgCookBook::randomValue( 0.5f, 2.0f );
        std::stringstream ss; ss << "Ball-" << i+1;
        
        osg::Vec3 min = pos - osg::Vec3(radius, radius, radius);
        osg::Vec3 max = pos + osg::Vec3(radius, radius, radius);
        osg::BoundingBox region(min, max);
        globalBound.expandBy( region );
        globalElements.push_back( OctreeBuilder::ElementInfo(ss.str(), region) );
    }
    
    OctreeBuilder octree;
    osg::ref_ptr<osg::Group> root = octree.build( 0, globalBound, globalElements );
    
    std::ofstream out("octree_output.txt");
    PrintNameVisitor printer( out );
    root->accept( printer );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
