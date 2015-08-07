#include <osg/Point>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "GI_Interface.h"

class VXGIDrawCallback : public osg::Camera::DrawCallback, public VXGI::IErrorCallback
{
public:
    VXGIDrawCallback();
    
    virtual void operator()( osg::RenderInfo& renderInfo ) const;
    
    virtual void signalError( const char* file, int line, const char* desc );
    
protected:
    virtual ~VXGIDrawCallback();
    
    VXGI::IGlobalIllumination* _gi;
    VXGI::IViewTracer* _giTracer;
};

VXGIDrawCallback::VXGIDrawCallback()
:   _gi(NULL), _giTracer(NULL)
{
    VXGI::GIParameters params;
    params.rendererInterface = NULL;
    params.errorCallback = this;
    params.shaderType = VXGI::GIParameters::ST_GLSL_430_SEPARATE_SHADER_OBJECTS;
    params.voxelizationParamters.opacityDirectionCount = VXGI::OpacityDirections::SIX_DIMENSIONAL;
    params.voxelizationParamters.emittanceDirectionCount = VXGI::EmittanceDirections::SIX_DIMENSIONAL;
    params.voxelizationParamters.mapSize = 128;
    params.voxelizationParamters.maxScatterIterations = 0;
    
    if ( VXGI_FAILED(VFX_VXGI_CreateGIObject(params, &_gi)) )
    {
        OSG_WARN << "Can't create VXGI object" << std::endl;
    }
    else if ( VXGI_FAILED(_gi->createNewTracer(&_giTracer)) )
    {
        OSG_WARN << "Can't create VXGI tracer" << std::endl;
    }
}

VXGIDrawCallback::~VXGIDrawCallback()
{
    if ( _gi )
    {
        if ( _giTracer ) _gi->destroyTracer( _giTracer );
        VFX_VXGI_DestroyGIObject( _gi );
    }
}

void VXGIDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
}

void VXGIDrawCallback::signalError( const char* file, int line, const char* desc )
{
    OSG_WARN << "[VXGIDrawCallback] Error: " << desc
             << ", (" << file << ", " << line << ")" << std::endl;
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("cow.osg") );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( scene.get() );
    viewer.getCamera()->setProjectionMatrixAsPerspective( 30.0f, 16.0f/9.0f, 0.1f, 1000.0f );
    viewer.getCamera()->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.setUpViewOnSingleScreen( 0 );
    
    osg::ref_ptr<VXGIDrawCallback> vxgi = new VXGIDrawCallback;
    viewer.getCamera()->setPostDrawCallback( vxgi.get() );
    
    viewer.run();
    return 0;
}
