#ifndef H_KINECTHANDLER
#define H_KINECTHANDLER

#include <XnCppWrapper.h>
#include <osgGA/GUIEventHandler>

#define KINECT_IMAGE_WIDTH 640
#define KINECT_IMAGE_HEIGHT 480
#define KINECT_SKELETON_COUNT XN_SKEL_RIGHT_FOOT

using namespace xn;

class KinectHandler : public osgGA::GUIEventHandler
{
public:
    KinectHandler() : _needPoseDetection(false)
    {
        sprintf( _poseName, "" );
        _points = new osg::Vec3Array(KINECT_SKELETON_COUNT);
        _depthBuffer = new unsigned char[KINECT_IMAGE_WIDTH*KINECT_IMAGE_HEIGHT];
    }
    
    virtual ~KinectHandler()
    { delete _depthBuffer; }
    
    virtual bool initialize( const char* configFile=NULL );
    virtual bool quit();
    
    virtual bool handleJoints( DepthMetaData& depthMetaData );
    virtual bool handleImages( DepthMetaData& depthMetaData );
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
    virtual void handleCurrentData() {}
    
    Context& getContext() { return _context; }
    UserGenerator& getSkeletonGenerator() { return _skeleton; }
    DepthGenerator& getDepthGenerator() { return _depth; }
    XnChar* getPoseName() { return _poseName; }
    bool needPoseDetection() const { return _needPoseDetection; }
    
protected:
    bool updateFrame();
    
    Context _context;
    ScriptNode _scriptNode;
    UserGenerator _skeleton;
    DepthGenerator _depth;
    //ImageGenerator _image;
    XnChar _poseName[20];
    bool _needPoseDetection;
    
    OpenThreads::Mutex _mutex;
    osg::ref_ptr<osg::Vec3Array> _points;
    unsigned char* _depthBuffer;
};

#endif
