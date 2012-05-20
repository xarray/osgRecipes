#include "SparkDrawable.h"

#define GET_TEXTURE_ID( name, var ) \
    GLuint var = 0; itr = textureIDMap.find(name); \
    if ( itr!=textureIDMap.end() ) var = itr->second;

SPK::SPK_ID createSimpleSystem( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
{
    SparkDrawable::TextureIDMap::const_iterator itr;
    GET_TEXTURE_ID( "flare", flareTexID );
    
    // Create the model
    SPK::Model* model = SPK::Model::create(
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA,
        SPK::FLAG_ALPHA, SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE );
    model->setParam( SPK::PARAM_ALPHA, 1.0f, 0.0f );
    model->setLifeTime( 1.0f, 2.0f );
    
    // Create the renderer
    SPK::GL::GLRenderer* renderer = NULL;
    if ( SPK::GL::GLPointRenderer::loadGLExtPointSprite() &&
         SPK::GL::GLPointRenderer::loadGLExtPointParameter() )
    {
        SPK::GL::GLPointRenderer* pointRenderer = SPK::GL::GLPointRenderer::create();
        pointRenderer->setType( SPK::POINT_SPRITE );
        pointRenderer->enableWorldSize( true );
        SPK::GL::GLPointRenderer::setPixelPerUnit( 45.0f * 3.14159f / 180.0f, screenHeight );
        pointRenderer->setSize( 0.1f );
        pointRenderer->setTexture( flareTexID );
        renderer = pointRenderer;
    }
    else
    {
        SPK::GL::GLQuadRenderer* quadRenderer = SPK::GL::GLQuadRenderer::create();
        quadRenderer->setTexturingMode( SPK::TEXTURE_2D );
        quadRenderer->setScale( 0.1f, 0.1f );
        quadRenderer->setTexture( flareTexID );
        renderer = quadRenderer;
    }
    
    renderer->enableBlending( true );
    renderer->setBlendingFunctions( GL_SRC_ALPHA, GL_ONE );
    renderer->setTextureBlending( GL_MODULATE );
    renderer->enableRenderingHint( SPK::DEPTH_TEST, false );
    
    // Create the zone
    SPK::Point* source = SPK::Point::create();
    
    // Creates the emitter
    SPK::RandomEmitter* emitter = SPK::RandomEmitter::create();
    emitter->setZone( source );
    emitter->setForce( 2.8f, 3.2f );
    emitter->setTank( 500 );
    emitter->setFlow( -1.0f );
    
    // Creates the Group
    SPK::Group* group = SPK::Group::create( model, 500 );
    group->addEmitter( emitter );
    group->setRenderer( renderer );
    group->setGravity( SPK::Vector3D(0.0f, 0.0f, -1.0f) );
    group->setFriction( 2.0f );
    group->enableAABBComputing( true );
    
    // Creates the System
    SPK::System* system = SPK::System::create();
    system->addGroup( group );
    system->enableAABBComputing( true );
    
    // Creates the base and gets a pointer to the base
    model->setShared( true );
    renderer->setShared( true );
    return system->getSPKID();
}

SPK::SPK_ID createSmoke( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
{
    SparkDrawable::TextureIDMap::const_iterator itr;
    GET_TEXTURE_ID( "smoke", textureParticle );
    
    SPK::GL::GLQuadRenderer* particleRenderer = SPK::GL::GLQuadRenderer::create();
    particleRenderer->setTexturingMode( SPK::TEXTURE_2D );
    particleRenderer->setAtlasDimensions( 2, 2 );
    particleRenderer->setTexture( textureParticle );
    particleRenderer->setTextureBlending( GL_MODULATE );
    particleRenderer->setScale( 0.05f, 0.05f );
    particleRenderer->setBlending( SPK::BLENDING_ADD );
    particleRenderer->enableRenderingHint( SPK::DEPTH_WRITE, false );
    
    // Model
    SPK::Model* particleModel = SPK::Model::create(
        SPK::FLAG_SIZE | SPK::FLAG_ALPHA | SPK::FLAG_TEXTURE_INDEX | SPK::FLAG_ANGLE,
        SPK::FLAG_SIZE | SPK::FLAG_ALPHA,
        SPK::FLAG_SIZE | SPK::FLAG_TEXTURE_INDEX | SPK::FLAG_ANGLE );
    particleModel->setParam( SPK::PARAM_SIZE, 0.5f, 1.0f, 10.0f, 20.0f );
    particleModel->setParam( SPK::PARAM_ALPHA, 1.0f, 0.0f );
    particleModel->setParam( SPK::PARAM_ANGLE, 0.0f, 2.0f * osg::PI );
    particleModel->setParam( SPK::PARAM_TEXTURE_INDEX, 0.0f, 4.0f );
    particleModel->setLifeTime( 2.0f, 5.0f );
    
    // Emitter
    SPK::SphericEmitter* particleEmitter = SPK::SphericEmitter::create(
        SPK::Vector3D(-1.0f, 0.0f, 0.0f), 0.0f, 0.1f * osg::PI );
    particleEmitter->setZone( SPK::Point::create(SPK::Vector3D(0.0f, 0.015f, 0.0f)) );
    particleEmitter->setFlow( 250.0 );
    particleEmitter->setForce( 1.5f, 1.5f );
    
    // Group
    SPK::Group* particleGroup = SPK::Group::create( particleModel, 500 );
    particleGroup->addEmitter( particleEmitter );
    particleGroup->setRenderer( particleRenderer );
    particleGroup->setGravity( SPK::Vector3D(0.0f, 0.0f, 0.05f) );
    particleGroup->enableAABBComputing( true );
    
    SPK::System* particleSystem = SPK::System::create();
    particleSystem->addGroup( particleGroup );
    particleSystem->enableAABBComputing( true );
    return particleSystem->getSPKID();
}
