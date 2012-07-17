#include "KinectHandler.h"

static void XN_CALLBACK_TYPE newUserFunc( UserGenerator& generator, XnUserID id, void* cookie )
{
    KinectHandler* kinect = (KinectHandler*)cookie;
    if ( kinect->needPoseDetection() )
        kinect->getSkeletonGenerator().GetPoseDetectionCap().StartPoseDetection( kinect->getPoseName(), id );
    else
        kinect->getSkeletonGenerator().GetSkeletonCap().RequestCalibration( id, TRUE );
}

static void XN_CALLBACK_TYPE lostUserFunc( UserGenerator& generator, XnUserID id, void* cookie )
{
}

static void XN_CALLBACK_TYPE calibrationStartFunc( SkeletonCapability& capability, XnUserID id, void* cookie )
{
}

static void XN_CALLBACK_TYPE calibrationCompleteFunc( SkeletonCapability& capability, XnUserID id, XnCalibrationStatus status, void* cookie )
{
    KinectHandler* kinect = (KinectHandler*)cookie;
    if ( status==XN_CALIBRATION_STATUS_OK )
        kinect->getSkeletonGenerator().GetSkeletonCap().StartTracking( id );
    else
    {
        if( status==XN_CALIBRATION_STATUS_MANUAL_ABORT )
            OSG_WARN << "Manual abort occured, so stop attempting to calibrate." << std::endl;
        else if ( kinect->needPoseDetection() )
            kinect->getSkeletonGenerator().GetPoseDetectionCap().StartPoseDetection( kinect->getPoseName(), id );
        else
            kinect->getSkeletonGenerator().GetSkeletonCap().RequestCalibration( id, TRUE );
    }
}

static void XN_CALLBACK_TYPE poseDetectedFunc( PoseDetectionCapability& capability, const XnChar* pose, XnUserID id, void* cookie )
{
    KinectHandler* kinect = (KinectHandler*)cookie;
    kinect->getSkeletonGenerator().GetPoseDetectionCap().StopPoseDetection( id );
    kinect->getSkeletonGenerator().GetSkeletonCap().RequestCalibration( id, TRUE );
}

bool KinectHandler::initialize( const char* configFile )
{
    XnStatus status;
    if ( configFile==NULL )
    {
        status = _context.Init();
        if ( status!=XN_STATUS_OK )
        {
            OSG_WARN << "Failed to connect to Kinect device: " << xnGetStatusString(status) << std::endl;
            return false;
        }
        
        status = _skeleton.Create( _context );
        if ( status!=XN_STATUS_OK )
        {
            OSG_WARN << "Failed to create user node: " << xnGetStatusString(status) << std::endl;
            return false;
        }
        
        status = _depth.Create( _context );
        //status = _image.Create( _context );
        if ( status!=XN_STATUS_OK )
        {
            OSG_WARN << "Failed to create depth node: " << xnGetStatusString(status) << std::endl;
            return false;
        }
    }
    else
    {
        EnumerationErrors errors;
        status = _context.InitFromXmlFile( configFile, _scriptNode, &errors );
        if ( status!=XN_STATUS_OK )
        {
            OSG_WARN << "Failed to connect to Kinect device: " << xnGetStatusString(status) << std::endl;
            return false;
        }
        
        status = _context.FindExistingNode( XN_NODE_TYPE_USER, _skeleton );
        if ( status!=XN_STATUS_OK )
        {
            OSG_WARN << "Failed to find user node: " << xnGetStatusString(status) << std::endl;
            return false;
        }
        
        status = _context.FindExistingNode( XN_NODE_TYPE_DEPTH, _depth );
        //status = _context.FindExistingNode( XN_NODE_TYPE_IMAGE, _image );
        if ( status!=XN_STATUS_OK )
        {
            OSG_WARN << "Failed to find depth node: " << xnGetStatusString(status) << std::endl;
            return false;
        }
    }
    
    if ( !_skeleton.IsCapabilitySupported(XN_CAPABILITY_SKELETON) )
    {
        OSG_WARN << "Current user node doesn't support skeleton: " << xnGetStatusString(status) << std::endl;
        return false;
    }
    
    XnCallbackHandle userCallbackHandle, calibrationStartHandle, calibrationCompleteHandle, poseDetectedHandle;
    _skeleton.RegisterUserCallbacks( newUserFunc, lostUserFunc, this, userCallbackHandle );
    _skeleton.GetSkeletonCap().RegisterToCalibrationStart( calibrationStartFunc, this, calibrationStartHandle );
    _skeleton.GetSkeletonCap().RegisterToCalibrationComplete( calibrationCompleteFunc, this, calibrationCompleteHandle );
    if ( _skeleton.GetSkeletonCap().NeedPoseForCalibration() )
    {
        if ( !_skeleton.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION) )
        {
            OSG_WARN << "Current user node doesn't support pose detection: " << xnGetStatusString(status) << std::endl;
            return false;
        }
        
        _needPoseDetection = true;
        _skeleton.GetPoseDetectionCap().RegisterToPoseDetected( poseDetectedFunc, this, poseDetectedHandle );
        _skeleton.GetSkeletonCap().GetCalibrationPose( _poseName );
    }
    
    XnMapOutputMode outputMode;
    outputMode.nXRes = KINECT_IMAGE_WIDTH;
    outputMode.nYRes = KINECT_IMAGE_HEIGHT;
    outputMode.nFPS = 30;
    _depth.SetMapOutputMode( outputMode );
    //_image.SetMapOutputMode( outputMode );
    
    _skeleton.GetSkeletonCap().SetSkeletonProfile( XN_SKEL_PROFILE_ALL );
    //_depth.GetAlternativeViewPointCap().SetViewPoint( _image );
    _context.StartGeneratingAll();
    return true;
}

