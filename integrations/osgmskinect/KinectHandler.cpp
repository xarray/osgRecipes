#include "KinectHandler.h"

bool KinectHandler::initialize( int kid )
{
    HRESULT hr = NuiCreateSensorByIndex( 0, &_context );
    if ( FAILED(hr) )
    {
        OSG_WARN << "Failed to connect to Kinect device." << std::endl;
        return false;
    }
    
    DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON
                   |  NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH;
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
            OSG_WARN << "Kinect device failed with error code " << std::hex << (long)hr << std::dec << std::endl;
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
    else*/
    {
        hr = _context->NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH, KINECT_IMAGE_RESOLUTION,
                                           0, 2, _nextImageFrameEvent, &_videoStreamHandle );
    }
    //hr = _context->NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR, KINECT_IMAGE_RESOLUTION,
    //                                   0, 2, _nextImageFrameEvent, &_videoStreamHandle );
    if ( FAILED(hr) )
    {
        OSG_WARN << "Unable to create image stream. Error code " << std::hex << (long)hr << std::dec << std::endl;
        return false;
    }
    return true;
}

bool KinectHandler::quit()
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

bool KinectHandler::handleJoints( NUI_SKELETON_DATA& skeletonData )
{
    POINT coordInDepth; USHORT depth = 0;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    
    for ( int i=0; i<NUI_SKELETON_POSITION_COUNT; ++i )
    {
        osg::Vec3& point = (*_points)[i];
        NuiTransformSkeletonToDepthImage(
            skeletonData.SkeletonPositions[i], &coordInDepth.x, &coordInDepth.y, &depth, KINECT_IMAGE_RESOLUTION );
        point.x() = (double)coordInDepth.x / KINECT_IMAGE_WIDTH;  // 0.0 - 1.0
        point.y() = (double)coordInDepth.y / KINECT_IMAGE_HEIGHT;  // 0.0 - 1.0
        
        // Transform depth to [0, 1], assuming original depth from 0 to 4000 in millmeters
        point.z() = (double)(depth >> NUI_IMAGE_PLAYER_INDEX_SHIFT);
        point.z() *= 0.00025;  // real value can't be less than 800, that is, 0.2
    }
    _points->dirty();
    return true;
}

bool KinectHandler::handleImages( NUI_IMAGE_FRAME& imageFrame )
{
    INuiFrameTexture* nuiTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT lockedRect;
    nuiTexture->LockRect( 0, &lockedRect, NULL, 0 );
    if ( lockedRect.Pitch!=NULL )
    {
        const USHORT* current = (const USHORT*)lockedRect.pBits;
        const USHORT* bufferEnd = current + (KINECT_IMAGE_WIDTH * KINECT_IMAGE_HEIGHT);
        unsigned char* ptr = _depthBuffer;
        
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        while ( current<bufferEnd )
        {
            // Note that for depth stream, realDepth = depth & 0x0fff
            // But for depth with player index, realDepth = (depth & 0xfff8) >> 3
            USHORT depth = NuiDepthPixelToDepth(*current);
            *ptr = static_cast<unsigned char>(depth % 256);
            ++current; ++ptr;
        }
    }
    nuiTexture->UnlockRect( 0 );
    return true;
}

bool KinectHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        handleCurrentData();
    }
    return false;
}

DWORD WINAPI KinectHandler::runNuiThread()
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
						if ( handleJoints(skeletonData) )
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
                else handleImages( imageFrame );
                _context->NuiImageStreamReleaseFrame( _videoStreamHandle, &imageFrame );
            }
            break;
        default: break;
        }
    }
    return 0;
}
