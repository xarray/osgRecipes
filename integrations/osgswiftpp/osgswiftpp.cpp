#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osg/TriangleFunctor>
#include <osgDB/ReadFile>
#include <osgUtil/SmoothingVisitor>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <SWIFT.h>

struct CollectFaceOperator
{
    void operator()( const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3, bool temp )
    {
        if ( v1==v2 || v2==v3 || v3==v1 ) return;
        indices.push_back( findIndex(v1) );
        indices.push_back( findIndex(v2) );
        indices.push_back( findIndex(v3) );
    }
    
    unsigned int findIndex( const osg::Vec3& v )
    {
        unsigned int index = 0;
        if ( vertexMap.find(v)==vertexMap.end() )
        {
            index = vertexMap.size();
            vertexMap[v] = index;
            
            vertices.push_back( v[0] );
            vertices.push_back( v[1] );
            vertices.push_back( v[2] );
        }
        else index = vertexMap[v];
        return index;
    }
    
    std::map<osg::Vec3, int> vertexMap;
    std::vector<SWIFT_Real> vertices;
    std::vector<int> indices;
};

class MoveVehicleHandler : public osgGA::GUIEventHandler
{
public:
    MoveVehicleHandler() : _actorID(-1)
    {
        _swiftScene = new SWIFT_Scene( true, false );
    }
    
    virtual ~MoveVehicleHandler()
    {
        delete _swiftScene;
    }
    
    void setActor( int id, osg::MatrixTransform* actor )
    { _actorID = id; _actor = actor; }
    
    int addSwiftObject( const osg::Matrix& matrix, osg::Drawable* drawable )
    {
        osg::TriangleFunctor<CollectFaceOperator> cf;
        drawable->accept( cf );
        
        int id = 0, vertexSize = (int)cf.vertices.size()/3, indexSize = (int)cf.indices.size()/3;
        if ( vertexSize>0 && indexSize>0 )
        {
            bool ok = _swiftScene->Add_Convex_Object(
                &(cf.vertices[0]), &(cf.indices[0]), vertexSize, indexSize, id, false );
            if ( !ok )
            {
                OSG_NOTICE << "Failed to create convex object" << std::endl;
                return -1;
            }
        }
        else
        {
            OSG_NOTICE << "Failed to find vertex and index data" << std::endl;
            return -1;
        }
        
        setSwiftTransformation( id, matrix );
        return id;
    }
    
    void finish()
    {
        // Query only the actor because it is the only dynamic object in the scene
        _swiftScene->Deactivate();
        _swiftScene->Activate( _actorID );
    }
    
    void setSwiftTransformation( int id, const osg::Matrix& matrix )
    {
        SWIFT_Real R[9];
        for ( int x=0; x<3; ++x )
        {
            for ( int y=0; y<3; ++y )
                R[x*3+y] = matrix(x, y);
        }
        
        SWIFT_Real T[3];
        for ( int i=0; i<3; ++i ) T[i] = matrix(3, i);
        _swiftScene->Set_Object_Transformation( id, R, T );
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( !_actor ) return false;
        if ( ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN )
        {
            // Handle user inputs
            osg::Matrix matrix = _actor->getMatrix();
            switch ( ea.getKey() )
            {
            case osgGA::GUIEventAdapter::KEY_Left:
                matrix = matrix * osg::Matrix::translate(-0.2f, 0.0f, 0.0f);
                break;
            case osgGA::GUIEventAdapter::KEY_Right:
                matrix = matrix * osg::Matrix::translate(0.2f, 0.0f, 0.0f);
                break;
            case osgGA::GUIEventAdapter::KEY_Down:
                matrix = matrix * osg::Matrix::translate(0.0f, -0.2f, 0.0f);
                break;
            case osgGA::GUIEventAdapter::KEY_Up:
                matrix = matrix * osg::Matrix::translate(0.0f, 0.2f, 0.0f);
                break;
            default: return false;
            }
            
            // Apply transformation for intersection query
            setSwiftTransformation( _actorID, matrix );
            
            // Check if there are intersections
            bool canMove = true;
            int numPairs = 0;
            int* pairIDs = NULL;
#if 0  // Compute exact intersection pairs and distances
            SWIFT_Real* dists = NULL;
            if ( _swiftScene->Query_Exact_Distance(false, SWIFT_INFINITY, numPairs, &pairIDs, &dists) )
            {
                for ( int i=0; i<numPairs; ++i )
                {
                    if ( dists[i]>0.1f ) continue;
                    std::cout << pairIDs[i<<1] << " and " << pairIDs[(i<<1) + 1]
                              << ": " << dists[i] << std::endl;
                    canMove = false;
                }
            }
#else  // Only do intersection test
            if ( _swiftScene->Query_Intersection(true, numPairs, &pairIDs) )
            {
                canMove = false;
            }
#endif
            
            // Apply the result to the scene graph
            if ( canMove )
                _actor->setMatrix( matrix );
        }
        return false;
    }
    
protected:
    osg::observer_ptr<osg::MatrixTransform> _actor;
    SWIFT_Scene* _swiftScene;
    int _actorID;
};

