#include <windows.h>
#include <NuiApi.h>

#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

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
    }
    
    virtual bool initialize( int kid )
    {
        HRESULT hr = NuiCreateSensorByIndex( 0, &_context );
        if ( FAILED(hr) )
        {
            OSG_WARN << "Failed to connect to Kinect device." << std::endl;
            return false;
        }
        
        DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
        hr = _context->NuiInitialize( nuiFlags );
        if ( FAILED(hr) )
        {
            switch ( hr )
            {
            case E_NUI_DEVICE_IN_USE:
                OSG_WARN << "Kinect device is already in use." << std::endl;
                break;
            case E_NUI_SKELETAL_ENGINE_BUSY:
                OSG_WARN << "Kinect device is busy at present." << std::endl;
                break;
            default:
                OSG_WARN << "Kinect device failed with error code " << (long)hr << std::endl;
                break;
            }
            return false;
        }
        
        bool hasSkeleton = HasSkeletalEngine(_context);
        if ( hasSkeleton )
        {
            _nextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
            hr = _context->NuiSkeletonTrackingEnable( _nextSkeletonEvent, 0 );
            if ( FAILED(hr) )
            {
                OSG_WARN << "Unable to start tracking skeleton." << std::endl;
                return false;
            }
            
            _nuiProcessStopEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
            _nuiProcess = CreateThread( NULL, 0, nuiProcessThread, this, 0, NULL );
        }
        else
        {
            OSG_WARN << "Current device don't have a skeleton engine." << std::endl;
        }
        
        _nextImageFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        /*if ( hasSkeleton )
        {
            hr = _context->NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, KINECT_IMAGE_RESOLUTION,
                                               0, 2, _nextImageFrameEvent, &_videoStreamHandle );
        }
        else
        {
            hr = _context->NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH, KINECT_IMAGE_RESOLUTION,
                                               0, 2, _nextImageFrameEvent, &_videoStreamHandle );
        }*/
        hr = _context->NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR, KINECT_IMAGE_RESOLUTION,
                                           0, 2, _nextImageFrameEvent, &_videoStreamHandle );
        if ( FAILED(hr) )
        {
            OSG_WARN << "Unable to create image stream." << std::endl;
            return false;
        }
        return true;
    }
    
    virtual bool quit()
    {
        if ( _nuiProcessStopEvent!=NULL )
        {
            SetEvent( _nuiProcessStopEvent );
            if ( _nuiProcess!=NULL )
            {
                WaitForSingleObject( _nuiProcess, INFINITE );
                CloseHandle( _nuiProcess );
            }
            CloseHandle( _nuiProcessStopEvent );
        }
        
        if ( _context ) _context->NuiShutdown();
        if ( _nextSkeletonEvent && (_nextSkeletonEvent!=INVALID_HANDLE_VALUE) )
            CloseHandle( _nextSkeletonEvent );
        if ( _nextImageFrameEvent && (_nextImageFrameEvent!=INVALID_HANDLE_VALUE) )
            CloseHandle( _nextImageFrameEvent );
        if ( _context ) _context->Release();
        return true;
    }
    
    virtual bool handleJoints( osg::Vec3d* points, NUI_SKELETON_DATA& skeleton )
    {
        // TODO
        return true;
    }
    
    virtual bool handleImages( const std::string& buffer, NUI_IMAGE_FRAME& image )
    {
        // TODO
        return true;
    }
    
