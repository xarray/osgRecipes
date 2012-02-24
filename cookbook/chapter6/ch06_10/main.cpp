/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 10
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"
#include "CgProgram"

CGcontext g_context;

static const char* cgProgramCode = {
    "struct app_input {\n"
    "    float4 vertex : POSITION;\n"
    "    float4 normal : NORMAL;\n"
    "};\n"
    
    "struct vertex_to_fragment {\n"
    "    float4 position : POSITION;\n"
    "    float3 normal3  : TEXCOORD0;\n"
    "};\n"
    
    "vertex_to_fragment vertex_main(app_input input)\n"
    "{\n"
    "   vertex_to_fragment output;\n"
    "   output.position = mul(glstate.matrix.mvp, input.vertex);\n"
    "   output.normal3 = input.normal.xyz;\n"
    "   return output;\n"
    "}\n"
    
    "float4 fragment_main(vertex_to_fragment input) : COLOR\n"
    "{\n"
    "   float4 output = float4(input.normal3.x, input.normal3.y, input.normal3.z, 1.0);\n"
    "   return output;\n"
    "}\n"
};

void error_callback()
{
    CGerror lastError = static_cast<CGerror>( cgGetError() );
    OSG_WARN << "Cg error: " << cgGetErrorString(lastError) << std::endl;
    
    if ( lastError == CG_COMPILER_ERROR )
        OSG_WARN << std::string(cgGetLastListing(g_context)) << std::endl;
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles( arguments );
    if ( !root ) root = osgDB::readNodeFile( "cow.osg" );
    
    // Initialize the viewer
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.setUpViewInWindow( 100, 100, 800, 600 );
    
    // Initialize Cg variables
    CGprofile vertProfile;
    CGprofile fragProfile;
    CGprogram vertProg;
    CGprogram fragProg;
    
    osg::GraphicsContext* gc = viewer.getCamera()->getGraphicsContext();
    if ( gc )
    {
        gc->realize();
        gc->makeCurrent();
        
        g_context = cgCreateContext();
        cgSetErrorCallback( error_callback );
        
        vertProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
        vertProg = cgCreateProgram(
            g_context, CG_SOURCE, cgProgramCode, vertProfile, "vertex_main", NULL );
        
        fragProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
        fragProg = cgCreateProgram(
            g_context, CG_SOURCE, cgProgramCode, fragProfile, "fragment_main", NULL );
        
        gc->releaseContext();
    }
    
    osg::ref_ptr<CgProgram> cgProg = new CgProgram;
    cgProg->addProfile( vertProfile );
    cgProg->addProfile( fragProfile );
    cgProg->addCompiledProgram( vertProg );
    cgProg->addCompiledProgram( fragProg );
    root->getOrCreateStateSet()->setAttribute( cgProg.get() );
    
    viewer.run();
    
    if ( gc )
    {
        cgDestroyProgram( vertProg );
        cgDestroyProgram( fragProg );
        cgDestroyContext( g_context );
    }
    return 0;
}