// Some code copied from osgmicropather
int main( int argc, char** argv )
{
    // Create path properties
    const int mapData[10 * 10] =
    { 
        1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 1, 0, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 1, 0, 1, 1,
        1, 0, 0, 0, 1, 0, 1, 1, 0, 1,
        1, 0, 1, 1, 1, 0, 0, 0, 1, 1,
        1, 0, 0, 1, 1, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 1, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        1, 1, 0, 1, 1, 1, 1, 1, 1, 1
    };
    
    // Create the interactive handler
    osg::ref_ptr<MoveVehicleHandler> handler = new MoveVehicleHandler;
    
    // Create the actor
    osg::ref_ptr<osg::ShapeDrawable> actorShape =
        new osg::ShapeDrawable( new osg::Box(osg::Vec3(), 3.0f) );
    actorShape->setColor( osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f) );
    
    osg::ref_ptr<osg::Geode> actorGeode = new osg::Geode;
    actorGeode->addDrawable( actorShape.get() );
    
    osg::ref_ptr<osg::MatrixTransform> actor = new osg::MatrixTransform;
    actor->addChild( actorGeode.get() );
    actor->setMatrix( osg::Matrix::translate(20.0f, 0.0f, 3.0f) );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( actor.get() );
    
    int id = handler->addSwiftObject( actor->getMatrix(), actorShape.get() );
    if ( id>=0 ) handler->setActor( id, actor.get() );
    
    // Create the map geometries
    osg::ref_ptr<osg::ShapeDrawable> groundShape = new osg::ShapeDrawable(
        new osg::Box(osg::Vec3(45.0f, 45.0f, 0.0f), 100.0f, 100.0f, 0.5f) );
    groundShape->setColor( osg::Vec4(0.2f, 0.2f, 0.2f, 0.2f) );
    
    osg::ref_ptr<osg::Geode> ground = new osg::Geode;
    ground->addDrawable( groundShape.get() );
    scene->addChild( ground.get() );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f, 0.0f, 5.0f), 10.0f)) );
    for ( int x=0; x<10; ++x )
    {
        for ( int y=0; y<10; ++y )
        {
            if ( mapData[y*10 + x]==0 ) continue;
            osg::ref_ptr<osg::MatrixTransform> boxNode = new osg::MatrixTransform;
            boxNode->setMatrix( osg::Matrix::translate(10.0f*(float)x, 10.0f*(float)y, 0.0f) );
            boxNode->addChild( geode.get() );
            scene->addChild( boxNode.get() );
            handler->addSwiftObject( boxNode->getMatrix(), geode->getDrawable(0) );
        }
    }
    
    // Create the viewer
    osgViewer::Viewer viewer;
    viewer.addEventHandler( handler.get() );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    
    handler->finish();
    return viewer.run();
}
