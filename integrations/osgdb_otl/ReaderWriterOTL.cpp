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

#include "otlv4.h"

/* Usage (DSN and table are optional):
 * Quickly write (and then logoff) to database: writeNodeFile(*node, "usr/pwd@DSN:table:filename.ive.otl");
 * Quickly read (and then logoff) from database: readNodeFile("usr/pwd@DSN:table:filename.ive.otl");
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
        supportsOption( "connect=<string>", "Specify the connection string, e.g. connect=user/password@DSN" );
        supportsOption( "disconnect", "Disconnect from current state" );
        supportsOption( "newtable", "Force drop old (if exist) and create new table" );
        supportsOption( "table=<string>", "The table name which will contain all the input data" );
        supportsOption( "key=<string>", "The key (ID) of the data, the size must be equa or less than 64" );
        supportsOption( "format=<string>", "Real file format to be loaded/written to files, e.g., format=ive" );
    }
    
    virtual ~ReaderWriterOTL()
    {
        if ( !_isConnected )
            _database.logoff();
    }
    
    struct ReadOperator
    {
        virtual ReadResult read( osgDB::ReaderWriter*, std::stringstream&, const Options* ) const = 0;
    };
    
    struct WriteOperator
    {
        virtual WriteResult write( osgDB::ReaderWriter*, const osg::Object*, std::stringstream&, const Options* ) const = 0;
    };
    
    virtual ReadResult readObject( const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        struct ReadObjectOperator : public ReadOperator
        {
            virtual ReadResult read( osgDB::ReaderWriter* rw, std::stringstream& in, const Options* opt ) const
            { return rw->readObject(in, opt); }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return readImplementation(file, realOptions.get(), ReadObjectOperator());
    }
    
    virtual ReadResult readImage( const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        struct ReadImageOperator : public ReadOperator
        {
            virtual ReadResult read( osgDB::ReaderWriter* rw, std::stringstream& in, const Options* opt ) const
            { return rw->readImage(in, opt); }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return readImplementation(file, realOptions.get(), ReadImageOperator());
    }
    
    virtual ReadResult readHeightField( const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        struct ReadHeightOperator : public ReadOperator
        {
            virtual ReadResult read( osgDB::ReaderWriter* rw, std::stringstream& in, const Options* opt ) const
            { return rw->readHeightField(in, opt); }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return readImplementation(file, realOptions.get(), ReadHeightOperator());
    }
    
    virtual ReadResult readNode( const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        struct ReadNodeOperator : public ReadOperator
        {
            virtual ReadResult read( osgDB::ReaderWriter* rw, std::stringstream& in, const Options* opt ) const
            { return rw->readNode(in, opt); }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return readImplementation(file, realOptions.get(), ReadNodeOperator());
    }
    
    virtual ReadResult readShader( const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        struct ReadShaderOperator : public ReadOperator
        {
            virtual ReadResult read( osgDB::ReaderWriter* rw, std::stringstream& in, const Options* opt ) const
            { return rw->readShader(in, opt); }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return readImplementation(file, realOptions.get(), ReadShaderOperator());
    }
    
    virtual WriteResult writeObject( const osg::Object& obj, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        struct WriteObjectOperator : public WriteOperator
        {
            virtual WriteResult write( osgDB::ReaderWriter* rw, const osg::Object* obj, std::stringstream& out, const Options* opt ) const
            {
                return obj ? rw->writeObject(*obj, out, opt) : WriteResult::FILE_NOT_HANDLED;
            }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return writeImplementation(&obj, file, realOptions.get(), WriteObjectOperator());
    }
    
    virtual WriteResult writeImage( const osg::Image& image, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        struct WriteImageOperator : public WriteOperator
        {
            virtual WriteResult write( osgDB::ReaderWriter* rw, const osg::Object* obj, std::stringstream& out, const Options* opt ) const
            {
                const osg::Image* image = dynamic_cast<const osg::Image*>(obj);
                return image ? rw->writeImage(*image, out, opt) : WriteResult::FILE_NOT_HANDLED;
            }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return writeImplementation(&image, file, realOptions.get(), WriteImageOperator());
    }
    
    virtual WriteResult writeHeightField( const osg::HeightField& hf, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        struct WriteHeightFieldOperator : public WriteOperator
        {
            virtual WriteResult write( osgDB::ReaderWriter* rw, const osg::Object* obj, std::stringstream& out, const Options* opt ) const
            {
                const osg::HeightField* hf = dynamic_cast<const osg::HeightField*>(obj);
                return hf ? rw->writeHeightField(*hf, out, opt) : WriteResult::FILE_NOT_HANDLED;
            }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return writeImplementation(&hf, file, realOptions.get(), WriteHeightFieldOperator());
    }
    
    virtual WriteResult writeNode( const osg::Node& node, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        struct WriteNodeOperator : public WriteOperator
        {
            virtual WriteResult write( osgDB::ReaderWriter* rw, const osg::Object* obj, std::stringstream& out, const Options* opt ) const
            {
                const osg::Node* node = dynamic_cast<const osg::Node*>(obj);
                return node ? rw->writeNode(*node, out, opt) : WriteResult::FILE_NOT_HANDLED;
            }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return writeImplementation(&node, file, realOptions.get(), WriteNodeOperator());
    }
    
    virtual WriteResult writeShader( const osg::Shader& shader, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension(file);
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        struct WriteShaderOperator : public WriteOperator
        {
            virtual WriteResult write( osgDB::ReaderWriter* rw, const osg::Object* obj, std::stringstream& out, const Options* opt ) const
            {
                const osg::Shader* shader = dynamic_cast<const osg::Shader*>(obj);
                return shader ? rw->writeShader(*shader, out, opt) : WriteResult::FILE_NOT_HANDLED;
            }
        };
        osg::ref_ptr<osgDB::Options> realOptions = createOptionsFromName(file, options);
        return writeImplementation(&shader, file, realOptions.get(), WriteShaderOperator());
    }
    
protected:
    ReadResult readImplementation( const std::string& file, const Options* realOptions, const ReadOperator& op ) const
    {
        // Get key and table names
        char key[64] = "";
        std::string tableName("osg_table");
        if ( realOptions->getNumPluginStringData()>0 )
        {
            std::string keyString = realOptions->getPluginStringData("key");
            memcpy( key, keyString.c_str(), 64 ); key[63] = '\0';
            tableName = realOptions->getPluginStringData("table");
        }
        
        int dataSize = 0;
        std::stringstream in;
        try
        {
            bool canDisconnect = false, canCreateTable = false;
            connectFromOptions( realOptions, canDisconnect, canCreateTable );
            
            // Select from table and find the data size
            std::stringstream ss; ss << "select f2 from " << tableName << " where f1='" << key << "'";
            otl_stream is1( 1, ss.str().c_str(), _database );
            if ( !is1.eof() ) is1 >> dataSize;
            else return ReadResult::FILE_NOT_FOUND;
            
            // Select from table and get the result node
            if ( dataSize>0 )
            {
                otl_long_string data(dataSize);
                _database.set_max_long_size( dataSize );
                
                ss.str( "" ); ss << "select f3 from " << tableName << " where f1='" << key << "'";
                otl_stream is2( 1, ss.str().c_str(), _database );
                if ( !is2.eof() )
                {
                    is2 >> data;
                    in << std::string((char*)data.v, data.get_buf_size());
                }
            }
            else return ReadResult::ERROR_IN_READING_FILE;
            
            // Really read the node from buffer
            osgDB::ReaderWriter* rw =
                osgDB::Registry::instance()->getReaderWriterForExtension( realOptions->getPluginStringData("format") );
            if ( !rw ) return ReadResult::FILE_NOT_HANDLED;
            ReadResult rr = op.read( rw, in, realOptions );
            
            // Reading work finished
            if ( canDisconnect ) disconnect();
            return rr;
        }
        catch ( otl_exception& e )
        {
            OSG_WARN << "ReaderWriterOTL: " << e.msg << " while executing " << e.stm_text << std::endl
                     << "SQL state: " << e.sqlstate << " and error at variable " << e.var_info << std::endl;
        }
        return ReadResult::FILE_NOT_HANDLED;
    }
    
    WriteResult writeImplementation( const osg::Object* obj, const std::string& file, const Options* realOptions, const WriteOperator& op ) const
    {
        // Get key and table names, also write the node to a temporary buffer
        char key[64] = "";
        std::string tableName("osg_table");
        std::stringstream out;
        if ( realOptions->getNumPluginStringData()>0 )
        {
            std::string keyString = realOptions->getPluginStringData("key");
            memcpy( key, keyString.c_str(), 64 ); key[63] = '\0';
            tableName = realOptions->getPluginStringData("table");
            
            osgDB::ReaderWriter* rw =
                osgDB::Registry::instance()->getReaderWriterForExtension( realOptions->getPluginStringData("format") );
            if ( !rw ) return WriteResult::FILE_NOT_HANDLED;
            
            WriteResult wr = op.write(rw, obj, out, realOptions);
            if ( wr.status()!=WriteResult::FILE_SAVED ) return wr;
        }
        
        // Ready to output to database
        std::string buffer = out.str();
        int dataSize = buffer.size();
        try
        {
            // Check if we should create new table and disconnect later
            bool canDisconnect = false, canCreateTable = false;
            connectFromOptions( realOptions, canDisconnect, canCreateTable );
            if ( canCreateTable )
            {
                std::stringstream ss; ss << "drop table " << tableName;
                otl_cursor::direct_exec( _database, ss.str().c_str(), otl_exception::disabled );
                
                ss.str( "" ); ss << "create table " << tableName << "(f1 varchar(63) primary key, f2 int, f3 blob)";
                otl_cursor::direct_exec( _database, ss.str().c_str() );
            }
            
            // Insert into table
            otl_long_string data(dataSize);
            _database.set_max_long_size( dataSize );
            
            std::stringstream ss;
            ss << "insert into " << tableName << " values(:f1<char[64]>, :f2<int>, :f3<varchar_long>)";
            otl_stream os( 1, ss.str().c_str(), _database );
            
            for ( int i=0; i<dataSize; ++i ) data[i] = buffer[i];
            data.set_len( dataSize );
            os << key << dataSize << data;
            
            // Writing work finished
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
    
    osgDB::Options* createOptionsFromName( const std::string& file, const Options* options ) const
    {
        osgDB::StringList args;
        std::string realName = osgDB::getNameLessExtension(file);
        osgDB::split( realName, args, ':' );  // splilt address and filename
        
        osgDB::Options* realOptions = options ? options->cloneOptions() : new osgDB::Options;
        if ( args.size()>1 )  // usr/pwd@DSN : [table :] filename
        {
            realOptions->setOptionString( realOptions->getOptionString()+" disconnect" );
            realOptions->setPluginStringData( "connect", args[0] );
            if ( args.size()>2 ) realOptions->setPluginStringData( "table", args[1] );
            realName = args.back();
        }
        
        realOptions->setPluginStringData( "key", realName );
        realOptions->setPluginStringData( "format", osgDB::getFileExtension(realName) );
        return realOptions;
    }
    
    void connectFromOptions( const Options* options, bool& canDisconnect, bool& canCreateTable ) const
    {
        if ( options )
        {
            if ( options->getNumPluginStringData()>0 )
            {
                std::string connectString = options->getPluginStringData("connect");
                _database.rlogon( connectString.c_str() );
                _isConnected = true;
            }
            
            if ( options->getOptionString().find("disconnect")!=std::string::npos )
                canDisconnect = true;
            if ( options->getOptionString().find("newtable")!=std::string::npos )
                canCreateTable = true;
        }
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
