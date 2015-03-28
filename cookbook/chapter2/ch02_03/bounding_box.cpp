/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 2 Recipe 3
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/ComputeBoundsVisitor>
#include <osg/ShapeDrawable>
#include <osg/AnimationPath>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

class BoundingBoxCallback : public osg::NodeCallback
{
public:
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osg::BoundingBox bb;
        for ( unsigned int i=0; i<_nodesToCompute.size(); ++i )
        {
            osg::Node* node = _nodesToCompute[i];
            osg::ComputeBoundsVisitor cbbv;
            node->accept( cbbv );
            
            osg::BoundingBox localBB = cbbv.getBoundingBox();
            osg::Matrix localToWorld = osg::computeLocalToWorld( node->getParent(0)->getParentalNodePaths()[0] );
            for ( unsigned int i=0; i<8; ++i )
                bb.expandBy( localBB.corner(i) * localToWorld );
        }
        
        osg::MatrixTransform* trans = static_cast<osg::MatrixTransform*>(node);
        trans->setMatrix(
            osg::Matrix::scale(bb.xMax()-bb.xMin(), bb.yMax()-bb.yMin(), bb.zMax()-bb.zMin()) *
            osg::Matrix::translate(bb.center()) );

        traverse(node, nv);
    }
    
    osg::NodePath _nodesToCompute;
};

osg::AnimationPath* createAnimationPath( float radius, float time )
{
    osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
    path->setLoopMode( osg::AnimationPath::LOOP );
    
    unsigned int numSamples = 32;
    float delta_yaw = 2.0f * osg::PI/((float)numSamples - 1.0f);
    float delta_time = time / (float)numSamples;
    for ( unsigned int i=0; i<numSamples; ++i )
    {
        float yaw = delta_yaw * (float)i;
        osg::Vec3 pos( sinf(yaw)*radius, cosf(yaw)*radius, 0.0f );
        osg::Quat rot( -yaw, osg::Z_AXIS );
        path->insert( delta_time * (float)i, osg::AnimationPath::ControlPoint(pos, rot) );
    }
    return path.release();    
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> cessna = new osg::MatrixTransform;
    cessna->addChild( osgDB::readNodeFile("cessna.osg.0,0,90.rot") );
    
    osg::ref_ptr<osg::AnimationPathCallback> apcb = new osg::AnimationPathCallback;
    apcb->setAnimationPath( createAnimationPath(50.0f, 6.0f) );
    cessna->addUpdateCallback( apcb.get() );
    
    osg::ref_ptr<osg::MatrixTransform> dumptruck = new osg::MatrixTransform;
    dumptruck->addChild( osgDB::readNodeFile("dumptruck.osg") );
    dumptruck->setMatrix( osg::Matrix::translate(0.0f, 0.0f, -100.0f) );
    
    osg::ref_ptr<osg::MatrixTransform> models = new osg::MatrixTransform;
    models->addChild( cessna.get() );
    models->addChild( dumptruck.get() );
    models->setMatrix( osg::Matrix::translate(0.0f, 0.0f, 200.0f) );
    
    // World bounding box callback & node
    osg::ref_ptr<BoundingBoxCallback> bbcb = new BoundingBoxCallback;
    bbcb->_nodesToCompute.push_back( cessna.get() );
    bbcb->_nodesToCompute.push_back( dumptruck.get() );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( new osg::ShapeDrawable(new osg::Box) );
    
    osg::ref_ptr<osg::MatrixTransform> boundingBoxNode = new osg::MatrixTransform;
    boundingBoxNode->addChild( geode.get() );
    boundingBoxNode->addUpdateCallback( bbcb.get() );
    boundingBoxNode->getOrCreateStateSet()->setAttributeAndModes(
        new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE) );
    boundingBoxNode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    
    // Build the scene
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( models.get() );
    root->addChild( osgDB::readNodeFile("lz.osg") );
    root->addChild( boundingBoxNode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );

    return viewer.run();
}
