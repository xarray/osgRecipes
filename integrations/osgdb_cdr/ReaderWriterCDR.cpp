#include <libcdr/libcdr.h>
#include <libwpd-stream/libwpd-stream.h>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

#include <osg/Image>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <iostream>
#include <sstream>

class NodeOutputInterface : public libwpg::WPGPaintInterface
{
public:
    NodeOutputInterface() : libwpg::WPGPaintInterface() {}
    osg::ref_ptr<osg::Node> node;
    
    void startGraphics( const WPXPropertyList& propList )
    { std::cout << "TODO: startGraphics" << std::endl; }
    
    void endGraphics()
    { std::cout << "TODO: endGraphics" << std::endl; }
    
    void startLayer( const WPXPropertyList& propList )
    { std::cout << "TODO: startLayer" << std::endl; }
    
    void endLayer()
    { std::cout << "TODO: endLayer" << std::endl; }
    
    void startEmbeddedGraphics( const WPXPropertyList& propList )
    { std::cout << "TODO: startEmbeddedGraphics" << std::endl; }
    
    void endEmbeddedGraphics()
    { std::cout << "TODO: endEmbeddedGraphics" << std::endl; }
    
    void setStyle( const WPXPropertyList& propList, const WPXPropertyListVector& gradient )
    { std::cout << "TODO: setStyle" << std::endl; }
    
    void drawRectangle( const WPXPropertyList& propList )
    { std::cout << "TODO: drawRectangle" << std::endl; }
    
    void drawEllipse( const WPXPropertyList& propList )
    { std::cout << "TODO: drawEllipse" << std::endl; }
    
    void drawPolyline( const WPXPropertyListVector& vertices )
    { std::cout << "TODO: drawPolyline" << std::endl; }
    
    void drawPolygon( const WPXPropertyListVector& vertices )
    { std::cout << "TODO: drawPolygon" << std::endl; }
    
    void drawPath( const WPXPropertyListVector& path )
    { std::cout << "TODO: drawPath" << std::endl; }
    
    void drawGraphicObject( const WPXPropertyList& propList, const WPXBinaryData& binaryData )
    { std::cout << "TODO: drawGraphicObject" << std::endl; }
    
    void startTextObject( const WPXPropertyList& propList, const WPXPropertyListVector& path )
    { std::cout << "TODO: startTextObject" << std::endl; }
    
    void endTextObject()
    { std::cout << "TODO: endTextObject" << std::endl; }
    
    void startTextLine( const WPXPropertyList& propList )
    { std::cout << "TODO: startTextLine" << std::endl; }
    
    void endTextLine()
    { std::cout << "TODO: endTextLine" << std::endl; }
    
    void startTextSpan( const WPXPropertyList& propList )
    { std::cout << "TODO: startTextSpan" << std::endl; }
    
    void endTextSpan()
    { std::cout << "TODO: endTextSpan" << std::endl; }
    
    void insertText( const WPXString& str )
    { std::cout << "TODO: insertText" << std::endl; }
};

class ReaderWriterCDR : public osgDB::ReaderWriter
{
public:
    ReaderWriterCDR()
    {
        supportsExtension( "cdr", "CoralDraw vector format reader" );
    }
    
    virtual ~ReaderWriterCDR()
    {
    }
    
    virtual ReadResult readNode( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        std::string fileName = osgDB::findDataFile( file, options );
        if ( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;
        
        WPXFileStream input( fileName.c_str() );
        if ( !libcdr::CDRDocument::isSupported(&input) )
        {
             OSG_NOTICE << "ReaderWriterCDR: Unsupported format of file " << fileName << std::endl;
             return ReadResult::FILE_NOT_HANDLED;
        }
        
        NodeOutputInterface output;
        libcdr::CDRDocument::parse(& input,& output );
        return output.node.get();
    }
    
    virtual WriteResult writeNode( const osg::Image& image, const std::string& file, const Options* options ) const
    {
        // TODO
        return WriteResult::FILE_NOT_HANDLED;
    }
};

REGISTER_OSGPLUGIN( cdr, ReaderWriterCDR )
