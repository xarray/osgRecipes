#include <osg/Texture2D>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgUtil/LineSegmentIntersector>
#include <osgText/Text>
#include <osgViewer/Viewer>

#include "EffectCompositor"

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

osgText::Text* createText( const osg::Vec3& pos, const std::string& content, float size,
                           const osg::Vec4& color, unsigned int drawMask=osgText::Text::TEXT )
{
    static osg::ref_ptr<osgText::Font> s_font = osgText::readFontFile("fonts/arial.ttf");
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setDataVariance( osg::Object::DYNAMIC );
    text->setFont( s_font.get() );
    text->setCharacterSize( size );
    text->setAxisAlignment( osgText::TextBase::XY_PLANE );
    text->setPosition( pos );
    text->setText( content );
    text->setColor( color );
    text->setBoundingBoxColor( color );
    text->setDrawMode( drawMask );
    return text.release();
}

osg::Geometry* createConnection( const osg::Vec3& s, const osg::Vec3& e,
                                 const osg::Vec4& color, bool midStep=true )
{
    osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array;
    va->push_back( s );
    if ( midStep )
    {
        float midY = s[1] * 0.6f + e[1] * 0.4f;
        va->push_back( osg::Vec3(s[0], midY, 0.0f) );
        va->push_back( osg::Vec3(e[0], midY, 0.0f) );
    }
    va->push_back( e );
    osg::ref_ptr<osg::Vec4Array> ca = new osg::Vec4Array(1);
    (*ca)[0] = color;
    
    osg::ref_ptr<osg::Geometry> line = new osg::Geometry;
    line->setVertexArray( va.get() );
    line->setColorArray( ca.get() );
    line->setColorBinding( osg::Geometry::BIND_OVERALL );
    line->addPrimitiveSet( new osg::DrawArrays(GL_LINE_STRIP, 0, va->size()) );
    return line.release();
}

osg::Geometry* createQuad( const osg::Vec3& pos, float w, float h,
                           float l=0.0f, float b=0.0f, float r=1.0f, float t=1.0f, bool copyTexCoords=false )
{
    osg::Geometry* quad = osg::createTexturedQuadGeometry(
        pos, osg::Vec3(w,0.0f,0.0f), osg::Vec3(0.0f,h,0.0f), l, b, r, t );
    if ( copyTexCoords )
    {
        osg::Array* texcoords = quad->getTexCoordArray(0);
        for ( unsigned int i=1; i<8; ++i )
            quad->setTexCoordArray( i, texcoords );
    }
    return quad;
}

class CompositorAnalysis : public osgGA::GUIEventHandler
{
public:
    struct Connection
    {
        std::vector<osg::Vec3> starts;
        std::vector<osg::Vec3> ends;
    };
    
    CompositorAnalysis( osg::Camera* cam, osgFX::EffectCompositor* c, int numBuffers=3 )
    :   _hudCamera(cam), _compositor(c), _startIndex(0)
    {
        _bufferDisplay = new osg::Geode;
        _bufferDisplay->setNodeMask( 0xffffffff );
        _uniformDisplay = new osg::Geode;
        _uniformDisplay->setNodeMask( 0 );
        _graphDisplay = new osg::Geode;
        _hudCamera->addChild( _bufferDisplay.get() );
        _hudCamera->addChild( _uniformDisplay.get() );
        _hudCamera->addChild( _graphDisplay.get() );
        
        // Allocate quads for displaying buffer contents
        float size = osg::minimum(1.0f/(float)numBuffers, 0.4f);
        for ( int i=0; i<numBuffers; ++i )
        {
            osg::Vec3 pos(size*(float)i, 0.0f, 0.0f);
            
            osg::Geometry* quad = createQuad( pos, size, size );
            _bufferQuads.push_back( quad );
            _bufferDisplay->addDrawable( quad );
            
            osgText::Text* text = createText( pos+osg::Vec3(0.0f,size,0.0f), "", 0.02f,
                                              osg::Vec4(1.0f,1.0f,0.0f,1.0f) );
            _bufferTexts.push_back( text );
            _bufferDisplay->addDrawable( text );
        }
        
        // Initialize compositor graph, uniforms and buffers
        applyEffectGraph( c, 0.01f, size + 0.1f, 0.98f, 0.4f );
        applyUniformList( c, 0.01f, size + 0.1f, 0.98f, 0.4f );
        _numBuffers = applyBufferData( c );
    }
    
