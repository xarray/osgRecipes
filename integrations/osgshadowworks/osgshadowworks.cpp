#include <osg/Point>
#include <osg/Texture2D>
#include <osg/FrameBufferObject>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <GFSDK_SSAO.h>
#include <osg/GLExtensions>
#include <gl/gl.h>

osg::Camera* createMRTCamera( std::vector<osg::Texture2D*>& attachedTextures, int w, int h )
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearColor( osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    camera->setRenderOrder( osg::Camera::PRE_RENDER );
    camera->setViewport( 0, 0, w, h );
    
    osg::Texture2D* tex = new osg::Texture2D;
    tex->setTextureSize( w, h );
    tex->setSourceType( GL_UNSIGNED_BYTE );
    tex->setSourceFormat( GL_RGBA );
    tex->setInternalFormat( GL_RGBA );
    tex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    tex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
    tex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    tex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    attachedTextures.push_back( tex );
    camera->attach( osg::Camera::COLOR_BUFFER, tex );
    
    tex = new osg::Texture2D;
    tex->setTextureSize( w, h );
    tex->setSourceType( GL_FLOAT );
    tex->setSourceFormat( GL_DEPTH_COMPONENT );
    tex->setInternalFormat( GL_DEPTH_COMPONENT32 );
    tex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    tex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
    tex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    tex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    attachedTextures.push_back( tex );
    camera->attach( osg::Camera::DEPTH_BUFFER, tex );
    return camera.release();
}

osg::Camera* createHUDCamera( double left, double right, double bottom, double top )
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setAllowEventFocus( false );
    camera->setProjectionMatrix( osg::Matrix::ortho2D(left, right, bottom, top) );
    camera->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    return camera.release();
}

class HBAODrawCallback : public osg::Camera::DrawCallback
{
public:
    HBAODrawCallback();
    
    void setInputDepthTexture( osg::Texture2D* tex ) { _depthTexture = tex; }
    osg::Texture2D* getInputDepthTexture() { return _depthTexture.get(); }
    
    void setInputNormalTexture( osg::Texture2D* tex ) { _normalTexture = tex; }
    osg::Texture2D* getInputNormalTexture() { return _normalTexture.get(); }
    
    void setOutputTexture( osg::Texture2D* tex ) { _outputTexture = tex; _dirtyFBO = true; }
    osg::Texture2D* getOutputTexture() { return _outputTexture.get(); }
    
    virtual void operator()( osg::RenderInfo& renderInfo ) const;
    
protected:
    virtual ~HBAODrawCallback();
    bool initializeContext( osg::RenderInfo& renderInfo );
    
    osg::ref_ptr<osg::Texture2D> _depthTexture;
    osg::ref_ptr<osg::Texture2D> _normalTexture;
    osg::ref_ptr<osg::Texture2D> _outputTexture;
    osg::ref_ptr<osg::FrameBufferObject> _fbo;
    GFSDK_SSAO_Context_GL* _aoContext;
    mutable bool _dirtyFBO;
};

HBAODrawCallback::HBAODrawCallback()
:   _aoContext(NULL), _dirtyFBO(false)
{
    _fbo = new osg::FrameBufferObject;
}

HBAODrawCallback::~HBAODrawCallback()
{
    if ( _aoContext ) delete _aoContext;
}

void HBAODrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
    osg::State& state = *(renderInfo.getState());
    if ( !_aoContext )
    {
        // Initialize AO
        HBAODrawCallback* nonconst = const_cast<HBAODrawCallback*>( this );
        if ( !nonconst->initializeContext(renderInfo) ) return;
    }
    
    // Handle scene elements
    osg::Matrixf projMatrix = state.getProjectionMatrix();
    osg::Matrixf viewMatrix = state.getModelViewMatrix();
    if ( !_depthTexture || !_outputTexture ) return;
    
    unsigned int contextID = renderInfo.getContextID();
    GLuint fboID = _fbo->getHandle(contextID);
    if ( _dirtyFBO )
    {
        _fbo->setAttachment( osg::Camera::COLOR_BUFFER,
                             osg::FrameBufferAttachment(_outputTexture.get()) );
        _dirtyFBO = false;
    }
    
    if ( fboID==0 )
    {
        _fbo->apply( state );
        fboID = _fbo->getHandle(contextID);
    }
    
    osg::Texture::TextureObject* depthObj = _depthTexture->getTextureObject(contextID);
    osg::Texture::TextureObject* normalObj =
        _normalTexture.valid() ? _normalTexture->getTextureObject(contextID) : NULL;
    if ( fboID==0 || !depthObj ) return;
    
    // Render AO
    GFSDK_SSAO_Parameters_GL aoParams;
    aoParams.Radius = 2.0f;
    aoParams.Bias = 0.2f;
    aoParams.DetailAO = 1.0f;
    aoParams.CoarseAO = 1.0f;
    aoParams.DepthStorage = GFSDK_SSAO_FP32_VIEW_DEPTHS;//GFSDK_SSAO_FP16_VIEW_DEPTHS;
    aoParams.PowerExponent = 10.0f;
    aoParams.Output.BlendMode = GFSDK_SSAO_OVERWRITE_RGB;
    aoParams.Blur.Enable = true;
    aoParams.Blur.Sharpness = 4.0f;
    aoParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_2;//GFSDK_SSAO_BLUR_RADIUS_4;//GFSDK_SSAO_BLUR_RADIUS_8;
    
    GFSDK_SSAO_InputData_GL input;
    input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
    input.DepthData.FullResDepthTexture = GFSDK_SSAO_Texture_GL(GL_TEXTURE_2D, depthObj->id());
    input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4(projMatrix.ptr());
    input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
    input.DepthData.MetersToViewSpaceUnits = 1.0f;
    if ( normalObj )
    {
        input.NormalData.Enable = true;
        input.NormalData.FullResNormalTexture = GFSDK_SSAO_Texture_GL(GL_TEXTURE_2D, normalObj->id());
        input.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4(viewMatrix.ptr());
        input.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
        input.NormalData.DecodeScale = 2.0f;
        input.NormalData.DecodeBias = -1.0f;
    }
    
    GFSDK_SSAO_RenderMask renderMask = GFSDK_SSAO_RENDER_AO;//GFSDK_SSAO_RENDER_DEBUG_NORMAL;
    glDisable( GL_DEPTH_TEST );
    
    GFSDK_SSAO_Status status = _aoContext->RenderAO( &input, &aoParams, fboID, renderMask );
    if ( status!=GFSDK_SSAO_OK )
    {
        OSG_WARN << "[HBAODrawCallback] Failed to render AO: " << status << std::endl;
    }
}

