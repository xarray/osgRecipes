/* -*-c++-*- OpenSceneGraph Cookbook
 * Common functions
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#ifndef H_COOKBOOK_COMMONFUNCTIONS
#define H_COOKBOOK_COMMONFUNCTIONS

#include <osg/AnimationPath>
#include <osg/Texture>
#include <osg/Camera>
#include <osgGA/GUIEventHandler>
#include <osgText/Text>
#include <osgUtil/LineSegmentIntersector>

namespace osgCookBook
{

    extern osg::AnimationPathCallback* createAnimationPathCallback( float radius, float time );
    extern osg::Camera* createRTTCamera( osg::Camera::BufferComponent buffer, osg::Texture* tex, bool isAbsolute=false );
    extern osg::Camera* createHUDCamera( double left, double right, double bottom, double top );
    extern osg::Geode* createScreenQuad( float width, float height, float scale=1.0f );
    extern osgText::Text* createText( const osg::Vec3& pos, const std::string& content, float size );
    
    extern float randomValue( float min, float max );
    extern osg::Vec3 randomVector( float min, float max );
    extern osg::Matrix randomMatrix( float min, float max );
    
    class PickHandler : public osgGA::GUIEventHandler
    {
    public:
        virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
        virtual void doUserOperations( osgUtil::LineSegmentIntersector::Intersection& result ) = 0;
    };

}

#endif
