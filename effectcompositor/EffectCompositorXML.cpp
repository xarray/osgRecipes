#include <osg/io_utils>
#include <osg/FrameBufferObject>
#include <osg/ImageStream>
#include <osg/Depth>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osg/Texture2DMultisample>
#include <osg/Texture3D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>
#include <osg/View>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <iostream>
#include <sstream>
#include "EffectCompositor"

using namespace osgFX;

template<typename T>
struct UniformAnimator : public osg::Uniform::Callback
{
    virtual void operator()( osg::Uniform* uniform, osg::NodeVisitor* nv )
    {
        double time = nv->getFrameStamp()->getSimulationTime();
        if ( loop )
        {
            double modulated_time = (time - startTime) / duration;
            double fraction_part = modulated_time - floor(modulated_time);
            time = startTime + fraction_part * duration;
        }
        
        typename std::map<double, T>::const_iterator itr = keyframes.lower_bound(time);
        if ( itr==keyframes.begin() )
            uniform->set( itr->second );
        else if ( itr==keyframes.end() )
            uniform->set( keyframes.rbegin()->second );
        else
        {
            typename std::map<double, T>::const_iterator itr0 = itr; --itr0;
            double delta_time = itr->first - itr0->first;
            if ( delta_time==0.0 )
                uniform->set( itr0->second );
            else
            {
                double k = (time - itr0->first) / delta_time;
                uniform->set( (T)(itr0->second * (1.0 - k) + itr->second * k) );
            }
        }
    }
    
    UniformAnimator( double d, bool l )
    : startTime(0.0), duration(d), loop(l) {}
    
    std::map<double, T> keyframes;
    double startTime;
    double duration;
    bool loop;
};

static bool isXMLNodeType( osgDB::XmlNode* xmlNode )
{
    switch ( xmlNode->type )
    {
    case osgDB::XmlNode::ATOM:
    case osgDB::XmlNode::NODE:
    case osgDB::XmlNode::GROUP:
        return true;
    default:
        return false;
    }
}

template<typename T>
static UniformAnimator<T>* createAnimatorFromXML( osgDB::XmlNode* xmlNode )
{
    double duration = atof( xmlNode->properties["duration"].c_str() );
    int loop = atoi( xmlNode->properties["loop"].c_str() );
    osg::ref_ptr< UniformAnimator<T> > animator = new UniformAnimator<T>( duration, loop>0?true:false );
    
    for ( unsigned int j=0; j<xmlNode->children.size(); ++j )
    {
        osgDB::XmlNode* xmlKeyframeNode = xmlNode->children[j].get();
        if ( !isXMLNodeType(xmlKeyframeNode) ) continue;
        
        double time = atof( xmlKeyframeNode->properties["time"].c_str() );
        if ( j==0 ) animator->startTime = time;
        
        std::stringstream ss( xmlKeyframeNode->getTrimmedContents() );
        T value; ss >> value;
        animator->keyframes[time] = value;
    }
    return animator.release();
}

static void inheritXmlNode( osgDB::XmlNode* target, osgDB::XmlNode* source )
{
    for ( osgDB::XmlNode::Properties::iterator itr=source->properties.begin();
          itr!=source->properties.end(); ++itr )
    {
        if ( target->properties.find(itr->first)==target->properties.end() )
            target->properties[itr->first] = itr->second;
    }
    target->children.insert( target->children.begin(), source->children.begin(), source->children.end() );
}

/* EffectCompositor - XML parsing methods */

