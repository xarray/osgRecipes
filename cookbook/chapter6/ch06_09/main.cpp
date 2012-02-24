/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 9
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <fstream>
#include <iostream>

#include "CommonFunctions"
#include "CloudBlock"

osg::Image* makeGlow( int width, int height, float expose, float sizeDisc )
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage( width, height, 1, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE );
    
	unsigned char* data = image->data();
	for ( int y=0; y<height; ++y )
	{
		float dy = (y+0.5f) / height - 0.5f;
		for ( int x=0; x<width; ++x )
		{
			float dx = (x+0.5f) / width - 0.5f;
			float dist = sqrtf( dx*dx + dy*dy );
			float intensity = 2 - osg::minimum( 2.0f, powf(2.0f, osg::maximum(dist-sizeDisc, 0.0f) * expose) );
			float noise = 1.0f;
			
			unsigned char color = (unsigned char)(noise * intensity * 255.0f + 0.5f);
			*(data++) = color;
			*(data++) = color;
		}
	}
	return image.release();
}

void readCloudCells( CloudBlock::CloudCells& cells, const std::string& file )
{
    std::ifstream is( file.c_str() );
    if ( !is ) return;
    
    double x, y, z;
    unsigned int density, brightness;
    while ( !is.eof() )
    {
        is >> x >> y >> z >> density >> brightness;
        
        CloudBlock::CloudCell cell;
        cell._pos.set( x, y, z );
        cell._density = (float)density;
        cell._brightness = (float)brightness;
        cells.push_back( cell );
    }
}

int main( int argc, char** argv )
{
    CloudBlock::CloudCells cells;
    readCloudCells( cells, "data.txt" );
    
    osg::ref_ptr<CloudBlock> clouds = new CloudBlock;
    clouds->setCloudCells( cells );
    
    osg::StateSet* ss = clouds->getOrCreateStateSet();
    ss->setAttributeAndModes( new osg::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA) );
    ss->setAttributeAndModes( new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false) );
    ss->setTextureAttributeAndModes( 0, new osg::Texture2D(makeGlow(32, 32, 2.0f, 0.0f)) );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( clouds.get() );
    
    osgViewer::Viewer viewer;
    viewer.setLightingMode( osg::View::SKY_LIGHT );
    viewer.setSceneData( geode.get() );
    return viewer.run();
}
