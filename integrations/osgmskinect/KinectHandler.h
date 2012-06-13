#ifndef H_KINECTHANDLER
#define H_KINECTHANDLER

#include <windows.h>
#include <NuiApi.h>
#include <osgGA/GUIEventHandler>

#define KINECT_IMAGE_WIDTH 640
#define KINECT_IMAGE_HEIGHT 480
#define KINECT_IMAGE_RESOLUTION NUI_IMAGE_RESOLUTION_640x480

class KinectHandler : public osgGA::GUIEventHandler
{
public:
    KinectHandler()
    {
        _context = NULL;
        _nuiProcess = NULL;
        _nuiProcessStopEvent = NULL;
        _nextSkeletonEvent = NULL;
        _nextImageFrameEvent = NULL;
        _videoStreamHandle = NULL;
        _points = new osg::Vec3Array(NUI_SKELETON_POSITION_COUNT);
        _depthBuffer = new unsigned char[KINECT_IMAGE_WIDTH*KINECT_IMAGE_HEIGHT];
    }
    
    virtual ~KinectHandler()
    { delete _depthBuffer; }
    
    virtual bool initialize( int kid );
    virtual bool quit();
    
    virtual bool handleJoints( NUI_SKELETON_DATA& skeletonData );
    virtual bool handleImages( NUI_IMAGE_FRAME& imageFrame );
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
    virtual void handleCurrentData() {}
    
protected:
    DWORD WINAPI runNuiThread();
    
    static DWORD WINAPI nuiProcessThread( LPVOID param )
    { KinectHandler* current = (KinectHandler*)param; return current->runNuiThread(); }
    
    INuiSensor* _context;
    HANDLE _nuiProcess;
    HANDLE _nuiProcessStopEvent;
    HANDLE _nextSkeletonEvent;
    HANDLE _nextImageFrameEvent;
    HANDLE _videoStreamHandle;
    
    OpenThreads::Mutex _mutex;
    osg::ref_ptr<osg::Vec3Array> _points;
    unsigned char* _depthBuffer;
};

#endif
