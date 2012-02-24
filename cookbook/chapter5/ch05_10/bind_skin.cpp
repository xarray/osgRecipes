/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 10
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/LineWidth>
#include <osg/Geometry>
#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/BasicAnimationManager>
#include <osgViewer/Viewer>

#include "CommonFunctions"

osgAnimation::Bone* createBone( const char* name, const osg::Vec3& trans, osg::Group* parent )
{
    osg::ref_ptr<osgAnimation::Bone> bone = new osgAnimation::Bone;
    parent->insertChild( 0, bone.get() );
    
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

void addVertices( const char* name1, float length1,
                  const char* name2, float length2, const osg::Vec3& dir,
                  osg::Geometry* geom, osgAnimation::VertexInfluenceMap* vim )
{
    osg::Vec3Array* va = static_cast<osg::Vec3Array*>( geom->getVertexArray() );
    unsigned int start = va->size();
    va->push_back( dir * 0.0f );
    va->push_back( dir * length1 );
    va->push_back( dir * length1 );
    (*vim)[name1].push_back( osgAnimation::VertexIndexWeight(start+2, 1.0f) );
    va->push_back( dir * length2 );
    (*vim)[name1].push_back( osgAnimation::VertexIndexWeight(start+3, 1.0f) );
    va->push_back( dir * length2 );
    (*vim)[name2].push_back( osgAnimation::VertexIndexWeight(start+4, 1.0f) );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_LINE_STRIP, start, 5) );
}

osg::Geode* createBoneShapeAndSkin()
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray( new osg::Vec3Array );
    
    osg::ref_ptr<osgAnimation::VertexInfluenceMap> vim = new osgAnimation::VertexInfluenceMap;
    (*vim)["bone11"].setName( "bone11" );
    (*vim)["bone12"].setName( "bone12" );
    (*vim)["bone21"].setName( "bone21" );
    (*vim)["bone22"].setName( "bone22" );
    (*vim)["bone31"].setName( "bone31" );
    (*vim)["bone32"].setName( "bone32" );
    (*vim)["bone41"].setName( "bone41" );
    (*vim)["bone42"].setName( "bone42" );
    
    addVertices( "bone11", 0.5f, "bone12", 1.5f, osg::X_AXIS, geometry.get(), vim.get() );
    addVertices( "bone21", 0.5f, "bone22", 1.5f,-osg::X_AXIS, geometry.get(), vim.get() );
    addVertices( "bone31", 0.5f, "bone32", 1.5f, osg::Y_AXIS, geometry.get(), vim.get() );
    addVertices( "bone41", 0.5f, "bone42", 1.5f,-osg::Y_AXIS, geometry.get(), vim.get() );
    
    osg::ref_ptr<osgAnimation::RigGeometry> rigGeom = new osgAnimation::RigGeometry;
    rigGeom->setSourceGeometry( geometry.get() );
    rigGeom->setInfluenceMap( vim.get() );
    rigGeom->setUseDisplayList( false );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( rigGeom.get() );
    geode->getOrCreateStateSet()->setAttributeAndModes( new osg::LineWidth(15.0f) );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    return geode.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osgAnimation::Skeleton> skelroot = new osgAnimation::Skeleton;
    skelroot->setDefaultUpdateCallback();
    skelroot->addChild( createBoneShapeAndSkin() );
    
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
