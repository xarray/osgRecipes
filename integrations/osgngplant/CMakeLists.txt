SET(NGPCORE_FILES
    ngpcore/p3dmath.cpp
    ngpcore/p3dmathrng.cpp
    ngpcore/p3dmathspline.cpp
    ngpcore/p3dplant.cpp
    ngpcore/p3dmodel.cpp
    ngpcore/p3dmodelstemtube.cpp
    ngpcore/p3dmodelstemquad.cpp
    ngpcore/p3dmodelstemwings.cpp
    ngpcore/p3dmodelstemgmesh.cpp
    ngpcore/p3dbalgbase.cpp
    ngpcore/p3dbalgstd.cpp
    ngpcore/p3dbalgwings.cpp
    ngpcore/p3diostream.cpp
    ngpcore/p3diostreamadd.cpp
    ngpcore/p3dexcept.cpp
    ngpcore/p3dhli.cpp
    ngpcore/p3dgmeshdata.cpp
)

SET(EXAMPLE_NAME osgngplant)
SET(EXAMPLE_FILES osgngplant.cpp ${NGPCORE_FILES})
START_EXAMPLE()
