/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 1
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/UpdateMatrixTransform>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgViewer/Viewer>
#include <algorithm>

#include "CommonFunctions"

osg::Node* createWall()
{
    osg::ref_ptr<osg::ShapeDrawable> wallLeft =
        new osg::ShapeDrawable( new osg::Box(osg::Vec3(-5.5f, 0.0f, 0.0f), 10.0f, 0.3f, 10.0f) );
    osg::ref_ptr<osg::ShapeDrawable> wallRight =
        new osg::ShapeDrawable( new osg::Box(osg::Vec3(10.5f, 0.0f, 0.0f), 10.0f, 0.3f, 10.0f) );
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( wallLeft.get() );
    geode->addDrawable( wallRight.get() );
    return geode.release();
}

osg::MatrixTransform* createDoor()
{
    osg::ref_ptr<osg::ShapeDrawable> doorShape =
        new osg::ShapeDrawable( new osg::Box(osg::Vec3(2.5f, 0.0f, 0.0f), 6.0f, 0.2f, 10.0f) );
    doorShape->setColor( osg::Vec4(1.0f, 1.0f, 0.8f, 1.0f) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( doorShape.get() );
    
    osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
    trans->addChild( geode.get() );
    return trans.release();
}

void generateDoorKeyframes( osgAnimation::FloatLinearChannel* ch, bool closed )
{
    osgAnimation::FloatKeyframeContainer* kfs = ch->getOrCreateSampler()->getOrCreateKeyframeContainer();
    kfs->clear();
    if ( closed )
    {
        kfs->push_back( osgAnimation::FloatKeyframe(0.0, 0.0f) );
        kfs->push_back( osgAnimation::FloatKeyframe(1.0, osg::PI_2) );
    }
    else
    {
        kfs->push_back( osgAnimation::FloatKeyframe(0.0, osg::PI_2) );
        kfs->push_back( osgAnimation::FloatKeyframe(1.0, 0.0f) );
    }
}

class OpenDoorHandler : public osgCookBook::PickHandler
{
public:
    OpenDoorHandler() : _closed(true) {}
    
    virtual void doUserOperations( osgUtil::LineSegmentIntersector::Intersection& result )
    {
        osg::NodePath::iterator itr = std::find(
            result.nodePath.begin(), result.nodePath.end(), _door.get() );
        if ( itr!=result.nodePath.end() )
        {
            if ( _manager->isPlaying(_animation.get()) )
                return;
            
            osgAnimation::FloatLinearChannel* ch = dynamic_cast<osgAnimation::FloatLinearChannel*>(
                _animation->getChannels().front().get() );
            if ( ch )
            {
                generateDoorKeyframes( ch, _closed );
                _closed = !_closed;
            }
            _manager->playAnimation( _animation.get() );
        }
    }
    
    osg::observer_ptr<osgAnimation::BasicAnimationManager> _manager;
    osg::observer_ptr<osgAnimation::Animation> _animation;
    osg::observer_ptr<osg::MatrixTransform> _door;
    bool _closed;
};

int main( int argc, char** argv )
{
    // Create the animation callback
    osg::ref_ptr<osgAnimation::FloatLinearChannel> ch = new osgAnimation::FloatLinearChannel;
    ch->setName( "euler" );
    ch->setTargetName( "DoorAnimCallback" );
    generateDoorKeyframes( ch.get(), true );
    
    osg::ref_ptr<osgAnimation::Animation> animation = new osgAnimation::Animation;
    animation->setPlayMode( osgAnimation::Animation::ONCE );
    animation->addChannel( ch.get() );
    
    osg::ref_ptr<osgAnimation::UpdateMatrixTransform> updater =
        new osgAnimation::UpdateMatrixTransform("DoorAnimCallback");
    updater->getStackedTransforms().push_back(
        new osgAnimation::StackedRotateAxisElement("euler", osg::Z_AXIS, 0.0) );
    
    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager = new osgAnimation::BasicAnimationManager;
    manager->registerAnimation( animation.get() );
    
    // Create the scene graph
    osg::MatrixTransform* animDoor = createDoor();
    animDoor->setUpdateCallback( updater.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( createWall() );
    root->addChild( animDoor );
    root->setUpdateCallback( manager.get() );
    
    osg::ref_ptr<OpenDoorHandler> handler = new OpenDoorHandler;
    handler->_manager = manager.get();
    handler->_animation = animation.get();
    handler->_door = animDoor;
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( handler.get() );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