osg::Camera* EffectCompositor::createPassFromXML( osgDB::XmlNode* xmlNode )
{
    std::string name = xmlNode->properties["name"];
    if ( name.empty() )
        OSG_NOTICE << "EffectCompositor: inappropriate to create pass with empty name" << std::endl;
    
    PassType passType = FORWARD_PASS;
    if ( xmlNode->name=="pass" )
    {
        std::string typeString = xmlNode->properties["type"];
        if ( typeString=="deferred" ) passType = DEFERRED_PASS;
    }
    else if ( xmlNode->name=="deferred_pass" )
        passType = DEFERRED_PASS;
    
    osg::Camera* camera = createNewPass( passType, name );
    osg::StateSet* stateset = camera->getOrCreateStateSet();
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->setName( name );
    
    osg::StateAttribute::GLModeValue shaderModeValue = osg::StateAttribute::ON;
    int override = atoi( xmlNode->properties["override"].c_str() );
    if ( override>0 ) shaderModeValue |= osg::StateAttribute::OVERRIDE;
    
    unsigned int numAttached = 0;
    for ( unsigned int i=0; i<xmlNode->children.size(); ++i )
    {
        osgDB::XmlNode* xmlChild = xmlNode->children[i].get();
        const std::string& childName = xmlChild->name;
        if ( !isXMLNodeType(xmlChild) ) continue;
        
        if ( childName=="output_buffer" )
        {
            std::string bufferTarget = xmlChild->properties["target"];
            osg::Camera::BufferComponent bc = osg::Camera::COLOR_BUFFER;
            if ( bufferTarget=="color" ) bc = osg::Camera::COLOR_BUFFER;
            else if ( bufferTarget=="depth" ) bc = osg::Camera::DEPTH_BUFFER;
            else if ( bufferTarget=="stencil" ) bc = osg::Camera::STENCIL_BUFFER;
            else if ( bufferTarget=="packed_depth_stencil" ) bc = osg::Camera::PACKED_DEPTH_STENCIL_BUFFER;
            else if ( bufferTarget.find("color")!=std::string::npos )
            {
                std::string colorBufferIndex = bufferTarget.substr( bufferTarget.find("color") + 5 );
                bc = (osg::Camera::BufferComponent)( osg::Camera::COLOR_BUFFER0 + atoi(colorBufferIndex.c_str()) );
            }
            
            int level = atoi( xmlChild->properties["level"].c_str() );
            int face = atoi( xmlChild->properties["face"].c_str() );
            int useMipmap = atoi( xmlChild->properties["use_mipmap"].c_str() );
            int samples = atoi( xmlChild->properties["samples"].c_str() );
            int colorSamples = atoi( xmlChild->properties["color_samples"].c_str() );
            if ( xmlChild->children.size()>0 )
            {
                osg::Texture* texture = createTextureFromXML( xmlChild, false );
                camera->setViewport( 0, 0, texture->getTextureWidth(), texture->getTextureHeight() );
                camera->attach( bc, texture, level, face, useMipmap>0?true:false, samples, colorSamples );
                numAttached++;
                OSG_NOTICE << "EffectCompositor: <pass> " << name << " is outputting to a local buffer which cannot be shared anymore" << std::endl;
            }
            else
            {
                osg::Texture* texture = getTexture( xmlChild->getTrimmedContents() );
                if ( texture )
                {
                    camera->setViewport( 0, 0, texture->getTextureWidth(), texture->getTextureHeight() );
                    camera->attach( bc, texture, level, face, useMipmap>0?true:false, samples, colorSamples );
                    numAttached++;
                }
                else
                    OSG_NOTICE << "EffectCompositor: <pass> can't find global texture object " << xmlChild->getTrimmedContents() << std::endl;
            }
        }
        else if ( childName=="texture" || childName=="input_buffer" )
        {
            int override = atoi( xmlChild->properties["override"].c_str() );
            osg::StateAttribute::GLModeValue modeValue = osg::StateAttribute::ON;
            if ( override>0 ) modeValue |= osg::StateAttribute::OVERRIDE;
            
            int unit = atoi( xmlChild->properties["unit"].c_str() );
            std::string varname = xmlChild->properties["varname"];
            if ( xmlChild->children.size()>0 )
            {
                if ( varname.empty() ) varname = xmlChild->properties["name"];
                stateset->addUniform( new osg::Uniform(varname.c_str(), unit) );
                
                osg::Texture* texture = createTextureFromXML( xmlChild, false );
                stateset->setTextureAttributeAndModes( unit, texture, modeValue );
                if ( childName=="input_buffer" )
                    OSG_NOTICE << "EffectCompositor: <pass> " << name << " is using a local buffer which cannot be attached by others" << std::endl;
            }
            else
            {
                if ( varname.empty() ) varname = xmlChild->getTrimmedContents();
                stateset->addUniform( new osg::Uniform(varname.c_str(), unit) );
                
                osg::Texture* texture = getTexture( xmlChild->getTrimmedContents() );
                if ( texture ) stateset->setTextureAttributeAndModes( unit, texture, modeValue );
                else OSG_NOTICE << "EffectCompositor: <pass> can't find global texture object " << xmlChild->getTrimmedContents() << std::endl;
            }
        }
        else if ( childName=="uniform" )
        {
            if ( xmlChild->children.size()>0 )
            {
                osg::Uniform* uniform = createUniformFromXML( xmlChild, false );
                stateset->addUniform( uniform );
            }
            else
            {
                osg::Uniform* uniform = getUniform( xmlChild->getTrimmedContents() );
                if ( uniform ) stateset->addUniform( uniform );
                else OSG_NOTICE << "EffectCompositor: <pass> can't find global uniform object " << xmlChild->getTrimmedContents() << std::endl;
            }
        }
        else if ( childName=="shader" )
        {
            if ( xmlChild->children.size()>0 )
            {
                osg::Shader* shader = createShaderFromXML( xmlChild, false );
                program->addShader( shader );
            }
            else
            {
                osg::Shader* shader = getShader( xmlChild->getTrimmedContents() );
                if ( shader ) program->addShader( shader );
                else OSG_NOTICE << "EffectCompositor: <pass> can't find global shader object " << xmlChild->getTrimmedContents() << std::endl;
            }
        }
        else if ( childName=="vertex_attribute" )
        {
            std::string attrName = xmlChild->properties["name"];
            std::string attrIndex = xmlChild->properties["index"];
            if ( !attrIndex.empty() )
                program->addBindAttribLocation( attrName, atoi(attrIndex.c_str()) );
            else
                OSG_NOTICE << "EffectCompositor: <vertex_attribute> " << attrName << " doesn't have a valid index" << std::endl;
        }
        else if ( childName=="clear_color" )
        {
            std::stringstream ss; ss << xmlChild->getTrimmedContents();
            osg::Vec4 color; ss >> color;
            camera->setClearColor( color );
        }
        else if ( childName=="clear_mask" )
        {
            GLbitfield mask = 0;
            std::stringstream ss; ss << xmlChild->getTrimmedContents();
            while( !ss.eof() )
            {
                std::string value; ss >> value;
                if ( value=="color" ) mask |= GL_COLOR_BUFFER_BIT;
                else if ( value=="depth" ) mask |= GL_DEPTH_BUFFER_BIT;
                else if ( value=="stencil" ) mask |= GL_STENCIL_BUFFER_BIT;
            }
            camera->setClearMask( mask );
        }
        else if ( childName=="render_config" )
        {
            int order = atoi( xmlChild->properties["order"].c_str() );
            camera->setRenderOrder( osg::Camera::PRE_RENDER, order );
            
            std::string target = xmlChild->properties["target_method"];
            if ( target=="frame_buffer" ) camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER );
            else if ( target=="pixel_buffer" ) camera->setRenderTargetImplementation( osg::Camera::PIXEL_BUFFER );
            else if ( target=="pixel_buffer_rtt" ) camera->setRenderTargetImplementation( osg::Camera::PIXEL_BUFFER_RTT );
            else camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
        }
        else if ( childName=="culling" )
        {
            std::string cullingMask = xmlChild->properties["mask"];
            if ( !cullingMask.empty() )
            {
                unsigned int mask = 0xffffffff;
                std::stringstream ss( cullingMask );
                ss >> std::hex >> mask >> std::dec;
                camera->setCullMask( mask );
                camera->setCullMaskLeft( mask );
                camera->setCullMaskRight( mask );
            }
            
            std::string lodScale = xmlChild->properties["lodscale"];
            if ( !lodScale.empty() ) camera->setLODScale( atof(lodScale.c_str()) );
        }
        else if ( childName=="near_far" )
        {
            std::string nearFarMode = xmlChild->properties["computation"];
            if ( nearFarMode=="none" ) camera->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
            else if ( nearFarMode=="primitives" ) camera->setComputeNearFarMode( osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES );
            else if ( nearFarMode=="near_primitives" ) camera->setComputeNearFarMode( osg::CullSettings::COMPUTE_NEAR_USING_PRIMITIVES );
            else camera->setComputeNearFarMode( osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES );
            
            std::string nearFarRatio = xmlChild->properties["ratio"];
            if ( !nearFarRatio.empty() ) camera->setNearFarRatio( atoi(nearFarRatio.c_str()) );
        }
        else  // TODO: more state attributes here
            OSG_NOTICE << "EffectCompositor: <pass> doesn't recognize child element " << xmlChild->name << std::endl;
    }
    
    if ( program->getNumShaders()>0 )
    {
        osg::Vec3 texSize(1024.0f, 1024.0f, 1.0f);  // FIXME: if no buffer attached, should be viewport size
        const osg::Texture* attached = camera->getBufferAttachmentMap().size()==0 ? NULL :
                                       camera->getBufferAttachmentMap().begin()->second._texture.get();
        if ( attached )
            texSize.set( attached->getTextureWidth(), attached->getTextureHeight(), attached->getTextureDepth() );
        stateset->addUniform( new osg::Uniform("osg_OutputBufferSize", texSize) );
        stateset->setAttributeAndModes( program.get(), shaderModeValue );
    }
    
    if ( !numAttached )
    {
        // Automatically treat cameras without outputs as nested ones in the normal scene
        camera->setRenderOrder( osg::Camera::NESTED_RENDER );
        camera->setClearMask( 0 );
        
        // Note that FBO should be changed back to FRAME_BUFFER for handling camera resizing
        camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER );
    }
    return camera;
}

