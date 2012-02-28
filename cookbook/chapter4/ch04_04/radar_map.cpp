/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 4 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Material>
#include <osg/ShapeDrawable>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

const unsigned int MAIN_CAMERA_MASK = 0x1;
const unsigned int RADAR_CAMERA_MASK = 0x2;
#define RAND(min, max) ((min) + (float)rand()/(RAND_MAX) * ((max)-(min)))

osg::Node* createObject( const std::string& filename, const osg::Vec4& color )
{
    float size = 5.0f;
    osg::ref_ptr<osg::Node> model_node = osgDB::readNodeFile(filename);
    if ( model_node.valid() ) model_node->setNodeMask( MAIN_CAMERA_MASK );
    
    osg::ref_ptr<osg::ShapeDrawable> mark_shape = new osg::ShapeDrawable;
    mark_shape->setShape( new osg::Box(osg::Vec3(), size) );
    
    osg::ref_ptr<osg::Geode> mark_node = new osg::Geode;
    mark_node->addDrawable( mark_shape.get() );
    mark_node->setNodeMask( RADAR_CAMERA_MASK );
    
    osg::ref_ptr<osg::Group> obj_node = new osg::Group;
    obj_node->addChild( model_node.get() );
    obj_node->addChild( mark_node.get() );
    
    osg::ref_ptr<osg::Material> material = new osg::Material;
    material->setColorMode( osg::Material::AMBIENT );
    material->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f) );
    material->setDiffuse( osg::Material::FRONT_AND_BACK, color*0.8f );
    material->setSpecular( osg::Material::FRONT_AND_BACK, color );
    material->setShininess( osg::Material::FRONT_AND_BACK, 1.0f );
    obj_node->getOrCreateStateSet()->setAttributeAndModes(
        material.get(), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
    return obj_node.release();
}

osg::MatrixTransform* createStaticNode( const osg::Vec3& center, osg::Node* child )
{
    osg::ref_ptr<osg::MatrixTransform> trans_node = new osg::MatrixTransform;
    trans_node->setMatrix( osg::Matrix::translate(center) );
    trans_node->addChild( child );
    return trans_node.release();
}

osg::MatrixTransform* createAnimateNode( const osg::Vec3& center, float radius, float time, osg::Node* child )
{
    osg::ref_ptr<osg::MatrixTransform> anim_node = new osg::MatrixTransform;
    anim_node->addUpdateCallback( osgCookBook::createAnimationPathCallback(radius, time) );
    anim_node->addChild( child );
    
    osg::ref_ptr<osg::MatrixTransform> trans_node = new osg::MatrixTransform;
    trans_node->setMatrix( osg::Matrix::translate(center) );
    trans_node->addChild( anim_node.get() );
    return trans_node.release();
}

int main( int argc, char** argv )
{
    osg::Node* obj1 = createObject( "dumptruck.osg", osg::Vec4(1.0f, 0.2f, 0.2f, 1.0f) );
    osg::Node* obj2 = createObject( "dumptruck.osg.0,0,180.rot", osg::Vec4(0.2f, 0.2f, 1.0f, 1.0f) );
    osg::Node* air_obj2 = createObject( "cessna.osg.0,0,90.rot", osg::Vec4(0.2f, 0.2f, 1.0f, 1.0f) );
    
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    for ( unsigned int i=0; i<10; ++i )
    {
        osg::Vec3 center1( RAND(-100, 100), RAND(-100, 100), 0.0f );
        scene->addChild( createStaticNode(center1, obj1) );
        
        osg::Vec3 center2( RAND(-100, 100), RAND(-100, 100), 0.0f );
        scene->addChild( createStaticNode(center2, obj2) );
    }
    for ( unsigned int i=0; i<5; ++i )
    {
        osg::Vec3 center( RAND(-50, 50), RAND(-50, 50), RAND(10, 100) );
        scene->addChild( createAnimateNode(center, RAND(10.0, 50.0), 5.0f, air_obj2) );
    }
    
    osg::ref_ptr<osg::Camera> radar = new osg::Camera;
    radar->setClearColor( osg::Vec4(0.0f, 0.2f, 0.0f, 1.0f) );
    radar->setRenderOrder( osg::Camera::POST_RENDER );
    radar->setAllowEventFocus( false );
    radar->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    radar->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    
    radar->setViewport( 0.0, 0.0, 200.0, 200.0 );
    radar->setViewMatrix( osg::Matrixd::lookAt(osg::Vec3(0.0f, 0.0f, 120.0f), osg::Vec3(), osg::Y_AXIS) );
    radar->setProjectionMatrix( osg::Matrixd::ortho2D(-120.0, 120.0, -120.0, 120.0) );
    radar->setCullMask( RADAR_CAMERA_MASK );
    radar->addChild( scene.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( radar.get() );
    root->addChild( scene.get() );
    
    osgViewer::Viewer viewer;
    viewer.getCamera()->setCullMask( MAIN_CAMERA_MASK );
    viewer.setSceneData( root.get() );
    viewer.setLightingMode( osg::View::SKY_LIGHT );
    return viewer.run();
}
