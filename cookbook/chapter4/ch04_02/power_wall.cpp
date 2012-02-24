/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 4 Recipe 2
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Camera>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

osg::Camera* createSlaveCamera( int x, int y, int width, int height )
{
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->screenNum = 0;
    traits->x = x;
    traits->y = y;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    
    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext( traits.get() );
    if ( !gc ) return NULL;
    
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext( gc.get() );
    camera->setViewport( new osg::Viewport(0, 0, width, height) );
    
    GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
    camera->setDrawBuffer( buffer );
    camera->setReadBuffer( buffer );
    return camera.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    int totalWidth = 1024, totalHeight = 768;
    arguments.read( "--total-width", totalWidth );
    arguments.read( "--total-height", totalHeight );
    
    int numColumns = 3, numRows = 3;
    arguments.read( "--num-columns", numColumns );
    arguments.read( "--num-rows", numRows );
    
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("cessna.osg");
    
    osgViewer::Viewer viewer;
    
    int tileWidth = totalWidth / numColumns;
    int tileHeight = totalHeight / numRows;
    for ( int i=0; i<numRows; ++i )
    {
        for ( int j=0; j<numColumns; ++j )
        {
            osg::Camera* camera = createSlaveCamera(
                tileWidth*j, totalHeight - tileHeight*(i+1), tileWidth-1, tileHeight-1 );
            osg::Matrix projOffset =
                osg::Matrix::scale(numColumns, numRows, 1.0) *
                osg::Matrix::translate(numColumns-1-2*j, numRows-1-2*i, 0.0);
            viewer.addSlave( camera, projOffset, osg::Matrix(), true );
        }
    }
    
    viewer.setSceneData( scene );
    return viewer.run();
}
