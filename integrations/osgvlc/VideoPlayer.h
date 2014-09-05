#ifndef H_VIDEOPLAYER
#define H_VIDEOPLAYER

#include <osg/ImageStream>
#include <vlc/vlc.h>

/** Video player based on VLC */
class VideoPlayer : public osg::ImageStream
{
public:
    static void* lockFunc( void* data, void** p_pixels );
    static void unlockFunc( void* data, void* id, void* const* p_pixels );
    static void displayFunc( void* data, void* id );
    static void videoStoppedFunc( const libvlc_event_t*, void* data );
    static void videoEndFunc( const libvlc_event_t*, void* data );
    
    VideoPlayer( const char* const* vlc_argv=0 );
    VideoPlayer( const VideoPlayer& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY );
    META_Object( osg, VideoPlayer )
    
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
    void open( const std::string& file, unsigned int w=1024, unsigned int h=1024 );
    int getPlayedTime() const { return _playedTime; }
    
    virtual void play();
    virtual void pause();
    virtual void rewind();
    virtual void quit( bool waitForThreadToExit=true );
    
    virtual void setReferenceTime( double time ) { libvlc_media_player_set_time(_vlcPlayer, (int)time); }
    virtual double getReferenceTime() const { return (int)libvlc_media_player_get_time(_vlcPlayer); }
    
    virtual void setTimeMultiplier( double m ) { libvlc_media_player_set_rate(_vlcPlayer, m); }
    virtual double getTimeMultiplier() const { return libvlc_media_player_get_rate(_vlcPlayer); }
    
    virtual void setVolume( float vol ) { libvlc_audio_set_volume(_vlcPlayer, (int)vol); }
    virtual float getVolume() const { return (int)libvlc_audio_get_volume(_vlcPlayer); }
    
    virtual bool requiresUpdateCall() const { return true; }
    virtual void update( osg::NodeVisitor* nv );
    
    libvlc_media_t* getMedia() { return _vlcMedia; }
    libvlc_media_player_t* getMediaPlayer() { return _vlcPlayer; }
    
protected:
    virtual ~VideoPlayer();
    
    libvlc_instance_t* _vlc;
    libvlc_media_t* _vlcMedia;
    libvlc_media_player_t* _vlcPlayer;
    int _playedTime;
    bool _requiresReplay;
};

#endif
