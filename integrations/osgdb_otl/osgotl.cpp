#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <iostream>

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    arguments.getApplicationUsage()->setDescription( arguments.getApplicationName() +
        " is the example which demonstrates how to use the osgdb_otl plugin to load/save data from ODBC database.");
    arguments.getApplicationUsage()->setCommandLineUsage( arguments.getApplicationName() +
        " [options] --input/--output usr/pwd@DSN:table:filename.ive.otl");
    arguments.getApplicationUsage()->addCommandLineOption( "-new-table", "Force to drop anbd create a new table before inserting data" );
    arguments.getApplicationUsage()->addCommandLineOption( "--input", "Read data from database" );
    arguments.getApplicationUsage()->addCommandLineOption( "--output", "Write data to database" );
    arguments.getApplicationUsage()->addCommandLineOption( "--src", "Source file to write" );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help", "Display this information" );
    if ( arguments.read("-h") || arguments.read("--help") )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }
    
    osgDB::Options* options = new osgDB::Options;
    if ( arguments.read("--new-table") ) options->setOptionString( "newtable " );
    
    std::string resName, otlExt(".otl");
    if ( arguments.read("--output", resName) )
    {
        std::string src; arguments.read( "--src", src );
        osg::Node* srcNode = osgDB::readNodeFile( src );
        
        if ( osgDB::getFileExtension(resName)=="otl" ) otlExt = "";
        if ( srcNode ) osgDB::writeNodeFile( *srcNode, resName + otlExt, options );
        else OSG_WARN << "No data to output." << std::endl;
    }
    else if ( arguments.read("--input", resName) )
    {
        osgViewer::Viewer viewer;
        viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
        viewer.addEventHandler( new osgViewer::StatsHandler );
        viewer.addEventHandler( new osgViewer::WindowSizeHandler );
        
        if ( osgDB::getFileExtension(resName)=="otl" ) otlExt = "";
        viewer.setSceneData( osgDB::readNodeFile(resName + otlExt, options) );
        viewer.run();
    }
    return 0;
}
