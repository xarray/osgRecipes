// Download ALVAR at http://virtual.vtt.fi/virtual/proj2/multimedia/alvar/index.html
// It already includes OSG examples but this is more straightforward to OSG developers
// Usage: ./osgalvar --image image_to_track.jpg cessna.osg
// Or you may create a classifier first using SampleMarkerlessCreator in ALVAR SDK and run:
// ./ osgalvar --classifier image_to_track.jpg.dat cessna.osg
// The cessna model will appear if the image is found and tracked in your webcam (augmented reality)

#include <osg/io_utils>
#include <osg/Depth>
#include <osg/Texture2D>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Switch>
#include <osg/MatrixTransform>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <Camera.h>
#include <Pose.h>
#include <CaptureFactory.h>
#include <FernImageDetector.h>
#include <FernPoseEstimator.h>

#include <iostream>
#include <sstream>
#include <fstream>

class DetectHandler : public osgGA::GUIEventHandler
{
public:
    DetectHandler( const alvar::CaptureDevice& device, const std::string& settingFile=std::string(),
                   const std::string& calibrationFile="calib.xml" )
    {
        _capturedImage = new osg::Image;
        _capturer = alvar::CaptureFactory::instance()->createCapture( device );
        _capturer->start();
        if ( !settingFile.empty() )
            _capturer->loadSettings( settingFile );
        
        // Reload the captured image until it is sure to work
        IplImage* dummy = NULL;
        for ( int i=0; i<10; ++i )
        {
            dummy = _capturer->captureImage();
            if ( dummy ) break;
            alvar::sleep( 100 );
        }
        
        _calibrationFile = calibrationFile;
        if ( !calibrationFile.empty() )
            _fernEstimator.setCalibration( calibrationFile, dummy->width, dummy->height );
        else
            _fernEstimator.setResolution( dummy->width, dummy->height );
        _gray = cv::Mat(dummy);
    }
    
    virtual ~DetectHandler()
    {
        if ( _capturer )
        {
            _capturer->stop();
            delete _capturer;
        }
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( ea.getEventType()==osgGA::GUIEventAdapter::RESIZE )
        {
            int w = ea.getWindowWidth(), h = ea.getWindowHeight();
            double matrixPtr[16];
            alvar::Camera cam;
            cam.SetCalib( _calibrationFile.c_str(), w, h );
            cam.GetOpenglProjectionMatrix( matrixPtr, w, h );
            
            osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
            if ( view ) view->getCamera()->setProjectionMatrix( osg::Matrix(matrixPtr) );
        }
        else if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
        {
            // Display the captured image
            IplImage* cvImage = _capturer->captureImage();
            if ( cvImage->nChannels==3 )
                _capturedImage->setImage( cvImage->width, cvImage->height, 1, 4, GL_BGR_EXT, GL_UNSIGNED_BYTE,
                                          (unsigned char*)cvImage->imageData, osg::Image::NO_DELETE );
            else if( cvImage->nChannels==1 )
                _capturedImage->setImage( cvImage->width, cvImage->height, 1, 4, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                                          (unsigned char*)cvImage->imageData, osg::Image::NO_DELETE );
            
            // Find features and display them
            if ( cvImage->nChannels==3 )
            {
                cv::Mat imgMat = cvarrToMat( cvImage );
                cv::cvtColor( imgMat, _gray, CV_RGB2GRAY );
            }
            else _gray = cvImage;
            
            std::vector<CvPoint2D64f> ipts;
            std::vector<CvPoint3D64f> mpts;
            for ( unsigned int i=0; i<_fernDetectors.size(); ++i )
            {
                alvar::FernImageDetector& detector = _fernDetectors[i];
                detector.findFeatures( _gray, true );
                detector.imagePoints( ipts );
                detector.modelPoints( mpts, true );
                double test = detector.inlierRatio();
                if ( test>0.25 && mpts.size()>12 )
                {
                    double matrixPtr[16];
                    _fernEstimator.calculateFromPointCorrespondences( mpts, ipts );
                    
                    alvar::Pose pose = _fernEstimator.pose();
                    pose.GetMatrixGL( matrixPtr );
                    handleTrackedObject( i, osg::Matrix(matrixPtr) );
                }
                else
                    handleUntrackedObject( i );
                ipts.clear();
                mpts.clear();
            }
        }
        return false;
    }
    
    virtual void handleUntrackedObject( unsigned int index )
    {
        _modelSwitch->setValue( index, false );
    }
    
    virtual void handleTrackedObject( unsigned int index, const osg::Matrix& matrix )
    {
        std::cout << index << ": POS " << matrix.getTrans() << std::endl;
        _modelSwitch->setValue( index, true );
        
        osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>( _modelSwitch->getChild(index) );
        if ( mt ) mt->setMatrix( matrix );
    }
    
