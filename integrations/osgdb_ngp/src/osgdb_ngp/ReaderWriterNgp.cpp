/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** ReaderWriterNgp.cpp
** Class to read in ngplant models and convert them to an osg::Geode
**
** Author: Brian Bailey www.code-hammer.com
** -------------------------------------------------------------------------*/

#include <osgDB/Registry>
#include <osgDB/ReaderWriter>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/Texture2D>
#include <osg/AlphaFunc>
#include <ngpcore/p3dhli.h>
#include "NgpFileName.h"

//============================================================================
//============================================================================
class NGPInputStream : public P3DInputStringStream
{
public:

	//============================================================================
	//============================================================================
    NGPInputStream( const std::string& filename ) 
	{ 
		_fileStream.open( filename.c_str() );  
	}
    
	//============================================================================
	//============================================================================
    virtual void ReadString( char* Buffer, unsigned int BufferSize )
	{
        _fileStream.getline( Buffer, BufferSize );
        if ( Buffer[_fileStream.gcount()-2]=='\r' )
            Buffer[_fileStream.gcount()-2] = '\0';
	}
    
	//============================================================================
	//============================================================================
    virtual bool Eof() const { return _fileStream.eof(); }

	//============================================================================
	//============================================================================
	virtual bool isOpen()
	{
		return _fileStream.is_open();
	}
    
protected:
    std::ifstream _fileStream;
};

//============================================================================
//============================================================================
class ReaderWriterNgp : public osgDB::ReaderWriter
{
public:

	//============================================================================
	//============================================================================
    ReaderWriterNgp()
    {
        supportsExtension( "ngp", "ngPlant" );
    }

	//============================================================================
	//============================================================================
	virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
	{
		std::string ext = osgDB::getLowerCaseFileExtension(file);
		if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

		NgpFileName fileName(file);

		std::string foundPath = osgDB::findDataFile( fileName.settings().filePath, options );
		if (foundPath.size() <= 0)
		{
			OSG_WARN << "Unable to find plant file: " << fileName.settings().filePath << std::endl;
			return NULL;
		}

		fileName.setFilePath(foundPath);
		osg::Geode *node = createPlant( fileName );
		return node;

	}

protected:

