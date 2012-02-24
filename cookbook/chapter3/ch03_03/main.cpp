/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 3 Recipe 3
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Geode>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"
#include "NurbsSurface"

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Vec3Array> ctrlPoints = new osg::Vec3Array;
    #define ADD_POINT(x, y, z) ctrlPoints->push_back( osg::Vec3(x, y, z) );
    ADD_POINT(-3.0f, 0.5f, 0.0f); ADD_POINT(-1.0f, 1.5f, 0.0f); ADD_POINT(-2.0f, 2.0f, 0.0f);
    ADD_POINT(-3.0f, 0.5f,-1.0f); ADD_POINT(-1.0f, 1.5f,-1.0f); ADD_POINT(-2.0f, 2.0f,-1.0f);
    ADD_POINT(-3.0f, 0.5f,-2.0f); ADD_POINT(-1.0f, 1.5f,-2.0f); ADD_POINT(-2.0f, 2.0f,-2.0f);
    
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    #define ADD_TEXCOORD(x, y) texcoords->push_back( osg::Vec2(x, y) );
    ADD_TEXCOORD(0.0f, 0.0f); ADD_TEXCOORD(0.5f, 0.0f); ADD_TEXCOORD(1.0f, 0.0f);
    ADD_TEXCOORD(0.0f, 0.5f); ADD_TEXCOORD(0.5f, 0.5f); ADD_TEXCOORD(1.0f, 0.5f);
    ADD_TEXCOORD(0.0f, 1.0f); ADD_TEXCOORD(0.5f, 1.0f); ADD_TEXCOORD(1.0f, 1.0f);
    
    osg::ref_ptr<osg::FloatArray> knots = new osg::FloatArray;
    knots->push_back(0.0f); knots->push_back(0.0f); knots->push_back(0.0f);
    knots->push_back(1.0f); knots->push_back(1.0f); knots->push_back(1.0f);
    
    osg::ref_ptr<NurbsSurface> nurbs = new NurbsSurface;
    nurbs->setVertexArray( ctrlPoints.get() );
    nurbs->setTexCoordArray( texcoords.get() );
    nurbs->setKnots( knots.get(), knots.get() );
    nurbs->setCounts( 3, 3 );
    nurbs->setOrders( 3, 3 );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( nurbs.get() );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0, new osg::Texture2D(osgDB::readImageFile("Images/osg256.png")) );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
