#include <assimp/ai_assert.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

#include <osg/PolygonMode>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

//Adapted from osgEarth/VPB
static std::string getFullPath(const std::string& relativeTo, const std::string &relativePath)
{
    if ( osgDB::isAbsolutePath(relativePath) || relativeTo.empty() )
    {
        return relativePath;
    }

    //If they didn't specify a relative path, just return the relativeTo
    if ( relativePath.empty() ) return relativeTo;
    
    //Note:  Modified from VPB

    //Concatinate the paths together
    std::string filename;
    if ( !osgDB::containsServerAddress( relativeTo ) )
        filename = osgDB::concatPaths( osgDB::getFilePath( osgDB::getRealPath( relativeTo )), relativePath );
    else
        filename = osgDB::concatPaths( osgDB::getFilePath( relativeTo ), relativePath);

    std::list<std::string> directories;
    int start = 0;
    for ( unsigned int i = 0; i < filename.size(); ++i )
    {
        if ( filename[i] == '\\' || filename[i] == '/' )
        {
            //Get the current directory
            std::string dir = filename.substr(start, i-start);
            if (dir != "..")
            {
                if (dir != ".")
                {
                    directories.push_back(dir);
                }
            }
            else if (!directories.empty())
            {
                directories.pop_back();
            }
            start = i + 1;
        }
    }

    std::string path;
    for ( std::list<std::string>::iterator itr = directories.begin();
            itr != directories.end(); ++itr )
    {
        path += *itr;
        path += "/";
    }
    path += filename.substr(start, std::string::npos);
    return path;
}

class ReaderWriterAssImp : public osgDB::ReaderWriter
{
    typedef std::map<std::string, osg::ref_ptr<osg::Texture> > TextureMap;
public:
    ReaderWriterAssImp()
    {
        Assimp::DefaultLogger::create( "", Assimp::Logger::NORMAL, aiDefaultLogStream_STDOUT );
        supportsExtension( "assimp", "AssImp reader/writer" );
        supportsExtension( "dae", "Collada" );
        supportsExtension( "blend", "Blender 3D" );
        supportsExtension( "3ds", "3ds Max 3DS" ); supportsExtension( "ase", "3ds Max ASE" );
        supportsExtension( "obj", "Wavefront Object" );
        supportsExtension( "xgl", "XGL" );
        supportsExtension( "ply", "Stanford Polygon Library" );
        supportsExtension( "dxf", "AutoCAD DXF" );
        supportsExtension( "lwo", "LightWave" ); supportsExtension( "lws", "LightWave Scene" );
        supportsExtension( "lxo", "Modo" );
        supportsExtension( "stl", "Stereolithography" );
        supportsExtension( "x", "DirectX X" );
        supportsExtension( "ac", "AC3D" );
        supportsExtension( "ms3d", "Milkshape 3D" );
        supportsExtension( "scn", "TrueSpace" );
        supportsExtension( "xml", "Ogre XML" );
        supportsExtension( "irrmesh", "Irrlicht Mesh" ); supportsExtension( "irr", "Irrlicht Scene" );
        supportsExtension( "mdl", "Quake I" ); supportsExtension( "md2", "Quake II" );
        supportsExtension( "md3", "Quake III Mesh" ); supportsExtension( "pk3", "Quake III Map/BSP" );
        supportsExtension( "md5", "Doom 3" );
        supportsExtension( "smd", "Valve Model" );
        supportsExtension( "m3", "Starcraft II M3" );
        supportsExtension( "3d", "Unreal" );
        supportsExtension( "q3d", "Quick3D" );
        supportsExtension( "off", "Object File Format" );
        supportsExtension( "ter", "Terragen Terrain" );
    }
    
    virtual ~ReaderWriterAssImp()
    {
        Assimp::DefaultLogger::kill();
    }
    