    bool addMarkerlessImage( const std::string& file, osg::Node* node )
    {
        std::string realFile = osgDB::findDataFile( file );
        if ( !realFile.empty() )
        {
            alvar::FernImageDetector detector;
            detector.train( realFile );
            _fernDetectors.push_back( detector );
            _modelSwitch->addChild( node, false );
            OSG_NOTICE << "*** Add markerless image: " << file << std::endl;
            return true;
        }
        else
            OSG_NOTICE << "*** Can't add markerless image: " << file << std::endl;
        return false;
    }
    
    bool addMarkerlessData( const std::string& file, osg::Node* node )
    {
        alvar::FernImageDetector detector;
        if ( detector.read(file) )
        {
            _fernDetectors.push_back( detector );
            _modelSwitch->addChild( node, false );
            OSG_NOTICE << "*** Add markerless classifier: " << file << std::endl;
            return true;
        }
        else
            OSG_NOTICE << "*** Can't add markerless classifier: " << file << std::endl;
        return false;
    }
    
    void setModelSwitchNode( osg::Switch* s ) { _modelSwitch = s; }
    osg::Switch* getModelSwitchNode() { return _modelSwitch.get(); }
    const osg::Switch* getModelSwitchNode() const { return _modelSwitch.get(); }
    
    osg::Image* getCapturedImage() { return _capturedImage.get(); }
    const osg::Image* getCapturedImage() const { return _capturedImage.get(); }
    
protected:
    osg::ref_ptr<osg::Image> _capturedImage;
    osg::observer_ptr<osg::Switch> _modelSwitch;
    
    alvar::Capture* _capturer;
    alvar::FernPoseEstimator _fernEstimator;
    std::vector<alvar::FernImageDetector> _fernDetectors;
    std::string _calibrationFile;
    cv::Mat _gray;
};

osg::Camera* createFullScreenImageCamera( osg::Image* image, bool inverseX, bool inverseY )
{
    float l = 0.0f, b = 0.0f, r = 1.0f, t = 1.0f;
    if ( inverseX ) { l = 1.0f; r = 0.0f; }
    if ( inverseY ) { b = 1.0f; t = 0.0f; }
    osg::ref_ptr<osg::Geode> imageQuad = new osg::Geode;
    imageQuad->addDrawable(
        osg::createTexturedQuadGeometry(osg::Vec3(), osg::X_AXIS, osg::Y_AXIS, l, b, r, t) );
    
    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D( image );
    tex2D->setResizeNonPowerOfTwoHint( false );
    imageQuad->getOrCreateStateSet()->setTextureAttributeAndModes( 0, tex2D.get() );
    
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearMask( 0 );
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setCullingActive( false );
    camera->setAllowEventFocus( false );
    camera->setProjectionMatrix( osg::Matrix::ortho2D(0.0, 1.0, 1.0, 0.0) );
    camera->addChild( imageQuad.get() );
    camera->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    camera->getOrCreateStateSet()->setAttributeAndModes( new osg::Depth(osg::Depth::LEQUAL, 1.0, 1.0) );
    return camera.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    unsigned int devID = 0; arguments.read( "--camera-id", devID );
    std::string devName; arguments.read( "--camera-name", devName );
    std::string camSettings; arguments.read( "--camera-settings", camSettings );
    
    std::string classifier; arguments.read( "--classifier", classifier );
    std::string classifierImage; arguments.read( "--image", classifierImage );
    
    alvar::CaptureFactory::CaptureDeviceVector devices = alvar::CaptureFactory::instance()->enumerateDevices();
    for ( unsigned int i=0; i<devices.size(); ++i )
    {
        if ( devName==devices[i].uniqueName() )
        { devID = i; break; }
    }
    OSG_NOTICE << "*** Use camera device: " << devices[devID].uniqueName() << std::endl;
    
    osg::ref_ptr<osg::MatrixTransform> model = new osg::MatrixTransform;
    model->addChild( osgDB::readNodeFiles(arguments) );
    
    osg::ref_ptr<DetectHandler> detector = new DetectHandler( devices[devID], camSettings );
    detector->setModelSwitchNode( new osg::Switch );
    if ( !classifierImage.empty() )
        detector->addMarkerlessImage( classifierImage, model.get() );
    else if ( !classifier.empty() )
        detector->addMarkerlessData( classifier, model.get() );
    
    // Setup root node and viewer
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( detector->getModelSwitchNode() );
    root->addChild( createFullScreenImageCamera(detector->getCapturedImage(), false, false) );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( detector.get() );
    viewer.setSceneData( root.get() );
    viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
    viewer.setUpViewOnSingleScreen( 0 );
    viewer.realize();
    
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>( viewer.getCamera()->getGraphicsContext() );
    if ( gw )
    {
        // Send window size event for initializing
        int x, y, w, h; gw->getWindowRectangle( x, y, w, h );
        viewer.getEventQueue()->windowResize( x, y, w, h );
    }
    while ( !viewer.done() )
    {
        viewer.frame();
    }
    return 0;
}
