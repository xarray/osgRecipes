/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 10 Recipe 6
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/ValueObject>
#include <osg/Camera>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <iostream>
#include <sstream>

#include "CommonFunctions"

class PhotoCallback : public osg::Camera::DrawCallback
{
public:
    PhotoCallback( osg::Image* img, osgText::Text* text )
    : _image(img), _text(text), _fileIndex(0) {}
    
    virtual void operator()( osg::RenderInfo& renderInfo ) const
    {
        bool capturing = false;
        if ( _image.valid() && _image->getUserValue("Capture", capturing) )
        {
            osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();
            if ( capturing && gc->getTraits() )
            {
                int width = gc->getTraits()->width;
                int height = gc->getTraits()->height;
                GLenum pixelFormat = (gc->getTraits()->alpha ? GL_RGBA : GL_RGB);
                _image->readPixels( 0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE );
                
                std::stringstream ss; ss << "Image_" << (++_fileIndex) << ".bmp";
                if ( osgDB::writeImageFile(*_image, ss.str()) && _text.valid() )
                    _text->setText( std::string("Saved to ") + ss.str() );
            }
            _image->setUserValue( "Capture", false );
        }
    }
    
protected:
    osg::ref_ptr<osg::Image> _image;
    osg::observer_ptr<osgText::Text> _text;
    mutable int _fileIndex;
};

class PhotoHandler : public osgGA::GUIEventHandler
{
public:
    PhotoHandler( osg::Image* img ) : _image(img) {}
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( _image.valid() && ea.getEventType()==osgGA::GUIEventAdapter::KEYUP )
        {
            if ( ea.getKey()=='p' || ea.getKey()=='P' )
                _image->setUserValue( "Capture", true );
        }
        return false;
    }
    
protected:
    osg::ref_ptr<osg::Image> _image;
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("cow.osg");
    
    osgText::Text* text = osgCookBook::createText(osg::Vec3(50.0f, 50.0f, 0.0f), "", 10.0f);
    osg::ref_ptr<osg::Geode> textGeode = new osg::Geode;
    textGeode->addDrawable( text );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0, 800, 0, 600);
    hudCamera->addChild( textGeode.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( hudCamera.get() );
    root->addChild( scene.get() );
    
    osg::ref_ptr<osg::Image> image = new osg::Image;
    osg::ref_ptr<PhotoCallback> pcb = new PhotoCallback( image.get(), text );
    osg::ref_ptr<PhotoHandler> ph = new PhotoHandler( image.get() );
    
    osgViewer::Viewer viewer;
    viewer.getCamera()->setPostDrawCallback( pcb.get() );
    viewer.addEventHandler( ph.get() );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
