/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 5
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/Node>
#include <osgAnimation/EaseMotion>
#include <osgDB/ReadFile>
#include <osgUtil/CullVisitor>
#include <osgViewer/Viewer>

#include "CommonFunctions"

class FadeInOutCallback : public osg::NodeCallback
{
public:
    FadeInOutCallback( osg::Material* mat )
    :   _material(mat), _lastDistance(-1.0f), _fadingState(0)
    { _motion = new osgAnimation::InOutCubicMotion; }
    
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        if ( _fadingState!=0 )
        {
            _motion->update( 0.05 );
            float value = _motion->getValue();
            float alpha = (_fadingState>0 ? value : 1.0f - value);
            _material->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, alpha) );
            
            if ( value>=1.0f ) _fadingState = 0;
            traverse( node, nv ); return;
        }
        
        osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>( nv );
        if ( cv )
        {
            float distance = cv->getDistanceFromEyePoint( node->getBound().center(), true );
            if ( _lastDistance>0.0f )
            {
                if ( _lastDistance>200.0f && distance<=200.0f )
                {
                    _fadingState = 1;
                    _motion->reset();
                }
                else if ( _lastDistance<200.0f && distance>=200.0f )
                {
                    _fadingState = -1;
                    _motion->reset();
                }
            }
            _lastDistance = distance;
        }
        traverse( node, nv );
    }
    
protected:
    osg::ref_ptr<osgAnimation::InOutCubicMotion> _motion;
    osg::observer_ptr<osg::Material> _material;
    float _lastDistance;
    int _fadingState;
};

int main( int argc, char** argv )
{
    osg::Node* loadedModel = osgDB::readNodeFile( "cessna.osg" );
    if ( !loadedModel ) return 1;
    
    osg::ref_ptr<osg::Material> material = new osg::Material;
    material->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    material->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
    loadedModel->getOrCreateStateSet()->setAttributeAndModes(
        material.get(), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );
    loadedModel->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc );
    loadedModel->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    loadedModel->addCullCallback( new FadeInOutCallback(material.get()) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( loadedModel );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
