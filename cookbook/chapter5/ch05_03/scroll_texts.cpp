/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 3
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgAnimation/EaseMotion>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <sstream>
#include <iomanip>

#include "CommonFunctions"

#define RAND(min, max) ((min) + (float)rand()/(RAND_MAX) * ((max)-(min)))

class ScrollTextCallback : public osg::Drawable::UpdateCallback
{
public:
    ScrollTextCallback()
    { _motion = new osgAnimation::LinearMotion; computeNewPosition(); }
    
    virtual void update( osg::NodeVisitor* nv, osg::Drawable* drawable )
    {
        osgText::Text* text = static_cast<osgText::Text*>( drawable );
        if ( !text ) return;
        
        _motion->update( 0.002 );
        float value = _motion->getValue();
        if ( value>=1.0f ) computeNewPosition();
        else _currentPos.x() = value * 800.0f;
        
        std::stringstream ss; ss << std::setprecision(3);
        ss << "XPos: " << std::setw(5) << std::setfill(' ') << _currentPos.x()
           << "; YPos: " << std::setw(5) << std::setfill(' ') << _currentPos.y();
        text->setPosition( _currentPos );
        text->setText( ss.str() );
    }
    
    void computeNewPosition()
    {
        _motion->reset();
        _currentPos.y() = RAND(50.0, 500.0);
    }
    
protected:
    osg::ref_ptr<osgAnimation::LinearMotion> _motion;
    osg::Vec3 _currentPos;
};

int main( int argc, char** argv )
{
    osgText::Text* text = osgCookBook::createText(osg::Vec3(), "", 20.0f);
    text->setUpdateCallback( new ScrollTextCallback );
    
    osg::ref_ptr<osg::Geode> textGeode = new osg::Geode;
    textGeode->addDrawable( text );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0, 800, 0, 600);
    hudCamera->addChild( textGeode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( hudCamera.get() );
    return viewer.run();
}
