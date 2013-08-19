#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/EventVisitor>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <pxcsession.h>
#include <pxcsmartptr.h>
#include <pxccapture.h>

int main( int argc, char** argv )
{
    PXCSmartPtr<PXCSession> session;
    pxcStatus sts = PXCSession_Create( &session );
    if ( sts<PXC_STATUS_NO_ERROR )
    {
        OSG_WARN << "Failed to create an SDK session" << std::endl;
        return 1;
    }
    
    PXCSession::ImplDesc templateDesc;
    memset( &templateDesc, 0, sizeof(templateDesc) );
    templateDesc.group = PXCSession::IMPL_GROUP_SENSOR;
    templateDesc.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE/* | PXCSession::IMPL_SUBGROUP_AUDIO_CAPTURE*/;
    
    for ( int m=0; ; ++m )
    {
        PXCSession::ImplDesc desc;
        if ( session->QueryImpl(&templateDesc, m, &desc)<PXC_STATUS_NO_ERROR ) break;
        std::cout << "Found module[" << m << "]: " << desc.friendlyName << std::endl;
        
        PXCSmartPtr<PXCCapture> capture;
        session->CreateImpl<PXCCapture>( &desc, &capture );
        
        for ( int d=0; ; ++d )
        {
            PXCSmartPtr<PXCCapture::Device> device;
            if ( capture->CreateDevice(d, &device)<PXC_STATUS_NO_ERROR ) break;
            
            for ( int s=0; ; ++s )
            {
                PXCCapture::Device::StreamInfo info;
                if ( device->QueryStream(s, &info)<PXC_STATUS_NO_ERROR) break;
                
                switch ( info.cuid )
                {
                case PXCCapture::VideoStream::CUID:
                    switch ( info.imageType )
                    {
                    case PXCImage::IMAGE_TYPE_COLOR:
                        std::cout << "Color stream!" << std::endl;
                        break;
                    case PXCImage::IMAGE_TYPE_DEPTH:
                        std::cout << "Depth stream!" << std::endl;
                        break;
                    }
                    break;
                case PXCCapture::AudioStream::CUID:
                    std::cout << "Audio stream!" << std::endl;
                    break;
                }
            }
        }
        
        //
    }
    
    // OSG scene graph
    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
	return viewer.run();
}