osg::Texture* EffectCompositor::createTextureFromXML( osgDB::XmlNode* xmlNode, bool asGlobal )
{
    osg::ref_ptr<osg::Texture> texture;
    std::string name = xmlNode->properties["name"];
    std::string type = xmlNode->properties["type"];
    if ( name.empty() )
        OSG_NOTICE << "EffectCompositor: inappropriate to create texture/buffer with empty name" << std::endl;
    
    bool isBufferObject = (xmlNode->name.find("buffer") != std::string::npos);
    bool useRelativeSize = (atoi(xmlNode->properties["relative_size"].c_str()) > 0);
    unsigned int levels = atoi( xmlNode->properties["max_levels"].c_str() );
    int w = _renderTargetResolution[0], h = _renderTargetResolution[1], d = _renderTargetResolution[2];
    if ( type=="1d" )
    {
        osg::Texture1D* tex1D = new osg::Texture1D;
        if ( isBufferObject )
        {
            if ( useRelativeSize ) w *= atof( xmlNode->properties["width"].c_str() );
            else w = atoi( xmlNode->properties["width"].c_str() );
            tex1D->setTextureWidth( w>0?w:_renderTargetResolution[0] );
            if ( levels>0 ) tex1D->setNumMipmapLevels( levels );
        }
        texture = tex1D;
    }
    else if ( type=="2d" )
    {
        osg::Texture2D* tex2D = new osg::Texture2D;
        if ( isBufferObject )
        {
            if ( useRelativeSize ) w *= atof( xmlNode->properties["width"].c_str() );
            else w = atoi( xmlNode->properties["width"].c_str() );
            if ( useRelativeSize ) h *= atof( xmlNode->properties["height"].c_str() );
            else h = atoi( xmlNode->properties["height"].c_str() );
            tex2D->setTextureSize( w>0?w:_renderTargetResolution[0], h>0?h:_renderTargetResolution[1] );
            if ( levels>0 ) tex2D->setNumMipmapLevels( levels );
        }
        texture = tex2D;
    }
    else if ( type=="2darray" )
    {
        osg::Texture2DArray* tex2DArray = new osg::Texture2DArray;
        if ( isBufferObject )
        {
            if ( useRelativeSize ) w *= atof( xmlNode->properties["width"].c_str() );
            else w = atoi( xmlNode->properties["width"].c_str() );
            if ( useRelativeSize ) h *= atof( xmlNode->properties["height"].c_str() );
            else h = atoi( xmlNode->properties["height"].c_str() );
            if ( useRelativeSize ) d *= atof( xmlNode->properties["depth"].c_str() );
            else d = atoi( xmlNode->properties["depth"].c_str() );
            tex2DArray->setTextureSize( w>0?w:_renderTargetResolution[0], h>0?h:_renderTargetResolution[1],
                                        d>0?d:_renderTargetResolution[2] );
            if ( levels>0 ) tex2DArray->setNumMipmapLevels( levels );
        }
        texture = tex2DArray;
    }
    else if ( type=="2dmultisample" )
    {
        osg::Texture2DMultisample* tex2DMultisample = new osg::Texture2DMultisample;
        if ( isBufferObject )
        {
            int samples = atoi( xmlNode->properties["samples"].c_str() );
            if ( useRelativeSize ) w *= atof( xmlNode->properties["width"].c_str() );
            else w = atoi( xmlNode->properties["width"].c_str() );
            if ( useRelativeSize ) h *= atof( xmlNode->properties["height"].c_str() );
            else h = atoi( xmlNode->properties["height"].c_str() );
            tex2DMultisample->setTextureSize( w>0?w:_renderTargetResolution[0],
                                              h>0?h:_renderTargetResolution[1] );
            tex2DMultisample->setNumSamples( samples );
        }
        texture = tex2DMultisample;
    }
    else if ( type=="3d" )
    {
        osg::Texture3D* tex3D = new osg::Texture3D;
        if ( isBufferObject )
        {
            if ( useRelativeSize ) w *= atof( xmlNode->properties["width"].c_str() );
            else w = atoi( xmlNode->properties["width"].c_str() );
            if ( useRelativeSize ) h *= atof( xmlNode->properties["height"].c_str() );
            else h = atoi( xmlNode->properties["height"].c_str() );
            if ( useRelativeSize ) d *= atof( xmlNode->properties["depth"].c_str() );
            else d = atoi( xmlNode->properties["depth"].c_str() );
            tex3D->setTextureSize( w>0?w:_renderTargetResolution[0], h>0?h:_renderTargetResolution[1],
                                   d>0?d:_renderTargetResolution[1] );
            if ( levels>0 ) tex3D->setNumMipmapLevels( levels );
        }
        texture = tex3D;
    }
    else if ( type=="rectangle" )
    {
        osg::TextureRectangle* texRect = new osg::TextureRectangle;
        if ( isBufferObject )
        {
            if ( useRelativeSize ) w *= atof( xmlNode->properties["width"].c_str() );
            else w = atoi( xmlNode->properties["width"].c_str() );
            if ( useRelativeSize ) h *= atof( xmlNode->properties["height"].c_str() );
            else h = atoi( xmlNode->properties["height"].c_str() );
            texRect->setTextureSize( w>0?w:_renderTargetResolution[0], h>0?h:_renderTargetResolution[1] );
        }
        texture = texRect;
    }
    else if ( type=="cubemap" )
    {
        osg::TextureCubeMap* texCubemap = new osg::TextureCubeMap;
        if ( isBufferObject )
        {
            if ( useRelativeSize ) w *= atof( xmlNode->properties["width"].c_str() );
            else w = atoi( xmlNode->properties["width"].c_str() );
            if ( useRelativeSize ) h *= atof( xmlNode->properties["height"].c_str() );
            else h = atoi( xmlNode->properties["height"].c_str() );
            texCubemap->setTextureSize( w>0?w:_renderTargetResolution[0], h>0?h:_renderTargetResolution[1] );
            if ( levels>0 ) texCubemap->setNumMipmapLevels( levels );
        }
        texture = texCubemap;
    }
    else texture = new osg::Texture2D;
    
    int resizeNPOT = atoi( xmlNode->properties["resize_npot"].c_str() );
    texture->setResizeNonPowerOfTwoHint( resizeNPOT>0?true:false );
    texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    
    osg::Image* image = NULL;
    unsigned char* imageData = NULL;
    for ( unsigned int i=0; i<xmlNode->children.size(); ++i )
    {
        osgDB::XmlNode* xmlChild = xmlNode->children[i].get();
        const std::string& childName = xmlChild->name;
        if ( !isXMLNodeType(xmlChild) ) continue;
        
        if ( childName=="file" )
        {
            std::string options = xmlChild->properties["options"];
            if ( options.empty() ) image = osgDB::readImageFile( xmlChild->getTrimmedContents() );
            else image = osgDB::readImageFile( xmlChild->getTrimmedContents(), new osgDB::Options(options) );
            
            std::string index = xmlChild->properties["index"];
            texture->setImage( atoi(index.c_str()), image );
            if ( !image )
            {
                OSG_NOTICE << "EffectCompositor: <texture> failed to load <file>: "
                           << xmlChild->getTrimmedContents() << std::endl;
            }
            
            std::string animated = xmlChild->properties["animated"];
            if ( atoi(animated.c_str())>0 )
            {
                osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*>( image );
                if ( imageStream ) imageStream->play();
            }
        }
        else if ( childName=="rawfile" )
        {
            int offset = atoi( xmlChild->properties["offset"].c_str() );
            w = atoi( xmlChild->properties["s"].c_str() );
            h = atoi( xmlChild->properties["t"].c_str() );
            d = atoi( xmlChild->properties["r"].c_str() );
            
            std::string rawfile = osgDB::findDataFile( xmlChild->getTrimmedContents() );
            std::ifstream ifs( rawfile.c_str(), std::ios::in|std::ios::binary|std::ios::ate );
            if ( ifs )
            {
                int length = (int)ifs.tellg() - offset;
                ifs.seekg( offset, std::ios::beg );
                ifs.clear();
                
                imageData = new unsigned char[length];
                ifs.read( (char*)imageData, length );
                ifs.close();
            }
            else
            {
                OSG_NOTICE << "EffectCompositor: <texture> failed to load <rawfile>: "
                           << rawfile << std::endl;
            }
        }
        else if ( childName=="wrap" )
        {
            std::string paramString = xmlChild->properties["param"];
            osg::Texture::WrapParameter param = osg::Texture::WRAP_S;
            if ( paramString=="t" ) param = osg::Texture::WRAP_T;
            else if ( paramString=="r" ) param = osg::Texture::WRAP_R;
            
            std::string modeString = xmlChild->getTrimmedContents();
            if ( modeString=="clamp_to_edge" ) texture->setWrap( param, osg::Texture::CLAMP_TO_EDGE );
            else if ( modeString=="clamp_to_border" ) texture->setWrap( param, osg::Texture::CLAMP_TO_BORDER );
            else if ( modeString=="repeat" ) texture->setWrap( param, osg::Texture::REPEAT );
            else if ( modeString=="mirror" ) texture->setWrap( param, osg::Texture::MIRROR );
            else texture->setWrap( param, osg::Texture::CLAMP );
        }
        else if ( childName=="filter" )
        {
            std::string paramString = xmlChild->properties["param"];
            osg::Texture::FilterParameter param = osg::Texture::MIN_FILTER;
            if ( paramString=="mag_filter" ) param = osg::Texture::MAG_FILTER;
            
            std::string modeString = xmlChild->getTrimmedContents();
            if ( modeString=="linear_mipmap_linear" ) texture->setFilter( param, osg::Texture::LINEAR_MIPMAP_LINEAR );
            else if ( modeString=="linear_mipmap_nearest" ) texture->setFilter( param, osg::Texture::LINEAR_MIPMAP_NEAREST );
            else if ( modeString=="nearest" ) texture->setFilter( param, osg::Texture::NEAREST );
            else if ( modeString=="nearest_mipmap_linear" ) texture->setFilter( param, osg::Texture::NEAREST_MIPMAP_LINEAR );
            else if ( modeString=="nearest_mipmap_nearest" ) texture->setFilter( param, osg::Texture::NEAREST_MIPMAP_NEAREST );
            else texture->setFilter( param, osg::Texture::LINEAR );
        }
        else if ( childName=="internal_format" )
        {
            std::string format = xmlChild->getTrimmedContents();
            if ( format=="rgb" ) texture->setInternalFormat( GL_RGB );
            else if ( format=="rgba" ) texture->setInternalFormat( GL_RGBA );
            else if ( format=="red" ) texture->setInternalFormat( GL_RED );
            else if ( format=="rg" ) texture->setInternalFormat( GL_RG );
            else if ( format=="rgb16f" ) texture->setInternalFormat( GL_RGB16F_ARB );
            else if ( format=="rgb32f" ) texture->setInternalFormat( GL_RGB32F_ARB );
            else if ( format=="rgba16f" ) texture->setInternalFormat( GL_RGBA16F_ARB );
            else if ( format=="rgba32f" ) texture->setInternalFormat( GL_RGBA32F_ARB );
            else if ( format=="red16f" ) texture->setInternalFormat( GL_R16F );
            else if ( format=="red32f" ) texture->setInternalFormat( GL_R32F );
            else if ( format=="rg16f" ) texture->setInternalFormat( GL_RG16F );
            else if ( format=="rg32f" ) texture->setInternalFormat( GL_RG32F );
            else if ( format=="depth16" ) texture->setInternalFormat( GL_DEPTH_COMPONENT16 );
            else if ( format=="depth24" ) texture->setInternalFormat( GL_DEPTH_COMPONENT24 );
            else if ( format=="depth32" ) texture->setInternalFormat( GL_DEPTH_COMPONENT32 );
            else if ( format=="depth24_stencil8" ) texture->setInternalFormat( GL_DEPTH24_STENCIL8_EXT );
            else
            {
                texture->setInternalFormat( atoi(format.c_str()) );
                OSG_NOTICE << "EffectCompositor: <texture> try to use internal format: "
                           << std::hex << atoi(format.c_str()) << std::dec << std::endl;
            }
            
            std::string compression = xmlChild->properties["compression"];
            if ( !compression.empty() )
            {
                if ( compression=="user" ) texture->setInternalFormatMode( osg::Texture::USE_USER_DEFINED_FORMAT );
                else if ( compression=="arb" ) texture->setInternalFormatMode( osg::Texture::USE_ARB_COMPRESSION );
                else if ( compression=="dxt1" ) texture->setInternalFormatMode( osg::Texture::USE_S3TC_DXT1_COMPRESSION );
                else if ( compression=="dxt3" ) texture->setInternalFormatMode( osg::Texture::USE_S3TC_DXT3_COMPRESSION );
                else if ( compression=="dxt5" ) texture->setInternalFormatMode( osg::Texture::USE_S3TC_DXT5_COMPRESSION );
                else if ( compression=="pvrtc_2bpp" ) texture->setInternalFormatMode( osg::Texture::USE_PVRTC_2BPP_COMPRESSION );
                else if ( compression=="pvrtc_4bpp" ) texture->setInternalFormatMode( osg::Texture::USE_PVRTC_4BPP_COMPRESSION );
                else if ( compression=="etc" ) texture->setInternalFormatMode( osg::Texture::USE_ETC_COMPRESSION );
                else if ( compression=="rgtc1" ) texture->setInternalFormatMode( osg::Texture::USE_RGTC1_COMPRESSION );
                else if ( compression=="rgtc2" ) texture->setInternalFormatMode( osg::Texture::USE_RGTC2_COMPRESSION );
                else texture->setInternalFormatMode( osg::Texture::USE_IMAGE_DATA_FORMAT );
            }
        }
        else if ( childName=="source_format" )
        {
            std::string format = xmlChild->getTrimmedContents();
            if ( format=="red" ) texture->setSourceFormat( GL_RED );
            else if ( format=="rg" ) texture->setSourceFormat( GL_RG );
            else if ( format=="rgb" ) texture->setSourceFormat( GL_RGB );
            else if ( format=="bgr" ) texture->setSourceFormat( GL_BGR );
            else if ( format=="rgba" ) texture->setSourceFormat( GL_RGBA );
            else if ( format=="bgra" ) texture->setSourceFormat( GL_BGRA );
            else if ( format=="depth" ) texture->setSourceFormat( GL_DEPTH_COMPONENT );
            else if ( format=="depth_stencil" ) texture->setSourceFormat( GL_DEPTH_STENCIL_EXT );
            else
            {
                texture->setSourceFormat( atoi(format.c_str()) );
                OSG_NOTICE << "EffectCompositor: <texture> try to use source format: "
                           << std::hex << atoi(format.c_str()) << std::dec << std::endl;
            }
        }
        else if ( childName=="source_type" )
        {
            std::string dataType = xmlChild->getTrimmedContents();
            if ( dataType=="byte" ) texture->setSourceType( GL_BYTE );
            else if ( dataType=="ubyte" ) texture->setSourceType( GL_UNSIGNED_BYTE );
            else if ( dataType=="short" ) texture->setSourceType( GL_SHORT );
            else if ( dataType=="ushort" ) texture->setSourceType( GL_UNSIGNED_SHORT );
            else if ( dataType=="int" ) texture->setSourceType( GL_INT );
            else if ( dataType=="uint" ) texture->setSourceType( GL_UNSIGNED_INT );
            else if ( dataType=="uint_24_8" ) texture->setSourceType( GL_UNSIGNED_INT_24_8_EXT );
            else if ( dataType=="float" ) texture->setSourceType( GL_FLOAT );
            else if ( dataType=="double" ) texture->setSourceType( GL_DOUBLE );
            else
            {
                texture->setSourceType( atoi(dataType.c_str()) );
                OSG_NOTICE << "EffectCompositor: <texture> try to use data type: "
                           << std::hex << atoi(dataType.c_str()) << std::dec << std::endl;
            }
        }
        else
            OSG_NOTICE << "EffectCompositor: <texture> doesn't recognize child element " << xmlChild->name << std::endl;
    }
    
    if ( imageData!=NULL )
    {
        image = new osg::Image;
        image->setImage( w>0?w:_renderTargetResolution[0], h>0?h:_renderTargetResolution[1], d>0?d:_renderTargetResolution[2],
                         texture->getInternalFormat(), texture->getSourceFormat(),
                         texture->getSourceType(), imageData, osg::Image::USE_NEW_DELETE );
        texture->setImage( 0, image );
    }
    
    if ( asGlobal )
    {
        if ( !setTexture(name, texture.get()) )
            OSG_NOTICE << "EffectCompositor: <texture> object name " << name << " already exists" << std::endl;
        return texture.get();
    }
    else
        return texture.release();
}

