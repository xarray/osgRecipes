/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 8 Recipe 8
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Point>
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <fstream>
#include <iostream>

#include "CommonFunctions"

const char* vertCode = {
    "uniform sampler2D defaultTex;\n"
    "uniform int width;\n"
    "uniform int height;\n"
    "varying float brightness;\n"
    "void main()\n"
    "{\n"
    "    float r = float(gl_InstanceID) / float(width);\n"
    "    vec2 uv = vec2(fract(r), floor(r) / float(height));\n"
    "    vec4 texValue = texture2D(defaultTex, uv);\n"
    "    vec4 pos = gl_Vertex + vec4(texValue.xyz, 1.0);\n"
    "    brightness = texValue.a;\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * pos;\n"
    "}\n"
};

const char* fragCode = {
    "varying float brightness;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(brightness, brightness, brightness, 1.0);\n"
    "}\n"
};

osg::Geometry* createInstancedGeometry( osg::Image* img, unsigned int numInstances )
{
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    geom->setVertexArray( new osg::Vec3Array(1) );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_POINTS, 0, 1, numInstances) );
    
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( img );
    texture->setInternalFormat( GL_RGBA32F_ARB );
    texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    geom->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    geom->getOrCreateStateSet()->addUniform( new osg::Uniform("defaultTex", 0) );
    geom->getOrCreateStateSet()->addUniform( new osg::Uniform("width", (int)img->s()) );
    geom->getOrCreateStateSet()->addUniform( new osg::Uniform("height", (int)img->t()) );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, vertCode) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, fragCode) );
    geom->getOrCreateStateSet()->setAttributeAndModes( program.get() );
    return geom.release();
}

osg::Geometry* readPointData( const std::string& file, unsigned int w, unsigned int h )
{
    std::ifstream is( file.c_str() );
    if ( !is ) return NULL;
    
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage( w, h, 1, GL_RGBA, GL_FLOAT );
    
    unsigned int density, brightness;
    osg::BoundingBox boundBox;
    float* data = (float*)image->data();
    while ( !is.eof() )
    {
        osg::Vec3 pos;
        is >> pos[0] >> pos[1] >> pos[2] >> density >> brightness;
        boundBox.expandBy( pos );
        
        *(data++) = pos[0];
        *(data++) = pos[1];
        *(data++) = pos[2];
        *(data++) = brightness / 255.0;
    }
    
    osg::ref_ptr<osg::Geometry> geom = createInstancedGeometry( image.get(), w*h );
    geom->setInitialBound( boundBox );
    geom->getOrCreateStateSet()->setAttributeAndModes( new osg::Point(5.0f) );
    return geom.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( readPointData("data.txt", 512, 512) );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
