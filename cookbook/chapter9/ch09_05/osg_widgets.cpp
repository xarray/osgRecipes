/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 9 Recipe 5
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Canvas>
#include <osgWidget/Label>
#include <osgWidget/ViewerEventHandlers>
#include <iostream>
#include <sstream>

#include "CommonFunctions"
extern bool tabPressed( osgWidget::Event& ev );

osgWidget::Label* createLabel( const std::string& name, const std::string& text,
                               float size, const osg::Vec4& color )
{
    osg::ref_ptr<osgWidget::Label> label = new osgWidget::Label(name);
    label->setLabel( text );
    label->setFont( "fonts/arial.ttf" );
    label->setFontSize( size );
    label->setFontColor( 1.0f, 1.0f, 1.0f, 1.0f );
    label->setColor( color );
    label->addSize( 10.0f, 10.0f );
    label->setCanFill( true );
    return label.release();
}

osgWidget::Window* createSimpleTabs( float winX, float winY )
{
    osg::ref_ptr<osgWidget::Canvas> contents = new osgWidget::Canvas("contents");
    osg::ref_ptr<osgWidget::Box> tabs =
        new osgWidget::Box("tabs", osgWidget::Box::HORIZONTAL);
    
    for ( unsigned int i=0; i<3; ++i )
    {
        osg::Vec4 color(0.0f, (float)i / 3.0f, 0.0f, 1.0f);
        std::stringstream ss, ss2;
        ss << "Tab-" << i;
        ss2 << "Tab content:" << std::endl << "Some text for Tab-" << i;
        
        osgWidget::Label* content = createLabel(ss.str(), ss2.str(), 10.0f, color);
        content->setLayer( osgWidget::Widget::LAYER_MIDDLE, i );
        contents->addWidget( content, 0.0f, 0.0f );
        
        osgWidget::Label* tab = createLabel(ss.str(), ss.str(), 20.0f, color);
        tab->setEventMask( osgWidget::EVENT_MOUSE_PUSH );
        tab->addCallback( new osgWidget::Callback(
            &tabPressed, osgWidget::EVENT_MOUSE_PUSH, content) );
        tabs->addWidget( tab );
    }
    
    osg::ref_ptr<osgWidget::Box> main = new osgWidget::Box("main", osgWidget::Box::VERTICAL);
    main->setOrigin( winX, winY );
    main->attachMoveCallback();
    main->addWidget( contents->embed() );
    main->addWidget( tabs->embed() );
    main->addWidget( createLabel("title", "Tabs Demo", 15.0f, osg::Vec4(0.0f, 0.4f, 1.0f, 1.0f)) );
    return main.release();
}

bool tabPressed( osgWidget::Event& ev )
{
    osgWidget::Label* content = static_cast<osgWidget::Label*>( ev.getData() );
    if ( !content ) return false;
    
    osgWidget::Canvas* canvas = dynamic_cast<osgWidget::Canvas*>( content->getParent() );
    if ( canvas )
    {
        osgWidget::Canvas::Vector& objs = canvas->getObjects();
        for( unsigned int i=0; i<objs.size(); ++i )
            objs[i]->setLayer( osgWidget::Widget::LAYER_MIDDLE, i );
        
        content->setLayer( osgWidget::Widget::LAYER_TOP, 0 );
        canvas->resize();
    }
    return true;
}

int main( int argc, char** argv )
{
    osgViewer::Viewer viewer;
    osg::ref_ptr<osgWidget::WindowManager> wm =
        new osgWidget::WindowManager(&viewer, 1024.0f, 768.0f, 0xf0000000);
    osg::Camera* camera = wm->createParentOrthoCamera();
    
    wm->addChild( createSimpleTabs(100.0f, 100.0f) );
    wm->resizeAllWindows();
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( osgDB::readNodeFile("cow.osg") );
    root->addChild( camera );
    
    viewer.setSceneData( root.get() );
    viewer.setUpViewInWindow( 50, 50, 1024, 768 );
    viewer.addEventHandler( new osgWidget::MouseHandler(wm.get()) );
    viewer.addEventHandler( new osgWidget::KeyboardHandler(wm.get()) );
    viewer.addEventHandler( new osgWidget::ResizeHandler(wm.get(), camera) );
    viewer.addEventHandler( new osgWidget::CameraSwitchHandler(wm.get(), camera) );
    return viewer.run();
}
