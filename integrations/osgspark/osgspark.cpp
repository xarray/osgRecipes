#include <osg/AnimationPath>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "SparkDrawable.h"
#include "SparkUpdatingHandler.h"

extern SPK::SPK_ID createSimpleSystem( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createSmoke( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createExplosion( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createFire( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createRain( const SparkDrawable::TextureIDMap&, int, int );

osg::AnimationPath* createAnimationPath( float radius, float time )
{
    osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
    path->setLoopMode( osg::AnimationPath::LOOP );
    
    unsigned int numSamples = 32;
    float delta_yaw = 2.0f * osg::PI/((float)numSamples - 1.0f);
    float delta_time = time / (float)numSamples;
    for ( unsigned int i=0; i<numSamples; ++i )
    {
        float yaw = delta_yaw * (float)i;
        osg::Vec3 pos( sinf(yaw)*radius, cosf(yaw)*radius, 0.0f );
        osg::Quat rot( -yaw, osg::Z_AXIS );
        path->insert( delta_time * (float)i, osg::AnimationPath::ControlPoint(pos, rot) );
    }
    return path.release();    
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    bool trackingModel = false;
    int effectType = 0;
    
    if ( arguments.read("--simple") ) effectType = 0;
    else if ( arguments.read("--explosion") ) effectType = 1;
    else if ( arguments.read("--fire") ) effectType = 2;
    else if ( arguments.read("--rain") ) effectType = 3;
    else if ( arguments.read("--smoke") ) effectType = 4;
    
    SPK::randomSeed = static_cast<unsigned int>( time(NULL) );
    SPK::System::setClampStep( true, 0.1f );
    SPK::System::useAdaptiveStep( 0.001f, 0.01f );
    
    osg::ref_ptr<SparkDrawable> spark = new SparkDrawable;
    switch ( effectType )
    {
    case 1:  // Explosion
        spark->setBaseSystemCreator( &createExplosion );
        spark->addParticleSystem();
        spark->setSortParticles( true );
        spark->addImage( "explosion", osgDB::readImageFile("data/explosion.bmp"), GL_ALPHA );
        spark->addImage( "flash", osgDB::readImageFile("data/flash.bmp"), GL_RGB );
        spark->addImage( "spark1", osgDB::readImageFile("data/spark1.bmp"), GL_RGB );
        spark->addImage( "spark2", osgDB::readImageFile("data/point.bmp"), GL_ALPHA );
        spark->addImage( "wave", osgDB::readImageFile("data/wave.bmp"), GL_RGBA );
        break;
    case 2:  // Fire
        spark->setBaseSystemCreator( &createFire );
        spark->addParticleSystem();
        spark->addImage( "fire", osgDB::readImageFile("data/fire2.bmp"), GL_ALPHA );
        spark->addImage( "explosion", osgDB::readImageFile("data/explosion.bmp"), GL_ALPHA );
        break;
    case 3:  // Rain
        spark->setBaseSystemCreator( &createRain, true );  // Must use the proto type directly
        spark->addImage( "waterdrops", osgDB::readImageFile("data/waterdrops.bmp"), GL_ALPHA );
        break;
    case 4:  // Smoke
        spark->setBaseSystemCreator( &createSmoke );
        spark->addParticleSystem();
        spark->addImage( "smoke", osgDB::readImageFile("data/smoke.png"), GL_RGBA );
        trackingModel = true;
        break;
    default:  // Simple
        spark->setBaseSystemCreator( &createSimpleSystem );
        spark->addParticleSystem();
        spark->addImage( "flare", osgDB::readImageFile("data/flare.bmp"), GL_ALPHA );
        break;
    }
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( spark.get() );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    
    osg::ref_ptr<SparkUpdatingHandler> handler = new SparkUpdatingHandler;
    handler->addSpark( spark.get() );
    
    osg::ref_ptr<osg::MatrixTransform> model = new osg::MatrixTransform;
    model->addChild( osgDB::readNodeFile("glider.osg") );
    if ( trackingModel )
    {
        handler->setTrackee( 0, model.get() );
        
        osg::ref_ptr<osg::AnimationPathCallback> apcb = new osg::AnimationPathCallback;
        apcb->setAnimationPath( createAnimationPath(4.0f, 6.0f) );
        model->setUpdateCallback( apcb.get() );
    }
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( geode.get() );
    root->addChild( model.get() );
    
    osgViewer::Viewer viewer;
    viewer.getCamera()->setClearColor( osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( handler.get() );
    return viewer.run();
}
