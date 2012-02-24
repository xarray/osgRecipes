#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for osgWeb
#
#\**********************************************************/

set(PLUGIN_NAME "osgWeb")
set(PLUGIN_PREFIX "OWE")
set(COMPANY_NAME "OSG")

# ActiveX constants:
set(FBTYPELIB_NAME osgWebLib)
set(FBTYPELIB_DESC "osgWeb 1.0 Type Library")
set(IFBControl_DESC "osgWeb Control Interface")
set(FBControl_DESC "osgWeb Control Class")
set(IFBComJavascriptObject_DESC "osgWeb IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "osgWeb ComJavascriptObject Class")
set(IFBComEventSource_DESC "osgWeb IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 31d50e5b-6b73-5ae6-ab7c-f33e0b4678fb)
set(IFBControl_GUID d8436e82-6830-5d99-b7b7-ab7780ad529c)
set(FBControl_GUID c7e7969a-0ac9-50ab-9517-08defcd94162)
set(IFBComJavascriptObject_GUID 5334d4ef-0220-5a37-91e0-e07e62c778e9)
set(FBComJavascriptObject_GUID 120ac231-7b0b-5109-ac13-7396d42fb65e)
set(IFBComEventSource_GUID b5c15ce7-2cdd-5ec3-b529-ca72ec761750)

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "OSG.osgWeb")
set(MOZILLA_PLUGINID "www.openscenegraph.org/osgWeb")

# strings
set(FBSTRING_CompanyName "OSG")
set(FBSTRING_FileDescription "OSG webplugin")
set(FBSTRING_PLUGIN_VERSION "1.0.0.0")
set(FBSTRING_LegalCopyright "Copyright 2011 OSG")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}.dll")
set(FBSTRING_ProductName "osgWeb")
set(FBSTRING_FileExtents "")
set(FBSTRING_PluginName "osgWeb")
set(FBSTRING_MIMEType "application/x-osgweb")

# Uncomment this next line if you're not planning on your plugin doing
# any drawing:

#set (FB_GUI_DISABLED 1)

# Mac plugin settings. If your plugin does not draw, set these all to 0
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 1)
set(FBMAC_USE_COCOA 1)
set(FBMAC_USE_COREGRAPHICS 1)
set(FBMAC_USE_COREANIMATION 0)
set(FBMAC_USE_INVALIDATINGCOREANIMATION 0)

# If you want to register per-machine on Windows, uncomment this line
#set (FB_ATLREG_MACHINEWIDE 1)
