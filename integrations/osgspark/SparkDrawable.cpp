#include "SparkDrawable.h"
#include <algorithm>

/* SparkDrawable::DeferredSystemHandler */

void SparkDrawable::DeferredSystemHandler::update( osg::NodeVisitor* nv, osg::Drawable* drawable )
{
    SparkDrawable* spark = static_cast<SparkDrawable*>(drawable);
    if ( !spark || !spark->isValid() ) return;
    
    if ( _newSystemsToAdd.size()>0 )
    {
        for ( unsigned int i=0; i<_newSystemsToAdd.size(); ++i )
        {
            const PosAndRotate& pr = _newSystemsToAdd[i];
            spark->createParticleSystem( pr.position, pr.rotationAxis, pr.rotationAngle );
        }
        _newSystemsToAdd.clear();
    }
}

/* SparkDrawable */

SparkDrawable::SparkDrawable()
:   _baseSystemCreator(NULL), _baseSystemID(SPK::NO_ID), _protoSystem(NULL),
    _lastTime(-1.0), _sortParticles(false), _useProtoSystem(true),
    _autoUpdateBound(true), _dirty(true)
{
    _activeContextID = 0;
    setUpdateCallback( new DeferredSystemHandler );
    setSupportsDisplayList( false );
    setDataVariance( osg::Object::DYNAMIC );
}
    
SparkDrawable::SparkDrawable( const SparkDrawable& copy,const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop),
    _textureObjMap(copy._textureObjMap), _particleSystems(copy._particleSystems),
    _baseSystemCreator(copy._baseSystemCreator), _baseSystemID(copy._baseSystemID),
    _protoSystem(copy._protoSystem), _activeContextID(copy._activeContextID),
    _lastTime(copy._lastTime), _sortParticles(copy._sortParticles),
    _useProtoSystem(copy._useProtoSystem), _autoUpdateBound(copy._autoUpdateBound),
    _dirty(copy._dirty)
{}

SparkDrawable::~SparkDrawable()
{
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        destroyParticleSystem( *itr, false );
    }
    if ( _protoSystem )
        destroyParticleSystem( _protoSystem, false );
}

unsigned int SparkDrawable::getNumParticles() const
{
    unsigned int count = 0;
    if ( _useProtoSystem && _protoSystem )
        count += _protoSystem->getNbParticles();
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        count += (*itr)->getNbParticles();
    }
    return count;
}

void SparkDrawable::setGlobalTransformMatrix( const osg::Matrix& matrix, bool useOffset )
{
    osg::Vec3d trans = matrix.getTrans();
    osg::Quat quat = matrix.getRotate();
    osg::Vec3d axis; double angle = 0.0f;
    quat.getRotate( angle, axis );
    
    SPK::Vector3D pos(trans.x(), trans.y(), trans.z());
    SPK::Vector3D rot(axis.x(), axis.y(), axis.z());
    if ( _useProtoSystem && _protoSystem )
        setTransformMatrix( _protoSystem, pos, rot, angle, useOffset );
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        setTransformMatrix( *itr, pos, rot, angle, useOffset );
    }
}

void SparkDrawable::setTransformMatrix( SPK::System* system, const SPK::Vector3D& pos, const SPK::Vector3D& rot,
                                        float angle, bool useOffset )
{
    if ( useOffset )
    {
        system->setTransformPosition( pos + system->getLocalTransformPos() );
        system->setTransformOrientation( rot, angle );  // FIXME: how to get rotation offset?
    }
    else
    {
        system->setTransformPosition( pos );
        system->setTransformOrientation( rot, angle );
    }
    system->updateTransform();
}

unsigned int SparkDrawable::addParticleSystem( const osg::Vec3& p, const osg::Quat& r )
{
    DeferredSystemHandler* updater = dynamic_cast<DeferredSystemHandler*>( getUpdateCallback() );
    if ( updater )
    {
        osg::Vec3 axis; double angle = 0.0f;
        r.getRotate( angle, axis );
        
        DeferredSystemHandler::PosAndRotate pr;
        pr.position = SPK::Vector3D( p.x(), p.y(), p.z() );
        pr.rotationAxis = SPK::Vector3D( axis.x(), axis.y(), axis.z() );
        pr.rotationAngle = angle;
        updater->_newSystemsToAdd.push_back( pr );
    }
    return _particleSystems.size() + updater->_newSystemsToAdd.size() - 1;
}

void SparkDrawable::addImage( const std::string& name, osg::Image* image, GLuint type, GLuint clamp )
{
    if ( image )
    {
        ImageAttribute attr;
        attr.image = image;
        attr.type = type;
        attr.clamp = clamp;
        _textureObjMap[name] = attr;
    }
}

