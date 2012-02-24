/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 7
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Program>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include "CommonFunctions"

static const char* vertSource = {
    "uniform vec3 lightPosition;\n"
    "varying vec3 normal, eyeVec, lightDir;\n"
    "void main()\n"
    "{\n"
    "    vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;\n"
    "    eyeVec = -vertexInEye.xyz;\n"
    "    lightDir = vec3(lightPosition - vertexInEye.xyz);\n"
    "    normal = gl_NormalMatrix * gl_Normal;\n"
    "    gl_Position = ftransform();\n"
    "}\n"
};

static const char* fragSource = {
    "uniform vec4 lightDiffuse;\n"
    "uniform vec4 lightSpecular;\n"
    "uniform float shininess;\n"
    "varying vec3 normal, eyeVec, lightDir;\n"
    "void main (void)\n"
    "{\n"
    "  vec4 finalColor = gl_FrontLightModelProduct.sceneColor;\n"
    "  vec3 N = normalize(normal);\n"
    "  vec3 L = normalize(lightDir);\n"
    "  float lambert = dot(N,L);\n"
    "  if (lambert > 0.0)\n"
    "  {\n"
    "    finalColor += lightDiffuse * lambert;\n"
    "    vec3 E = normalize(eyeVec);\n"
    "    vec3 R = reflect(-L, N);\n"
    "    float specular = pow(max(dot(R, E), 0.0), shininess);\n"
    "    finalColor += lightSpecular * specular;\n"
    "  }\n"
    "  gl_FragColor = finalColor;\n"
    "}\n"
};

class LightPosCallback : public osg::Uniform::Callback
{
public:
    virtual void operator()( osg::Uniform* uniform, osg::NodeVisitor* nv )
    {
        const osg::FrameStamp* fs = nv->getFrameStamp();
        if ( !fs ) return;
        
        float angle = osg::inDegrees( (float)fs->getFrameNumber() );
        uniform->set( osg::Vec3(20.0f * cosf(angle), 20.0f * sinf(angle), 1.0f) );
    }
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile( "cow.osg" );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, vertSource) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, fragSource) );
    
    osg::StateSet* stateset = model->getOrCreateStateSet();
    stateset->setAttributeAndModes( program.get() );
    stateset->addUniform( new osg::Uniform("lightDiffuse", osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f)) );
    stateset->addUniform( new osg::Uniform("lightSpecular", osg::Vec4(1.0f, 1.0f, 0.4f, 1.0f)) );
    stateset->addUniform( new osg::Uniform("shininess", 64.0f) );
    
    osg::ref_ptr<osg::Uniform> lightPos = new osg::Uniform( "lightPosition", osg::Vec3() );
    lightPos->setUpdateCallback( new LightPosCallback );
    stateset->addUniform( lightPos.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( model.get() );
    return viewer.run();
}
