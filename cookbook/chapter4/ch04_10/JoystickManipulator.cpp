/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 4 Recipe 10
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include "JoystickManipulator"

LPDIRECTINPUT8 g_inputDevice;
LPDIRECTINPUTDEVICE8 g_joystick;

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* didInstance, VOID* )
{
    HRESULT hr;
    if ( g_inputDevice )
    {
        hr = g_inputDevice->CreateDevice( didInstance->guidInstance, &g_joystick, NULL );
    }
    if ( FAILED(hr) ) return DIENUM_CONTINUE;
    return DIENUM_STOP;
}

TwoDimManipulator::TwoDimManipulator()
:   _distance(1.0)
{
    HRESULT hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&g_inputDevice, NULL );
    if ( FAILED(hr) || !g_inputDevice ) return;
    
    hr = g_inputDevice->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY );
}

TwoDimManipulator::~TwoDimManipulator()
{
    if ( g_joystick ) 
    {
        g_joystick->Unacquire();
        g_joystick->Release();
    }
    if ( g_inputDevice ) g_inputDevice->Release();
}

osg::Matrixd TwoDimManipulator::getMatrix() const
{
    osg::Matrixd matrix;
    matrix.makeTranslate( 0.0f, 0.0f, _distance );
    matrix.postMultTranslate( _center );
    return matrix;
}

osg::Matrixd TwoDimManipulator::getInverseMatrix() const
{
    osg::Matrixd matrix;
    matrix.makeTranslate( 0.0f, 0.0f,-_distance );
    matrix.preMultTranslate( -_center );
    return matrix;
}

void TwoDimManipulator::setByMatrix( const osg::Matrixd& matrix )
{
    setByInverseMatrix( osg::Matrixd::inverse(matrix) );
}

void TwoDimManipulator::setByInverseMatrix( const osg::Matrixd& matrix )
{
    osg::Vec3d eye, center, up;
    matrix.getLookAt( eye, center, up );
    
    _center = center;
    if ( _node.valid() )
        _distance = abs((_node->getBound().center() - eye).z());
    else
        _distance = abs((eye - center).length());
}

void TwoDimManipulator::init( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    const osgViewer::GraphicsWindowWin32* gw =
            dynamic_cast<const osgViewer::GraphicsWindowWin32*>( ea.getGraphicsContext() );
    if ( gw && g_joystick )
    {
        DIDATAFORMAT format = c_dfDIJoystick2;
        g_joystick->SetDataFormat( &format );
        g_joystick->SetCooperativeLevel( gw->getHWND(), DISCL_EXCLUSIVE|DISCL_FOREGROUND );
        g_joystick->Acquire();
    }
}

bool TwoDimManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    if ( g_joystick && ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
    {
        HRESULT hr = g_joystick->Poll();
        if ( FAILED(hr) ) g_joystick->Acquire();
        
        DIJOYSTATE2 state;
        hr = g_joystick->GetDeviceState( sizeof(DIJOYSTATE2), &state );
        if ( FAILED(hr) ) return false;
        
        double dx = 0.0, dy = 0.0;
        if ( state.lX==0x0000 ) dx -= 0.01;
        else if ( state.lX==0xffff ) dx += 0.01;
        
        if ( state.lY==0 ) dy -= 0.01;
        else if ( state.lY==0xffff ) dy += 0.01;
        
        if ( state.rgbButtons[0] )
            performMovementLeftMouseButton( 0.0, dx, dy );
        if ( state.rgbButtons[1] )
            performMovementRightMouseButton( 0.0, dx, dy );
    }
    return false;
}

void TwoDimManipulator::home( double )
{
    if ( _node.valid() )
    {
        _center = _node->getBound().center();
        _distance = 2.5 * _node->getBound().radius();
    }
    else
    {
        _center.set( osg::Vec3() );
        _distance = 1.0;
    }
}

bool TwoDimManipulator::performMovementLeftMouseButton(
    const double eventTimeDelta, const double dx, const double dy )
{
    _center.x() -= 100.0f * dx;
    _center.y() -= 100.0f * dy;
    return false;
}

bool TwoDimManipulator::performMovementRightMouseButton(
    const double eventTimeDelta, const double dx, const double dy )
{
    _distance *= (1.0 + dy);
    if ( _distance<1.0 ) _distance = 1.0;
    return false;
}
