#include "SparkDrawable.h"

#define GET_TEXTURE_ID( name, var ) \
    GLuint var = 0; itr = textureIDMap.find(name); \
    if ( itr!=textureIDMap.end() ) var = itr->second;

SPK::SPK_ID createFire( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
{
    SparkDrawable::TextureIDMap::const_iterator itr;
    GET_TEXTURE_ID( "fire", textureFire );
    GET_TEXTURE_ID( "explosion", textureSmoke );
    
    // Renderers
    SPK::GL::GLQuadRenderer* fireRenderer = SPK::GL::GLQuadRenderer::create();
    fireRenderer->setScale(0.3f,0.3f);
    fireRenderer->setTexturingMode(SPK::TEXTURE_2D);
    fireRenderer->setTexture(textureFire);
    fireRenderer->setTextureBlending(GL_MODULATE);
    fireRenderer->setBlending(SPK::BLENDING_ADD);
    fireRenderer->enableRenderingHint(SPK::DEPTH_WRITE,false);
    fireRenderer->setAtlasDimensions(2,2);

    SPK::GL::GLQuadRenderer* smokeRenderer = SPK::GL::GLQuadRenderer::create();
    smokeRenderer->setScale(0.3f,0.3f);
    smokeRenderer->setTexturingMode(SPK::TEXTURE_2D);
    smokeRenderer->setTexture(textureSmoke);
    smokeRenderer->setTextureBlending(GL_MODULATE);
    smokeRenderer->setBlending(SPK::BLENDING_ALPHA);
    smokeRenderer->enableRenderingHint(SPK::DEPTH_WRITE,false);
    smokeRenderer->setAtlasDimensions(2,2);

    // Models
    SPK::Model* fireModel = SPK::Model::create(
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA |
        SPK::FLAG_SIZE | SPK::FLAG_ANGLE | SPK::FLAG_TEXTURE_INDEX,
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_ALPHA | SPK::FLAG_ANGLE,
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_TEXTURE_INDEX | SPK::FLAG_ANGLE,
        SPK::FLAG_SIZE);
    fireModel->setParam(SPK::PARAM_RED,0.8f,0.9f,0.8f,0.9f);
    fireModel->setParam(SPK::PARAM_GREEN,0.5f,0.6f,0.5f,0.6f);
    fireModel->setParam(SPK::PARAM_BLUE,0.3f);
    fireModel->setParam(SPK::PARAM_ALPHA,0.4f,0.0f);
    fireModel->setParam(SPK::PARAM_ANGLE,0.0f,2.0f * osg::PI,0.0f,2.0f * osg::PI);
    fireModel->setParam(SPK::PARAM_TEXTURE_INDEX,0.0f,4.0f);
    fireModel->setLifeTime(1.0f,1.5f);

    SPK::Interpolator* interpolator = fireModel->getInterpolator(SPK::PARAM_SIZE);
    interpolator->addEntry(0.5f,2.0f,5.0f);
    interpolator->addEntry(1.0f,0.0f);

    SPK::Model* smokeModel = SPK::Model::create(
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA |
        SPK::FLAG_SIZE | SPK::FLAG_ANGLE | SPK::FLAG_TEXTURE_INDEX,
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_SIZE | SPK::FLAG_ANGLE,
        SPK::FLAG_TEXTURE_INDEX | SPK::FLAG_ANGLE,
        SPK::FLAG_ALPHA);
    smokeModel->setParam(SPK::PARAM_RED,0.3f,0.2f);
    smokeModel->setParam(SPK::PARAM_GREEN,0.25f,0.2f);
    smokeModel->setParam(SPK::PARAM_BLUE,0.2f);
    smokeModel->setParam(SPK::PARAM_ALPHA,0.2f,0.0f);
    smokeModel->setParam(SPK::PARAM_SIZE,5.0,10.0f);
    smokeModel->setParam(SPK::PARAM_TEXTURE_INDEX,0.0f,4.0f);
    smokeModel->setParam(SPK::PARAM_ANGLE,0.0f,2.0f * osg::PI,0.0f,2.0f * osg::PI);
    smokeModel->setLifeTime(5.0f,5.0f);

    interpolator = smokeModel->getInterpolator(SPK::PARAM_ALPHA);
    interpolator->addEntry(0.0f,0.0f);
    interpolator->addEntry(0.2f,0.2f);
    interpolator->addEntry(1.0f,0.0f);

    // Emitters
    // The emitters are arranged so that the fire looks realistic
    SPK::StraightEmitter* fireEmitter1 = SPK::StraightEmitter::create(SPK::Vector3D(0.0f,0.0f,1.0f));
    fireEmitter1->setZone(SPK::Sphere::create(SPK::Vector3D(0.0f,0.0f,-1.0f),0.5f));
    fireEmitter1->setFlow(40);
    fireEmitter1->setForce(1.0f,2.5f);

    SPK::StraightEmitter* fireEmitter2 = SPK::StraightEmitter::create(SPK::Vector3D(1.0f,0.0f,0.6f));
    fireEmitter2->setZone(SPK::Sphere::create(SPK::Vector3D(0.15f,0.075f,-1.2f),0.1f));
    fireEmitter2->setFlow(15);
    fireEmitter2->setForce(0.5f,1.5f);

    SPK::StraightEmitter* fireEmitter3 = SPK::StraightEmitter::create(SPK::Vector3D(-0.6f,-0.8f,0.8f));
    fireEmitter3->setZone(SPK::Sphere::create(SPK::Vector3D(-0.375f,-0.375f,-1.15f),0.3f));
    fireEmitter3->setFlow(15);
    fireEmitter3->setForce(0.5f,1.5f);

    SPK::StraightEmitter* fireEmitter4 = SPK::StraightEmitter::create(SPK::Vector3D(-0.8f,0.2f,0.5f));
    fireEmitter4->setZone(SPK::Sphere::create(SPK::Vector3D(-0.255f,0.225f,-1.2f),0.2f));
    fireEmitter4->setFlow(10);
    fireEmitter4->setForce(0.5f,1.5f);

    SPK::StraightEmitter* fireEmitter5 = SPK::StraightEmitter::create(SPK::Vector3D(0.1f,-1.0f,0.8f));
    fireEmitter5->setZone(SPK::Sphere::create(SPK::Vector3D(-0.075f,-0.3f,-1.2f),0.2f));
    fireEmitter5->setFlow(10);
    fireEmitter5->setForce(0.5f,1.5f);

    SPK::SphericEmitter* smokeEmitter = SPK::SphericEmitter::create(SPK::Vector3D(0.0f,0.0f,1.0f),0.0f,0.5f * osg::PI);
    smokeEmitter->setZone(SPK::Sphere::create(SPK::Vector3D(),1.2f));
    smokeEmitter->setFlow(25);
    smokeEmitter->setForce(0.5f,1.0f);

    // Groups
    SPK::Group* fireGroup = SPK::Group::create(fireModel,135);
    fireGroup->addEmitter(fireEmitter1);
    fireGroup->addEmitter(fireEmitter2);
    fireGroup->addEmitter(fireEmitter3);
    fireGroup->addEmitter(fireEmitter4);
    fireGroup->addEmitter(fireEmitter5);
    fireGroup->setRenderer(fireRenderer);
    fireGroup->setGravity(SPK::Vector3D(0.0f,0.0f,3.0f));

    SPK::Group* smokeGroup = SPK::Group::create(smokeModel,135);
    smokeGroup->addEmitter(smokeEmitter);
    smokeGroup->setRenderer(smokeRenderer);
    smokeGroup->setGravity(SPK::Vector3D(0.0f,0.0f,0.4f));
    
    // System
    SPK::System* particleSystem = SPK::System::create();
    particleSystem->addGroup(smokeGroup);
    particleSystem->addGroup(fireGroup);
    return particleSystem->getSPKID();
}
