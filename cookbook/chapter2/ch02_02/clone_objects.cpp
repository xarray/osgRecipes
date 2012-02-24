/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 2 Recipe 2
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>

#include "CommonFunctions"

class SetShapeColorHandler : public osgCookBook::PickHandler
{
    virtual void doUserOperations( osgUtil::LineSegmentIntersector::Intersection& result )
    {
        osg::ShapeDrawable* shape = dynamic_cast<osg::ShapeDrawable*>( result.drawable.get() );
        if ( shape ) shape->setColor( osg::Vec4(1.0f, 1.0f, 1.0f, 2.0f) - shape->getColor() );
    }
};

osg::Node* createMatrixTransform( osg::Geode* geode, const osg::Vec3& pos )
{
    osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
    trans->setMatrix( osg::Matrix::translate(pos) );
    trans->addChild( geode );
    return trans.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable( new osg::Sphere );
    shape->setColor( osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f) );
    shape->setDataVariance( osg::Object::DYNAMIC );
    shape->setUseDisplayList( false );
    
    osg::ref_ptr<osg::Geode> geode1 = new osg::Geode;
    geode1->addDrawable( shape.get() );
    
    osg::ref_ptr<osg::Geode> geode2 = dynamic_cast<osg::Geode*>( geode1->clone(osg::CopyOp::SHALLOW_COPY) );
    osg::ref_ptr<osg::Geode> geode3 = dynamic_cast<osg::Geode*>( geode1->clone(osg::CopyOp::DEEP_COPY_ALL) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( createMatrixTransform(geode1.get(), osg::Vec3(0.0f, 0.0f, 0.0f)) );
    root->addChild( createMatrixTransform(geode2.get(), osg::Vec3(-2.0f, 0.0f, 0.0f)) );
    root->addChild( createMatrixTransform(geode3.get(), osg::Vec3(2.0f, 0.0f, 0.0f)) );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new SetShapeColorHandler );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
