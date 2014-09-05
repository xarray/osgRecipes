#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgGA/GUIEventAdapter>
#include <sstream>

#include "VideoPlayer.h"

/* VideoPlayer */

void* VideoPlayer::lockFunc( void* data, void** p_pixels )
{
    VideoPlayer* stream = (VideoPlayer*)data;
    *p_pixels = (void*)stream->data();
    return NULL;
}

void VideoPlayer::unlockFunc( void* data, void* id, void* const* p_pixels )
{
    VideoPlayer* stream = (VideoPlayer*)data;
    stream->dirty();
}

void VideoPlayer::displayFunc( void* data, void* id )
{
}

void VideoPlayer::videoStoppedFunc( const libvlc_event_t*, void* data )
{
    VideoPlayer* stream = (VideoPlayer*)data;
    stream->_status = INVALID;
}

void VideoPlayer::videoEndFunc( const libvlc_event_t*, void* data )
{
    VideoPlayer* stream = (VideoPlayer*)data;
    stream->_status = INVALID;
    stream->_playedTime++;
    if ( stream->getLoopingMode()==osg::ImageStream::LOOPING )
        stream->_requiresReplay = true;
}

VideoPlayer::VideoPlayer( const char* const* vlc_argv )
:   osg::ImageStream(), _vlcMedia(0), _playedTime(0), _requiresReplay(false)
{
    if ( !vlc_argv )
    {
        const char* vlc_args[] = {
            "--ignore-config",      // don't use VLC's config
            "--quiet",//"--verbose=0",
            "--no-video-title-show",
            "--network-caching=120"
            //"--plugin-path=plugins/"
        };
        _vlc = libvlc_new( sizeof(vlc_args)/sizeof(vlc_args[0]), vlc_args );
    }
    else
        _vlc = libvlc_new( sizeof(vlc_argv)/sizeof(vlc_argv[0]), vlc_argv );
    _vlcPlayer = libvlc_media_player_new( _vlc );
    
    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager( _vlcPlayer );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerStopped, &VideoPlayer::videoStoppedFunc, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerEndReached, &VideoPlayer::videoEndFunc, this );
    _status = INVALID;
}

VideoPlayer::VideoPlayer( const VideoPlayer& copy, const osg::CopyOp& op )
:   osg::ImageStream(copy, op), _vlc(copy._vlc), _vlcMedia(copy._vlcMedia),
    _vlcPlayer(copy._vlcPlayer), _playedTime(copy._playedTime), _requiresReplay(copy._requiresReplay)
{
}

VideoPlayer::~VideoPlayer()
{
    if ( _status!=INVALID )
    {
        libvlc_media_player_stop( _vlcPlayer );
        libvlc_media_player_release( _vlcPlayer );
    }
    libvlc_release( _vlc );
}

void VideoPlayer::open( const std::string& file, unsigned int w, unsigned int h )
{
    if ( libvlc_media_player_is_playing(_vlcPlayer)==1 )
    {
        libvlc_media_player_stop( _vlcPlayer );
        libvlc_media_release( _vlcMedia );
    }
    
    std::string protocol = osgDB::getServerProtocol( file );
    if ( !protocol.empty() ) _vlcMedia = libvlc_media_new_location( _vlc, file.c_str() );
    else _vlcMedia = libvlc_media_new_path( _vlc, file.c_str() );
    
    libvlc_media_player_set_media( _vlcPlayer, _vlcMedia );
    libvlc_video_set_callbacks( _vlcPlayer, &VideoPlayer::lockFunc, &VideoPlayer::unlockFunc,
                                &VideoPlayer::displayFunc, this );
    libvlc_video_set_format( _vlcPlayer, "RGBA", w, h, w*4 );
    
    allocateImage( w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE );
    setInternalTextureFormat( GL_RGBA );
}

void VideoPlayer::play()
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

void VideoPlayer::pause()
{
    if ( _status==PLAYING )
    {
        libvlc_media_player_set_pause( _vlcPlayer, true );
        _status = PAUSED;
    }
}

void VideoPlayer::rewind()
{
    libvlc_media_player_stop( _vlcPlayer );
    libvlc_media_player_set_time( _vlcPlayer, 0 );
    _status = INVALID;
}

void VideoPlayer::quit( bool waitForThreadToExit )
{
    libvlc_media_player_stop( _vlcPlayer );
    libvlc_media_player_release( _vlcPlayer );
    _status = INVALID;
}

void VideoPlayer::update( osg::NodeVisitor* nv )
{
    if ( _requiresReplay )
    {
        rewind(); play();
        _requiresReplay = false;
    }
}

/* ReaderWriterVLC */

class ReaderWriterVLC : public osgDB::ReaderWriter
{
public:
    ReaderWriterVLC()
    {
        supportsExtension( "vlc", "VLC media reader/writer" );
    }
    
    virtual ~ReaderWriterVLC()
    {
    }
    
    virtual ReadResult readImage( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        std::string url = osgDB::getNameLessExtension(file);
        int w = 1024, h = 1024;
        
        osg::ref_ptr<VideoPlayer> image;
        if ( options )
        {
            int ww = atoi( options->getPluginStringData("height").c_str() );
            int hh = atoi( options->getPluginStringData("width").c_str() );
            if ( ww>0 ) w = ww; if ( hh>0 ) h = hh;
            
            const void* args = options->getPluginData("arguments");
            if ( args ) image = new VideoPlayer( (const char**)args );
        }
        
        if ( !image ) image = new VideoPlayer;
        image->open( url, w, h );
        return image.get();
    }
};

REGISTER_OSGPLUGIN( vlc, ReaderWriterVLC )
