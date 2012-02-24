/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 9 Recipe 6
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgDB/ReadFile>
#include <osgUtil/SceneView>
#include <GL/glut.h>

osg::ref_ptr<osgUtil::SceneView> g_sceneView = new osgUtil::SceneView;
osg::Matrix g_viewMatrix, g_projMatrix;
float g_width = 800.0f, g_height = 600.0f;
unsigned int g_frameNumber = 0;
osg::Timer_t g_startTick;

void initializeFunc( osg::Node* model )
{
    if ( model )
    {
        const osg::BoundingSphere& bs = model->getBound();
        g_viewMatrix.makeLookAt( bs.center() - osg::Vec3(0.0f,4.0f*bs.radius(),0.0f),
                                 bs.center(), osg::Z_AXIS );
    }
    g_projMatrix.makePerspective( 30.0f, g_width/g_height, 1.0f, 10000.0f );
    g_startTick = osg::Timer::instance()->tick();
    
    g_sceneView->setDefaults();
    g_sceneView->setSceneData( model );
    g_sceneView->setClearColor( osg::Vec4(0.2f,0.2f,0.6f,1.0f) );
}

void displayFunc()
{
    osg::Timer_t currTick = osg::Timer::instance()->tick();
    double refTime = osg::Timer::instance()->delta_s(g_startTick, currTick);
    
    osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
    frameStamp->setReferenceTime( refTime );
    frameStamp->setFrameNumber( g_frameNumber++ );
    
    g_sceneView->setFrameStamp( frameStamp.get() );
    g_sceneView->setViewport( 0.0f, 0.0f, g_width, g_height );
    g_sceneView->setViewMatrix( g_viewMatrix );
    g_sceneView->setProjectionMatrix( g_projMatrix );
    
    g_sceneView->update();
    g_sceneView->cull();
    g_sceneView->draw();
    glutSwapBuffers();
}

void reshapeFunc( int width, int height )
{
    g_width = width; g_height = height;
    g_projMatrix.makePerspective( 30.0f, g_width/g_height, 1.0f, 10000.0f );
}

int main( int argc, char** argv )
{
    glutInit( &argc, argv );
    glutInitWindowSize( 800, 600 );
    glutInitDisplayMode( GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH );
    glutCreateWindow( "OSG in GLUT" );
    
    glutReshapeFunc( reshapeFunc );
    glutDisplayFunc( displayFunc );
    initializeFunc( osgDB::readNodeFile("cow.osg") );
    
    glutMainLoop();
    return 0;
}