    void applyEffectGraph( osgFX::EffectCompositor* c, float x, float y, float w, float h )
    {
        int numX = 1, numY = 0;
        std::map<osg::Texture*, Connection> bufferConnectionMap;
        _graphDisplay->removeDrawables( 0, _graphDisplay->getNumDrawables() );
        
        const osgFX::EffectCompositor::PassList& passes = c->getPassList();
        for ( unsigned int i=0; i<passes.size(); ++i )
        {
            const osgFX::EffectCompositor::PassData& pd = passes[i];
            if ( pd.type!=osgFX::EffectCompositor::FORWARD_PASS ) numX++;
            numY++;
        }
        
        // Create pass blocks
        float intervalX = w/(float)numX, intervalY = h/(float)numY;
        float incX = x + intervalX, incY = y;
        for ( unsigned int i=0; i<passes.size(); ++i )
        {
            osg::Vec3 leftBottom(x, y, 0.0f), rightTop;
            osgText::Text* text = NULL;
            const osgFX::EffectCompositor::PassData& pd = passes[i];
            if ( pd.type==osgFX::EffectCompositor::FORWARD_PASS )
            {
                leftBottom.y() = incY;
                incY += intervalY;
                text = createText( leftBottom, pd.name, 0.02f,
                    osg::Vec4(0.0f,0.5f,1.0f,1.0f), osgText::Text::TEXT|osgText::Text::BOUNDINGBOX );
            }
            else
            {
                osg::Vec4 color(0.0f, 1.0f, 0.0f, 1.0f);
                if ( pd.isDisplayPass() ) color.set(1.0f, 0.5f, 0.0f, 1.0f);
                leftBottom.set( incX, incY, 0.0f );
                incX += intervalX; incY += intervalY;
                text = createText( leftBottom, pd.name, 0.02f,
                    color, osgText::Text::TEXT|osgText::Text::BOUNDINGBOX );
            }
            osg::BoundingBox bb = text->computeBoundingBox();
            float centerY = (bb.yMax() + bb.yMin()) * 0.5f;
            leftBottom = bb._min;
            rightTop = bb._max;
            _graphDisplay->addDrawable( text );
            
            // Connection start at pass outputs
            const osg::Camera::BufferAttachmentMap& attachments = pd.pass->getBufferAttachmentMap();
            float invOutputSize = 1.0f / (float)attachments.size();
            for ( osg::Camera::BufferAttachmentMap::const_iterator itr=attachments.begin();
                  itr!=attachments.end(); ++itr )
            {
                osg::Texture* tex = itr->second._texture.get();
                bufferConnectionMap[tex].starts.push_back( rightTop );
                rightTop.x() -= (bb.xMax() - bb.xMin()) * invOutputSize;
            }
            
            // Connection end at pass inputs
            osg::StateAttribute::TypeMemberPair type(osg::StateAttribute::TEXTURE, 0);
            const osg::StateSet::TextureAttributeList& texList =
                pd.pass->getOrCreateStateSet()->getTextureAttributeList();
            float invInputSize = 1.0f / (float)texList.size();
            for ( unsigned int j=0; j<texList.size(); ++j )
            {
                const osg::StateSet::AttributeList& attrList = texList[j];
                osg::StateSet::AttributeList::const_iterator aitr = attrList.find(type);
                if ( aitr==attrList.end() ) continue;
                
                osg::Texture* tex = dynamic_cast<osg::Texture*>( aitr->second.first.get() );
                bufferConnectionMap[tex].ends.push_back( leftBottom );
                leftBottom.x() += (bb.xMax() - bb.xMin()) * invInputSize;
            }
        }
        
        // Create connections
        const osgFX::EffectCompositor::TextureMap& buffers = c->getTextureMap();
        for ( std::map<osg::Texture*, Connection>::iterator itr=bufferConnectionMap.begin();
              itr!=bufferConnectionMap.end(); ++itr )
        {
            std::string name;
            osg::Texture* tex = itr->first;
            for ( osgFX::EffectCompositor::TextureMap::const_iterator titr=buffers.begin();
                  titr!=buffers.end(); ++titr )
            {
                if ( titr->second==tex ) name = titr->first;
            }
            
            Connection& connection = itr->second;
            unsigned int size = osg::minimum(connection.starts.size(), connection.ends.size());
            for ( unsigned int n=0; n<size; ++n )
            {
                osg::Vec3 s = connection.starts[n], e = connection.ends[n];
                osg::Geometry* line = createConnection( s, e, osg::Vec4(0.8f,0.2f,0.2f, 1.0f) );
                _graphDisplay->addDrawable( line );
                
                osgText::Text* text = createText( s, name, 0.01f, osg::Vec4(0.8f,0.2f,0.2f, 1.0f) );
                _graphDisplay->addDrawable( text );
            }
        }
    }
    
