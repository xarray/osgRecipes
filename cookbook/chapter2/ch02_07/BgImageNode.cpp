/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 2 Recipe 7
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include "BgImageNode"

CgStartCallback::CgStartCallback()
{
}

CgStartCallback::CgStartCallback( const CgStartCallback& copy, osg::CopyOp copyop )
:   osg::Camera(copy, copyop)
{
}

CgStartCallback::~CgStartCallback()
{
}
