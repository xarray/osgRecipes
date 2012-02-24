/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 3 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/AnimationPath>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>

#include "CommonFunctions"

osg::Node* createNeedle( float w, float h, float depth, const osg::Vec4& color,
                         float angle, double period )
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(5);
    (*vertices)[0].set(-h*0.5f, 0.0f,-w*0.1f );
    (*vertices)[1].set( h*0.5f, 0.0f,-w*0.1f );
    (*vertices)[2].set(-h*0.5f, 0.0f, w*0.8f );
    (*vertices)[3].set( h*0.5f, 0.0f, w*0.8f );
    (*vertices)[4].set( 0.0f, 0.0f, w*0.9f );
    
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(1);
    (*normals)[0].set( 0.0f,-1.0f, 0.0f );
    
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0] = color;
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->setNormalArray( normals.get() );
    geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
    geom->setColorArray( colors.get() );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 5) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( geom.get() );
    
    osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
    trans->addChild( geode.get() );
    
    osg::ref_ptr<osg::AnimationPath> clockPath = new osg::AnimationPath;
    clockPath->setLoopMode( osg::AnimationPath::LOOP );
    clockPath->insert( 0.0, osg::AnimationPath::ControlPoint(
        osg::Vec3(0.0f, depth, 0.0f), osg::Quat(angle, osg::Y_AXIS)) );
    clockPath->insert( period*0.5, osg::AnimationPath::ControlPoint(
        osg::Vec3(0.0f, depth, 0.0f), osg::Quat(angle+osg::PI, osg::Y_AXIS)) );
    clockPath->insert( period, osg::AnimationPath::ControlPoint(
        osg::Vec3(0.0f, depth, 0.0f), osg::Quat(angle+osg::PI*2.0f, osg::Y_AXIS)) );
    
    osg::ref_ptr<osg::AnimationPathCallback> apcb = new osg::AnimationPathCallback;
    apcb->setAnimationPath( clockPath.get() );
    trans->addUpdateCallback( apcb.get() );
    return trans.release();
}

osg::Node* createFace( float radius )
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(67);
    (*vertices)[0].set( 0.0f, 0.0f, 0.0f );
    for ( unsigned int i=1; i<=65; ++i )
    {
        float angle = (float)(i-1) * osg::PI / 32.0f;
        (*vertices)[i].set( radius * cosf(angle), 0.0f, radius * sinf(angle) );
    }
    
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(1);
    (*normals)[0].set( 0.0f,-1.0f, 0.0f );
    
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].set( 1.0f, 1.0f, 1.0f, 1.0f );  // Avoid color state inheriting
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->setNormalArray( normals.get() );
    geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
    geom->setColorArray( colors.get() );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 67) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( geom.get() );
    return geode.release();
}

int main( int argc, char** argv )
{
    float hour_time = 10.0f, min_time = 30.0f, sec_time = 0.0f;
    osg::Node* hour = createNeedle(6.0f, 1.0f,-0.02f, osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f),
                                   osg::PI * hour_time / 6.0f, 3600*60.0);
    osg::Node* minute = createNeedle(8.0f, 0.6f,-0.04f, osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f),
                                     osg::PI * min_time / 30.0f, 3600.0);
    osg::Node* second = createNeedle(10.0f, 0.2f,-0.06f, osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f),
                                     osg::PI * sec_time / 30.0f, 60.0);
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( hour );
    root->addChild( minute );
    root->addChild( second );
    root->addChild( createFace(10.0f) );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
