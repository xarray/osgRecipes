/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 8 Recipe 9
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <sstream>
#include <iostream>

#include "CommonFunctions"
//#define FAST_INTERSECTION  // Comment this to disable the use of kd-tree
//#define ACCURATE_INTERSECTION  // Comment this to disable computation of paged LODs

class PagedPickHandler : public osgGA::GUIEventHandler
{
public:
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
            return false;
        
        osgViewer::View* viewer = dynamic_cast<osgViewer::View*>(&aa);
        if ( viewer && _text.valid() )
        {
            osg::Timer_t t1 = osg::Timer::instance()->tick();
            osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
                new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
            osgUtil::IntersectionVisitor iv( intersector.get() );
            iv.setReadCallback( _pagedReader.get() );
            viewer->getCamera()->accept( iv );
            
            if ( intersector->containsIntersections() )
            {
                osgUtil::LineSegmentIntersector::Intersection result = *(intersector->getIntersections().begin());
                osg::Vec3 point = result.getWorldIntersectPoint();
                osg::Timer_t t2 = osg::Timer::instance()->tick();
                
                std::stringstream ss;
                ss << "X = " << point.x() << "; ";
                ss << "Y = " << point.y() << "; ";
                ss << "Z = " << point.z() << "; ";
                ss << "Delta time = " << osg::Timer::instance()->delta_m(t1, t2)
                   << "ms" << std::endl;
                _text->setText( ss.str() );
            }
        }
        return false;
    }
    
    osg::ref_ptr<osgUtil::IntersectionVisitor::ReadCallback> _pagedReader;
    osg::ref_ptr<osgText::Text> _text;
};

struct PagedReaderCallback : public osgUtil::IntersectionVisitor::ReadCallback
{
    virtual osg::Node* readNodeFile( const std::string& filename )
    { return osgDB::readNodeFile(filename); }
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
#ifdef FAST_INTERSECTION
    osgDB::Registry::instance()->setBuildKdTreesHint( osgDB::Options::BUILD_KDTREES );
#endif
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    
    osgText::Text* text = osgCookBook::createText(osg::Vec3(50.0f, 50.0f, 0.0f), "", 10.0f);
    osg::ref_ptr<osg::Geode> textGeode = new osg::Geode;
    textGeode->addDrawable( text );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0, 800, 0, 600);
    hudCamera->addChild( textGeode.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( hudCamera.get() );
    root->addChild( loadedModel.get() );
    
    osg::ref_ptr<PagedPickHandler> picker = new PagedPickHandler;
    picker->_text = text;
#ifdef ACCURATE_INTERSECTION
    picker->_pagedReader = new PagedReaderCallback;
#endif
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( picker.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