    virtual ReadResult readNode( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( ext=="assimp" ) return readNode(osgDB::getNameLessExtension(file), options);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        std::string fileName = osgDB::findDataFile( file, options );
        if ( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;
        
        Assimp::Importer importer;
        const aiScene* aiScene = importer.ReadFile( fileName.c_str(), aiProcessPreset_TargetRealtime_Quality );
        if ( !aiScene )
        {
            OSG_WARN << "ReaderWriterAssImp:: fail to load " + file << ", because of "
                     << importer.GetErrorString() << std::endl;
            return ReadResult::ERROR_IN_READING_FILE;
        }
        
        // Read scene nodes recursively
        TextureMap textures;
        osg::Node* root = traverseAIScene( fileName, aiScene, aiScene->mRootNode, textures, options );
        return root;
    }
    
    virtual WriteResult writeNode( const osg::Node& node, const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if ( ext=="assimp" ) return writeNode(node, osgDB::getNameLessExtension(file), options);
        
        // TODO
        return WriteResult::FILE_NOT_HANDLED;
    }
    
protected:
    osg::Node* traverseAIScene( const std::string& filename, const struct aiScene* aiScene, const struct aiNode* aiNode,
                                TextureMap& textures, const osgDB::Options* options ) const
    {
        osg::Geode* geode = new osg::Geode;
        for ( unsigned int n=0; n<aiNode->mNumMeshes; ++n )
        {
            // Create geometry basic properties
            const struct aiMesh* mesh = aiScene->mMeshes[ aiNode->mMeshes[n] ];
            osg::Geometry* geom = new osg::Geometry;
            geode->addDrawable( geom );
            
            osg::Vec3Array* va = new osg::Vec3Array(mesh->mNumVertices);
            osg::Vec3Array* na = (mesh->mNormals ? new osg::Vec3Array(mesh->mNumVertices) : NULL);
            osg::Vec4Array* ca = (mesh->mColors[0] ? new osg::Vec4Array(mesh->mNumVertices) : NULL);
            for ( unsigned int i=0; i<mesh->mNumVertices; ++i )
            {
                const aiVector3D& v = mesh->mVertices[i];
                (*va)[i].set( v.x, v.y, v.z );
                if ( na )
                {
                    const aiVector3D& n = mesh->mNormals[i];
                    (*na)[i].set( n.x, n.y, n.z );
                }
                if ( ca )
                {
                    const aiColor4D& c = mesh->mColors[0][i];
                    (*ca)[i].set( c.r, c.g, c.b, c.a );
                }
            }
            
            geom->setVertexArray( va );
            if ( na )
            {
                geom->setNormalArray( na );
                geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
            }
            if ( ca )
            {
                geom->setColorArray( ca );
                geom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
            }
            
            // Create geometry texture coordinates
            unsigned int unit = 0;
            const aiVector3D* aiTexCoords = mesh->mTextureCoords[unit];
            while ( aiTexCoords!=NULL )
            {
                switch ( mesh->mNumUVComponents[unit] )
                {
                case 1:
                    {
                        osg::FloatArray* ta = new osg::FloatArray(mesh->mNumVertices);
                        for ( unsigned int i=0; i<mesh->mNumVertices; ++i )
                            (*ta)[i] = aiTexCoords[i].x;
                        geom->setTexCoordArray( unit, ta );
                    }
                    break;
                case 2:
                    {
                        osg::Vec2Array* ta = new osg::Vec2Array(mesh->mNumVertices);
                        for ( unsigned int i=0; i<mesh->mNumVertices; ++i )
                        {
                            const aiVector3D& t = aiTexCoords[i];
                            (*ta)[i].set( t.x, t.y );
                        }
                        geom->setTexCoordArray( unit, ta );
                    }
                    break;
                case 3:
                    {
                        osg::Vec3Array* ta = new osg::Vec3Array(mesh->mNumVertices);
                        for ( unsigned int i=0; i<mesh->mNumVertices; ++i )
                        {
                            const aiVector3D& t = aiTexCoords[i];
                            (*ta)[i].set( t.x, t.y, t.z );
                        }
                        geom->setTexCoordArray( unit, ta );
                    }
                    break;
                }
                aiTexCoords = mesh->mTextureCoords[++unit];
            }
            
            // Create geometry primitives
            osg::ref_ptr<osg::DrawElementsUInt> de[5];
            de[1] = new osg::DrawElementsUInt(GL_POINTS);
            de[2] = new osg::DrawElementsUInt(GL_LINES);
            de[3] = new osg::DrawElementsUInt(GL_TRIANGLES);
            de[4] = new osg::DrawElementsUInt(GL_QUADS);
            de[0] = new osg::DrawElementsUInt(GL_POLYGON);
            
            osg::DrawElementsUInt* current = NULL;
            for ( unsigned int f=0; f<mesh->mNumFaces; ++f )
            {
                const struct aiFace& face = mesh->mFaces[f];
                if ( face.mNumIndices>4 ) current = de[0].get();
                else current = de[face.mNumIndices].get();
                
                for ( unsigned i=0; i<face.mNumIndices; ++i )
                    current->push_back( face.mIndices[i] );
            }
            
            for ( unsigned int i=0; i<5; ++i )
            {
                if ( de[i]->size()>0 )
                    geom->addPrimitiveSet( de[i].get() );
            }
            
            // Create textures
            osg::StateSet* ss = geom->getOrCreateStateSet();
            const aiMaterial* aiMtl = aiScene->mMaterials[mesh->mMaterialIndex];
            aiReturn texFound = AI_SUCCESS;
            aiTextureOp envOp = aiTextureOp_Multiply;
            aiTextureMapMode wrapMode[3] = {aiTextureMapMode_Clamp};
            unsigned int texIndex = 0;
            aiString path;
            
            while ( texFound==AI_SUCCESS )
            {
                texFound = aiMtl->GetTexture(
                    aiTextureType_DIFFUSE, texIndex++, &path, NULL, &unit, NULL, &envOp, &(wrapMode[0]) );
                if ( unit>0 ) unit--;  // The output UV seems to start at 1?
                if ( texFound!=AI_SUCCESS ) break;
                
                std::string texFile(path.data);
                if ( !osgDB::isAbsolutePath(texFile) ) texFile = getFullPath( filename, texFile );
                
                TextureMap::iterator itr = textures.find(texFile);
                if ( itr==textures.end() )
                {
                    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
                    tex2D->setWrap( osg::Texture::WRAP_S, getWrapMode(wrapMode[0]) );
                    tex2D->setWrap( osg::Texture::WRAP_T, getWrapMode(wrapMode[1]) );
                    tex2D->setWrap( osg::Texture::WRAP_R, getWrapMode(wrapMode[2]) );
                    tex2D->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
                    tex2D->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
                    tex2D->setImage( osgDB::readImageFile(texFile, options) );
                    textures[texFile] = tex2D;
                }
                
                ss->setTextureAttributeAndModes( unit, textures[texFile].get() );
                if ( unit>0 ) ss->setTextureAttributeAndModes( unit, new osg::TexEnv(getEnvMode(envOp)) );
            }
            
            // Create materials
            createMaterialData( ss, aiMtl );
        }
        
        aiMatrix4x4 m = aiNode->mTransformation;
        m.Transpose();
        
        // Create the node and continue looking for children
        osg::ref_ptr<osg::MatrixTransform> mt;
        mt = new osg::MatrixTransform;
        mt->setMatrix( osg::Matrixf((float*)&m) );
        for ( unsigned int n=0; n<aiNode->mNumChildren; ++n )
        {
            osg::Node* child = traverseAIScene( filename, aiScene, aiNode->mChildren[n], textures, options );
            if ( child ) mt->addChild( child );
        }
        mt->addChild( geode );
        return mt.release();
    }
    
    void createMaterialData( osg::StateSet* ss, const aiMaterial* aiMtl ) const
    {
        aiColor4D c;
        osg::Material* material = new osg::Material;
        if ( aiGetMaterialColor(aiMtl, AI_MATKEY_COLOR_AMBIENT, &c)==AI_SUCCESS )
            material->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4(c.r, c.g, c.b, c.a) );
        if ( aiGetMaterialColor(aiMtl, AI_MATKEY_COLOR_DIFFUSE, &c)==AI_SUCCESS )
            material->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4(c.r, c.g, c.b, c.a) );
        if ( aiGetMaterialColor(aiMtl, AI_MATKEY_COLOR_SPECULAR, &c)==AI_SUCCESS )
            material->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4(c.r, c.g, c.b, c.a) );
        if ( aiGetMaterialColor(aiMtl, AI_MATKEY_COLOR_EMISSIVE, &c)==AI_SUCCESS )
            material->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4(c.r, c.g, c.b, c.a) );
        
        unsigned int maxValue = 1;
        float shininess = 0.0f, strength = 1.0f;
        if ( aiGetMaterialFloatArray(aiMtl, AI_MATKEY_SHININESS, &shininess, &maxValue)==AI_SUCCESS )
        {
            maxValue = 1;
            if ( aiGetMaterialFloatArray( aiMtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &maxValue )==AI_SUCCESS )
                shininess *= strength;
            material->setShininess( osg::Material::FRONT_AND_BACK, shininess );
        }
        else
        {
            material->setShininess( osg::Material::FRONT_AND_BACK, 0.0f );
            material->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4() );
        }
        ss->setAttributeAndModes( material );
        
        int wireframe = 0; maxValue = 1;
        if ( aiGetMaterialIntegerArray(aiMtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &maxValue)==AI_SUCCESS )
        {
            ss->setAttributeAndModes( new osg::PolygonMode(
                osg::PolygonMode::FRONT_AND_BACK, wireframe ? osg::PolygonMode::LINE : osg::PolygonMode::FILL) );
        }
    }
    
    osg::TexEnv::Mode getEnvMode( aiTextureOp mode ) const
    {
        switch ( mode )
        {
        case aiTextureOp_Multiply: return osg::TexEnv::MODULATE;
        case aiTextureOp_Add: return osg::TexEnv::ADD;
        case aiTextureOp_Subtract: return osg::TexEnv::DECAL;
        case aiTextureOp_SmoothAdd: case aiTextureOp_SignedAdd:
            return osg::TexEnv::ADD;
        }
        return osg::TexEnv::REPLACE;
    }
    
    osg::Texture::WrapMode getWrapMode( aiTextureMapMode mode ) const
    {
        switch ( mode )
        {
        case aiTextureMapMode_Wrap: return osg::Texture::REPEAT;
        case aiTextureMapMode_Clamp: return osg::Texture::CLAMP;
        case aiTextureMapMode_Decal: return osg::Texture::CLAMP_TO_BORDER;
        case aiTextureMapMode_Mirror: return osg::Texture::MIRROR;
        }
        return osg::Texture::CLAMP;
    }
};

REGISTER_OSGPLUGIN( assimp, ReaderWriterAssImp )
