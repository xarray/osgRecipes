#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/EventVisitor>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <FxParser.h>
#define NvFxProgram_ID 0x8000

class NvFxProgramManager : public osg::Referenced
{
public:
    static NvFxProgramManager* instance()
    {
        static osg::ref_ptr<NvFxProgramManager> s_instance = new NvFxProgramManager;
        return s_instance.get();
    }
    
    static void errorCallbackFunc( const char* msg )
    {
        OSG_WARN << "[NvFxProgram] " << msg << std::endl;
    }
    
    static void includeCallbackFunc( const char* includeName, FILE*& fp, const char*& buffer )
    {
        fp = fopen( includeName, "r" );
    }
    
    void passUpdated( nvFX::IPass* p ) { _updatedPasses.push_back(p); }
    
    void unbindAllPasses()
    {
        for ( unsigned int i=0; i<_updatedPasses.size(); ++i )
        {
            _updatedPasses[i]->unbindProgram();
        }
        _updatedPasses.clear();
    }
    
protected:
    NvFxProgramManager() {}
    virtual ~NvFxProgramManager() {}
    
    std::vector<nvFX::IPass*> _updatedPasses;
};

class NvFxProgram : public osg::StateAttribute
{
public:
    NvFxProgram()
    :   _effect(NULL), _technique(NULL), _techniqueIndex(0), _initialized(false)
    {}
    
    NvFxProgram( const NvFxProgram& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
    :   osg::StateAttribute(copy, copyop),
        _effect(copy._effect), _technique(copy._technique),
        _effectFile(copy._effectFile), _techniqueIndex(copy._techniqueIndex),
        _initialized(copy._initialized)
    {}
    
    META_StateAttribute( osg, NvFxProgram, (osg::StateAttribute::Type)NvFxProgram_ID );
    
    void initialize( const std::string& name, const std::string& file )
    {
        nvFX::setErrorCallback( NvFxProgramManager::errorCallbackFunc );
        nvFX::setIncludeCallback( NvFxProgramManager::includeCallbackFunc );
        _effect = nvFX::IContainer::create( name.c_str() );
        if ( !file.empty() ) setEffectFile(file);
    }
    
    void setEffectFile( const std::string& file ) { _effectFile = file; }
    const std::string& getEffectFile() const { return _effectFile; }
    
    void setTechnique( unsigned int index )
    {
        _techniqueIndex = index;
        if ( _effect && _technique )
            _technique = _effect->findTechnique(index);
    }
    
    virtual int compare( const osg::StateAttribute& sa ) const
    {
        COMPARE_StateAttribute_Types(NvFxProgram, sa)
        COMPARE_StateAttribute_Parameter(_effect)
        COMPARE_StateAttribute_Parameter(_technique)
        COMPARE_StateAttribute_Parameter(_effectFile)
        COMPARE_StateAttribute_Parameter(_initialized)
        return 0;
    }
    
    virtual void apply(osg::State& state) const
    {
        if ( !_effect )
        {
            // Default attribute will unbind all programs
            NvFxProgramManager::instance()->unbindAllPasses();
        }
        
        if ( !_initialized )
        {
            // Initialize the effect
            bool loaded = nvFX::loadEffectFromFile( _effect, _effectFile.c_str() );
            NvFxProgram* nonconst = const_cast<NvFxProgram*>( this );
            if ( loaded )
            {
                for ( int t=0; nonconst->_technique=_effect->findTechnique(t); ++t )
                    nonconst->_technique->validate();
                nonconst->_technique = _effect->findTechnique(_techniqueIndex);
                
                // load the default resources that the effect might need
                nvFX::IResource* res = NULL;
                for ( int i=0; res=_effect->findResource(i); ++i )
                {
                    // TODO
                }
            }
            nonconst->_initialized = true;
        }
        
        if ( _technique )
        {
            nvFX::IPass* pass = _technique->getPass(0);
            pass->execute();
            NvFxProgramManager::instance()->passUpdated( pass );
        }
    }
    
protected:
    virtual ~NvFxProgram()
    {
        nvFX::IContainer::destroy( _effect );
    }
    
    nvFX::IContainer* _effect;
    nvFX::ITechnique* _technique;
    std::string _effectFile;
    int _techniqueIndex;
    bool _initialized;
};

int main( int argc, char** argv )
{
    osg::ref_ptr<NvFxProgram> fxProgram = new NvFxProgram;
    fxProgram->initialize( "nvfxProgram1", "simpleEffect.glslfx" );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("cow.osg") );
    scene->getOrCreateStateSet()->setAttribute( fxProgram.get() );
    
    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
    root->addChild( scene.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
	viewer.realize();
	return viewer.run();
}
