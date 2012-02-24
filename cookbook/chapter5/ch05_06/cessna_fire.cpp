/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 6
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Point>
#include <osg/PointSprite>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ParticleSystemUpdater>
#include <osgViewer/Viewer>

#include "CommonFunctions"

osgParticle::ParticleSystem* createFireParticles( osg::Group* parent )
{
    osg::ref_ptr<osgParticle::ParticleSystem> ps = new osgParticle::ParticleSystem;
    ps->getDefaultParticleTemplate().setLifeTime( 1.5f );
    ps->getDefaultParticleTemplate().setShape( osgParticle::Particle::QUAD );
    ps->getDefaultParticleTemplate().setSizeRange( osgParticle::rangef(3.0f, 1.5f) );
    ps->getDefaultParticleTemplate().setAlphaRange( osgParticle::rangef(1.0f, 0.0f) );
    ps->getDefaultParticleTemplate().setColorRange(
        osgParticle::rangev4(osg::Vec4(1.0f,1.0f,0.5f,1.0f), osg::Vec4(1.0f,0.5f,0.0f,1.0f)) );
    ps->setDefaultAttributes( "Images/smoke.rgb", true, false );
    
    osg::ref_ptr<osgParticle::RandomRateCounter> rrc = new osgParticle::RandomRateCounter;
    rrc->setRateRange( 30, 50 );
    
    osg::ref_ptr<osgParticle::RadialShooter> shooter = new osgParticle::RadialShooter;
    shooter->setThetaRange( -osg::PI_4, osg::PI_4 );
    shooter->setPhiRange( -osg::PI_4, osg::PI_4 );
    shooter->setInitialSpeedRange( 5.0f, 7.5f );
    
    osg::ref_ptr<osgParticle::ModularEmitter> emitter = new osgParticle::ModularEmitter;
    emitter->setParticleSystem( ps.get() );
    emitter->setCounter( rrc.get() );
    emitter->setShooter( shooter.get() );
    parent->addChild( emitter.get() );
    return ps.get();
}

osgParticle::ParticleSystem* createSmokeParticles( osg::Group* parent )
{
    osg::ref_ptr<osgParticle::ParticleSystem> ps = new osgParticle::ParticleSystem;
    ps->getDefaultParticleTemplate().setLifeTime( 3.0f );
    ps->getDefaultParticleTemplate().setShape( osgParticle::Particle::QUAD );
    ps->getDefaultParticleTemplate().setSizeRange( osgParticle::rangef(1.0f, 12.0f) );
    ps->getDefaultParticleTemplate().setAlphaRange( osgParticle::rangef(1.0f, 0.0f) );
    ps->getDefaultParticleTemplate().setColorRange(
        osgParticle::rangev4(osg::Vec4(0.4f,0.4f,0.1f,1.0f), osg::Vec4(0.1f,0.1f,0.1f,0.5f)) );
    ps->setDefaultAttributes( "Images/smoke.rgb", true, false );
    
    osg::ref_ptr<osgParticle::RandomRateCounter> rrc = new osgParticle::RandomRateCounter;
    rrc->setRateRange( 30, 50 );
    
    osg::ref_ptr<osgParticle::RadialShooter> shooter = new osgParticle::RadialShooter;
    shooter->setThetaRange( -osg::PI_4*0.5f, osg::PI_4*0.5f );
    shooter->setPhiRange( -osg::PI_4*0.5f, osg::PI_4*0.5f );
    shooter->setInitialSpeedRange( 10.0f, 15.0f );
    
    osg::ref_ptr<osgParticle::ModularEmitter> emitter = new osgParticle::ModularEmitter;
    emitter->setParticleSystem( ps.get() );
    emitter->setCounter( rrc.get() );
    emitter->setShooter( shooter.get() );
    parent->addChild( emitter.get() );
    return ps.get();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> parent = new osg::MatrixTransform;
    parent->setMatrix( osg::Matrix::rotate(-osg::PI_2, osg::X_AXIS) *
                       osg::Matrix::translate(8.0f,-10.0f,-3.0f) );
    
    osgParticle::ParticleSystem* fire = createFireParticles( parent.get() );
    osgParticle::ParticleSystem* smoke = createSmokeParticles( parent.get() );
    
    osg::ref_ptr<osgParticle::ParticleSystemUpdater> updater = new osgParticle::ParticleSystemUpdater;
    updater->addParticleSystem( fire );
    updater->addParticleSystem( smoke );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( fire );
    geode->addDrawable( smoke );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( osgDB::readNodeFile("cessna.osg") );
    root->addChild( parent.get() );
    root->addChild( updater.get() );
    root->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
