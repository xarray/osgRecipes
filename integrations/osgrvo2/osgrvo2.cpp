// Thanks Christian Buchner for the original osgrvo2 example on osg-submissions.

#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgUtil/SmoothingVisitor>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <RVOSimulator.h>
#include <stdarg.h>

class SimulationHandler : public osgGA::GUIEventHandler
{
public:
    SimulationHandler()
    {
        _rvo = new RVO::RVOSimulator;
        _rvo->setTimeStep( 0.25f );
        _rvo->setAgentDefaults( 15.0f, 10, 5.0f, 5.0f, 1.0f, 2.0f );
        _goal = RVO::Vector2( 0.0f, 0.0f );
    }
    
    virtual ~SimulationHandler()
    {
        if ( _rvo ) delete _rvo;
        _rvo = NULL;
    }
    
    void addAgent( osg::MatrixTransform* node, const RVO::Vector2& pos,
                   const RVO::Vector2& vel=RVO::Vector2(0.0f, 0.0f), float radius=1.0f )
    {
        RVO::Vector2 initVelocity( vel.x(), vel.y() );
        if ( RVO::absSq(initVelocity)>1.0f ) initVelocity = RVO::normalize(initVelocity);
        
        _agents[_rvo->getNumAgents()] = node;
        _rvo->addAgent( pos, 15.0f, 10, 5.0f, 5.0f, radius, 2.0f, initVelocity );
    }
    
    void setAgentVelocity( unsigned int i, const RVO::Vector2& vel )
    {
        RVO::Vector2 newVelocity( vel.x(), vel.y() );
        if ( RVO::absSq(newVelocity)>1.0f ) newVelocity = RVO::normalize(newVelocity);
        _rvo->setAgentPrefVelocity( i, newVelocity );
        
        //  Perturb a little to avoid deadlocks due to perfect symmetry
        float angle = std::rand() * 2.0f * osg::PI / RAND_MAX;
        float length = std::rand() * 0.0001f / RAND_MAX;
        _rvo->setAgentPrefVelocity( i, _rvo->getAgentPrefVelocity(i) +
            length * RVO::Vector2(std::cos(angle), std::sin(angle)) );
    }
    
    void addObstacle( const std::vector<RVO::Vector2>& vertices )
    { _rvo->addObstacle( vertices ); }
    
    void finishObstacles()
    { _rvo->processObstacles(); }
    
    void setGoalPosition( const RVO::Vector2& goal )
    { _goal = goal; }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
        if ( !_rvo || !view ) return false;
        
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::DOUBLECLICK:
            if ( ea.getButton()==osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
            {
                osgUtil::LineSegmentIntersector::Intersections intersections;
                if ( view->computeIntersections(ea.getX(), ea.getY(), intersections) )
                {
                    const osgUtil::LineSegmentIntersector::Intersection& result = *(intersections.begin());
                    osg::Vec3 pt = result.getWorldIntersectPoint();
                    setGoalPosition( RVO::Vector2(pt.x(), pt.y()) );
                }
            }
            break;
        case osgGA::GUIEventAdapter::FRAME:
            for ( unsigned int i=0; i<_rvo->getNumAgents(); ++i )
            {
                // Set agent velocity
                const RVO::Vector2& v = _rvo->getAgentPosition(i);
                setAgentVelocity( i, _goal - v );
                
                // Set node matrix
                osg::MatrixTransform* node = _agents[i].get();
                if ( node ) node->setMatrix( osg::Matrix::translate(v.x(), v.y(), 1.0f) );
            }
            _rvo->doStep();
            break;
        default: break;
        }
        return false;
    }
    
protected:
    typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > AgentMap;
    AgentMap _agents;
    RVO::RVOSimulator* _rvo;
    RVO::Vector2 _goal;
};

osg::Node* createObstacle( SimulationHandler* sim, int num, ... )
{
    va_list args;
    va_start( args, num );
    
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(2 * num);
    std::vector<RVO::Vector2> polygon(num);
    for ( int i=0; i<num; ++i )
    {
        RVO::Vector2 pt;
        pt = va_arg( args, RVO::Vector2 );
        
        (*vertices)[2*i + 0] = osg::Vec3(pt.x(), pt.y(), 5.0f);
        (*vertices)[2*i + 1] = osg::Vec3(pt.x(), pt.y(), 0.0f);
        polygon[i] = pt;
    }
    sim->addObstacle( polygon );
    va_end( args );
    
    osg::ref_ptr<osg::DrawElementsUInt> walls = new osg::DrawElementsUInt(GL_QUAD_STRIP);
    osg::ref_ptr<osg::DrawElementsUInt> ceiling = new osg::DrawElementsUInt(GL_POLYGON);
    for ( int i=0; i<num; ++i )
    {
        walls->push_back( 2*i ); walls->push_back( 2*i+1 );
        ceiling->push_back( 2*i );
    }
    walls->push_back( 0 ); walls->push_back( 1 );
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->addPrimitiveSet( walls.get() );
    geom->addPrimitiveSet( ceiling.get() );
    osgUtil::SmoothingVisitor::smooth( *geom );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( geom.get() );
    return geode.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    osg::ref_ptr<SimulationHandler> simulator = new SimulationHandler;
    
    // Create all agents
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(), 1.0f)) );
    for ( unsigned int x=0; x<10; ++x )
    {
        for ( unsigned int y=0; y<10; ++y )
        {
            osg::ref_ptr<osg::MatrixTransform> agent = new osg::MatrixTransform;
            agent->setMatrix( osg::Matrix::translate(2.0f*(float)x, 2.0f*(float)y, 1.0f) );
            agent->addChild( geode.get() );
            
            scene->addChild( agent.get() );
            simulator->addAgent( agent.get(), RVO::Vector2(2.0f*(float)x, 2.0f*(float)y) );
        }
    }
    
    // Create ground and obstacles
    osg::ref_ptr<osg::ShapeDrawable> groundShape = new osg::ShapeDrawable(new osg::Box(osg::Vec3(), 200.0f, 200.0f, 0.5f) );
    groundShape->setColor( osg::Vec4(0.2f, 0.2f, 0.2f, 0.2f) );
    
    osg::ref_ptr<osg::Geode> ground = new osg::Geode;
    ground->addDrawable( groundShape.get() );
    scene->addChild( ground.get() );
    
    for ( unsigned int i=0; i<24; ++i )
    {
        RVO::Vector2 corner(cosf(osg::PI * (float)i / 12.0f), sinf(osg::PI * (float)i / 12.0f));
        RVO::Vector2 vert(-corner.y(), corner.x());
        scene->addChild( createObstacle(simulator.get(), 3, corner*30.0f, corner*40.0f, corner*40.0f + vert*5.0f) );
    }
    simulator->finishObstacles();
    
    // Create the viewer
    osgViewer::Viewer viewer;
    viewer.addEventHandler( simulator.get() );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
