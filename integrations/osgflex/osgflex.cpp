#include <osg/Point>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "FlexDrawable.h"

static osg::Vec3 randomVector( float k )
{
    float phi = rand() * osg::PI * 2.0f;
    float theta = rand() * osg::PI * 2.0f;
    float cosTheta = cosf(theta);
    float sinTheta = sinf(theta);
    float cosPhi = cosf(phi);	
    float sinPhi = sinf(phi);
    return osg::Vec3(cosTheta * sinPhi, cosPhi, sinTheta * sinPhi) * k;
}

static void createParticleGrid( std::vector<FlexDrawable::Particle>& particles,
                                int nx, int ny, int nz, float radius )
{
    for ( int x=0; x<nx; ++x )
    {
        for ( int y=0; y<ny; ++y )
        {
            for ( int z=0; z<nz; ++z )
            {
                osg::Vec3 pos = osg::Vec3(x, y, z) * radius + randomVector(0.05f);
                particles.push_back( FlexDrawable::Particle(pos, 1.0f) );
            }
        }
    }
}

static void createTestFluid( FlexDrawable* flex, float radius )
{
    std::vector<FlexDrawable::Particle> particles;
    createParticleGrid( particles, 64, 64, 16, radius );
    flex->prepareParticles( particles, flexMakePhase(0, eFlexPhaseSelfCollide|eFlexPhaseFluid) );
    
    FlexParams params = flex->createDefaultParameters();
    params.mFluid = true;
    params.mRadius = radius;
    params.mVorticityConfinement = 40.f;
    params.mAnisotropyScale = 20.0f;
    params.mFluidRestDistance = 0.55f * radius;
    flex->initialize( 0, 0, &params );
}

static void createRigids( FlexDrawable* flex, float radius )
{
    std::vector<FlexDrawable::Particle> particles;
    createParticleGrid( particles, 16, 16, 32, radius );
    
    FlexDrawable::RigidManager* rm = new FlexDrawable::RigidManager;
    rm->add( flex, particles, 0.0f, flexMakePhase(0, eFlexPhaseSelfCollide) );
    
    FlexParams params = flex->createDefaultParameters();
    params.mRadius = radius;
    flex->initialize( 0, 0, &params, NULL, rm );
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    osg::ref_ptr<FlexDrawable> flex = new FlexDrawable;
    if ( 1 ) createRigids( flex, 0.5f );
    else createTestFluid( flex, 0.5f );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addChild( flex.get() );
    geode->setCullingActive( false );
    geode->getOrCreateStateSet()->setAttributeAndModes( new osg::Point(5.0) );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("cow.osg") );
    scene->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( scene.get() );
    viewer.getCamera()->setProjectionMatrixAsPerspective( 30.0f, 16.0f/9.0f, 0.1f, 1000.0f );
    viewer.getCamera()->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.setUpViewOnSingleScreen( 0 );
    viewer.run();
    
    flex = NULL;  // be sure to destroy solvers before global shutdown
    return 0;
}