protected:
    DWORD WINAPI runNuiThread()
    {
        const int numEvents = 3;
        HANDLE events[numEvents] = { _nuiProcessStopEvent, _nextSkeletonEvent, _nextImageFrameEvent };
        int eventIndex = 0;
        DWORD trackingID = -1;
        
    	NUI_TRANSFORM_SMOOTH_PARAMETERS smoothParams;
        smoothParams.fCorrection = 0.5f;
        smoothParams.fJitterRadius = 1.0f;
        smoothParams.fMaxDeviationRadius = 0.5f;
        smoothParams.fPrediction = 0.4f;
        smoothParams.fSmoothing = 0.2f;
        
        bool continueProcessing = true;
        while ( continueProcessing )
        {
            eventIndex = WaitForMultipleObjects( numEvents, events, FALSE, 100 );
            switch ( eventIndex )
            {
            case WAIT_TIMEOUT: continue;
            case WAIT_OBJECT_0: continueProcessing = false; continue;
            case WAIT_OBJECT_0+1:  // get skeleton
                if ( _context )
                {
                    NUI_SKELETON_FRAME skeletonFrame = {0};
                    if ( FAILED(_context->NuiSkeletonGetNextFrame(0, &skeletonFrame)) ) continue;
                    
                    HRESULT hr = _context->NuiTransformSmooth( &skeletonFrame, &smoothParams );
                    if ( FAILED(hr) ) continue;
    
                    int startIndex = 0, maxCount = NUI_SKELETON_COUNT;
                    for ( int n=0 ; n<maxCount; ++n )
                    {
                        NUI_SKELETON_DATA& skeletonData = skeletonFrame.SkeletonData[n];
                        if ( trackingID==skeletonData.dwTrackingID )
                        {
                            startIndex = n;
                            break;
                        }
                    }
                    
                    for ( int n=startIndex; n<maxCount; )
                    {
                        NUI_SKELETON_DATA& skeletonData = skeletonFrame.SkeletonData[n];
                        if ( skeletonData.eTrackingState==NUI_SKELETON_TRACKED &&
                             skeletonData.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER]!=NUI_SKELETON_POSITION_NOT_TRACKED )
                        {
                            POINT coordInDepth; USHORT depth = 0;
                            osg::Vec3d points[NUI_SKELETON_POSITION_COUNT];
                            for ( int i=0; i<NUI_SKELETON_POSITION_COUNT; ++i )
                            {
                                osg::Vec3d& point = points[i];
                                NuiTransformSkeletonToDepthImage(
                                    skeletonData.SkeletonPositions[i], &coordInDepth.x, &coordInDepth.y, &depth, KINECT_IMAGE_RESOLUTION );
                                point.x() = (double)coordInDepth.x / KINECT_IMAGE_WIDTH;  // 0.0 - 1.0
                                point.y() = (double)coordInDepth.y / KINECT_IMAGE_HEIGHT;  // 0.0 - 1.0
                                
                                // Transform depth to [0, 1], assuming original depth from 0 to 4000 in millmeters
                                point.z() = (double)(depth >> NUI_IMAGE_PLAYER_INDEX_SHIFT);
                                point.z() *= 0.00025;  // real value can't be less than 800, that is, 0.2
                            }
                            
                            // Transfer the results
    						if ( handleJoints(points, skeletonData) )
                            {
                                trackingID = skeletonData.dwTrackingID;
                                break;  // Only handle the first available skeleton
                            }
                        }
    
                        if ( startIndex>0 && n==NUI_SKELETON_COUNT-1 )
                        {
                            n = 0;
                            maxCount = startIndex;
                        }
                        else ++n;
                    }
                }
                break;
            case WAIT_OBJECT_0+2:  // get depth image
                if ( _context && _videoStreamHandle )
                {
                    NUI_IMAGE_FRAME imageFrame;
                    HRESULT hr = _context->NuiImageStreamGetNextFrame( _videoStreamHandle, 0, &imageFrame );
                    if ( FAILED(hr) ) continue;
                    
                    INuiFrameTexture * nuiTexture = imageFrame.pFrameTexture;
                    NUI_LOCKED_RECT lockedRect;
                    nuiTexture->LockRect( 0, &lockedRect, NULL, 0 );
                    if ( lockedRect.Pitch!=NULL )
                    {
                        // FIXME: here we always assume the image is 640x480, also the copy operations may cost
                        std::string buffer(reinterpret_cast<char*>(lockedRect.pBits), KINECT_IMAGE_WIDTH*KINECT_IMAGE_HEIGHT*4);
                        handleImages( buffer, imageFrame );
                    }
                    nuiTexture->UnlockRect( 0 );
                    _context->NuiImageStreamReleaseFrame( _videoStreamHandle, &imageFrame );
                }
                break;
            default: break;
            }
        }
        return 0;
    }
    
    static DWORD WINAPI nuiProcessThread( LPVOID param )
    { KinectHandler* current = (KinectHandler*)param; return current->runNuiThread(); }
    
    INuiSensor* _context;
    HANDLE _nuiProcess;
    HANDLE _nuiProcessStopEvent;
    HANDLE _nextSkeletonEvent;
    HANDLE _nextImageFrameEvent;
    HANDLE _videoStreamHandle;
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
	return viewer.run();
}
