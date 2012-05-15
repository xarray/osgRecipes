/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 10 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/ShapeDrawable>
#include <osgDB/ObjectWrapper>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <iostream>

#include "CommonFunctions"

namespace testNS
{

class ComplexNode : public osg::Node
{
public:
    enum StyleType
    {
        NO_STYLE,
        NORMAL_STYLE,
        ADVANCED_STYLE,
        USER_STYLE,
    };
    
    struct ChildData
    {
        int active;
        std::string name;
        osg::ref_ptr<osg::Image> image;
    };
    
    ComplexNode() : osg::Node(), _type(NO_STYLE) {}
    
    ComplexNode(const ComplexNode& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
    :   osg::Node(copy, copyop), _type(copy._type),
        _shape(copy._shape), _children(copy._children)
    {}
    
    META_Node(testNS, ComplexNode)
    
    void setStyleType( StyleType type ) { _type = type; }
    StyleType getStyleType() const { return _type; }
    
    void setShape( osg::Shape* shape ) { _shape = shape; }
    const osg::Shape* getShape() const { return _shape.get(); }
    
    void addChildData( ChildData data ) { _children.push_back(data); }
    const ChildData& getChildData( unsigned int i ) const { return _children[i]; }
    unsigned int sizeOfChildren() const { return _children.size(); }
    
protected:
    StyleType _type;
    osg::ref_ptr<osg::Shape> _shape;
    std::vector<ChildData> _children;
};

}

namespace
{

static bool checkChildData( const testNS::ComplexNode& node )
{
    return node.sizeOfChildren()>0;
}

static bool readChildData( osgDB::InputStream& is, testNS::ComplexNode& node )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        testNS::ComplexNode::ChildData data;
        std::string childFlag("Child");
        is >> childFlag >> data.active >> data.name;
        
        bool hasImage = false;
        is >> hasImage >> is.BEGIN_BRACKET;
        if ( hasImage ) is >> data.image >> is.END_BRACKET;
        node.addChildData( data );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeChildData( osgDB::OutputStream& os, const testNS::ComplexNode& node )
{
    unsigned int size = node.sizeOfChildren();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        const testNS::ComplexNode::ChildData& data = node.getChildData(i);
        os << std::string("Child") << data.active << data.name;
        if ( data.image.valid() )
        {
            os << true << os.BEGIN_BRACKET << std::endl;
            os << data.image << os.END_BRACKET << std::endl;
        }
        else os << false << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( ComplexNode_Wrapper,
                         new testNS::ComplexNode,
                         testNS::ComplexNode,
                         "osg::Object osg::Node testNS::ComplexNode" )
{
    BEGIN_ENUM_SERIALIZER( StyleType, NO_STYLE );
        ADD_ENUM_VALUE( NO_STYLE );
        ADD_ENUM_VALUE( NORMAL_STYLE );
        ADD_ENUM_VALUE( ADVANCED_STYLE );
        ADD_ENUM_VALUE( USER_STYLE );
    END_ENUM_SERIALIZER();
    
    ADD_OBJECT_SERIALIZER( Shape, osg::Shape, NULL );
    ADD_USER_SERIALIZER( ChildData );
}

}

int main( int argc, char** argv )
{
    testNS::ComplexNode::ChildData data1;
    data1.active = 10;
    data1.name = "data1";
    data1.image = osgDB::readImageFile("Images/smoke.rgb");
    
    testNS::ComplexNode::ChildData data2;
    data2.active = 20;
    data2.name = "data2";
    
    osg::ref_ptr<testNS::ComplexNode> node = new testNS::ComplexNode;
    node->setStyleType( testNS::ComplexNode::ADVANCED_STYLE );
    node->setShape( new osg::Box() );
    node->addChildData( data1 );
    node->addChildData( data2 );
    
    osgDB::writeNodeFile( *node, "ComplexNode.osgt" );
    return 0;
}
