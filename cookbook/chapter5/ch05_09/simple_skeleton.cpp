/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 9
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/LineWidth>
#include <osg/Geometry>
#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/BasicAnimationManager>
#include <osgViewer/Viewer>

#include "CommonFunctions"

osg::Geode* createBoneShape( const osg::Vec3& trans, const osg::Vec4& color )
{
    osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array;
    va->push_back( osg::Vec3() ); va->push_back( trans );
    
    osg::ref_ptr<osg::Vec4Array> ca = new osg::Vec4Array;
    ca->push_back( color );
    
    osg::ref_ptr<osg::Geometry> line = new osg::Geometry;
    line->setVertexArray( va.get() );
    line->setColorArray( ca.get() );
    line->setColorBinding( osg::Geometry::BIND_OVERALL );
    line->addPrimitiveSet( new osg::DrawArrays(GL_LINES, 0, 2) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( line.get() );
    geode->getOrCreateStateSet()->setAttributeAndModes( new osg::LineWidth(15.0f) );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    return geode.release();
}

osgAnimation::Bone* createBone( const char* name, const osg::Vec3& trans, osg::Group* parent )
{
    osg::ref_ptr<osgAnimation::Bone> bone = new osgAnimation::Bone;
    parent->insertChild( 0, bone.get() );
    parent->addChild( createBoneShape(trans, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)) );
    
    osg::ref_ptr<osgAnimation::UpdateBone> updater = new osgAnimation::UpdateBone(name);
    updater->getStackedTransforms().push_back( new osgAnimation::StackedTranslateElement("translate", trans) );
    updater->getStackedTransforms().push_back( new osgAnimation::StackedQuaternionElement("quaternion") );
    
    bone->setUpdateCallback( updater.get() );
    bone->setMatrixInSkeletonSpace( osg::Matrix::translate(trans) * bone->getMatrixInSkeletonSpace() );
    bone->setName( name );
    return bone.get();
}

osgAnimation::Bone* createEndBone( const char* name, const osg::Vec3& trans, osg::Group* parent )
{
    osgAnimation::Bone* bone = createBone( name, trans, parent );
    bone->addChild( createBoneShape(trans, osg::Vec4(0.4f, 1.0f, 0.4f, 1.0f)) );
    return bone;
}

osgAnimation::Channel* createChannel( const char* name, const osg::Vec3& axis, float rad )
{
    osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> ch = new osgAnimation::QuatSphericalLinearChannel;
    ch->setName( "quaternion" );
    ch->setTargetName( name );
    
    osgAnimation::QuatKeyframeContainer* kfs = ch->getOrCreateSampler()->getOrCreateKeyframeContainer();
    kfs->push_back( osgAnimation::QuatKeyframe(0.0, osg::Quat(0.0, axis)) );
    kfs->push_back( osgAnimation::QuatKeyframe(8.0, osg::Quat(rad, axis)) );
    return ch.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osgAnimation::Skeleton> skelroot = new osgAnimation::Skeleton;
    skelroot->setDefaultUpdateCallback();
    
    osgAnimation::Bone* bone0 = createBone( "bone0", osg::Vec3(0.0f,0.0f,0.0f), skelroot.get() );
    osgAnimation::Bone* bone11 = createBone( "bone11", osg::Vec3(0.5f,0.0f,0.0f), bone0 );
    osgAnimation::Bone* bone12 = createEndBone( "bone12", osg::Vec3(1.0f,0.0f,0.0f), bone11 );
    osgAnimation::Bone* bone21 = createBone( "bone21", osg::Vec3(-0.5f,0.0f,0.0f), bone0 );
    osgAnimation::Bone* bone22 = createEndBone( "bone22", osg::Vec3(-1.0f,0.0f,0.0f), bone21 );
    osgAnimation::Bone* bone31 = createBone( "bone31", osg::Vec3(0.0f,0.5f,0.0f), bone0 );
    osgAnimation::Bone* bone32 = createEndBone( "bone32", osg::Vec3(0.0f,1.0f,0.0f), bone31 );
    osgAnimation::Bone* bone41 = createBone( "bone41", osg::Vec3(0.0f,-0.5f,0.0f), bone0 );
    osgAnimation::Bone* bone42 = createEndBone( "bone42", osg::Vec3(0.0f,-1.0f,0.0f), bone41 );
    
    osg::ref_ptr<osgAnimation::Animation> anim = new osgAnimation::Animation;
    anim->setPlayMode( osgAnimation::Animation::PPONG );
    anim->addChannel( createChannel("bone11", osg::Y_AXIS, osg::PI_4) );
    anim->addChannel( createChannel("bone12", osg::Y_AXIS, osg::PI_2) );
    anim->addChannel( createChannel("bone21", osg::Y_AXIS,-osg::PI_4) );
    anim->addChannel( createChannel("bone22", osg::Y_AXIS,-osg::PI_2) );
    anim->addChannel( createChannel("bone31", osg::X_AXIS,-osg::PI_4) );
    anim->addChannel( createChannel("bone32", osg::X_AXIS,-osg::PI_2) );
    anim->addChannel( createChannel("bone41", osg::X_AXIS, osg::PI_4) );
    anim->addChannel( createChannel("bone42", osg::X_AXIS, osg::PI_2) );
    
    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager = new osgAnimation::BasicAnimationManager;
    manager->registerAnimation( anim.get() );
    manager->playAnimation( anim.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( skelroot.get() );
    root->setUpdateCallback( manager.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
