/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 3 Recipe 9
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

const char* vertCode = {
    "uniform sampler2D defaultTex;\n"
    "varying float height;\n"
    "void main()\n"
    "{\n"
    "    vec2 uv = gl_MultiTexCoord0.xy;\n"
    "    vec4 color = texture2D(defaultTex, uv);\n"
    "    height = 0.3*color.x + 0.59*color.y + 0.11*color.z;\n"
    
    "    vec4 pos = gl_Vertex;\n"
    "    pos.z = pos.z + 100.0*height;\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * pos;\n"
    "}\n"
};

const char* fragCode = {
    "varying float height;\n"
    "const vec4 lowerColor = vec4(0.1, 0.1, 0.1, 1.0);\n"
    "const vec4 higherColor = vec4(0.2, 1.0, 0.2, 1.0);\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = mix(lowerColor, higherColor, height);\n"
    "}\n"
};

osg::Geometry* createGridGeometry( unsigned int column, unsigned int row )
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(column * row);
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(column * row);
    for ( unsigned int i=0; i<row; ++i )
    {
        for ( unsigned int j=0; j<column; ++j )
        {
            (*vertices)[i*column + j].set( (float)i, (float)j, 0.0f );
            (*texcoords)[i*column + j].set( (float)i/(float)row, (float)j/(float)column );
        }
    }
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    geom->setVertexArray( vertices.get() );
    geom->setTexCoordArray( 0, texcoords.get() );
    for ( unsigned int i=0; i<row-1; ++i )
    {
        osg::ref_ptr<osg::DrawElementsUInt> de = new osg::DrawElementsUInt(GL_QUAD_STRIP, column*2);
        for ( unsigned int j=0; j<column; ++j )
        {
            (*de)[j*2 + 0] = i*column + j;
            (*de)[j*2 + 1] = (i+1)*column + j;
        }
        geom->addPrimitiveSet( de.get() );
    }
    geom->setInitialBound( osg::BoundingBox(-1.0f,-1.0f,-100.0f, 1.0f, 1.0f, 100.0f) );
    
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( osgDB::readImageFile("Images/osg256.png") );
    texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    geom->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    geom->getOrCreateStateSet()->addUniform( new osg::Uniform("defaultTex", 0) );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, vertCode) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, fragCode) );
    geom->getOrCreateStateSet()->setAttributeAndModes( program.get() );
    return geom.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( createGridGeometry(512, 512) );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