osg::Uniform* EffectCompositor::createUniformFromXML( osgDB::XmlNode* xmlNode, bool asGlobal )
{
    std::string name = xmlNode->properties["name"];
    if ( name.empty() )
        OSG_NOTICE << "EffectCompositor: inappropriate to create uniform with empty name" << std::endl;
    
    std::string typeString = xmlNode->properties["type"];
    osg::Uniform::Type type = osg::Uniform::getTypeId( typeString );
    osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform( type, name );
    
    int globalFlag = atoi( xmlNode->properties["global"].c_str() );
    if ( globalFlag>0 ) getOrCreateStateSet()->addUniform( uniform.get() );
    
    GLenum dataType = osg::Uniform::getInternalArrayType(type);
    int numValues = osg::Uniform::getTypeNumComponents(type);
    for ( unsigned int i=0; i<xmlNode->children.size(); ++i )
    {
        osgDB::XmlNode* xmlChild = xmlNode->children[i].get();
        const std::string& childName = xmlChild->name;
        if ( !isXMLNodeType(xmlChild) ) continue;
        
        if ( childName=="value" )
        {
            std::stringstream ss;
            ss << xmlChild->getTrimmedContents();
            switch ( dataType )
            {
            case GL_FLOAT:
                for ( int n=0; n<numValues; ++n )
                {
                    float v = 0.0f; ss >> v;
                    (*(uniform->getFloatArray()))[n] = v;
                }
                break;
            case GL_DOUBLE:
                for ( int n=0; n<numValues; ++n )
                {
                    double v = 0.0; ss >> v;
                    (*(uniform->getDoubleArray()))[n] = v;
                }
                break;
            case GL_INT:
                for ( int n=0; n<numValues; ++n )
                {
                    int v = 0; ss >> v;
                    (*(uniform->getIntArray()))[n] = v;
                }
                break;
            case GL_UNSIGNED_INT:
                for ( int n=0; n<numValues; ++n )
                {
                    unsigned int v = 0; ss >> v;
                    (*(uniform->getUIntArray()))[n] = v;
                }
                break;
            default:
                OSG_NOTICE << "EffectCompositor: <uniform> " << name << " doesn't have a recognizable value type: " << typeString << std::endl;
                break;
            }
            
            if ( ss.rdstate()&ss.failbit )
                OSG_NOTICE << "EffectCompositor: <uniform> " << name << " doesn't have a recognizable value: " << xmlChild->getTrimmedContents() << std::endl;
            uniform->dirty();
        }
        else if ( childName=="inbuilt_value" )
        {
            // These are already defined in OSG:
            //     osg_ModelViewMatrix, osg_ProjectionMatrix, osg_ModelViewProjectionMatrix, osg_NormalMatrix
            //     osg_FrameNumber, osg_FrameTime, osg_DeltaFrameTime, osg_SimulationTime, osg_DeltaSimulationTime,
            //     osg_ViewMatrix, osg_ViewMatrixInverse
            // These are defined in the EffectCompositor:
            //     osg_OutputBufferSize
            
            std::string valueName = xmlChild->getTrimmedContents();
            if ( valueName=="eye_position" ) addInbuiltUniform( EYE_POSITION, uniform );
            else if ( valueName=="view_point" ) addInbuiltUniform( VIEW_POINT, uniform );
            else if ( valueName=="look_vector" ) addInbuiltUniform( LOOK_VECTOR, uniform );
            else if ( valueName=="up_vector" ) addInbuiltUniform( UP_VECTOR, uniform );
            else if ( valueName=="left_vector" ) addInbuiltUniform( LEFT_VECTOR, uniform );
            else if ( valueName=="viewport_x" ) addInbuiltUniform( VIEWPORT_X, uniform );
            else if ( valueName=="viewport_y" ) addInbuiltUniform( VIEWPORT_Y, uniform );
            else if ( valueName=="viewport_width" ) addInbuiltUniform( VIEWPORT_WIDTH, uniform );
            else if ( valueName=="viewport_height" ) addInbuiltUniform( VIEWPORT_HEIGHT, uniform );
            else if ( valueName=="window_matrix" ) addInbuiltUniform( WINDOW_MATRIX, uniform );
            else if ( valueName=="inv_window_matrix" ) addInbuiltUniform( INV_WINDOW_MATRIX, uniform );
            else if ( valueName=="near_plane" ) addInbuiltUniform( FRUSTUM_NEAR_PLANE, uniform );
            else if ( valueName=="far_plane" ) addInbuiltUniform( FRUSTUM_FAR_PLANE, uniform );
            
            // These are defined for obtaining main scene matrices in deferred passes
            else if ( valueName=="fov" ) addInbuiltUniform( SCENE_FOV_IN_RADIANS, uniform );
            else if ( valueName=="aspect_ratio" ) addInbuiltUniform( SCENE_ASPECT_RATIO, uniform );
            else if ( valueName=="modelview_matrix" ) addInbuiltUniform( SCENE_MODELVIEW_MATRIX, uniform );
            else if ( valueName=="inv_modelview_matrix" ) addInbuiltUniform( SCENE_INV_MODELVIEW_MATRIX, uniform );
            else if ( valueName=="projection_matrix" ) addInbuiltUniform( SCENE_PROJECTION_MATRIX, uniform );
            else if ( valueName=="inv_projection_matrix" ) addInbuiltUniform( SCENE_INV_PROJECTION_MATRIX, uniform );
            else
            {
                OSG_NOTICE << "EffectCompositor: <inbuilt_value> of " << name << " doesn't have a recognizable value: " << valueName << std::endl;
            }
        }
        else if ( childName=="animation" )
        {
            bool unsupported = false;
            switch ( dataType )
            {
            case GL_FLOAT:
                switch ( numValues )
                {
                case 1: uniform->setUpdateCallback( createAnimatorFromXML<float>(xmlChild) ); break;
                case 2: uniform->setUpdateCallback( createAnimatorFromXML<osg::Vec2>(xmlChild) ); break;
                case 3: uniform->setUpdateCallback( createAnimatorFromXML<osg::Vec3>(xmlChild) ); break;
                case 4: uniform->setUpdateCallback( createAnimatorFromXML<osg::Vec4>(xmlChild) ); break;
                default: unsupported = true; break;
                }
                break;
            case GL_DOUBLE:
                switch ( numValues )
                {
                case 1: uniform->setUpdateCallback( createAnimatorFromXML<double>(xmlChild) ); break;
                case 2: uniform->setUpdateCallback( createAnimatorFromXML<osg::Vec2d>(xmlChild) ); break;
                case 3: uniform->setUpdateCallback( createAnimatorFromXML<osg::Vec3d>(xmlChild) ); break;
                case 4: uniform->setUpdateCallback( createAnimatorFromXML<osg::Vec4d>(xmlChild) ); break;
                default: unsupported = true; break;
                }
                break;
            case GL_INT:
                if ( numValues==1 ) uniform->setUpdateCallback( createAnimatorFromXML<int>(xmlChild) );
                else unsupported = true;
                break;
            case GL_UNSIGNED_INT:
                if ( numValues==1 ) uniform->setUpdateCallback( createAnimatorFromXML<unsigned int>(xmlChild) );
                else unsupported = true;
                break;
            default:
                OSG_NOTICE << "EffectCompositor: <uniform> " << name << " doesn't have a recognizable value type: " << typeString << std::endl;
                continue;
            }
            
            if ( unsupported )
            {
                OSG_NOTICE << "EffectCompositor: <animation> of " << name << " can't support the data type" << typeString << std::endl;
                continue;
            }
            
            // We have to notify the compositor to update the uniform, but as passes are not actual children of the compositor,
            // we must manually add the to-update number here; otherwise the update visitor won't traverse here
            setNumChildrenRequiringUpdateTraversal( getNumChildrenRequiringUpdateTraversal() + 1 );
        }
        else
            OSG_NOTICE << "EffectCompositor: <uniform> doesn't recognize child element " << xmlChild->name << std::endl;
    }
    
    if ( asGlobal )
    {
        if ( !setUniform(name, uniform.get()) )
            OSG_NOTICE << "EffectCompositor: <uniform> object name " << name << " already exists" << std::endl;
        return uniform.get();
    }
    else
        return uniform.release();
}

