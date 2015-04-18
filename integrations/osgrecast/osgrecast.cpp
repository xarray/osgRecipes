// Recastnavigation is original created by Mikko Mononen memon@inside.org

#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgUtil/SmoothingVisitor>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <osg/io_utils>
#include "RecastManager.h"

class SimulationHandler : public osgGA::GUIEventHandler
{
public:
    SimulationHandler( osg::MatrixTransform* s )
    :   _scene(s), _lastSimulationTime(0.0)
    {
        _recast = new RecastManager( osg::Matrix::rotate(-osg::PI_2, osg::X_AXIS) );
        _recast->buildScene( s );
        
        _agentShape = new osg::Geode;
        _agentShape->addDrawable(
            new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(0.0f, 0.0f, 0.9f), 0.5f, 1.8f)) );
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
        if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
        {
            double time = ea.getTime();
            _recast->update( time - _lastSimulationTime );
            _lastSimulationTime = time;
        }
        else if ( ea.getEventType()==osgGA::GUIEventAdapter::RELEASE ||
                  ea.getEventType()==osgGA::GUIEventAdapter::DOUBLECLICK )
        {
            osgUtil::LineSegmentIntersector::Intersections intersections;
            if ( view->computeIntersections(ea.getX(), ea.getY(), intersections) )
            {
                const osgUtil::LineSegmentIntersector::Intersection& result = *(intersections.begin());
                osg::Vec3 pt = result.getWorldIntersectPoint();
                
                if ( ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL )
                {
                    osg::ref_ptr<osg::MatrixTransform> agent = new osg::MatrixTransform;
                    agent->setMatrix( osg::Matrix::translate(pt) );
                    agent->addChild( _agentShape.get() );
                    _scene->addChild( agent.get() );
                    _recast->addAgent( pt, agent.get() );
                }
                else if ( ea.getEventType()==osgGA::GUIEventAdapter::DOUBLECLICK )
                    _recast->moveTo( pt );
            }
        }
        return false;
    }
    
protected:
    osg::observer_ptr<osg::MatrixTransform> _scene;
    osg::ref_ptr<osg::Geode> _agentShape;
    osg::ref_ptr<RecastManager> _recast;
    double _lastSimulationTime;
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("nav_test.obj") );
    scene->getOrCreateStateSet()->setMode( GL_CULL_FACE, osg::StateAttribute::ON );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new SimulationHandler(scene.get()) );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
