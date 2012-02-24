/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 3 Recipe 5
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/AnimationPath>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

const unsigned int g_numPoints = 400;
const float g_halfWidth = 4.0f;

osg::Geometry* createRibbon( const osg::Vec3& colorRGB )
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(g_numPoints);
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(g_numPoints);
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(g_numPoints);
    
    osg::Vec3 origin = osg::Vec3(0.0f, 0.0f, 0.0f);
    osg::Vec3 normal = osg::Vec3(0.0f, 0.0f, 1.0f);
    for ( unsigned int i=0; i<g_numPoints-1; i+=2 )
    {
        (*vertices)[i] = origin; (*vertices)[i+1] = origin;
        (*normals)[i] = normal; (*normals)[i+1] = normal;
        
        float alpha = sinf(osg::PI * (float)i / (float)g_numPoints);
        (*colors)[i] = osg::Vec4(colorRGB, alpha);
        (*colors)[i+1] = osg::Vec4(colorRGB, alpha);
    }
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setDataVariance( osg::Object::DYNAMIC );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    
    geom->setVertexArray( vertices.get() );
    geom->setNormalArray( normals.get() );
    geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
    geom->setColorArray( colors.get() );
    geom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_QUAD_STRIP, 0, g_numPoints) );
    return geom.release();
}

class TrailerCallback : public osg::NodeCallback
{
public:
    TrailerCallback( osg::Geometry* ribbon ) : _ribbon(ribbon) {}
    
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osg::MatrixTransform* trans = static_cast<osg::MatrixTransform*>(node);
        if ( trans && _ribbon.valid() )
        {
            osg::Matrix matrix = trans->getMatrix();
            osg::Vec3Array* vertices = static_cast<osg::Vec3Array*>( _ribbon->getVertexArray() );
            osg::Vec3Array* normals = static_cast<osg::Vec3Array*>( _ribbon->getNormalArray() );
            
            for ( unsigned int i=0; i<g_numPoints-3; i+=2 )
            {
                (*vertices)[i] = (*vertices)[i+2];
                (*vertices)[i+1] = (*vertices)[i+3];
                (*normals)[i] = (*normals)[i+2];
                (*normals)[i+1] = (*normals)[i+3];
            }
            
            (*vertices)[g_numPoints-2] = osg::Vec3(0.0f,-g_halfWidth, 0.0f) * matrix;
            (*vertices)[g_numPoints-1] = osg::Vec3(0.0f, g_halfWidth, 0.0f) * matrix;
            vertices->dirty();
            
            osg::Vec3 normal = osg::Vec3(0.0f, 0.0f, 1.0f) * matrix;
            normal.normalize();
            (*normals)[g_numPoints-2] = normal;
            (*normals)[g_numPoints-1] = normal;
            normals->dirty();
            
            _ribbon->dirtyBound();
        }
        traverse( node, nv );
    }
    
protected:
    osg::observer_ptr<osg::Geometry> _ribbon;
};

int main( int argc, char** argv )
{
    osg::Geometry* geometry = createRibbon( osg::Vec3(1.0f, 0.0f, 1.0f) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( geometry );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    geode->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::MatrixTransform> cessna = new osg::MatrixTransform;
    cessna->addChild( osgDB::readNodeFile("cessna.osg.0,0,90.rot") );
    cessna->addUpdateCallback( osgCookBook::createAnimationPathCallback(50.0f, 6.0f) );
    cessna->addUpdateCallback( new TrailerCallback(geometry) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( geode.get() );
    root->addChild( cessna.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