osg::Shader* EffectCompositor::createShaderFromXML( osgDB::XmlNode* xmlNode, bool asGlobal )
{
    osg::ref_ptr<osg::Shader> shader;
    std::string name = xmlNode->properties["name"];
    if ( name.empty() )
        OSG_NOTICE << "EffectCompositor: inappropriate to create shader with empty name" << std::endl;
    
    std::string type = xmlNode->properties["type"];
    if ( type=="vertex" ) shader = new osg::Shader(osg::Shader::VERTEX);
    else if ( type=="fragment" ) shader = new osg::Shader(osg::Shader::FRAGMENT);
    else if ( type=="geometry" ) shader = new osg::Shader(osg::Shader::GEOMETRY);
    else if ( type=="tess_control" ) shader = new osg::Shader(osg::Shader::TESSCONTROL);
    else if ( type=="tess_evaluation" ) shader = new osg::Shader(osg::Shader::TESSEVALUATION);
    else shader = new osg::Shader;
    shader->setName( name );
    
    std::string filePath;
    for ( unsigned int i=0; i<xmlNode->children.size(); ++i )
    {
        osgDB::XmlNode* xmlChild = xmlNode->children[i].get();
        const std::string& childName = xmlChild->name;
        if ( !isXMLNodeType(xmlChild) ) continue;
        
        if ( childName=="source" )
        {
            if ( xmlChild->children.size()>0 )
            {
                if ( xmlChild->children[0]->type==osgDB::XmlNode::INFORMATION )
                    shader->setShaderSource( xmlChild->children[0]->getTrimmedContents() );
                else
                    OSG_NOTICE << "EffectCompositor: <shader> " << name << " doesn't have a valid <source> definition" << std::endl;
            }
            else
            {
                OSG_NOTICE << "EffectCompositor: <shader> " << name << " specifies the source without using CDATA" << std::endl;
                shader->setShaderSource( xmlChild->getTrimmedContents() );
            }
        }
        else if ( childName=="file" )
        {
            std::string shaderFile = osgDB::findDataFile( xmlChild->getTrimmedContents() );
            if ( shaderFile.empty() )
            {
                OSG_NOTICE << "EffectCompositor: <shader> failed to load <file>: "
                           << xmlChild->getTrimmedContents() << std::endl;
            }
            else
            {
                filePath = osgDB::getFilePath( shaderFile );
                shader->loadShaderSourceFromFile( shaderFile );
            }
        }
        else
            OSG_NOTICE << "EffectCompositor: <shader> doesn't recognize child element " << xmlChild->name << std::endl;
    }
    
    std::string code = shader->getShaderSource();
    std::string::size_type pos = 0;
    while ( (pos = code.find("#include", pos))!=std::string::npos )
    {
        // Find all "#include" and handle them
        std::string::size_type pos2 = code.find_first_not_of(" ", pos + 8);
        if ( pos2==std::string::npos || code[pos2]!='\"' ) break;
        
        std::string::size_type pos3 = code.find("\"", pos2 + 1);
        if ( pos3==std::string::npos ) break;
        
        std::string filename = code.substr(pos2 + 1, pos3 - pos2 - 1);
        filename = osgDB::findDataFile( filename );
        if ( filename.empty() ) filename = osgDB::findDataFile( filePath + "/" + filename );
        
        osg::ref_ptr<osg::Shader> innerShader = osgDB::readShaderFile( shader->getType(), filename );
        if ( !innerShader ) break;
        
        code.replace( pos, pos3 - pos + 1, innerShader->getShaderSource() );
        pos += innerShader->getShaderSource().size();
    }
    shader->setShaderSource( code );
    
    if ( asGlobal )
    {
        if ( !setShader(name, shader.get()) )
            OSG_NOTICE << "EffectCompositor: <shader> object name " << name << " already exists" << std::endl;
        return shader.get();
    }
    else
        return shader.release();
}

