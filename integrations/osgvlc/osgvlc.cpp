#include <osg/ImageStream>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <vlc/vlc.h>

#define VLC_PLUGIN_PATH "plugins/"

class VLCImageStream : public osg::ImageStream
{
public:
    static void* lockFunc( void* data, void** p_pixels )
    {
        VLCImageStream* stream = (VLCImageStream*)data;
        *p_pixels = (void*)stream->data();
        return NULL;
    }
    
    static void unlockFunc( void* data, void* id, void* const* p_pixels )
    {
        VLCImageStream* stream = (VLCImageStream*)data;
        stream->dirty();
    }
    
    static void displayFunc( void* data, void* id )
    {
    }
    
    static void videoEndFunc( const libvlc_event_t*, void* data )
    {
        VLCImageStream* stream = (VLCImageStream*)data;
        stream->_status = INVALID;
    }
    
    VLCImageStream( const char* const* vlc_argv=0 )
    :   osg::ImageStream(), _vlcMedia(0)
    {
        if ( !vlc_argv )
        {
            const char* vlc_args[] = {
                "--ignore-config",      // don't use VLC's config
                "--data-path="VLC_PLUGIN_PATH
            };
            _vlc = libvlc_new( sizeof(vlc_args)/sizeof(vlc_args[0]), vlc_args );
        }
        else
            _vlc = libvlc_new( sizeof(vlc_argv)/sizeof(vlc_argv[0]), vlc_argv );
        _vlcPlayer = libvlc_media_player_new( _vlc );
        
        libvlc_event_attach( libvlc_media_player_event_manager(_vlcPlayer), libvlc_MediaPlayerStopped,
                             &VLCImageStream::videoEndFunc, this );
        _status = INVALID;
    }
    
    VLCImageStream( const VLCImageStream& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY )
    :   osg::ImageStream(copy, op), _vlc(copy._vlc), _vlcMedia(copy._vlcMedia),
        _vlcPlayer(copy._vlcPlayer) {}
    
    META_Object( osg, VLCImageStream )
    
    /** File has to be in one of the following formats:
        [file://]filename              Plain media file
        http://ip:port/file            HTTP URL
        ftp://ip:port/file             FTP URL
        mms://ip:port/file             MMS URL
        screen://                      Screen capture
        [dvd://][device][@raw_device]  DVD device
        [vcd://][device]               VCD device
        [cdda://][device]              Audio CD device
        udp:[[<source address>]@[<bind address>][:<bind port>]]
    */
    void open( const std::string& file, bool needPlay=true, unsigned int w=512, unsigned int h=512 )
    {
        _vlcMedia = libvlc_media_new_path( _vlc, file.c_str() );
        libvlc_media_player_set_media( _vlcPlayer, _vlcMedia );
        libvlc_video_set_callbacks( _vlcPlayer, &VLCImageStream::lockFunc, &VLCImageStream::unlockFunc,
                                    &VLCImageStream::displayFunc, this );
        libvlc_video_set_format( _vlcPlayer, "RGBA", w, h, w*4 );
        
        allocateImage( w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE );
        if ( needPlay ) play();
    }
    
    virtual void play()
    {
        if ( _status==PAUSED )
        {
            libvlc_media_player_set_pause( _vlcPlayer, false );
        }
        else if ( _status!=PLAYING )
        {
            libvlc_media_player_play( _vlcPlayer );
        }
        _status = PLAYING;
    }
    
    virtual void pause()
    {
        libvlc_media_player_set_pause( _vlcPlayer, true );
        _status = PAUSED;
    }

    virtual void rewind()
    {
        libvlc_media_player_stop( _vlcPlayer );
        libvlc_media_player_set_time( _vlcPlayer, 0 );
        libvlc_media_player_next_frame( _vlcPlayer );
        _status = INVALID;
    }

    virtual void quit( bool waitForThreadToExit=true )
    {
        libvlc_media_player_stop( _vlcPlayer );
        libvlc_media_player_release( _vlcPlayer );
        _status = INVALID;
    }
    
    virtual void setReferenceTime( double time ) { libvlc_media_player_set_time(_vlcPlayer, (int)time); }
    virtual double getReferenceTime() const { return (int)libvlc_media_player_get_time(_vlcPlayer); }
    
    virtual void setTimeMultiplier( double m ) { libvlc_video_set_scale(_vlcPlayer, m); }
    virtual double getTimeMultiplier() const { return libvlc_video_get_scale(_vlcPlayer); }
    
    virtual void setVolume( float vol ) { libvlc_audio_set_volume(_vlcPlayer, (int)vol); }
    virtual float getVolume() const { return (int)libvlc_audio_get_volume(_vlcPlayer); }
    
protected:
    virtual ~VLCImageStream()
    {
        if ( _status!=INVALID )
        {
            libvlc_media_player_stop( _vlcPlayer );
            libvlc_media_player_release( _vlcPlayer );
        }
        libvlc_release( _vlc );
    }
    
    libvlc_instance_t* _vlc;
    libvlc_media_t* _vlcMedia;
    libvlc_media_player_t* _vlcPlayer;
};

osg::Node* createVideoQuad( const osg::Vec3& corner, const std::string& file )
{
    osg::ref_ptr<VLCImageStream> imageStream = new VLCImageStream;
    if ( imageStream ) imageStream->open( file );
    
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( imageStream.get() );
    texture->setResizeNonPowerOfTwoHint( false );
    
    osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
        corner, osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f, 1.0f, 0.0f );
    quad->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( quad.get() );
    return geode.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    std::string file("plush1_720p_10s.m2v");  // Borrowed from CUDA (an H.264 segment)
    if ( arguments.argc()>1 ) file = arguments[1];
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( createVideoQuad(osg::Vec3(), file) );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
