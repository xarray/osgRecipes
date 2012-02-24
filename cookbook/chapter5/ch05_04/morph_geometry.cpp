/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Point>
#include <osg/Geometry>
#include <osg/Geode>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/BasicAnimationManager>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

typedef void (*VertexFunc)( osg::Vec3Array* );
osg::Geometry* createEmoticonGeometry( VertexFunc func )
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(15);
    (*vertices)[0] = osg::Vec3(-0.5f, 0.0f, 1.0f);
    (*vertices)[1] = osg::Vec3(0.5f, 0.0f, 1.0f);
    (*func)( vertices.get() );
    
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(15);
    for ( unsigned int i=0; i<15; ++i )
        (*normals)[i] = osg::Vec3(0.0f, -1.0f, 0.0f);
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->setNormalArray( normals.get() );
    geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_POINTS, 0, 2) );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_LINE_STRIP, 2, 13) );
    return geom.release();
}

void emoticonSource( osg::Vec3Array* va )
{
    for ( int i=0; i<13; ++i )
        (*va)[i+2] = osg::Vec3((float)(i-6)*0.15f, 0.0f, 0.0f);
}

void emoticonTarget( osg::Vec3Array* va )
{
    float angleStep = osg::PI / 12.0f;
    for ( int i=0; i<13; ++i )
    {
        float angle = osg::PI - angleStep * (float)i;
        (*va)[i+2] = osg::Vec3(0.9f*cosf(angle), 0.0f,-0.2f*sinf(angle));
    }
}

void createMorphKeyframes( osgAnimation::FloatLinearChannel* ch )
{
    osgAnimation::FloatKeyframeContainer* kfs = ch->getOrCreateSampler()->getOrCreateKeyframeContainer();
    kfs->push_back( osgAnimation::FloatKeyframe(0.0, 0.0) );
    kfs->push_back( osgAnimation::FloatKeyframe(2.0, 1.0) );
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osgAnimation::FloatLinearChannel> channel = new osgAnimation::FloatLinearChannel;
    channel->setName( "0" );
    channel->setTargetName( "MorphCallback" );
    createMorphKeyframes( channel.get() );
    
    osg::ref_ptr<osgAnimation::Animation> animation = new osgAnimation::Animation;
    animation->setPlayMode( osgAnimation::Animation::PPONG );
    animation->addChannel( channel.get() );
    
    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager = new osgAnimation::BasicAnimationManager;
    manager->registerAnimation( animation.get() );
    manager->playAnimation( animation.get() );
    
    osg::ref_ptr<osgAnimation::MorphGeometry> morph =
        new osgAnimation::MorphGeometry( *createEmoticonGeometry(emoticonSource) );
    morph->addMorphTarget( createEmoticonGeometry(emoticonTarget) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( morph.get() );
    geode->setUpdateCallback( new osgAnimation::UpdateMorph("MorphCallback") );
    geode->getOrCreateStateSet()->setAttributeAndModes( new osg::Point(20.0f) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( geode.get() );
    root->setUpdateCallback( manager.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
