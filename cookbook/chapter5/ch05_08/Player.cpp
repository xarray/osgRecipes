/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 8
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/Geode>
#include <osgDB/ReadFile>
#include "Player"

Player::Player()
:   _type(INVALID_OBJ)
{
}

Player::Player( float width, float height, const std::string& texfile )
:   _type(INVALID_OBJ)
{
    _size.set( width, height );
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( osgDB::readImageFile(texfile) );
    
    osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
        osg::Vec3(-width*0.5f, -height*0.5f, 0.0f),
        osg::Vec3(width, 0.0f, 0.0f), osg::Vec3(0.0f, height, 0.0f) );
    quad->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    quad->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    quad->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( quad.get() );
    addChild( geode.get() );
}

bool Player::update( const osgGA::GUIEventAdapter& ea, osg::Group* root )
{
    bool emitBullet = false;
    switch ( _type )
    {
    case PLAYER_OBJ:
        if ( ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN )
        {
            switch ( ea.getKey() )
            {
            case osgGA::GUIEventAdapter::KEY_Left:
                _speedVec = osg::Vec3(-0.1f, 0.0f, 0.0f);
                break;
            case osgGA::GUIEventAdapter::KEY_Right:
                _speedVec = osg::Vec3(0.1f, 0.0f, 0.0f);
                break;
            case osgGA::GUIEventAdapter::KEY_Return:
                emitBullet = true;
                break;
            default: break;
            }
        }
        else if ( ea.getEventType()==osgGA::GUIEventAdapter::KEYUP )
            _speedVec = osg::Vec3();
        break;
    case ENEMY_OBJ:
        if ( RAND(0, 2000)<1 ) emitBullet = true;
        break;
    default: break;
    }
    
    osg::Vec3 pos = getMatrix().getTrans();
    if ( emitBullet )
    {
        osg::ref_ptr<Player> bullet = new Player(0.4f, 0.8f, "bullet.png");
        if ( _type==PLAYER_OBJ )
        {
            bullet->setPlayerType( PLAYER_BULLET_OBJ );
            bullet->setMatrix( osg::Matrix::translate(pos + osg::Vec3(0.0f, 0.9f, 0.0f)) );
            bullet->setSpeedVector( osg::Vec3(0.0f, 0.2f, 0.0f) );
        }
        else
        {
            bullet->setPlayerType( ENEMY_BULLET_OBJ );
            bullet->setMatrix( osg::Matrix::translate(pos - osg::Vec3(0.0f, 0.9f, 0.0f)) );
            bullet->setSpeedVector( osg::Vec3(0.0f,-0.2f, 0.0f) );
        }
        root->addChild( bullet.get() );
    }
    
    if ( ea.getEventType()!=osgGA::GUIEventAdapter::FRAME )
        return true;
    
    float halfW = width() * 0.5f, halfH = height() * 0.5f;
    pos += _speedVec;
    if ( pos.x()<halfW || pos.x()>ea.getWindowWidth()-halfW )
        return false;
    if ( pos.y()<halfH || pos.y()>ea.getWindowHeight()-halfH )
        return false;
    setMatrix( osg::Matrix::translate(pos) );
    return true;
}

bool Player::intersectWith( Player* player ) const
{
    osg::Vec3 pos = getMatrix().getTrans();
    osg::Vec3 pos2 = player->getMatrix().getTrans();
    return fabs(pos[0] - pos2[0]) < (width() + player->width()) * 0.5f &&
           fabs(pos[1] - pos2[1]) < (height() + player->height()) * 0.5f;
}