bool KinectHandler::quit()
{
    _context.StopGeneratingAll();
    _scriptNode.Release();
    _depth.Release();
    _skeleton.Release();
    _context.Release();
    return true;
}

bool KinectHandler::handleJoints( DepthMetaData& depthMetaData )
{
    static const int c_maxNumUsers = 15;
    XnUserID users[c_maxNumUsers];
    XnUInt16 current = 0, numUsers = c_maxNumUsers;
    _skeleton.GetUsers( users, numUsers );
    for ( XnUInt16 current=0; current<numUsers; ++current )
    {
        if ( _skeleton.GetSkeletonCap().IsTracking(users[current])==TRUE )
            break;
    }
    
    unsigned int width = depthMetaData.XRes(), height = depthMetaData.YRes(), zMax = depthMetaData.ZRes();
    if ( current<numUsers )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        for ( int i=0; i<KINECT_SKELETON_COUNT; ++i )
        {
            XnSkeletonJointTransformation joint;
            _skeleton.GetSkeletonCap().GetSkeletonJoint(users[current], (XnSkeletonJoint)(i+1), joint );
            
            XnPoint3D proj;
            _depth.ConvertRealWorldToProjective( 1, &(joint.position.position), &proj );
            proj.X = proj.X / (float)width;
            proj.Y = proj.Y / (float)height;
            proj.Z = proj.Z / (float)zMax;  // convert all to [0, 1]
            (*_points)[i].set( proj.X, proj.Y, proj.Z );
        }
        _points->dirty();
    }
    return true;
}

bool KinectHandler::handleImages( DepthMetaData& depthMetaData )
{
    XnDepthPixel zMax = depthMetaData.ZRes();
    const XnDepthPixel* depthData = depthMetaData.Data();
    unsigned int size = depthMetaData.XRes() * depthMetaData.YRes();
    if ( depthData!=NULL )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        for ( unsigned int i=0; i<size; ++i )
        {
            *(_depthBuffer + i) = 255 * (float(*(depthData + i)) / float(zMax));
        }
    }
    return true;
}

bool KinectHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
    {
        XnStatus status = _context.WaitAndUpdateAll();
        if ( status!=XN_STATUS_OK )
        {
            OSG_WARN << "Failed to read Kinect data." << std::endl;
            return false;
        }
        
        DepthMetaData depthMetaData;
        _depth.GetMetaData( depthMetaData );
        if ( handleImages(depthMetaData) )
        {
            if ( handleJoints(depthMetaData) )
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
                handleCurrentData();
            }
        }
    }
    return false;
}
