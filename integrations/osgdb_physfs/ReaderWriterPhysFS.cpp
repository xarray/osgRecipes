#include <physfs.h>

#include <osg/Image>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <iostream>
#include <sstream>

class ReaderWriterPhysFS : public osgDB::ReaderWriter
{
public:
    ReaderWriterPhysFS()
    {
        PHYSFS_init( NULL );
        supportsExtension( "physfs", "PhysicsFS virtual file system" );
    }
    
    virtual ~ReaderWriterPhysFS()
    {
        PHYSFS_deinit();
    }
    
    virtual ReadResult readNode( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        std::string fileName = osgDB::getNameLessExtension(file);
        osgDB::ReaderWriter* rw = handleOptions(fileName, options);
        if ( !rw ) return ReadResult::FILE_NOT_HANDLED;
        
        std::string buffer;
        if ( !readDataFromVFS(buffer, fileName) )
            return ReadResult::ERROR_IN_READING_FILE;
        
        std::stringstream in; in << buffer;
        return rw->readNode( in, options );
    }
    
    virtual ReadResult readImage( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        std::string fileName = osgDB::getNameLessExtension(file);
        osgDB::ReaderWriter* rw = handleOptions(fileName, options);
        if ( !rw ) return ReadResult::FILE_NOT_HANDLED;
        
        std::string buffer;
        if ( !readDataFromVFS(buffer, fileName) )
            return ReadResult::ERROR_IN_READING_FILE;
        
        std::stringstream in; in << buffer;
        return rw->readImage( in, options );
    }
    
    virtual WriteResult writeNode( const osg::Image& image, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        std::string fileName = osgDB::getNameLessExtension(file);
        osgDB::ReaderWriter* rw = handleOptions(fileName, options);
        if ( !rw ) return WriteResult::FILE_NOT_HANDLED;
        
        // TODO
        return WriteResult::FILE_NOT_HANDLED;
    }
    
protected:
    bool readDataFromVFS( std::string& buffer, const std::string& fileName ) const
    {
        PHYSFS_File* handle = PHYSFS_openRead( fileName.c_str() );
        if ( !handle )
        {
            OSG_NOTICE << "[ReaderWriterPhysFS] Cannot open requested file: " << fileName << std::endl;
            return false;
        }
        
        PHYSFS_sint64 filesize = PHYSFS_fileLength( handle );
        if ( filesize<=0 )
        {
            OSG_NOTICE << "[ReaderWriterPhysFS] Cannot determine file size: " << fileName << std::endl;
            return false;
        }
        
        buffer.resize( filesize + 1 );
        PHYSFS_sint64 count = PHYSFS_read( handle, &(buffer[0]), filesize, 1 );
        PHYSFS_close( handle );
        
        if ( count<0 )
        {
            OSG_NOTICE << "[ReaderWriterPhysFS] Cannot read file data: " << fileName << std::endl;
            return false;
        }
        return true;
    }
    
    osgDB::ReaderWriter* handleOptions( const std::string& fileName, const Options* options ) const
    {
        osgDB::ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(
            osgDB::getLowerCaseFileExtension(fileName) );
        if ( !options ) return rw;
        if ( options->getOptionString().empty() ) return rw;
        
        std::string line;
        std::stringstream ss; ss << options->getOptionString();
        while ( std::getline(ss, line) )
        {
            std::string archiveName = osgDB::findDataFile( line, options );
            if ( !archiveName.empty() && _readPhysfsFiles.find(archiveName)==_readPhysfsFiles.end() )
            {
                PHYSFS_addToSearchPath( archiveName.c_str(), 1 );
                _readPhysfsFiles.insert( archiveName );
            }
        }
        return rw;
    }
    
    mutable std::set<std::string> _readPhysfsFiles;
};

REGISTER_OSGPLUGIN( physfs, ReaderWriterPhysFS )
