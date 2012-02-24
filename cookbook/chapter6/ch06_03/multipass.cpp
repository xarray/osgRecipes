/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 3
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/BlendFunc>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/Material>
#include <osgDB/ReadFile>
#include <osgFX/Effect>
#include <osgViewer/Viewer>

#include "CommonFunctions"

class TransparencyTechnique : public osgFX::Technique
{
public:
    TransparencyTechnique() : osgFX::Technique() {}
    virtual bool validate( osg::State& ss ) const { return true; }
    
protected:
    virtual void define_passes()
    {
        osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
        ss->setAttributeAndModes( new osg::ColorMask(false, false, false, false) );
        ss->setAttributeAndModes( new osg::Depth(osg::Depth::LESS) );
        addPass( ss.get() );
        
        ss = new osg::StateSet;
        ss->setAttributeAndModes( new osg::ColorMask(true, true, true, true) );
        ss->setAttributeAndModes( new osg::Depth(osg::Depth::EQUAL) );
        addPass( ss.get() );
    }
};

class TransparencyNode : public osgFX::Effect
{
public:
    TransparencyNode() : osgFX::Effect() {}
    TransparencyNode( const TransparencyNode& copy, const osg::CopyOp op=osg::CopyOp::SHALLOW_COPY )
    :   osgFX::Effect(copy, op) {}
    
    META_Effect( osgFX, TransparencyNode, "TransparencyNode", "", "" );
    
protected:
    virtual bool define_techniques()
    { addTechnique(new TransparencyTechnique); return true; }
};

int main( int argc, char** argv )
{
    osg::Node* loadedModel = osgDB::readNodeFile( "cessna.osg" );
    
    osg::ref_ptr<osg::Material> material = new osg::Material;
    material->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    material->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.5f) );
    loadedModel->getOrCreateStateSet()->setAttributeAndModes(
        material.get(), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
    loadedModel->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc );
    loadedModel->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<TransparencyNode> fxNode = new TransparencyNode;
    fxNode->addChild( loadedModel );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( fxNode.get() );
    return viewer.run();
}
