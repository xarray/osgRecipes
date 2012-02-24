/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 3 Recipe 6
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Geometry>
#include <osg/Geode>
#include <osgUtil/SmoothingVisitor>
#include <osgViewer/Viewer>

#include "CommonFunctions"

const osg::Vec4 normalColor(1.0f, 1.0f, 1.0f, 1.0f);
const osg::Vec4 selectedColor(1.0f, 0.0f, 0.0f, 0.5f);

class SelectModelHandler : public osgCookBook::PickHandler
{
public:
    SelectModelHandler() : _lastDrawable(0) {}
    
    virtual void doUserOperations( osgUtil::LineSegmentIntersector::Intersection& result )
    {
        if ( _lastDrawable.valid() )
        {
            setDrawableColor( _lastDrawable.get(), normalColor );
            _lastDrawable = NULL;
        }
        
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>( result.drawable.get() );
        if ( geom )
        {
            setDrawableColor( geom, selectedColor );
            _lastDrawable = geom;
        }
    }
    
    void setDrawableColor( osg::Geometry* geom, const osg::Vec4& color )
    {
        osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>( geom->getColorArray() );
        if ( colors && colors->size()>0 )
        {
            colors->front() = color;
            colors->dirty();
        }
    }
    
protected:
    osg::observer_ptr<osg::Geometry> _lastDrawable;
};

osg::Geometry* createSimpleGeometry()
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(8);
    (*vertices)[0].set(-0.5f,-0.5f,-0.5f);
    (*vertices)[1].set( 0.5f,-0.5f,-0.5f);
    (*vertices)[2].set( 0.5f, 0.5f,-0.5f);
    (*vertices)[3].set(-0.5f, 0.5f,-0.5f);
    (*vertices)[4].set(-0.5f,-0.5f, 0.5f);
    (*vertices)[5].set( 0.5f,-0.5f, 0.5f);
    (*vertices)[6].set( 0.5f, 0.5f, 0.5f);
    (*vertices)[7].set(-0.5f, 0.5f, 0.5f);
    
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0] = normalColor;
    
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_QUADS, 24);
    (*indices)[0] = 0; (*indices)[1] = 1; (*indices)[2] = 2; (*indices)[3] = 3;
    (*indices)[4] = 4; (*indices)[5] = 5; (*indices)[6] = 6; (*indices)[7] = 7;
    (*indices)[8] = 0; (*indices)[9] = 1; (*indices)[10]= 5; (*indices)[11]= 4;
    (*indices)[12]= 1; (*indices)[13]= 2; (*indices)[14]= 6; (*indices)[15]= 5;
    (*indices)[16]= 2; (*indices)[17]= 3; (*indices)[18]= 7; (*indices)[19]= 6;
    (*indices)[20]= 3; (*indices)[21]= 0; (*indices)[22]= 4; (*indices)[23]= 7;
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setDataVariance( osg::Object::DYNAMIC );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    geom->setVertexArray( vertices.get() );
    geom->setColorArray( colors.get() );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );
    geom->addPrimitiveSet( indices.get() );
    
    osgUtil::SmoothingVisitor::smooth( *geom );
    return geom.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( createSimpleGeometry() );
    geode->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new SelectModelHandler );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
