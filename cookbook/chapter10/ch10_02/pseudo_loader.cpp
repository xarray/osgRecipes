/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 10 Recipe 2
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>
#include <osgUtil/Simplifier>
#include <osgViewer/Viewer>
#include <sstream>
#include <iostream>

#include "CommonFunctions"

class ReaderWriterSIMP : public osgDB::ReaderWriter
{
public:
    ReaderWriterSIMP() { supportsExtension("simp", "Simplification Pseudo-loader"); }
    
    virtual const char* className() const { return "simplification pseudo-loader"; }
    
    virtual bool acceptsExtension( const std::string& ext ) const
    { return osgDB::equalCaseInsensitive(ext, "simp"); }
    
    virtual ReadResult readNode( const std::string& fileName,
                                 const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        double ratio = 1.0;
        if ( options )
        {
            std::stringstream ss( options->getOptionString() );
            ss >> ratio;
        }
        
        osg::Node* scene = osgDB::readNodeFile( osgDB::getNameLessExtension(fileName) );
        if ( scene )
        {
            osgUtil::Simplifier simplifier(ratio);
            scene->accept( simplifier );
        }
        return scene;
    }
};

REGISTER_OSGPLUGIN( simp, ReaderWriterSIMP )

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    std::string ratioStr("0.2");
    if ( argc>1 ) ratioStr = arguments[1];
    
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFile(
        "cow.osg.simp", new osgDB::ReaderWriter::Options(ratioStr) );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
