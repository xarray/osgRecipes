#include <FreeImage.h>
#include <osg/Image>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

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
    }
    
    virtual ~ReaderWriterFreeImage()
    {
#ifdef FREEIMAGE_LIB
        FreeImage_DeInitialise();
#endif
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
        
        if ( FreeImage_GetPalette(dib) || FreeImage_GetBPP(dib)==16 )
        {
            // Convert paletted image or 16-bit image to a standard one
            FIBITMAP* temp = FreeImage_ConvertTo32Bits(dib);
            FreeImage_Unload( dib );
            dib = temp;
        }
        
        BYTE* bits = FreeImage_GetBits(dib);
        unsigned int width = FreeImage_GetWidth(dib);
        unsigned int height = FreeImage_GetHeight(dib);
        if ( bits==0 || width==0 || height==0 )
        {
            OSG_WARN << "ReaderWriterFreeImage: Unable to load image file " << file
                     << ", maybe invalid." << std::endl;
            FreeImage_Unload( dib );
            return ReadResult::ERROR_IN_READING_FILE;
        }
        
        GLenum internalFormat = 0, dataType = 0;
        unsigned int pixelFormat = 0, dataSize = 0;
        int bpp = FreeImage_GetBPP(dib);
        FREE_IMAGE_TYPE type = FreeImage_GetImageType(dib);
        if ( type==FIT_RGBF || type==FIT_RGBAF )
        {
            internalFormat = (type==FIT_RGBF ? 3 : 4);
            dataType = GL_FLOAT;
            pixelFormat = GL_RGB;
            dataSize = width * height * internalFormat * sizeof(float);
        }
        else if ( type==FIT_FLOAT )
        {
            switch ( bpp )
            {
            case 32:
                internalFormat = 1;
                pixelFormat = GL_ALPHA;
                break;
            case 96:
                internalFormat = 3;
                pixelFormat = GL_RGB;
                break;
            case 128:
                internalFormat = 4;
                pixelFormat = GL_RGBA;
                break;
            }
            dataType = GL_FLOAT;
            dataSize = width * height * internalFormat * sizeof(float);
        }
        else
        {
            switch ( bpp )
            {
            case 8:
                internalFormat = 1;
                pixelFormat = GL_ALPHA;
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
        }
        
        unsigned char* imageData = NULL;
        switch ( internalFormat )
        {
        case 1: case 3: case 4:
            imageData = new unsigned char[dataSize];
            memcpy( imageData, bits, dataSize );
            FreeImage_Unload( dib );
            break;
        default:
            OSG_WARN << "ReaderWriterFreeImage: Unable to load image file " << file
                     << ", unsupported bpp = " << bpp << std::endl;
            FreeImage_Unload( dib );
            return ReadResult::ERROR_IN_READING_FILE;
        }
        
        osg::Image* image = new osg::Image;
        image->setImage( width, height, 1, internalFormat, pixelFormat, dataType,
            static_cast<unsigned char*>(imageData), osg::Image::USE_NEW_DELETE );
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
};

REGISTER_OSGPLUGIN( freeimage, ReaderWriterFreeImage )
