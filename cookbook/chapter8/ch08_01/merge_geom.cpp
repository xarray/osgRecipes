/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 8 Recipe 1
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Geometry>
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "CommonFunctions"
#define MERGE_GEOMETRY  // Comment this to disable merging geometries

#ifndef MERGE_GEOMETRY

osg::Node* createTiles( unsigned int cols, unsigned int rows )
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    for ( unsigned int y=0; y<rows; ++y )
    {
        for ( unsigned int x=0; x<cols; ++x )
        {
            osg::Vec3 center((float)x, 0.0f, (float)y);
            
            osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array(4);
            (*va)[0] = center + osg::Vec3(-0.45f, 0.0f,-0.45f);
            (*va)[1] = center + osg::Vec3( 0.45f, 0.0f,-0.45f);
            (*va)[2] = center + osg::Vec3( 0.45f, 0.0f, 0.45f);
            (*va)[3] = center + osg::Vec3(-0.45f, 0.0f, 0.45f);
            
            osg::ref_ptr<osg::Vec3Array> na = new osg::Vec3Array(1);
            na->front() = osg::Vec3(0.0f, -1.0f, 0.0f);
            
            osg::ref_ptr<osg::Vec4Array> ca = new osg::Vec4Array(1);
            ca->front() = osg::Vec4(osgCookBook::randomVector(0.0f, 1.0f), 1.0f);
            
            osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
            geom->setVertexArray( va.get() );
            geom->setNormalArray( na.get() );
            geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
            geom->setColorArray( ca.get() );
            geom->setColorBinding( osg::Geometry::BIND_OVERALL );
            geom->addPrimitiveSet( new osg::DrawArrays(GL_QUADS, 0, 4) );
            geode->addDrawable( geom.get() );
        }
    }
    return geode.release();
}

#else

osg::Node* createTiles( unsigned int cols, unsigned int rows )
{
    unsigned int totalNum = cols * rows, index = 0;
    osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array(totalNum * 4);
    osg::ref_ptr<osg::Vec3Array> na = new osg::Vec3Array(totalNum);
    osg::ref_ptr<osg::Vec4Array> ca  = new osg::Vec4Array(totalNum);
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( va.get() );
    geom->setNormalArray( na.get() );
    geom->setNormalBinding( osg::Geometry::BIND_PER_PRIMITIVE_SET );
    geom->setColorArray( ca.get() );
    geom->setColorBinding( osg::Geometry::BIND_PER_PRIMITIVE_SET );
    
    for ( unsigned int y=0; y<rows; ++y )
    {
        for ( unsigned int x=0; x<cols; ++x )
        {
            unsigned int vIndex = 4 * index;
            osg::Vec3 center((float)x, 0.0f, (float)y);
            (*va)[vIndex+0] = center + osg::Vec3(-0.45f, 0.0f,-0.45f);
            (*va)[vIndex+1] = center + osg::Vec3( 0.45f, 0.0f,-0.45f);
            (*va)[vIndex+2] = center + osg::Vec3( 0.45f, 0.0f, 0.45f);
            (*va)[vIndex+3] = center + osg::Vec3(-0.45f, 0.0f, 0.45f);
            
            (*na)[index] = osg::Vec3(0.0f, -1.0f, 0.0f);
            (*ca)[index] = osg::Vec4(osgCookBook::randomVector(0.0f, 1.0f), 1.0f);
            geom->addPrimitiveSet( new osg::DrawArrays(GL_QUADS, vIndex, 4) );
            index++;
        }
    }
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( geom.get() );
    return geode.release();
}

#endif

int main( int argc, char** argv )
{
    osgViewer::Viewer viewer;
    viewer.setSceneData( createTiles(300, 300) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
