/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 10
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include "CgProgram"

static std::vector<CGprofile> g_profiles;

void CgProgram::addProfile( CGprofile profile )
{
    _profiles.push_back(profile);
    g_profiles.push_back(profile);
}

int CgProgram::compare( const osg::StateAttribute& sa ) const
{
    COMPARE_StateAttribute_Types(CgProgram, sa)
    COMPARE_StateAttribute_Parameter(_profiles)
    COMPARE_StateAttribute_Parameter(_programs)
    COMPARE_StateAttribute_Parameter(_initialized)
    return 0;
}

void CgProgram::apply(osg::State& state) const
{
    if ( !_profiles.size() )
    {
        // Disable all profiles in the default attribute
        for ( unsigned int i=0; i<g_profiles.size(); ++i )
        {
            cgGLDisableProfile( g_profiles[i] );
        }
        return;
    }
    
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