bool HBAODrawCallback::initializeContext( osg::RenderInfo& renderInfo )
{
    GFSDK_SSAO_GLFunctions GL;
    osg::setGLExtensionFuncPtr( GL.glActiveTexture, "glActiveTexture", "glActiveTextureARB" );
    osg::setGLExtensionFuncPtr( GL.glAttachShader, "glAttachShader", "glAttachObjectARB" );
    osg::setGLExtensionFuncPtr( GL.glBindBuffer, "glBindBuffer", "glBindBufferARB" );
    osg::setGLExtensionFuncPtr( GL.glBindBufferBase, "glBindBufferBase", "glBindBufferBaseEXT", "glBindBufferBaseNV" );
    osg::setGLExtensionFuncPtr( GL.glBindFramebuffer, "glBindFramebuffer", "glBindFramebufferEXT", "glBindFramebufferOES" );
    osg::setGLExtensionFuncPtr( GL.glBindFragDataLocation, "glBindFragDataLocation", "glBindFragDataLocationEXT" );
    GL.glBindTexture = glBindTexture;
    osg::setGLExtensionFuncPtr( GL.glBindVertexArray, "glBindVertexArray" );
    osg::setGLExtensionFuncPtr( GL.glBlendColor, "glBlendColor", "glBlendColorEXT" );
    osg::setGLExtensionFuncPtr( GL.glBlendEquationSeparate, "glBlendEquationSeparate", "glBlendEquationSeparateEXT" );
    osg::setGLExtensionFuncPtr( GL.glBlendFuncSeparate, "glBlendFuncSeparate", "glBlendFuncSeparateEXT" );
    osg::setGLExtensionFuncPtr( GL.glBufferData, "glBufferData", "glBufferDataARB" );
    osg::setGLExtensionFuncPtr( GL.glBufferSubData, "glBufferSubData", "glBufferSubDataARB" );
    osg::setGLExtensionFuncPtr( GL.glColorMaski, "glColorMaski", "glColorMaskiARB" );
    osg::setGLExtensionFuncPtr( GL.glCompileShader, "glCompileShader", "glCompileShaderARB" );
    osg::setGLExtensionFuncPtr( GL.glCreateShader, "glCreateShader", "glCreateShaderObjectARB" );
    osg::setGLExtensionFuncPtr( GL.glCreateProgram, "glCreateProgram", "glCreateProgramObjectARB" );
    osg::setGLExtensionFuncPtr( GL.glDeleteBuffers, "glDeleteBuffers", "glDeleteBuffersARB" );
    osg::setGLExtensionFuncPtr( GL.glDeleteFramebuffers, "glDeleteFramebuffers", "glDeleteFramebuffersEXT", "glDeleteFramebuffersOES" );
    osg::setGLExtensionFuncPtr( GL.glDeleteProgram, "glDeleteProgram" );
    osg::setGLExtensionFuncPtr( GL.glDeleteShader, "glDeleteShader" );
    osg::setGLExtensionFuncPtr( GL.glDeleteTextures, "glDeleteTextures" );
    osg::setGLExtensionFuncPtr( GL.glDeleteVertexArrays, "glDeleteVertexArrays" );
    GL.glDisable = glDisable;
    osg::setGLExtensionFuncPtr( GL.glDrawBuffers, "glDrawBuffers", "glDrawBuffersARB" );
    GL.glEnable = glEnable;
    GL.glDrawArrays = glDrawArrays;
    osg::setGLExtensionFuncPtr( GL.glFramebufferTexture, "glFramebufferTexture", "glFramebufferTextureEXT", "glFramebufferTextureOES" );
    osg::setGLExtensionFuncPtr( GL.glFramebufferTexture2D, "glFramebufferTexture2D", "glFramebufferTexture2DEXT", "glFramebufferTexture2DOES" );
    osg::setGLExtensionFuncPtr( GL.glFramebufferTextureLayer, "glFramebufferTextureLayer", "glFramebufferTextureLayerEXT", "glFramebufferTextureLayerOES" );
    osg::setGLExtensionFuncPtr( GL.glGenBuffers, "glGenBuffers", "glGenBuffersARB" );
    osg::setGLExtensionFuncPtr( GL.glGenFramebuffers, "glGenFramebuffers", "glGenFramebuffersEXT", "glGenFramebuffersOES" );
    GL.glGenTextures = glGenTextures;
    osg::setGLExtensionFuncPtr( GL.glGenVertexArrays, "glGenVertexArrays" );
    GL.glGetError = glGetError;
    osg::setGLExtensionFuncPtr( GL.glGetBooleani_v, "glGetBooleani_v" );
    GL.glGetFloatv = glGetFloatv;
    GL.glGetIntegerv = glGetIntegerv;
    osg::setGLExtensionFuncPtr( GL.glGetIntegeri_v, "glGetIntegeri_v" );
    osg::setGLExtensionFuncPtr( GL.glGetProgramiv, "glGetProgramiv" );
    osg::setGLExtensionFuncPtr( GL.glGetProgramInfoLog, "glGetProgramInfoLog" );
    osg::setGLExtensionFuncPtr( GL.glGetShaderiv, "glGetShaderiv" );
    osg::setGLExtensionFuncPtr( GL.glGetShaderInfoLog, "glGetShaderInfoLog" );
    GL.glGetString = glGetString;
    osg::setGLExtensionFuncPtr( GL.glGetUniformBlockIndex, "glGetUniformBlockIndex" );
    osg::setGLExtensionFuncPtr( GL.glGetUniformLocation, "glGetUniformLocation", "glGetUniformLocationARB" );
    osg::setGLExtensionFuncPtr( GL.glGetTexLevelParameteriv, "glGetTexLevelParameteriv", "glGetTexLevelParameterivEXT", "glGetTexLevelParameterivNV" );
    GL.glIsEnabled = glIsEnabled;
    osg::setGLExtensionFuncPtr( GL.glIsEnabledi, "glIsEnabledi" );
    osg::setGLExtensionFuncPtr( GL.glLinkProgram, "glLinkProgram", "glLinkProgramARB" );
    GL.glPolygonOffset = glPolygonOffset;
    osg::setGLExtensionFuncPtr( GL.glShaderSource, "glShaderSource", "glShaderSourceARB" );
    GL.glTexImage2D = glTexImage2D;
    osg::setGLExtensionFuncPtr( GL.glTexImage3D, "glTexImage3D","glTexImage3DEXT" );
    GL.glTexParameteri = glTexParameteri;
    GL.glTexParameterfv = glTexParameterfv;
    osg::setGLExtensionFuncPtr( GL.glUniform1i, "glUniform1i", "glUniform1iARB" );
    osg::setGLExtensionFuncPtr( GL.glUniformBlockBinding, "glUniformBlockBinding" );
    osg::setGLExtensionFuncPtr( GL.glUseProgram, "glUseProgram", "glUseProgramObjectARB" );
    GL.glViewport = glViewport;
    
    GFSDK_SSAO_CustomHeap customHeap;
    customHeap.new_ = ::operator new;
    customHeap.delete_ = ::operator delete;
    
    GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_GL( &_aoContext, &GL, &customHeap );
    if ( status!=GFSDK_SSAO_OK )
    {
        OSG_WARN << "[HBAODrawCallback] Failed to initialize context: " << status << std::endl;
        return false;
    }
    return true;
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("sponza.ive") );
    
    // Create MRT camera and setup draw callback
    int w = 1600, h = 900;
    std::vector<osg::Texture2D*> textures;
    
    osg::Camera* mrtCam = createMRTCamera( textures, w, h );
    mrtCam->addChild( scene.get() );
    
    osg::ref_ptr<HBAODrawCallback> vxgi = new HBAODrawCallback;
    vxgi->setInputNormalTexture( textures[0] );
    vxgi->setInputDepthTexture( textures[1] );
    mrtCam->setPostDrawCallback( vxgi.get() );
    
    osg::ref_ptr<osg::Texture2D> outputTex = new osg::Texture2D;
    outputTex->setTextureSize( w, h );
    outputTex->setSourceType( GL_UNSIGNED_BYTE );
    outputTex->setSourceFormat( GL_RGBA );
    outputTex->setInternalFormat( GL_RGBA );
    outputTex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    outputTex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
    outputTex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    outputTex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    vxgi->setOutputTexture( outputTex.get() );
    
    // Create shader program to be used
    const char* mrtVertexCode = {
        "varying vec4 vecInEye;\n"
        "void main() {\n"
        "   vecInEye = gl_ModelViewMatrix * gl_Vertex;\n"
        "   gl_Position = ftransform();\n"
        "}\n"
    };
    
    const char* mrtFragmentCode = {
        "uniform mat4 osg_ViewMatrixInverse;\n"
        "varying vec4 vecInEye;\n"
        "void main() {\n"
        "   vec3 normal = normalize(cross(dFdx(vecInEye.xyz), dFdy(vecInEye.xyz)));\n"
        "   normal = mat3(osg_ViewMatrixInverse) * vec3(normal.x, -normal.y, -normal.z);\n"
        "   gl_FragColor = vec4(normal * 0.5 + vec3(0.5), 1.0);\n"
        "}\n"
    };
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, mrtVertexCode) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, mrtFragmentCode) );

    osg::StateSet* ss = mrtCam->getOrCreateStateSet();
    ss->setAttributeAndModes( program.get() );
    
    // Create screen quad to contain the MRT result
    osg::ref_ptr<osg::Geode> quad = new osg::Geode;
    quad->addDrawable( osg::createTexturedQuadGeometry(
        osg::Vec3(), osg::Vec3(0.5f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f),
        0.0f, 0.0f, 0.5f, 1.0f) );
    
    osg::Camera* hudCam = createHUDCamera( 0.0, 1.0, 0.0, 1.0 );
    hudCam->getOrCreateStateSet()->setTextureAttributeAndModes( 0, outputTex.get() );
    hudCam->addChild( quad.get() );
    
    // Construct scene graph and viewer
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( mrtCam );
    root->addChild( hudCam );
    root->addChild( scene.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    //viewer.getCamera()->setProjectionMatrixAsPerspective( 30.0f, 16.0f/9.0f, 0.1f, 1000.0f );
    //viewer.getCamera()->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.setUpViewOnSingleScreen( 0 );
    
    viewer.run();
    return 0;
}