    void applyUniformList( osgFX::EffectCompositor* c, float x, float y, float w, float h )
    {
    }
    
    unsigned int applyBufferData( osgFX::EffectCompositor* c, int startIndex=0 )
    {
        int index = 0, displaySize = (int)_bufferQuads.size();
        const osgFX::EffectCompositor::TextureMap& textures = c->getTextureMap();
        for ( osgFX::EffectCompositor::TextureMap::const_iterator itr=textures.begin();
              itr!=textures.end(); ++itr )
        {
            osg::Texture* tex = itr->second.get();
            if ( !tex->getTextureWidth() || tex->getImage(0)!=NULL ) continue;  // not a buffer
            
            if ( startIndex<=index && index<displaySize+startIndex )
            {
                osg::StateSet* ss = _bufferQuads[index-startIndex]->getOrCreateStateSet();
                ss->setTextureAttributeAndModes( 0, tex );
                _bufferTexts[index-startIndex]->setText( itr->first );
            }
            ++index;
        }
        return index;
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::DRAG:
            {
                osg::ref_ptr<osgUtil::LineSegmentIntersector> li = new osgUtil::LineSegmentIntersector(
                    osgUtil::Intersector::PROJECTION, ea.getXnormalized(), ea.getYnormalized() );
                osgUtil::IntersectionVisitor iv( li.get() );
                _hudCamera->accept( iv );
                
                if ( li->containsIntersections() )
                {
                }
            }
        case osgGA::GUIEventAdapter::KEYUP:
            if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_F1 )
            {
                unsigned int mask = 0xffffffff;
                if ( _hudCamera->getNodeMask()==mask ) mask = 0;
                _hudCamera->setNodeMask( mask );
            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_F2 )
            {
                unsigned int mask = 0xffffffff;
                if ( _graphDisplay->getNodeMask()==mask ) mask = 0;
                _graphDisplay->setNodeMask( mask );
                _uniformDisplay->setNodeMask( ~mask );
            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Page_Up )
            {
                if ( _startIndex<_numBuffers-_bufferQuads.size() ) _startIndex++;
                applyBufferData( _compositor.get(), _startIndex );
            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Page_Down )
            {
                if ( _startIndex>0 ) _startIndex--;
                applyBufferData( _compositor.get(), _startIndex );
            }
            break;
        case osgGA::GUIEventAdapter::FRAME:
            for ( std::map<osg::Uniform*, osgText::Text*>::iterator itr=_uniformTextMap.begin();
                  itr!=_uniformTextMap.end(); ++itr )
            {
                // TODO
            }
            break;
        default: break;
        }
        return false;
    }
    
protected:
    std::map<osg::Uniform*, osgText::Text*> _uniformTextMap;
    std::vector<osg::Geometry*> _bufferQuads;
    std::vector<osgText::Text*> _bufferTexts;
    osg::observer_ptr<osg::Geode> _bufferDisplay;
    osg::observer_ptr<osg::Geode> _uniformDisplay;
    osg::observer_ptr<osg::Geode> _graphDisplay;
    osg::observer_ptr<osg::Camera> _hudCamera;
    osg::observer_ptr<osgFX::EffectCompositor> _compositor;
    unsigned int _numBuffers;
    unsigned int _startIndex;
};

void configureViewerForMode( osgViewer::Viewer& viewer, osgFX::EffectCompositor* compositor,
                             osg::Node* model, int displayMode )
{
    osg::Group* root = viewer.getSceneData()->asGroup();
    if ( !root ) return;
    
    switch ( displayMode )
    {
    case 1:  // analysis mode
        {
            osg::Camera* camera = createHUDCamera( 0.0, 1.0, 0.0, 1.0 );
            camera->setRenderOrder( osg::Camera::POST_RENDER, 10 );  // make sure to render after the compositor
            root->addChild( camera );
            
            viewer.addEventHandler( new CompositorAnalysis(camera, compositor) );
        }
        break;
    case 2:  // compare mode
        if ( compositor->getNumPasses()>0 )
        {
            // Use a half-quad as the final output surface and render to origin model again, so that to compare them in one screen
            osg::ref_ptr<osg::Geode> halfQuad = new osg::Geode;
            halfQuad->addDrawable( createQuad(osg::Vec3(), 0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, true) );
            
            osgFX::EffectCompositor::PassData& lastPass = compositor->getPassList().back();
            lastPass.pass->addChild( halfQuad.get() );
            root->addChild( model );
        }
        break;
    default: break;
    }
}
