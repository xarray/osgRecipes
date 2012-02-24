/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 8 Recipe 5
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/FirstPersonManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <fstream>
#include <sstream>
#include <iostream>

#include "CommonFunctions"
#include "MazeCullCallback"
#define USE_CULLCALLBACK  // Comment this to disable custom cull callback

CellMap g_mazeMap;

osg::Geode* getOrCreatePlane()
{
    static osg::ref_ptr<osg::Geode> s_quad;
    if ( !s_quad )
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setImage( osgDB::readImageFile("Images/skin.tga") );
        
        osg::ref_ptr<osg::Drawable> drawable = osg::createTexturedQuadGeometry(
            osg::Vec3(-0.5f,-0.5f, 0.0f), osg::X_AXIS, osg::Y_AXIS );
        drawable->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
        
        s_quad = new osg::Geode;
        s_quad->addDrawable( drawable.get() );
    }
    return s_quad.get();
}

osg::Geode* getOrCreateBox()
{
    static osg::ref_ptr<osg::Geode> s_box;
    if ( !s_box )
    {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setImage( osgDB::readImageFile("Images/Brick-Std-Orange.TGA") );
        
        osg::ref_ptr<osg::Drawable> drawable = new osg::ShapeDrawable(
            new osg::Box(osg::Vec3(0.0f, 0.0f, 0.5f), 1.0f) );
        drawable->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
        
        s_box = new osg::Geode;
        s_box->addDrawable( drawable.get() );
    }
    return s_box.get();
}

osg::Node* createMaze( const std::string& file )
{
    std::ifstream is( file.c_str() );
    if ( is )
    {
        std::string line;
        int col = 0, row = 0;
        while ( std::getline(is, line) )
        {
            std::stringstream ss(line);
            while ( !ss.eof() )
            {
                int value = 0; ss >> value;
                g_mazeMap[CellIndex(col, row)] = value;
                col++;
            }
            col = 0;
            row++;
        }
    }
    
    osg::ref_ptr<osg::Group> mazeRoot = new osg::Group;
    for ( CellMap::iterator itr=g_mazeMap.begin(); itr!=g_mazeMap.end(); ++itr )
    {
        const CellIndex& index = itr->first;
        osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
        trans->setMatrix( osg::Matrix::translate(index.first, index.second, 0.0f) );
        mazeRoot->addChild( trans.get() );
        
        int value = itr->second;
        if ( !value )  // Ground
            trans->addChild( getOrCreatePlane() );
        else  // Wall
            trans->addChild( getOrCreateBox() );
    }
    return mazeRoot.release();
}

class MazeManipulator : public osgGA::FirstPersonManipulator
{
public:
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osg::Matrix lastMatrix = getMatrix();
        bool ok = osgGA::FirstPersonManipulator::handle(ea, aa);
        
        if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME ||
             ea.getEventType()==osgGA::GUIEventAdapter::SCROLL )
        {
            osg::Matrix matrix = getMatrix();
            osg::Vec3 pos = matrix.getTrans();
            if ( pos[2]!=0.5f )  // Fix the player height
            {
                pos[2] = 0.5f;
                matrix.setTrans( pos );
                setByMatrix( matrix );
            }
            
            CellIndex index(int(pos[0] + 0.5f), int(pos[1] + 0.5f));
            CellMap::iterator itr = g_mazeMap.find(index);
            if ( itr==g_mazeMap.end() )  // Outside the maze
                setByMatrix( lastMatrix );
            else if ( itr->second!=0 )  // Don't intersect with walls
                setByMatrix( lastMatrix );
        }
        return ok;
    }
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
    root->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    root->addChild( createMaze("maze.txt") );
    
    osg::Node* loadedModel = osgDB::readNodeFile("dumptruck.osg" );
    for ( int i=0; i<2000; ++i )
    {
        float x = osgCookBook::randomValue(0.5f, 6.5f);
        float y = osgCookBook::randomValue(0.5f, 6.5f);
        float z = osgCookBook::randomValue(0.0f, 1.0f);
        
        osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
        trans->setMatrix( osg::Matrix::scale(0.001, 0.001, 0.001) *
                          osg::Matrix::translate(x, y, z) );
        trans->addChild( loadedModel );
        
        osg::ref_ptr<osg::Group> parent = new osg::Group;
#ifdef USE_CULLCALLBACK
        parent->setCullCallback( new MazeCullCallback );
#endif
        parent->addChild( trans.get() );
        root->addChild( parent.get() );
    }
    
    osg::ref_ptr<MazeManipulator> manipulator = new MazeManipulator;
    manipulator->setHomePosition( osg::Vec3(6.0f, 0.0f, 0.5f), osg::Vec3(6.0f, 1.0f, 0.5f), osg::Z_AXIS );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setCameraManipulator( manipulator.get() );
    return viewer.run();
}
