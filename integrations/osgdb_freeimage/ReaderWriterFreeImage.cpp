#include <FreeImage.h>
#include <osg/Image>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <iostream>
#include <sstream>

static unsigned int DLL_CALLCONV global_readProc( void* buffer, unsigned int size, unsigned int count, fi_handle handle )
{
    std::istream* fin = (std::istream*)handle;
    if ( fin )
    {
        char* local = (char*)buffer;
        for ( unsigned c=0; c<count; ++c )
        {
            fin->read( local, size );
            local += size;
        }
        return count;
    }
    return 0;
}

static unsigned int DLL_CALLCONV global_writeProc( void* buffer, unsigned int size, unsigned int count, fi_handle handle )
{
    // TODO: not implemented
    return size;
}

static int DLL_CALLCONV global_seekProc( fi_handle handle, long offset, int origin )
{
    std::istream* fin = (std::istream*)handle;
    if ( fin )
    {
        if ( origin==SEEK_SET )
        {
            fin->seekg( offset, std::ios::beg );
        }
        else
        {
            fin->seekg( offset, std::ios::cur );
        }
    }
    return 0;
}

static long DLL_CALLCONV global_tellProc( fi_handle handle )
{
    std::istream* fin = (std::istream*)handle;
    return fin ? fin->tellg() : 0;
}

class ReaderWriterFreeImage : public osgDB::ReaderWriter
{
public:
    ReaderWriterFreeImage()
    {
#ifdef FREEIMAGE_LIB
        FreeImage_Initialise();
#endif
        
        supportsExtension( "freeimage", "FreeImage reader/writer" );
        supportsExtension( "bmp", "" ); supportsExtension( "wbmp", "" );
        supportsExtension( "dds", "" );
        supportsExtension( "exr", "" );
        supportsExtension( "gif", "" );
        supportsExtension( "hdr", "" );
        supportsExtension( "ico", "" );
        supportsExtension( "iff", "" );
        supportsExtension( "jng", "" );
        supportsExtension( "jpg", "" ); supportsExtension( "jpeg", "" );
        supportsExtension( "jp2", "" );
        supportsExtension( "mng", "" );
        supportsExtension( "pbm", "" ); supportsExtension( "ppm", "" );
        supportsExtension( "pcx", "" );
        supportsExtension( "png", "" );
        supportsExtension( "psd", "" );
        supportsExtension( "tga", "" );
        supportsExtension( "tif", "" ); supportsExtension( "tiff", "" );
        supportsExtension( "xbm", "" );
        supportsExtension( "xpm", "" );
        
        supportsOption( "format=<image format>", "Set the image format when reading from stream, e.g. 'format=BMP'" );
    }
    
    virtual ~ReaderWriterFreeImage()
    {
#ifdef FREEIMAGE_LIB
        FreeImage_DeInitialise();
#endif
    }
    
    virtual ReadResult readImage( std::istream& fin, const osgDB::ReaderWriter::Options* options ) const
    {
        FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
        if ( options )
        {
            if ( options->getNumPluginStringData()>0 )
            {
                std::string format = options->getPluginStringData("format");
                fif = FreeImage_GetFIFFromFormat( format.c_str() );
            }
            else
                fif = FreeImage_GetFIFFromFormat( options->getOptionString().c_str() );
        }
        
        if ( fif==FIF_UNKNOWN )
        {
            OSG_WARN << "ReaderWriterFreeImage: Please specify the 'format=???' option before reading a stream." << std::endl;
            return NULL;
        }
        
        FreeImageIO io;
        io.read_proc = global_readProc;
        io.write_proc = global_writeProc;
        io.tell_proc = global_tellProc;
        io.seek_proc = global_seekProc;
        
        FIBITMAP* dib = FreeImage_LoadFromHandle( fif, &io, (fi_handle)&fin );
        if ( !dib ) return ReadResult::FILE_NOT_HANDLED;
        
        osg::Image* image = readImageData(dib);
        if ( !image ) return ReadResult::ERROR_IN_READING_FILE;
        return image;
    }
    
    virtual ReadResult readImage( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( ext=="freeimage" ) return readImage(osgDB::getNameLessExtension(file), options);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        std::string fileName = osgDB::findDataFile( file, options );
        if ( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;
        
        FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename( fileName.c_str() );
        if ( fif==FIF_UNKNOWN ) fif = FreeImage_GetFileType( fileName.c_str(), 0 );
        if ( fif==FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif) )
            return ReadResult::FILE_NOT_HANDLED;
        
        FIBITMAP* dib = FreeImage_Load( fif, fileName.c_str() );
        if ( !dib ) return ReadResult::FILE_NOT_HANDLED;
        
        osg::Image* image = readImageData(dib);
        if ( !image ) return ReadResult::ERROR_IN_READING_FILE;
        return image;
    }
    
