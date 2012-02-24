/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 10 Recipe 5
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgDB/OutputStream>
#include <osgDB/InputStream>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <iostream>
#include <sstream>

#include "CommonFunctions"
#include "BinaryStreamOperator.h"

class ClassInfo;
class ClassInstance;
class Method;

class ClassInfo : public osg::Referenced
{
public:
    ClassInfo( osgDB::ObjectWrapper* w=0 ) : _wrapper(w) {}
    
    ClassInstance* createInstance();
    Method* getMethod( const std::string& );
    
protected:
    virtual ~ClassInfo() {}
    osgDB::ObjectWrapper* _wrapper;
};

class ClassInstance : public osg::Referenced
{
public:
    ClassInstance( osg::Object* obj ) : _object(obj) {}
    
    void setObject( osg::Object* obj ) { _object = obj; }
    osg::Object* getObject() { return _object.get(); }
    const osg::Object* getObject() const { return _object.get(); }
    
protected:
    virtual ~ClassInstance() {}
    osg::ref_ptr<osg::Object> _object;
};

class Method : public osg::Referenced
{
public:
    Method( osgDB::BaseSerializer* s=0 ) : _serializer(s) {}
    
    template<typename T> bool set( ClassInstance*, T );
    template<typename T> bool get( ClassInstance*, T& );
    bool set( ClassInstance*, const ClassInstance& );
    bool get( ClassInstance*, ClassInstance& );
    
protected:
    virtual ~Method() {}
    osgDB::BaseSerializer* _serializer;
};

class ReflectionManager : public osg::Referenced
{
public:
    static ReflectionManager* instance()
    {
        static osg::ref_ptr<ReflectionManager> s_manager = new ReflectionManager;
        return s_manager.get();
    }
    
    osgDB::OutputStream& getOutputStream() { return *_outputStream; }
    osgDB::InputStream& getInputStream() { return *_inputStream; }
    std::stringstream& getSource() { return _source; }
    
    ClassInfo* getClassInfo( const std::string& name )
    {
        osgDB::ObjectWrapperManager* wrapperManager =
            osgDB::Registry::instance()->getObjectWrapperManager();
        if ( wrapperManager )
        {
            osgDB::ObjectWrapper* wrapper = wrapperManager->findWrapper(name);
            if ( wrapper ) return getClassInfo(wrapper);
        }
        return NULL;
    }
    
    ClassInfo* getClassInfo( osgDB::ObjectWrapper* wrapper )
    {
        ClassInfoMap::iterator itr = _classInfoMap.find(wrapper);
        if ( itr!=_classInfoMap.end() ) return itr->second.get();
        
        ClassInfo* info = new ClassInfo( wrapper );
        _classInfoMap[wrapper] = info;
        return info;
    }
    
    Method* getMethod( osgDB::BaseSerializer* serializer )
    {
        MethodMap::iterator itr = _methodMap.find(serializer);
        if ( itr!=_methodMap.end() ) return itr->second.get();
        
        Method* method = new Method( serializer );
        _methodMap[serializer] = method;
        return method;
    }
    
protected:
    ReflectionManager()
    {
        _outputStream = new osgDB::OutputStream(NULL);
		_inputStream = new osgDB::InputStream(NULL);
		_outputStream->setOutputIterator( new BinaryOutputIterator(&_source) );
        _inputStream->setInputIterator( new BinaryInputIterator(&_source) );
    }
    
    virtual ~ReflectionManager()
	{ delete _outputStream; delete _inputStream; }
    
    typedef std::map<osgDB::ObjectWrapper*, osg::ref_ptr<ClassInfo> > ClassInfoMap;
    ClassInfoMap _classInfoMap;
    
    typedef std::map<osgDB::BaseSerializer*, osg::ref_ptr<Method> > MethodMap;
    MethodMap _methodMap;
    
    osgDB::OutputStream* _outputStream;
    osgDB::InputStream* _inputStream;
    std::stringstream _source;
};

ClassInstance* ClassInfo::createInstance()
{
    if ( _wrapper )
    {
        const osg::Object* obj = _wrapper->getProto();
        if ( obj ) return new ClassInstance( obj->cloneType() );
    }
    return NULL;
}

Method* ClassInfo::getMethod( const std::string& name )
{
    if ( _wrapper )
    {
        osgDB::BaseSerializer* serializer = _wrapper->getSerializer(name);
        if ( serializer ) return ReflectionManager::instance()->getMethod(serializer);
    }
    return NULL;
}

template<typename T>
bool Method::set( ClassInstance* clsObject, T value )
{
    bool ok = false;
    ReflectionManager::instance()->getOutputStream() << value;
    if ( _serializer )
    {
        ok = _serializer->read( ReflectionManager::instance()->getInputStream(), *(clsObject->getObject()) );
    }
    ReflectionManager::instance()->getSource().clear();
    return ok;
}

template<typename T>
bool Method::get( ClassInstance* clsObject, T& value )
{
    bool ok = false;
    if ( _serializer )
    {
        ok = _serializer->write( ReflectionManager::instance()->getOutputStream(), *(clsObject->getObject()) );
    }
    if ( ok ) ReflectionManager::instance()->getInputStream() >> value;
    ReflectionManager::instance()->getSource().clear();
    return ok;
}

bool Method::set( ClassInstance* clsObject, const ClassInstance& instance )
{
    bool ok = false;
    ReflectionManager::instance()->getOutputStream() << (instance.getObject()!=NULL) << instance.getObject();
    if ( _serializer )
    {
        ok = _serializer->read( ReflectionManager::instance()->getInputStream(), *(clsObject->getObject()) );
    }
    ReflectionManager::instance()->getSource().clear();
    return ok;
}

bool Method::get( ClassInstance* clsObject, ClassInstance& instance )
{
    bool ok = false;
    if ( _serializer )
    {
        ok = _serializer->write( ReflectionManager::instance()->getOutputStream(), *(clsObject->getObject()) );
    }
    if ( ok )
    {
        osg::Object* obj = ReflectionManager::instance()->getInputStream().readObject();
        instance.setObject( obj );
    }
    ReflectionManager::instance()->getSource().clear();
    return ok;
}

int main( int argc, char **argv )
{
    ClassInfo* boxInfo = ReflectionManager::instance()->getClassInfo("osg::Box");
    Method* boxMethod = boxInfo->getMethod("HalfLengths");
    ClassInstance* box = boxInfo->createInstance();
    boxMethod->set( box, osg::Vec3(5.0f, 5.0f, 2.0f) );
    
    ClassInfo* drawableInfo = ReflectionManager::instance()->getClassInfo("osg::ShapeDrawable");
    Method* drawableMethod = drawableInfo->getMethod("Shape");
    ClassInstance* drawable = drawableInfo->createInstance();
    drawableMethod->set( drawable, *box );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( dynamic_cast<osg::Drawable*>(drawable->getObject()) );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( geode.get() );
    return viewer.run();
}