bool SparkDrawable::update( double currentTime, const osg::Vec3d& eye )
{
    bool active = false;
    if ( _lastTime>0.0 )
    {
        if ( _sortParticles )
            std::sort( _particleSystems.begin(), _particleSystems.end(), SortParticlesOperator(eye) );
        
        double deltaTime = currentTime - _lastTime;
        SPK::Vector3D eyePos(eye.x(), eye.y(), eye.z());
        if ( _useProtoSystem && _protoSystem )
        {
            _protoSystem->setCameraPosition( eyePos );
            active = _protoSystem->update(deltaTime);
        }
        
        ParticleSystemList::iterator itr = _particleSystems.begin();
        while( itr!=_particleSystems.end() )
        {
            (*itr)->setCameraPosition( eyePos );
            if ( !(*itr)->update(deltaTime) )
            {
                destroyParticleSystem( *itr, false );
                itr = _particleSystems.erase( itr );
            }
            else
            {
                active = true;
                ++itr;
            }
        }
        
        if ( _autoUpdateBound )
            dirtyBound();  // Update the particle bound for near/far computing and culling
    }
    else
        active = true;
    
    _lastTime = currentTime;
    return active;
}

osg::BoundingSphere SparkDrawable::computeBound() const
{
    osg::BoundingBox bb;
    SPK::Vector3D min, max;
    if ( _useProtoSystem && _protoSystem )
    {
        if ( _protoSystem->isAABBComputingEnabled() )
        {
            _protoSystem->computeAABB();
            min = _protoSystem->getAABBMin(); bb.expandBy( osg::Vec3(min.x, min.y, min.z) );
            max = _protoSystem->getAABBMax(); bb.expandBy( osg::Vec3(max.x, max.y, max.z) );
        }
    }
    
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        SPK::System* system = *itr;
        if ( system->isAABBComputingEnabled() )
        {
            system->computeAABB();
            min = system->getAABBMin(); bb.expandBy( osg::Vec3(min.x, min.y, min.z) );
            max = system->getAABBMax(); bb.expandBy( osg::Vec3(max.x, max.y, max.z) );
        }
    }
    return bb;
}

void SparkDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    unsigned int contextID = renderInfo.getContextID();
    if ( _dirty )
    {
        if ( _baseSystemCreator )
        {
            TextureIDMap textureIDMap;
            for ( TextureObjMap::const_iterator itr=_textureObjMap.begin();
                  itr!=_textureObjMap.end(); ++itr )
            {
                const ImageAttribute& attr = itr->second;
                textureIDMap[itr->first] =
                    compileInternalTexture(attr.image.get(), attr.type, attr.clamp);
            }
            _baseSystemID = (*_baseSystemCreator)( textureIDMap, 800, 600 );
            _protoSystem = SPK_Get( SPK::System, _baseSystemID );
        }
        
        _activeContextID = contextID;
        _dirty = false;
    }
    
    osg::State* state = renderInfo.getState();
    state->disableAllVertexArrays();
    
    // Make sure the client unit and active unit are unified
    state->setClientActiveTextureUnit( 0 );
    state->setActiveTextureUnit( 0 );
    
    SPK::GL::GLRenderer::saveGLStates();
    if ( _useProtoSystem && _protoSystem )
        _protoSystem->render();
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        (*itr)->render();
    }
    SPK::GL::GLRenderer::restoreGLStates();
}

SPK::System* SparkDrawable::createParticleSystem( const SPK::Vector3D& pos, const SPK::Vector3D& rot, float angle )
{
    SPK::System* system = SPK_Copy( SPK::System, _baseSystemID );
    if ( !system ) return NULL;
    else setTransformMatrix( system, pos, rot, angle );
    
    _particleSystems.push_back( system );
    return system;
}

GLuint SparkDrawable::compileInternalTexture( osg::Image* image, GLuint type, GLuint clamp ) const
{
    GLuint index;
    glGenTextures( 1, &index );
    glBindTexture( GL_TEXTURE_2D, index );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    
    unsigned int numCurrent = osg::Image::computeNumComponents( image->getPixelFormat() );
    unsigned int numRequired = osg::Image::computeNumComponents( type );
    if ( numCurrent!=numRequired && image->getDataType()==GL_UNSIGNED_BYTE )
        convertData( image, type, numCurrent, numRequired );
    
    /*if ( mipmap )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        gluBuild2DMipmaps( GL_TEXTURE_2D, type, image->s(), image->t(),
                           type, GL_UNSIGNED_BYTE, image->data() );
    }
    else*/
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexImage2D( GL_TEXTURE_2D, 0, type, image->s(), image->t(), 0,
                      type, GL_UNSIGNED_BYTE, image->data() );
    }
    return index;
}

void SparkDrawable::convertData( osg::Image* image, GLuint type,
                                 unsigned int numCurrent, unsigned int numRequired ) const
{
    int newRowWidth = osg::Image::computeRowWidthInBytes(image->s(), type, GL_UNSIGNED_BYTE, 1);
    unsigned char* newData = new unsigned char[newRowWidth * image->t() * image->r()];
    
    for ( int t=0; t<image->t(); ++t )
    {
        unsigned char* source = image->data(0, t);
        unsigned char* dest = newData + t * newRowWidth;
        for ( int s=0; s<image->s(); ++s )
        {
            if ( numRequired==1 )  // RGB/RGBA -> ALPHA
            {
                *dest++ = *source;
                source += numCurrent;
            }
            else
            {
                OSG_WARN << image->getFileName() << ": no conversation from "
                         << numCurrent << " elements to " << numRequired << " elements" << std::endl;
            }
        }
    }
    image->setImage( image->s(), image->t(), image->r(), numRequired, type,
                     GL_UNSIGNED_BYTE, newData, osg::Image::USE_NEW_DELETE );
}
