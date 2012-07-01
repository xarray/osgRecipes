#include "otlv4.h"

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/ObjectWrapper>
#include <sstream>

/* Usage (DSN and table are optional):
 * Quickly write (and then logoff) to database: writeNodeFile(*node, "usr/pwd@DSN/table:filename.ive.otl");
 * Quickly read (and then logoff) from database: readNodeFile("usr/pwd@DSN/table:filename.ive.otl");
 * Write multiple files to database:
 *     writeNodeFile(*node1, "table:filename1.ive.otl", "connect=usr/pwd@DSN");
 *     writeNodeFile(*node2, "table:filename2.ive.otl"); ...
 *     writeNodeFile(*nodeN, "table:filename3.ive.otl", "disconnect");
 */

class ReaderWriterOTL : public osgDB::ReaderWriter
{
public:
    ReaderWriterOTL() : _isConnected(false)
    {
        otl_connect::otl_initialize();
        supportsExtension( "otl", "ODBC template database" );
        supportsOption( "connect", "Specify the connection string, e.g. connect=user/password@DSN" );
        supportsOption( "disconnect", "Disconnect from current state" );
        supportsOption( "table", "The table name which will contain all the input data" );
        supportsOption( "key", "The key (ID) of the data, the size must be equa or less than 64" );
        supportsOption( "format", "Real file format to be loaded/written to files, e.g., format=ive" );
    }
    
    virtual ~ReaderWriterOTL()
    {
        if ( !_isConnected )
            _database.logoff();
    }
    
    virtual ReadResult readNode( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        try
        {
            bool canDisconnect = connectFromOptions( realOptions );
            // TODO
            if ( canDisconnect ) disconnect();
        }
        catch ( otl_exception& e )
        {
            OSG_WARN << "ReaderWriterOTL: " << e.msg << " while executing " << e.stm_text << std::endl
                     << "SQL state: " << e.sqlstate << " and error at variable " << e.var_info << std::endl;
        }
        return ReadResult::FILE_NOT_HANDLED;
    }
    
    virtual WriteResult writeNode( const osg::Node& node, std::ostream& fout, const Options* options ) const
    {
        char key[64] = "";
        std::string tableName("osg_node");
        if ( options && options->getNumPluginStringData()>0 )
        {
            std::string keyString = options->getPluginStringData("key");
            memcpy( key, keyString.c_str(), 64 ); key[63] = '\0';
            tableName = options->getPluginStringData("table");
            
            osgDB::ReaderWriter* rw =
                osgDB::Registry::instance()->getReaderWriterForExtension( options->getPluginStringData("format") );
            if ( !rw ) return WriteResult::FILE_NOT_HANDLED;
            rw->writeNode( node, fout, options );
        }
        
        std::ostringstream* sout = dynamic_cast<std::ostringstream*>(&fout);
        if ( !sout ) return WriteResult::ERROR_IN_WRITING_FILE;
        
        std::string buffer = sout->str();
        int dataSize = buffer.size();
        try
        {
            bool canDisconnect = connectFromOptions( options );
            otl_long_string data(dataSize);
            _database.set_max_long_size( dataSize );
            
            std::stringstream ss;
            ss << "insert into " << tableName << "values(:f1<char[64]>,:f2<varchar_long>)";
            otl_stream os( 1, ss.str().c_str(), _database );
            
            for ( int i=0; i<dataSize; ++i ) data[i] = buffer[i];
            data.set_len( dataSize );
            os << key << data;
            
            if ( canDisconnect ) disconnect();
            return WriteResult::FILE_SAVED;
        }
        catch ( otl_exception& e )
        {
            OSG_WARN << "ReaderWriterOTL: " << e.msg << " while executing " << e.stm_text << std::endl
                     << "SQL state: " << e.sqlstate << " and error at variable " << e.var_info << std::endl;
        }
        return WriteResult::FILE_NOT_HANDLED;
    }
    
    virtual WriteResult writeNode( const osg::Node& node, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        std::ostringstream fout;
        return writeNode( node, fout, realOptions.get() );
    }
    
protected:
    osgDB::Options* createOptionsFromName( const std::string& file, const Options* options ) const
    {
        osgDB::StringList args;
        std::string realName = osgDB::getNameLessExtension(file);
        osgDB::split( realName, args, ':' );  // splilt address and filename
        
        osgDB::Options* realOptions = options ? options->cloneOptions() : new osgDB::Options;
        if ( args.size()>1 )
        {
            osgDB::StringList addressArgs;
            osgDB::split( args.front(), addressArgs, '/' );  // split address and table name
            if ( addressArgs.size()>2 )  // usr/pwd@DSN/table
            {
                realOptions->setPluginStringData( "table", addressArgs.back() );
                realOptions->setPluginStringData( "connect", addressArgs[0] + "/" + addressArgs[1] );
                realOptions->setOptionString( realOptions->getOptionString()+" disconnect" );
            }
            realName = args.back();
        }
        
        realOptions->setPluginStringData( "key", osgDB::getNameLessExtension(realName) );
        realOptions->setPluginStringData( "format", osgDB::getFileExtension(realName) );
        return realOptions;
    }
    
    bool connectFromOptions( const Options* options ) const
    {
        if ( options )
        {
            if ( options->getNumPluginStringData()>0 )
            {
                std::string connectString = options->getPluginStringData("connect");
                _database.rlogon( connectString.c_str() );
                _isConnected = true;
            }
            else if ( options->getOptionString().find("disconnect")!=std::string::npos )
                return true;
        }
        return false;
    }
    
    bool disconnect() const
    {
        if ( _isConnected )
        {
            _database.logoff();
            _isConnected = false;
            return true;
        }
        return false;
    }
    
    mutable otl_connect _database;
    mutable bool _isConnected;
};

REGISTER_OSGPLUGIN( otl, ReaderWriterOTL )
