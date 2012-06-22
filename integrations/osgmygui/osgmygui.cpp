#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "MYGUIManager.h"

// This class is modified from the Demo_Themes example of MyGUI
class CustomMYGUIManager : public MYGUIManager
{
protected:
    virtual void setupResources()
    {
        MYGUIManager::setupResources();
        _platform->getDataManagerPtr()->addResourceLocation( _rootMedia + "/Demos/Demo_Themes", false );
        _platform->getDataManagerPtr()->addResourceLocation( _rootMedia + "/Common/Demos", false );
        _platform->getDataManagerPtr()->addResourceLocation( _rootMedia + "/Common/Themes", false );
    }
    
    virtual void initializeControls()
    {
        MyGUI::LayoutManager::getInstance().loadLayout("Wallpaper.layout");
        const MyGUI::VectorWidgetPtr& root = MyGUI::LayoutManager::getInstance().loadLayout("HelpPanel.layout");
        if ( root.size()==1 )
        {
            root.at(0)->findWidget("Text")->castType<MyGUI::TextBox>()->setCaption(
                "Select skin theme in combobox to see default MyGUI themes.");
        }
        createDemo( 0 );
    }
    
    void notifyComboAccept( MyGUI::ComboBox* sender, size_t index )
    {
        createDemo( index );
    }
    
    void createDemo( int index )
    {
        destroyDemo();
        switch ( index )
        {
        case 0:
            MyGUI::ResourceManager::getInstance().load("MyGUI_BlueWhiteTheme.xml");
            break;
        case 1:
            MyGUI::ResourceManager::getInstance().load("MyGUI_BlackBlueTheme.xml");
            break;
        case 2:
            MyGUI::ResourceManager::getInstance().load("MyGUI_BlackOrangeTheme.xml");
            break;
        default: break;
        }
        
        MyGUI::VectorWidgetPtr windows = MyGUI::LayoutManager::getInstance().loadLayout("Themes.layout");
        if ( windows.size()<1 )
        {
            OSG_WARN << "Error load layout" << std::endl;
            return;
        }
        
        _demoView = windows[0];
        _comboSkins = MyGUI::Gui::getInstance().findWidget<MyGUI::ComboBox>("Combo");
        if ( _comboSkins )
        {
            _comboSkins->setComboModeDrop( true );
            _comboSkins->addItem( "blue & white" );
            _comboSkins->addItem( "black & blue" );
            _comboSkins->addItem( "black & orange" );
            _comboSkins->setIndexSelected( index );
            _comboSkins->eventComboAccept += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyComboAccept);
        }
    }
    
    void destroyDemo()
    {
        if ( _demoView )
            MyGUI::WidgetManager::getInstance().destroyWidget( _demoView );
        _demoView = NULL;
        _comboSkins = NULL;
    }
    
    MyGUI::Widget* _demoView;
    MyGUI::ComboBox* _comboSkins;
};

int main( int argc, char** argv )
{
    osg::ref_ptr<CustomMYGUIManager> mygui = new CustomMYGUIManager;
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setCullingActive( false );
    geode->addDrawable( mygui.get() );
    geode->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setAllowEventFocus( false );
    camera->setProjectionMatrix( osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0) );
    camera->addChild( geode.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    //root->addChild( osgDB::readNodeFile("cow.osg") );
    root->addChild( camera.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new MYGUIHandler(camera.get(), mygui.get()) );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.realize();
    
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>( viewer.getCamera()->getGraphicsContext() );
    if ( gw )
    {
        // Send window size for MyGUI to initialize
        int x, y, w, h; gw->getWindowRectangle( x, y, w, h );
        viewer.getEventQueue()->windowResize( x, y, w, h );
    }
    return viewer.run();
}
