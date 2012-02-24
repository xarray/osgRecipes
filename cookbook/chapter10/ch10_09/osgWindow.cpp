#include "StdAfx.h"
#include "osgWindow.h"
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osgViewer/Viewer>

using namespace System::Threading;

osg::ref_ptr<osgViewer::Viewer> g_viewer;

void cliTest::osgWindow::runThread()
{
	Object^ dummyViewerObject = gcnew Object;
	while ( !g_viewer->done() )
	{
		Monitor::Enter(dummyViewerObject);
		g_viewer->frame();
		Monitor::Exit(dummyViewerObject);
	}
}

void cliTest::osgWindow::initializeOSG()
{
	RECT rect;
	HWND hwnd = (HWND)Handle.ToInt32();
	GetWindowRect( hwnd, &rect );

	osg::ref_ptr<osg::GraphicsContext::Traits> traits=new osg::GraphicsContext::Traits;
	traits->x = 0;
	traits->y = 0;
	traits->width = rect.right - rect.left;
	traits->height = rect.bottom - rect.top;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->inheritedWindowData = new osgViewer::GraphicsWindowWin32::WindowData(hwnd);

	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setGraphicsContext( osg::GraphicsContext::createGraphicsContext(traits.get()) );
	camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
	camera->setClearColor( osg::Vec4(0.2f, 0.2f, 0.6f, 1.0f) );
	camera->setProjectionMatrixAsPerspective(
        30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );
	camera->setDrawBuffer( GL_BACK );
	camera->setReadBuffer( GL_BACK );

	g_viewer = new osgViewer::Viewer;
	g_viewer->setCameraManipulator( new osgGA::TrackballManipulator );
	g_viewer->setCamera( camera.get() );
	g_viewer->setSceneData( osgDB::readNodeFile("cessna.osg") );

	System::Threading::Thread^ threadObject = gcnew System::Threading::Thread(
		gcnew System::Threading::ThreadStart(this, &cliTest::osgWindow::runThread) );
	threadObject->Priority = Threading::ThreadPriority::BelowNormal;
	threadObject->Start();
}
