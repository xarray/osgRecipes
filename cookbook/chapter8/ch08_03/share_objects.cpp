/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 8 Recipe 3
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <fstream>
#include <iostream>

#include "CommonFunctions"
const std::string c_removeMark("Removable");

class ReadAndShareCallback : public osgDB::ReadFileCallback
{
public:
    virtual osgDB::ReaderWriter::ReadResult readNode( const std::string& filename, const osgDB::Options* options )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock( _shareMutex );
        osg::Node* node = getNodeByName( filename );
        if ( !node )
        {
            osgDB::ReaderWriter::ReadResult rr = 
                osgDB::Registry::instance()->readNodeImplementation( filename, options );
            if ( rr.success() ) _nodeMap[filename] = rr.getNode();
            return rr;
        }
        else
            std::cout << "[SHARING] The name " << filename << " is already added to the sharing list." << std::endl;
        return node;
    }
    
    void prune( int second )
    {
        if ( !(second%5) )  // Prune the scene every 5 seconds
            return;
        
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock( _shareMutex );
        for ( NodeMap::iterator itr=_nodeMap.begin(); itr!=_nodeMap.end(); )
        {
            if ( itr->second.valid() )
            {
                if ( itr->second->referenceCount()<=1 )
                {
                    std::cout << "[REMOVING] The name " << itr->first << " is removed from the sharing list." << std::endl;
                    itr->second = NULL;
                }
            }
            ++itr;
        }
    }
    
protected:
    osg::Node* getNodeByName( const std::string& filename )
    {
        NodeMap::iterator itr = _nodeMap.find(filename);
        if ( itr!=_nodeMap.end() ) return itr->second.get();
        return NULL;
    }
    
    typedef std::map<std::string, osg::ref_ptr<osg::Node> > NodeMap;
    NodeMap _nodeMap;
    OpenThreads::Mutex _shareMutex;
};

class RemoveModelHandler : public osgCookBook::PickHandler
{
public:
    RemoveModelHandler( ReadAndShareCallback* cb ) : _callback(cb) {}
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
        {
            if ( _callback.valid() )
                _callback->prune( (int)ea.getTime() );
        }
        return osgCookBook::PickHandler::handle(ea, aa);
    }
    
    virtual void doUserOperations( osgUtil::LineSegmentIntersector::Intersection& result )
    {
        for ( osg::NodePath::iterator itr=result.nodePath.begin();
              itr!=result.nodePath.end(); ++itr )
        {
            if ( (*itr)->getName()==c_removeMark )
            {
                osg::Group* parent = ((*itr)->getNumParents()>0 ? (*itr)->getParent(0) : NULL);
                if ( parent ) parent->removeChild( (*itr) );
                break;
            }
        }
    }
    
    osg::observer_ptr<ReadAndShareCallback> _callback;
};

void addFileList( osg::Group* root, const std::string& file )
{
    std::ifstream is( file.c_str() );
    if ( !is ) return;
    
    while ( !is.eof() )
    {
        osg::Vec3 pos;
        std::string name;
        is >> name >> pos[0] >> pos[1] >> pos[2];
        
        osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
        trans->addChild( osgDB::readNodeFile(name) );
        trans->setMatrix( osg::Matrix::translate(pos) );
        trans->setName( c_removeMark );
        root->addChild( trans.get() );
    }
}

int main( int argc, char** argv )
{
    osg::ref_ptr<ReadAndShareCallback> sharer = new ReadAndShareCallback;
    osgDB::Registry::instance()->setReadFileCallback( sharer.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    addFileList( root.get(), "files.txt" );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new RemoveModelHandler(sharer.get()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