    virtual WriteResult writeImage( const osg::Image& image, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( ext=="freeimage" ) return writeImage(image, osgDB::getNameLessExtension(file), options);
        
        FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename( file.c_str() );
        if ( fif==FIF_UNKNOWN || !FreeImage_FIFSupportsWriting(fif) )
            return WriteResult::FILE_NOT_HANDLED;
        
        // TODO
        return WriteResult::FILE_NOT_HANDLED;
    }
    
protected:
    osg::Image* readImageData( FIBITMAP* src ) const
    {
        FIBITMAP* dib = src;
        if ( FreeImage_GetPalette(dib) || FreeImage_GetBPP(dib)<8 )
        {
            // Convert paletted images to standard ones
            FIBITMAP* temp = FreeImage_ConvertTo32Bits(dib);
            FreeImage_Unload( dib );
            dib = temp;
        }
        
        BYTE* bits = FreeImage_GetBits(dib);
        unsigned int width = FreeImage_GetWidth(dib);
        unsigned int height = FreeImage_GetHeight(dib);
        if ( bits==0 || width==0 || height==0 )
        {
            OSG_WARN << "ReaderWriterFreeImage: Unable to load image, maybe it is invalid." << std::endl;
            FreeImage_Unload( dib );
            return NULL;
        }
        
        // Determine the attributes of the image
        GLenum internalFormat = 0, dataType = 0;
        unsigned int pixelFormat = 0, dataSize = 0;
        int bpp = FreeImage_GetBPP(dib);
        FREE_IMAGE_TYPE type = FreeImage_GetImageType(dib);
        switch ( type )
        {
        case FIT_BITMAP:  // standard image : 8-, 16-, 24-, 32-bit
            switch ( bpp )
            {
            case 8:
                internalFormat = 1;
                pixelFormat = GL_ALPHA;  // FIXME: only consider as ALPHA?
                break;
            case 16:
                internalFormat = 2;
                pixelFormat = GL_LUMINANCE_ALPHA;
                break;
            case 24:
                internalFormat = 3;
                pixelFormat = GL_RGB;
                break;
            case 32:
                internalFormat = 4;
                pixelFormat = GL_RGBA;
                break;
            }
            dataType = GL_UNSIGNED_BYTE;
            dataSize = width * height * internalFormat * sizeof(unsigned char);
            break;
        case FIT_RGB16: case FIT_RGBA16:  // 3(4) x 16-bit
            internalFormat = (type==FIT_RGB16 ? 3 : 4);
            pixelFormat = (type==FIT_RGB16 ? GL_RGB : GL_RGBA);
            dataType = GL_UNSIGNED_SHORT;
            dataSize = width * height * internalFormat * sizeof(unsigned short);
            break;
        case FIT_RGBF: case FIT_RGBAF:  // 3(4) x 32-bit IEEE floating point
            internalFormat = (type==FIT_RGBF ? 3 : 4);
            pixelFormat = (type==FIT_RGBF ? GL_RGB : GL_RGBA);
            dataType = GL_FLOAT;
            dataSize = width * height * internalFormat * sizeof(float);
            break;
        case FIT_COMPLEX:  // 2 x 64-bit IEEE floating point
            internalFormat = 2;
            pixelFormat = GL_LUMINANCE_ALPHA;
            dataType = GL_DOUBLE;
            dataSize = width * height * internalFormat * sizeof(double);
            break;
        default:
            internalFormat = 1;
            pixelFormat = GL_ALPHA;  // FIXME: only consider as ALPHA?
            if ( type==FIT_UINT16 || type==FIT_INT16 )
            {
                dataType = (type==FIT_UINT16 ? GL_UNSIGNED_SHORT : GL_SHORT);
                dataSize = width * height * internalFormat * sizeof(unsigned short);
            }
            else if ( type==FIT_UINT32 || type==FIT_INT32 )
            {
                dataType = (type==FIT_UINT32 ? GL_UNSIGNED_INT : GL_INT);
                dataSize = width * height * internalFormat * sizeof(unsigned long);
            }
            else if ( type==FIT_FLOAT )
            {
                dataType = GL_FLOAT;
                dataSize = width * height * internalFormat * sizeof(float);
            }
            else if ( type==FIT_DOUBLE )
            {
                dataType = GL_DOUBLE;
                dataSize = width * height * internalFormat * sizeof(double);
            }
            break;
        }
        
        // Copy image data and create new OSG object
        unsigned char* imageData = NULL;
        switch ( internalFormat )
        {
        case 1: case 2: case 3: case 4:
            imageData = new unsigned char[dataSize];
            convertImageData( (char*)imageData, (char*)bits, dataSize, internalFormat, dataType );
            FreeImage_Unload( dib );
            break;
        default:
            OSG_WARN << "ReaderWriterFreeImage: Unable to load image, unsupported bpp = " << bpp << std::endl;
            FreeImage_Unload( dib );
            return NULL;
        }
        
        osg::Image* image = new osg::Image;
        image->setImage( width, height, 1, internalFormat, pixelFormat, dataType,
            static_cast<unsigned char*>(imageData), osg::Image::USE_NEW_DELETE );
        return image;
    }
    
    void convertImageData( char* dest, const char* src, unsigned int size, int internalFormat, int dataType ) const
    {
#if FREEIMAGE_COLORORDER==FREEIMAGE_COLORORDER_BGR
        // This really only affects 24 and 32 bit formats, the rest are always RGB order.
        if ( dataType==GL_UNSIGNED_BYTE )
        {
            if ( internalFormat==3 )  // BGR8 -> RGB8
            {
                for ( unsigned int i=0; i<size; i+=3 )
                {
                    dest[i+0] = src[i+2];
                    dest[i+1] = src[i+1];
                    dest[i+2] = src[i+0];
                }
                return;
            }
            else if ( internalFormat==4 )  // BGRA8 -> RGBA8
            {
                for ( unsigned int i=0; i<size; i+=4 )
                {
                    dest[i+0] = src[i+2];
                    dest[i+1] = src[i+1];
                    dest[i+2] = src[i+0];
                    dest[i+3] = src[i+3];
                }
                return;
            }
        }
#endif
        memcpy( dest, src, size );
    }
};

REGISTER_OSGPLUGIN( freeimage, ReaderWriterFreeImage )
