#ifndef H_SPARKUPDATINGHANDLER
#define H_SPARKUPDATINGHANDLER

#include <osg/Transform>
#include <osgGA/GUIEventHandler>
#include "SparkDrawable.h"

/** The spark updater which record all SparkDrawables and update them */
class SparkUpdatingHandler : public osgGA::GUIEventHandler
{
public:
    SparkUpdatingHandler() {}
    
    void addSpark( SparkDrawable* spark, osg::Transform* trackee=0 )
    { _sparks.push_back(SparkObject(spark, trackee)); }
    
    void removeSpark( unsigned int i )
    { if (i<_sparks.size()) _sparks.erase(_sparks.begin()+i); }
    
    void setTrackee( unsigned int i, osg::Transform* t )
    { _sparks[i]._trackee = t; _sparks[i]._dirtyMatrix = true; }
    
    osg::Transform* getTrackee( unsigned int i ) { return _sparks[i]._trackee.get(); }
    const osg::Transform* getTrackee( unsigned int i ) const { return _sparks[i]._trackee.get(); }
    
    SparkDrawable* getSpark( unsigned int i ) { return _sparks[i]._spark.get(); }
    const SparkDrawable* getSpark( unsigned int i ) const { return _sparks[i]._spark.get(); }
    unsigned int getNumSparks() const { return _sparks.size(); }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
    
protected:
    osg::Matrix computeTransformMatrix( SparkDrawable* spark, osg::Transform* trackee );
    
    struct SparkObject
    {
        SparkObject( SparkDrawable* s, osg::Transform* t )
        : _spark(s), _trackee(t), _dirtyMatrix(true) {}
        
        osg::observer_ptr<SparkDrawable> _spark;
        osg::observer_ptr<osg::Transform> _trackee;  // The trackee will be followed by the spark
        osg::Matrix _transformMatrix;  // The matrix from trackee coordinate to spark coordinate
        bool _dirtyMatrix;  // Dirty the matrix if trackee is changed in the scene graph
    };
    std::vector<SparkObject> _sparks;
};

#endif
