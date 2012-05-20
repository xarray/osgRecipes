#include "SparkDrawable.h"

#define GET_TEXTURE_ID( name, var ) \
    GLuint var = 0; itr = textureIDMap.find(name); \
    if ( itr!=textureIDMap.end() ) var = itr->second;

#define RAIN_RATIO 0.9f
template<class T> T RAIN_PARAM( T min, T max )
{ return static_cast<T>(min + RAIN_RATIO * (max - min)); }

SPK::Group* g_splashGroup = NULL;
SPK::Group* g_dropGroup = NULL;
SPK::Emitter* g_dropEmitter = NULL;

bool killRain( SPK::Particle& particle, float deltaTime )
{
    if ( particle.position().z<=0.0f )
    {
        particle.position().set( particle.position().x, particle.position().y, 0.01f );
        g_splashGroup->addParticles( 1, particle.position(), SPK::Vector3D() );
        g_dropGroup->addParticles( RAIN_PARAM(2,8), particle.position(), g_dropEmitter );
        return true;
    }
    return false;
}

SPK::SPK_ID createRain( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
{
    SparkDrawable::TextureIDMap::const_iterator itr;
    GET_TEXTURE_ID( "waterdrops", textureSplash );
    
    // Inits Particle Engine
    SPK::Vector3D gravity = SPK::Vector3D(0.0f,0.0f,-2.0f);

    // Renderers
    // the size ratio is used with renderers whose size are defined in pixels. This is to adapt to any resolution
    float sizeRatio = static_cast<float>(screenWidth) / 1440;

    // point renderer
    SPK::GL::GLPointRenderer* dropRenderer = SPK::GL::GLPointRenderer::create();
    dropRenderer->setType(SPK::POINT_CIRCLE);
    dropRenderer->setSize(2.0f * sizeRatio);
    dropRenderer->enableBlending(true);
    
    // line renderer
    SPK::GL::GLLineRenderer* rainRenderer = SPK::GL::GLLineRenderer::create();
    rainRenderer->setLength(-0.1f);
    rainRenderer->enableBlending(true);

    // quad renderer
    SPK::GL::GLQuadRenderer* splashRenderer = SPK::GL::GLQuadRenderer::create();
    splashRenderer->setScale(0.05f,0.05f);
    splashRenderer->setTexturingMode(SPK::TEXTURE_2D);
    splashRenderer->setTexture(textureSplash);
    splashRenderer->enableBlending(true);
    splashRenderer->enableRenderingHint(SPK::DEPTH_WRITE,false);
    
    // Models
    // rain model
    SPK::Model* rainModel = SPK::Model::create(
        SPK::FLAG_GREEN | SPK::FLAG_RED | SPK::FLAG_BLUE | SPK::FLAG_ALPHA | SPK::FLAG_MASS,
        0, SPK::FLAG_MASS);
    rainModel->setParam(SPK::PARAM_ALPHA,0.2f);
    rainModel->setImmortal(true);

    // drop model
    SPK::Model* dropModel = SPK::Model::create(
        SPK::FLAG_GREEN | SPK::FLAG_RED | SPK::FLAG_BLUE | SPK::FLAG_ALPHA | SPK::FLAG_MASS,
        0, SPK::FLAG_MASS);
    dropModel->setParam(SPK::PARAM_ALPHA,0.6f);

    // splash model
    SPK::Model* splashModel = SPK::Model::create(
        SPK::FLAG_GREEN | SPK::FLAG_RED | SPK::FLAG_BLUE |
        SPK::FLAG_ALPHA | SPK::FLAG_SIZE | SPK::FLAG_ANGLE,
        SPK::FLAG_SIZE | SPK::FLAG_ALPHA,
        SPK::FLAG_SIZE | SPK::FLAG_ANGLE);
    splashModel->setParam(SPK::PARAM_ANGLE,0.0f,2.0f * osg::PI);
    splashModel->setParam(SPK::PARAM_ALPHA,1.0f,0.0f);

    // rain emitter
    SPK::Ring* rainZone = SPK::Ring::create(SPK::Vector3D(0.0f,0.0f,5.0f), SPK::Vector3D(0.0f,0.0f,1.0f));
    SPK::SphericEmitter* rainEmitter = SPK::SphericEmitter::create(SPK::Vector3D(0.0f,0.0f,-1.0f),0.0f,0.03f * osg::PI);
    rainEmitter->setZone(rainZone);

    // drop emitter
    SPK::SphericEmitter* dropEmitter = SPK::SphericEmitter::create(SPK::Vector3D(0.0f,0.0f,1.0f),0.0f,0.2f * osg::PI);
    
    // Groups
    // rain group
    SPK::Group* rainGroup = SPK::Group::create(rainModel,8000);
    rainGroup->setCustomUpdate(&killRain);
    rainGroup->setRenderer(rainRenderer);
    rainGroup->addEmitter(rainEmitter);
    rainGroup->setFriction(0.7f);
    rainGroup->setGravity(gravity);

    // drop group
    SPK::Group* dropGroup = SPK::Group::create(dropModel,16000);
    dropGroup->setRenderer(dropRenderer);
    //dropGroup->addEmitter(dropEmitter);
    dropGroup->setFriction(0.7f);
    dropGroup->setGravity(gravity);

    // splash group
    SPK::Group* splashGroup = SPK::Group::create(splashModel,2400);
    splashGroup->setRenderer(splashRenderer);

    // System
    SPK::System* particleSystem = SPK::System::create();
    particleSystem->addGroup(splashGroup);
    particleSystem->addGroup(dropGroup);
    particleSystem->addGroup(rainGroup);
    
    // Compute rain RAIN_PARAMeters
    rainModel->setParam(SPK::PARAM_RED,RAIN_PARAM(1.0f,0.40f));
    rainModel->setParam(SPK::PARAM_GREEN,RAIN_PARAM(1.0f,0.40f));
    rainModel->setParam(SPK::PARAM_BLUE,RAIN_PARAM(1.0f,0.42f));
    rainModel->setParam(SPK::PARAM_MASS,RAIN_PARAM(0.4f,0.8f),RAIN_PARAM(0.8f,1.6f));

    dropModel->setParam(SPK::PARAM_RED,RAIN_PARAM(1.0f,0.40f));
    dropModel->setParam(SPK::PARAM_GREEN,RAIN_PARAM(1.0f,0.40f));
    dropModel->setParam(SPK::PARAM_BLUE,RAIN_PARAM(1.0f,0.42f));
    dropModel->setParam(SPK::PARAM_MASS,RAIN_PARAM(0.4f,0.8f),RAIN_PARAM(3.0f,4.0f));
    dropModel->setLifeTime(RAIN_PARAM(0.05f,0.3f),RAIN_PARAM(0.1f,0.5f));

    splashModel->setParam(SPK::PARAM_RED,RAIN_PARAM(1.0f,0.40f));
    splashModel->setParam(SPK::PARAM_GREEN,RAIN_PARAM(1.0f,0.40f));
    splashModel->setParam(SPK::PARAM_BLUE,RAIN_PARAM(1.0f,0.42f));
    splashModel->setParam(SPK::PARAM_SIZE,0.0f,0.0f,RAIN_PARAM(0.375f,2.25f),RAIN_PARAM(0.75f,3.78f));
    splashModel->setLifeTime(RAIN_PARAM(0.2f,0.3f),RAIN_PARAM(0.4f,0.5f));

    rainEmitter->setFlow(RAIN_PARAM(0.0f,4800.0f));
    rainEmitter->setForce(RAIN_PARAM(3.0f,5.0f),RAIN_PARAM(6.0f,10.0f));
    rainZone->setRadius(0.0f,RAIN_PARAM(20.0f,5.0f));

    dropEmitter->setForce(RAIN_PARAM(0.1f,1.0f),RAIN_PARAM(0.2f,2.0f));
    dropRenderer->setSize(RAIN_PARAM(1.0f,3.0f) * sizeRatio);
    rainRenderer->setWidth(RAIN_PARAM(1.0f,4.0f) * sizeRatio);
    
    g_splashGroup = splashGroup;
    g_dropGroup = dropGroup;
    g_dropEmitter = dropEmitter;
    return particleSystem->getSPKID();
}