bool EffectCompositor::loadFromXML( osgDB::XmlNode* xmlNode, XmlTemplateMap& templateMap, const osgDB::Options* options )
{
    if ( xmlNode->type==osgDB::XmlNode::ROOT )
    {
        for ( unsigned int i=0; i<xmlNode->children.size(); ++i )
        {
            osgDB::XmlNode* xmlChild = xmlNode->children[i];
            if ( xmlChild->name=="compositor" )
                return loadFromXML( xmlChild, templateMap, options );
        }
    }
    
    for ( unsigned int i=0; i<xmlNode->children.size(); ++i )
    {
        osgDB::XmlNode* xmlChild = xmlNode->children[i];
        if ( !isXMLNodeType(xmlChild) ) continue;
        
        const std::string& childName = xmlChild->name;
        if ( childName.find("template")!=std::string::npos )
        {
            std::string name = xmlChild->properties["name"];
            templateMap[name] = xmlChild;
            continue;
        }
        else if ( childName=="include" )
        {
            osg::ref_ptr<osgDB::XmlNode> xmlIncludedRoot = osgDB::readXmlFile( xmlChild->getTrimmedContents(), options );
            if ( xmlIncludedRoot.valid() ) loadFromXML( xmlIncludedRoot.get(), templateMap, options );
            continue;
        }
        else
        {
            // Check if have template to inherit from for all other elements
            std::string ref = xmlChild->properties["template"];
            if ( !ref.empty() )
            {
                osgDB::XmlNode* tempNode = templateMap[ref].get();
                if ( tempNode!=NULL ) inheritXmlNode( xmlChild, tempNode );
                else OSG_NOTICE << "EffectCompositor: <template> " << ref << " not found while applying to " << xmlChild->name << std::endl;
            }
        }
        
        if ( childName=="technique" )
        {
            std::string name = xmlChild->properties["name"];
            setCurrentTechnique( name.empty() ? "default" : name );
            for ( unsigned int j=0; j<xmlChild->children.size(); ++j )
            {
                osgDB::XmlNode* xmlPassChild = xmlChild->children[j];
                if ( !isXMLNodeType(xmlPassChild) ) continue;
                
                std::string ref = xmlPassChild->properties["template"];
                if ( !ref.empty() )
                {
                    osgDB::XmlNode* tempNode = templateMap[ref].get();
                    if ( tempNode!=NULL ) inheritXmlNode( xmlPassChild, tempNode );
                    else OSG_NOTICE << "EffectCompositor: <template> " << ref << " not found while applying to " << xmlPassChild->name << std::endl;
                }
                
                if ( xmlPassChild->name.find("pass")!=std::string::npos )
                    createPassFromXML( xmlPassChild );
                else
                    OSG_NOTICE << "EffectCompositor: <technique> doesn't recognize child element " << xmlPassChild->name << std::endl;
            }
        }
        else if ( childName=="buffer" || childName=="texture" )
            createTextureFromXML( xmlChild, true );
        else if ( childName=="uniform" )
            createUniformFromXML( xmlChild, true );
        else if ( childName=="shader" )
            createShaderFromXML( xmlChild, true );
        else
            OSG_NOTICE << "EffectCompositor: doesn't recognize global element " << childName << std::endl;
    }
    return true;
}

/* Global functions */

EffectCompositor* osgFX::readEffectFile( const std::string& filename, const osgDB::Options* options )
{
    osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlFile( filename, options );
    if ( xmlRoot.valid() )
    {
        osgDB::FilePathList& filePaths = osgDB::getDataFilePathList();
        filePaths.push_back( osgDB::getFilePath(filename) );
        
        osg::ref_ptr<EffectCompositor> compositor = new EffectCompositor;
        EffectCompositor::XmlTemplateMap templateMap;
        compositor->loadFromXML( xmlRoot.get(), templateMap, options );
        
        filePaths.pop_back();
        return compositor.release();
    }
    return NULL;
}

EffectCompositor* osgFX::readEffectStream( std::istream& stream, const osgDB::Options* options )
{
    osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlStream( stream );
    if ( xmlRoot.valid() )
    {
        osg::ref_ptr<EffectCompositor> compositor = new EffectCompositor;
        EffectCompositor::XmlTemplateMap templateMap;
        compositor->loadFromXML( xmlRoot.get(), templateMap, options );
        return compositor.release();
    }
    return NULL;
}
