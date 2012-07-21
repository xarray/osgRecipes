/***************************************************************************

 The osgngplant example is under the following license.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. Neither the name of the author nor the names of contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

***************************************************************************/

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgUtil/SmoothingVisitor>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <iostream>
#include <fstream>

#include <ngpcore/p3dhli.h>

class NGPInputStream : public P3DInputStringStream
{
public:
    NGPInputStream( const std::string& filename )
    { _fileStream.open( filename.c_str() ); }
    
    virtual void ReadString( char* Buffer, unsigned int BufferSize )
    {
        _fileStream.getline( Buffer, BufferSize );
        if ( Buffer[_fileStream.gcount()-2]=='\r' )
            Buffer[_fileStream.gcount()-2] = '\0';
    }
    
    virtual bool Eof() const
    { return _fileStream.eof(); }
    
protected:
    std::ifstream _fileStream;
};

osg::Geode* createPlant( const std::string& filename, unsigned int seed=0 )
{
    NGPInputStream stream( filename );
    P3DHLIPlantTemplate plantTemplate( &stream );
    P3DHLIPlantInstance* plantInstance = plantTemplate.CreateInstance(seed);
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
        ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        ss->setMode( GL_BLEND, osg::StateAttribute::ON );
        if ( hasTexture )
        {
            osg::ref_ptr<osg::Texture2D> diffuseTex = new osg::Texture2D;
            diffuseTex->setImage( osgDB::readImageFile(diffuseName) );
            ss->setTextureAttributeAndModes( 0, diffuseTex.get() );
        }
    }
    return geode.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    std::string plantName("data/palm.ngp");
    unsigned int seed = 1234;
    arguments.read( "--plant", plantName );
    arguments.read( "--seed", seed );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( createPlant(plantName, seed) );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
