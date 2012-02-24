/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 2 Recipe 9
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include "CgCallbacks"

CgStartDrawCallback::CgStartDrawCallback()
:   _initialized(false)
{
}

void CgStartDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
    if ( !_initialized )
    {
        for ( unsigned int i=0; i<_programs.size(); ++i )
        {
            cgGLLoadProgram( _programs[i] );
        }
        _initialized = true;
    }
    
    for ( unsigned int i=0; i<_programs.size(); ++i )
    {
        cgGLBindProgram( _programs[i] );
    }
    
    for ( unsigned int i=0; i<_profiles.size(); ++i )
    {
        cgGLEnableProfile( _profiles[i] );
    }
}

void CgEndDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
    for ( unsigned int i=0; i<_profiles.size(); ++i )
    {
        cgGLDisableProfile( _profiles[i] );
    }
}
