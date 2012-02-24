/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 8
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"
#include "Player"

class GameControllor : public osgGA::GUIEventHandler
{
public:
    GameControllor( osg::Group* root )
    : _root(root), _direction(0.1f), _distance(0.0f) {}
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        _distance += fabs(_direction);
        if ( _distance>30.0f )
        {
            _direction = -_direction;
            _distance = 0.0f;
        }
        
        osg::NodePath toBeRemoved;
        for ( unsigned i=0; i<_root->getNumChildren(); ++i )
        {
            Player* player = static_cast<Player*>( _root->getChild(i) );
            if ( !player ) continue;
            
            if ( !player->update(ea, _root.get()) )
            {
                if ( player->isBullet() )
                    toBeRemoved.push_back( player );
            }
            
            if ( player->getPlayerType()==Player::ENEMY_OBJ )
                player->setSpeedVector( osg::Vec3(_direction, 0.0f, 0.0f) );
            if ( !player->isBullet() ) continue;
            
            for ( unsigned j=0; j<_root->getNumChildren(); ++j )
            {
                Player* player2 = static_cast<Player*>( _root->getChild(j) );
                if ( !player2 || player==player2 ) continue;
                
                if ( player->getPlayerType()==Player::ENEMY_BULLET_OBJ &&
                     player2->getPlayerType()==Player::ENEMY_OBJ )
                {
                    continue;
                }
                else if ( player->intersectWith(player2) )
                {
                    toBeRemoved.push_back( player );
                    toBeRemoved.push_back( player2 );
                }
            }
        }
        
        for ( unsigned i=0; i<toBeRemoved.size(); ++i )
            _root->removeChild( toBeRemoved[i] );
        return false;
    }
    
protected:
    osg::observer_ptr<osg::Group> _root;
    float _direction;
    float _distance;
};

int main( int argc, char** argv )
{
    osg::ref_ptr<Player> player = new Player(1.0f, 1.0f, "player.png");
    player->setMatrix( osg::Matrix::translate(40.0f, 5.0f, 0.0f) );
    player->setPlayerType( Player::PLAYER_OBJ );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0, 80, 0, 30);
    hudCamera->addChild( player.get() );
    
    for ( unsigned int i=0; i<5; ++i )
    {
        for ( unsigned int j=0; j<10; ++j )
        {
            osg::ref_ptr<Player> enemy = new Player(1.0f, 1.0f, "enemy.png");
            enemy->setMatrix( osg::Matrix::translate(
                20.0f+1.5f*(float)j, 25.0f-1.5f*(float)i, 0.0f) );
            enemy->setPlayerType( Player::ENEMY_OBJ );
            hudCamera->addChild( enemy.get() );
        }
    }
    
    osgViewer::Viewer viewer;
    viewer.getCamera()->setClearColor( osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    viewer.addEventHandler( new GameControllor(hudCamera.get()) );
    viewer.setSceneData( hudCamera.get() );
    return viewer.run();
}
