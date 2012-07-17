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
    P3DHLIPlantTemplate plantTemplate(stream);
    P3DHLIPlantInstance* plantInstance = plantTemplate.CreateInstance(seed);
    unsigned int branchGroupCount = plantTemplate.GetGroupCount();
    if ( !branchGroupCount )
    {
        OSG_WARN << "Loaded empty plant, so ignore it." << std::end;
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
        bool isTransparent = material->IsTransparent();
        
        const char* diffuseName = material->GetTexName( P3D_TEX_DIFFUSE );
        bool hasTexture = (diffuseName==NULL);
        if ( hasTexture )
        {
            const char* normalName = material->GetTexName( P3D_TEX_NORMAL_MAP );
            const char* aux0Name = material->GetTexName( P3D_TEX_AUX0 );
            const char* aux1Name = material->GetTexName( P3D_TEX_AUX1 );
            // TODO
        }
        
        // Read vertex buffer
        float* vertexBuffer = new float[vertexCount * 3];
        float* normalBuffer = new float[vertexCount * 3];
        P3DHLIVAttrBuffers attrBuffers;
        attrBuffers.AddAttr( P3D_ATTR_VERTEX, vertexBuffer, 0, sizeof(float)*3 );
        attrBuffers.AddAttr( P3D_ATTR_NORMAL, normalBuffer, 0, sizeof(float)*3 );
        if ( hasTexture )
        {
            float* texcoodBuffer = new float[vertexCount * 2];
            attrBuffers.AddAttr( P3D_ATTR_TEXCOORD0, texcoodBuffer, 0, sizeof(float)*2 );
        }
        plantInstance->FillVAttrBuffersI( &attrBuffers, i );
        
        // Read index buffer
        unsigned int* indexBuffer = new unsigned int[indexCount * branchCount];
        unsigned int attrCount = plantTemplate->GetVAttrCountI(i);
        for ( unsigned int c=0; c<branchCount; ++c )
        {
            plantTemplate->FillIndexBuffer( &(indexBuffer[c*indexCount]), i,
                                            P3D_TRIANGLE_LIST, P3D_UNSIGNED_INT, c*attrCount );
        }
    }
    return geode.release();
}

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
