SET(EXTERNAL_INCLUDE_DIR ./guichan ${OPENGL_INCLUDE_DIR})
SET(EXTERNAL_LIBRARIES ${OPENGL_LIBRARIES})

SET(EXAMPLE_NAME osgguichan)
SET(EXAMPLE_FILES
    osgguichan.cpp
    GuichanWrapper.cpp
    
    guichan/actionevent.cpp
    guichan/cliprectangle.cpp
    guichan/color.cpp
    guichan/containerevent.cpp
    guichan/defaultfont.cpp
    guichan/event.cpp
    guichan/exception.cpp
    guichan/focushandler.cpp
    guichan/font.cpp
    guichan/genericinput.cpp
    guichan/graphics.cpp
    guichan/gui.cpp
    guichan/guichan.cpp
    guichan/image.cpp
    guichan/imagefont.cpp
    guichan/inputevent.cpp
    guichan/key.cpp
    guichan/keyevent.cpp
    guichan/keyinput.cpp
    guichan/mouseevent.cpp
    guichan/mouseinput.cpp
    guichan/rectangle.cpp
    guichan/selectionevent.cpp
    guichan/text.cpp
    guichan/widget.cpp
    
    guichan/widgets/button.cpp
    guichan/widgets/checkbox.cpp
    guichan/widgets/container.cpp
    guichan/widgets/dropdown.cpp
    guichan/widgets/icon.cpp
    guichan/widgets/imagebutton.cpp
    guichan/widgets/label.cpp
    guichan/widgets/listbox.cpp
    guichan/widgets/radiobutton.cpp
    guichan/widgets/scrollarea.cpp
    guichan/widgets/slider.cpp
    guichan/widgets/tab.cpp
    guichan/widgets/tabbedarea.cpp
    guichan/widgets/textbox.cpp
    guichan/widgets/textfield.cpp
    guichan/widgets/window.cpp
    
    guichan/opengl/opengl.cpp
    guichan/opengl/openglgraphics.cpp
    guichan/opengl/openglimage.cpp
)

IF(BUILD_WITH_LITTLE_ENDIAN)
    ADD_DEFINITIONS(-DGUICHAN_LITTLE_ENDIAN)
ELSEIF(BUILD_WITH_LITTLE_ENDIAN)
    ADD_DEFINITIONS(-DGUICHAN_BIG_ENDIAN)
ENDIF(BUILD_WITH_LITTLE_ENDIAN)
START_EXAMPLE()