	//============================================================================
	//============================================================================
	osg::Geode* createPlant( const NgpFileName &ngpfile ) const
	{
		std::string filename = ngpfile.settings().filePath;
		std::string folder = osgDB::getFilePath(filename);

		NGPInputStream stream( filename );
		if (!stream.isOpen())
		{
			OSG_WARN << "Unable to open plant file: " << filename << std::endl;
			return NULL;
		}

		P3DHLIPlantTemplate plantTemplate( &stream );
		P3DHLIPlantInstance* plantInstance = plantTemplate.CreateInstance(ngpfile.settings().seed);
		unsigned int branchGroupCount = plantTemplate.GetGroupCount();
		if ( !branchGroupCount )
		{
			OSG_WARN << "Loaded empty plant, so ignore it." << std::endl;
			return NULL;
		}
    
		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		for ( unsigned int i=0; i<branchGroupCount; ++i )
		{
			unsigned int branchCount = plantInstance->GetBranchCount(i);
			unsigned int vertexCount = plantInstance->GetVAttrCountI(i);
			unsigned int indexCount = plantTemplate.GetIndexCount( i, P3D_TRIANGLE_LIST );
        
			// Read materials
			const P3DMaterialDef* material = plantTemplate.GetMaterial(i);
			float r, g, b; material->GetColor( &r, &g, &b );
        
			const char* diffuseName = material->GetTexName( P3D_TEX_DIFFUSE );
			bool hasTexture = (diffuseName!=NULL);
			std::string txfile;

			if (hasTexture)
			{
				txfile = osgDB::concatPaths(folder, diffuseName);
				if (!osgDB::fileExists(txfile))
				{
					OSG_WARN << "Texture file " << filename << " doesn't exist." << std::endl;
					hasTexture = false;
				}
			}

			if ( hasTexture )
			{
				const char* normalName = material->GetTexName( P3D_TEX_NORMAL_MAP );
				const char* aux0Name = material->GetTexName( P3D_TEX_AUX0 );
				const char* aux1Name = material->GetTexName( P3D_TEX_AUX1 );
				// TODO: how to handle these textures?
			}
        
			// Read vertex buffer
			float* vertexBuffer = new float[vertexCount * 3];
			float* normalBuffer = new float[vertexCount * 3];
			float* texcoodBuffer = NULL;
			P3DHLIVAttrBuffers attrBuffers;
			attrBuffers.AddAttr( P3D_ATTR_VERTEX, vertexBuffer, 0, sizeof(float)*3 );
			attrBuffers.AddAttr( P3D_ATTR_NORMAL, normalBuffer, 0, sizeof(float)*3 );
			if ( hasTexture )
			{
				texcoodBuffer = new float[vertexCount * 2];
				attrBuffers.AddAttr( P3D_ATTR_TEXCOORD0, texcoodBuffer, 0, sizeof(float)*2 );
			}
			plantInstance->FillVAttrBuffersI( &attrBuffers, i );
        
			// scale the verts if needed
			if (ngpfile.hasScale())
			{
				for (unsigned int i=0; i<vertexCount*3; i++)
				{
					vertexBuffer[i] *= ngpfile.settings().scale;
				}
			}
			// Read index buffer
			unsigned int indexCountAll = indexCount * branchCount;
			unsigned int* indexBuffer = new unsigned int[indexCountAll];
			unsigned int attrCount = plantTemplate.GetVAttrCountI(i);
			for ( unsigned int c=0; c<branchCount; ++c )
			{
				plantTemplate.FillIndexBuffer( &(indexBuffer[c*indexCount]), i,
												P3D_TRIANGLE_LIST, P3D_UNSIGNED_INT, c*attrCount );
			}
        
			// Write data to OSG geometry
			osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array(vertexCount);
			osg::ref_ptr<osg::Vec3Array> na = new osg::Vec3Array(vertexCount);
			osg::ref_ptr<osg::Vec2Array> ta = hasTexture ? new osg::Vec2Array(vertexCount) : NULL;
			for ( unsigned int n=0; n<vertexCount; ++n )
			{
				(*va)[n].set( *(vertexBuffer + 3*n), *(vertexBuffer + 3*n + 1), *(vertexBuffer + 3*n + 2) );
				(*na)[n].set( *(normalBuffer + 3*n), *(normalBuffer + 3*n + 1), *(normalBuffer + 3*n + 2) );
				if ( hasTexture ) (*ta)[n].set( *(texcoodBuffer + 2*n), *(texcoodBuffer + 2*n + 1) );
			}
        
			osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES, indexCountAll);
			for ( unsigned int n=0; n<indexCountAll; ++n )
				(*indices)[n] = indexBuffer[n];
        
			osg::ref_ptr<osg::Vec4Array> ca = new osg::Vec4Array(1);
			ca->front() = osg::Vec4(r, g, b, 1.0f);
        
			osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
			geode->addDrawable( geometry.get() );
        
			geometry->setVertexArray( va.get() );
			geometry->setTexCoordArray( 0, ta.get() );
			geometry->setNormalArray( na.get() );
			geometry->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
			geometry->setColorArray( ca.get() );
			geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
			geometry->addPrimitiveSet( indices.get() );
        
			osg::StateSet* ss = geometry->getOrCreateStateSet();
			if (ngpfile.settings().alphaMode == NgpFileName::Test)
			{
				osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
				alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, ngpfile.settings().alphaThreshold);
				ss->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
			}
			else
			{
				ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
				ss->setMode( GL_BLEND, osg::StateAttribute::ON );
			}

			if ( hasTexture )
			{
				osg::ref_ptr<osg::Texture2D> diffuseTex = new osg::Texture2D;
				diffuseTex->setImage( osgDB::readImageFile(txfile) );
				ss->setTextureAttributeAndModes( 0, diffuseTex.get() );
			}

			 if (vertexBuffer) delete []  vertexBuffer;
			 if (normalBuffer) delete []  normalBuffer;
			 if (texcoodBuffer) delete [] texcoodBuffer;
		}
		return geode.release();
	}
};

REGISTER_OSGPLUGIN( ngp, ReaderWriterNgp );