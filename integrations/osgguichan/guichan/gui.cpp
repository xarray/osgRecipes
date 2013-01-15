/*      _______   __   __   __   ______   __   __   _______   __   __
 *     / _____/\ / /\ / /\ / /\ / ____/\ / /\ / /\ / ___  /\ /  |\/ /\
 *    / /\____\// / // / // / // /\___\// /_// / // /\_/ / // , |/ / /
 *   / / /__   / / // / // / // / /    / ___  / // ___  / // /| ' / /
 *  / /_// /\ / /_// / // / // /_/_   / / // / // /\_/ / // / |  / /
 * /______/ //______/ //_/ //_____/\ /_/ //_/ //_/ //_/ //_/ /|_/ /
 * \______\/ \______\/ \_\/ \_____\/ \_\/ \_\/ \_\/ \_\/ \_\/ \_\/
 *
 * Copyright (c) 2004 - 2008 Olof Naessén and Per Larsson
 *
 *
 * Per Larsson a.k.a finalman
 * Olof Naessén a.k.a jansem/yakslem
 *
 * Visit: http://guichan.sourceforge.net
 *
 * License: (BSD)
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Guichan nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * For comments regarding functions please see the header file.
 */

#include "guichan/gui.hpp"

#include "guichan/exception.hpp"
#include "guichan/focushandler.hpp"
#include "guichan/graphics.hpp"
#include "guichan/input.hpp"
#include "guichan/keyinput.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/mouseinput.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/widget.hpp"
#include <iterator>
#include <algorithm>

namespace gcn
{
    Gui::Gui()
            :mTop(NULL),
             mGraphics(NULL),
             mInput(NULL),
             mTabbing(true),
             mShiftPressed(false),
             mMetaPressed(false),
             mControlPressed(false),
             mAltPressed(false),
             mLastMousePressButton(0),
             mLastMousePressTimeStamp(0),
             mLastMouseX(0),
             mLastMouseY(0),
             mClickCount(1),
             mLastMouseDragButton(0)
    {
        mFocusHandler = new FocusHandler();
    }

    Gui::~Gui()
    {
        if (Widget::widgetExists(mTop))
        {
            setTop(NULL);
        }

        delete mFocusHandler;
    }

    void Gui::setTop(Widget* top)
    {
        if (mTop != NULL)
        {
            mTop->_setFocusHandler(NULL);
        }
        if (top != NULL)
        {
            top->_setFocusHandler(mFocusHandler);
        }

        mTop = top;
    }

    Widget* Gui::getTop() const
    {
        return mTop;
    }

    void Gui::setGraphics(Graphics* graphics)
    {
        mGraphics = graphics;
    }

    Graphics* Gui::getGraphics() const
    {
        return mGraphics;
    }

    void Gui::setInput(Input* input)
    {
        mInput = input;
    }

    Input* Gui::getInput() const
    {
        return mInput;
    }

    void Gui::logic()
    {
        if (mTop == NULL)
            throw GCN_EXCEPTION("No top widget set");

        handleModalFocus();
        handleModalMouseInputFocus();

        if (mInput != NULL)
        {
            mInput->_pollInput();

            handleKeyInput();
            handleMouseInput();
        }

        mTop->_logic();
    }

    void Gui::draw()
    {
        if (mTop == NULL)
            throw GCN_EXCEPTION("No top widget set");

        if (mGraphics == NULL)
            throw GCN_EXCEPTION("No graphics set");

        if (!mTop->isVisible())
            return;

        mGraphics->_beginDraw();
        mTop->_draw(mGraphics);
        mGraphics->_endDraw();
    }

    void Gui::focusNone()
    {
        mFocusHandler->focusNone();
    }

    void Gui::setTabbingEnabled(bool tabbing)
    {
        mTabbing = tabbing;
    }

    bool Gui::isTabbingEnabled()
    {
        return mTabbing;
    }

    void Gui::addGlobalKeyListener(KeyListener* keyListener)
    {
        mKeyListeners.push_back(keyListener);
    }

    void Gui::removeGlobalKeyListener(KeyListener* keyListener)
    {
        mKeyListeners.remove(keyListener);
    }

    void Gui::handleMouseInput()
    {
        while (!mInput->isMouseQueueEmpty())
        {
            MouseInput mouseInput = mInput->dequeueMouseInput();

            switch (mouseInput.getType())
            {
            case MouseInput::Pressed:
               handleMousePressed(mouseInput);
               break;
            case MouseInput::Released:
               handleMouseReleased(mouseInput);
               break;
            case MouseInput::Moved:
               handleMouseMoved(mouseInput);
               break;
            case MouseInput::WheelMovedDown:
               handleMouseWheelMovedDown(mouseInput);
               break;
            case MouseInput::WheelMovedUp:
               handleMouseWheelMovedUp(mouseInput);
               break;
            default:
               throw GCN_EXCEPTION("Unknown mouse input type.");
               break;
            }
            
             // Save the current mouse state. It's needed to send
             // mouse exited events and mouse entered events when
             // the mouse exits a widget and when a widget releases
             // modal mouse input focus.
             mLastMouseX = mouseInput.getX();
             mLastMouseY = mouseInput.getY();
         }
    }

    void Gui::handleKeyInput()
    {
        while (!mInput->isKeyQueueEmpty())
        {
            KeyInput keyInput = mInput->dequeueKeyInput();

            // Save modifiers state
            mShiftPressed = keyInput.isShiftPressed();
            mMetaPressed = keyInput.isMetaPressed();
            mControlPressed = keyInput.isControlPressed();
            mAltPressed = keyInput.isAltPressed();

            KeyEvent keyEventToGlobalKeyListeners(NULL,
                                                  NULL,
                                                  mShiftPressed,
                                                  mControlPressed,
                                                  mAltPressed,
                                                  mMetaPressed,
                                                  keyInput.getType(),
                                                  keyInput.isNumericPad(),
                                                  keyInput.getKey());

            distributeKeyEventToGlobalKeyListeners(keyEventToGlobalKeyListeners);

            // If a global key listener consumes the event it will not be
            // sent further to the source of the event.
            if (keyEventToGlobalKeyListeners.isConsumed())
            {
                continue;
            }

            bool keyEventConsumed = false;
            
            // Send key inputs to the focused widgets
            if (mFocusHandler->getFocused() != NULL)
            {
                Widget* source = getKeyEventSource();
                KeyEvent keyEvent(source,
                                  source,
                                  mShiftPressed,
                                  mControlPressed,
                                  mAltPressed,
                                  mMetaPressed,
                                  keyInput.getType(),
                                  keyInput.isNumericPad(),
                                  keyInput.getKey());
                

                if (!mFocusHandler->getFocused()->isFocusable())
                    mFocusHandler->focusNone();
                else                 
                    distributeKeyEvent(keyEvent);

                keyEventConsumed = keyEvent.isConsumed();
            }

            // If the key event hasn't been consumed and
            // tabbing is enable check for tab press and
            // change focus.
            if (!keyEventConsumed
                && mTabbing
                && keyInput.getKey().getValue() == Key::Tab
                && keyInput.getType() == KeyInput::Pressed)
            {
                if (keyInput.isShiftPressed())
                    mFocusHandler->tabPrevious();
                else
                    mFocusHandler->tabNext();
            }                           
        }
    }

    void Gui::handleMouseMoved(const MouseInput& mouseInput)
    {
        // Get tha last widgets with the mouse using the
        // last known mouse position.
        std::set<Widget*> mLastWidgetsWithMouse = getWidgetsAt(mLastMouseX, mLastMouseY);

        // Check if the mouse has left the application window.
        if (mouseInput.getX() < 0
            || mouseInput.getY() < 0
            || !mTop->getDimension().isContaining(mouseInput.getX(), mouseInput.getY()))
        {
            std::set<Widget*>::const_iterator iter;
            for (iter = mLastWidgetsWithMouse.begin(); 
                 iter != mLastWidgetsWithMouse.end();
                 iter++)
            {
                distributeMouseEvent((*iter),
                                     MouseEvent::Exited,
                                     mouseInput.getButton(),
                                     mouseInput.getX(),
                                     mouseInput.getY(),
                                     true,
                                     true);
            }
        }
        // The mouse is in the application window.
        else
        {
            // Calculate which widgets should receive a mouse exited event
            // and which should receive a mouse entered event by using the 
            // last known mouse position and the latest mouse position.
            std::set<Widget*> mWidgetsWithMouse = getWidgetsAt(mouseInput.getX(), mouseInput.getY());
            std::set<Widget*> mWidgetsWithMouseExited;
            std::set<Widget*> mWidgetsWithMouseEntered;
            std::set_difference(mLastWidgetsWithMouse.begin(),
                                mLastWidgetsWithMouse.end(),
                                mWidgetsWithMouse.begin(),
                                mWidgetsWithMouse.end(),
                                std::inserter(mWidgetsWithMouseExited, mWidgetsWithMouseExited.begin()));
            std::set_difference(mWidgetsWithMouse.begin(),
                                mWidgetsWithMouse.end(),
                                mLastWidgetsWithMouse.begin(),
                                mLastWidgetsWithMouse.end(),
                                std::inserter(mWidgetsWithMouseEntered, mWidgetsWithMouseEntered.begin()));

            std::set<Widget*>::const_iterator iter;
            for (iter = mWidgetsWithMouseExited.begin(); 
                 iter != mWidgetsWithMouseExited.end();
                 iter++)
            {
                distributeMouseEvent((*iter),
                                     MouseEvent::Exited,
                                     mouseInput.getButton(),
                                     mouseInput.getX(),
                                     mouseInput.getY(),
                                     true,
                                     true);   
                // As the mouse has exited a widget we need
                // to reset the click count and the last mouse
                // press time stamp.
                mClickCount = 1;
                mLastMousePressTimeStamp = 0;
            }

            for (iter = mWidgetsWithMouseEntered.begin(); 
                 iter != mWidgetsWithMouseEntered.end();
                 iter++)
            {
                Widget* widget = (*iter);
                // If a widget has modal mouse input focus we
                // only want to send entered events to that widget
                // and the widget's parents.
                if ((mFocusHandler->getModalMouseInputFocused() != NULL
                     && widget->isModalMouseInputFocused())
                     || mFocusHandler->getModalMouseInputFocused() == NULL)
                {
                    distributeMouseEvent(widget,
                                         MouseEvent::Entered,
                                         mouseInput.getButton(),
                                         mouseInput.getX(),
                                         mouseInput.getY(),
                                         true,
                                         true);
                }
            }
        }
    
        if (mFocusHandler->getDraggedWidget() != NULL)
        {
            distributeMouseEvent(mFocusHandler->getDraggedWidget(),
                                 MouseEvent::Dragged,
                                 mLastMouseDragButton,
                                 mouseInput.getX(),
                                 mouseInput.getY());
        }
        else
        {
            Widget* sourceWidget = getMouseEventSource(mouseInput.getX(), mouseInput.getY());
            distributeMouseEvent(sourceWidget,
                                 MouseEvent::Moved,
                                 mouseInput.getButton(),
                                 mouseInput.getX(),
                                 mouseInput.getY());
        }
    }

    void Gui::handleMousePressed(const MouseInput& mouseInput)
    {
        Widget* sourceWidget = getMouseEventSource(mouseInput.getX(), mouseInput.getY());

        if (mFocusHandler->getDraggedWidget() != NULL)
            sourceWidget = mFocusHandler->getDraggedWidget();

        int sourceWidgetX, sourceWidgetY;
        sourceWidget->getAbsolutePosition(sourceWidgetX, sourceWidgetY);

        if ((mFocusHandler->getModalFocused() != NULL
            && sourceWidget->isModalFocused())
            || mFocusHandler->getModalFocused() == NULL)
        {
            sourceWidget->requestFocus();
        }

        if (mouseInput.getTimeStamp() - mLastMousePressTimeStamp < 250
            && mLastMousePressButton == mouseInput.getButton())
            mClickCount++;
        else
            mClickCount = 1;

        distributeMouseEvent(sourceWidget,
                             MouseEvent::Pressed,
                             mouseInput.getButton(),
                             mouseInput.getX(),
                             mouseInput.getY());

        mFocusHandler->setLastWidgetPressed(sourceWidget);

        mFocusHandler->setDraggedWidget(sourceWidget);
        mLastMouseDragButton = mouseInput.getButton();

        mLastMousePressButton = mouseInput.getButton();
        mLastMousePressTimeStamp = mouseInput.getTimeStamp();
    }

    void Gui::handleMouseWheelMovedDown(const MouseInput& mouseInput)
    {
        Widget* sourceWidget = getMouseEventSource(mouseInput.getX(), mouseInput.getY());

        if (mFocusHandler->getDraggedWidget() != NULL)
            sourceWidget = mFocusHandler->getDraggedWidget();

        int sourceWidgetX, sourceWidgetY;
        sourceWidget->getAbsolutePosition(sourceWidgetX, sourceWidgetY);

        distributeMouseEvent(sourceWidget,
                             MouseEvent::WheelMovedDown,
                             mouseInput.getButton(),
                             mouseInput.getX(),
                             mouseInput.getY());
    }

    void Gui::handleMouseWheelMovedUp(const MouseInput& mouseInput)
    {
        Widget* sourceWidget = getMouseEventSource(mouseInput.getX(), mouseInput.getY());

        if (mFocusHandler->getDraggedWidget() != NULL)
            sourceWidget = mFocusHandler->getDraggedWidget();

        int sourceWidgetX, sourceWidgetY;
        sourceWidget->getAbsolutePosition(sourceWidgetX, sourceWidgetY);

        distributeMouseEvent(sourceWidget,
                             MouseEvent::WheelMovedUp,
                             mouseInput.getButton(),
                             mouseInput.getX(),
                             mouseInput.getY());
    }

    void Gui::handleMouseReleased(const MouseInput& mouseInput)
    {
        Widget* sourceWidget = getMouseEventSource(mouseInput.getX(), mouseInput.getY());

        if (mFocusHandler->getDraggedWidget() != NULL)
        {
            if (sourceWidget != mFocusHandler->getLastWidgetPressed())
                mFocusHandler->setLastWidgetPressed(NULL);
            
            sourceWidget = mFocusHandler->getDraggedWidget();
        }

        int sourceWidgetX, sourceWidgetY;
        sourceWidget->getAbsolutePosition(sourceWidgetX, sourceWidgetY);
        
        distributeMouseEvent(sourceWidget,
                             MouseEvent::Released,
                             mouseInput.getButton(),
                             mouseInput.getX(),
                             mouseInput.getY());

        if (mouseInput.getButton() == mLastMousePressButton            
            && mFocusHandler->getLastWidgetPressed() == sourceWidget)
        {
            distributeMouseEvent(sourceWidget,
                                 MouseEvent::Clicked,
                                 mouseInput.getButton(),
                                 mouseInput.getX(),
                                 mouseInput.getY());
            
            mFocusHandler->setLastWidgetPressed(NULL);
        }
        else
        {
            mLastMousePressButton = 0;
            mClickCount = 0;
        }

        if (mFocusHandler->getDraggedWidget() != NULL)
            mFocusHandler->setDraggedWidget(NULL);
    }

    Widget* Gui::getWidgetAt(int x, int y)
    {
        // If the widget's parent has no child then we have found the widget..
        Widget* parent = mTop;
        Widget* child = mTop;

        while (child != NULL)
        {
            Widget* swap = child;
            int parentX, parentY;
            parent->getAbsolutePosition(parentX, parentY);
            child = parent->getWidgetAt(x - parentX, y - parentY);
            parent = swap;
        }

        return parent;
    }

    std::set<Widget*> Gui::getWidgetsAt(int x, int y)
    {
        std::set<Widget*> result;

        Widget* widget = mTop;

        while (widget != NULL)
        {
            result.insert(widget);
            int absoluteX, absoluteY;
            widget->getAbsolutePosition(absoluteX, absoluteY);
            widget = widget->getWidgetAt(x - absoluteX, y - absoluteY);
        }

        return result;
    }

    Widget* Gui::getMouseEventSource(int x, int y)
    {
        Widget* widget = getWidgetAt(x, y);

        if (mFocusHandler->getModalMouseInputFocused() != NULL
            && !widget->isModalMouseInputFocused())
            return mFocusHandler->getModalMouseInputFocused();

        return widget;
    }

    Widget* Gui::getKeyEventSource()
    {
        Widget* widget = mFocusHandler->getFocused();

        while (widget->_getInternalFocusHandler() != NULL
               && widget->_getInternalFocusHandler()->getFocused() != NULL)
        {
            widget = widget->_getInternalFocusHandler()->getFocused();
        }

        return widget;
    }

    void Gui::distributeMouseEvent(Widget* source,
                                   int type,
                                   int button,
                                   int x,
                                   int y,
                                   bool force,
                                   bool toSourceOnly)
    {
        Widget* parent = source;
        Widget* widget = source;

        if (mFocusHandler->getModalFocused() != NULL
            && !widget->isModalFocused()
            && !force)
            return;

        if (mFocusHandler->getModalMouseInputFocused() != NULL
            && !widget->isModalMouseInputFocused()
            && !force)
            return;

        MouseEvent mouseEvent(source,
                              source,
                              mShiftPressed,
                              mControlPressed,
                              mAltPressed,
                              mMetaPressed,
                              type,
                              button,
                              x,
                              y,
                              mClickCount);

        while (parent != NULL)
        {
            // If the widget has been removed due to input
            // cancel the distribution.
            if (!Widget::widgetExists(widget))
                break;

            parent = widget->getParent();

            if (widget->isEnabled() || force)
            {
                int widgetX, widgetY;
                widget->getAbsolutePosition(widgetX, widgetY);

                mouseEvent.mX = x - widgetX;
                mouseEvent.mY = y - widgetY;
                mouseEvent.mDistributor = widget;                      
                std::list<MouseListener*> mouseListeners = widget->_getMouseListeners();

                // Send the event to all mouse listeners of the widget.
                for (std::list<MouseListener*>::iterator it = mouseListeners.begin();
                     it != mouseListeners.end();
                     ++it)
                {
                    switch (mouseEvent.getType())
                    {
                      case MouseEvent::Entered:
                          (*it)->mouseEntered(mouseEvent);
                          break;
                      case MouseEvent::Exited:
                          (*it)->mouseExited(mouseEvent);
                          break;
                      case MouseEvent::Moved:
                          (*it)->mouseMoved(mouseEvent);
                          break;
                      case MouseEvent::Pressed:
                          (*it)->mousePressed(mouseEvent);
                          break;
                      case MouseEvent::Released:
                          (*it)->mouseReleased(mouseEvent);
                          break;
                      case MouseEvent::WheelMovedUp:
                          (*it)->mouseWheelMovedUp(mouseEvent);
                          break;
                      case MouseEvent::WheelMovedDown:
                          (*it)->mouseWheelMovedDown(mouseEvent);
                          break;
                      case MouseEvent::Dragged:
                          (*it)->mouseDragged(mouseEvent);
                          break;
                      case MouseEvent::Clicked:
                          (*it)->mouseClicked(mouseEvent);
                          break;
                      default:
                          throw GCN_EXCEPTION("Unknown mouse event type.");
                    }                    
                }
                
                if (toSourceOnly)
                    break;

            }

            Widget* swap = widget;
            widget = parent;
            parent = swap->getParent();

            // If a non modal focused widget has been reach
            // and we have modal focus cancel the distribution.
            if (mFocusHandler->getModalFocused() != NULL
                && widget != NULL
                && !widget->isModalFocused())
                break;

            // If a non modal mouse input focused widget has been reach
            // and we have modal mouse input focus cancel the distribution.
            if (mFocusHandler->getModalMouseInputFocused() != NULL
                && widget != NULL
                && !widget->isModalMouseInputFocused())
                break;
        }
    }

    void Gui::distributeKeyEvent(KeyEvent& keyEvent)
    {
        Widget* parent = keyEvent.getSource();
        Widget* widget = keyEvent.getSource();

        if (mFocusHandler->getModalFocused() != NULL
            && !widget->isModalFocused())
            return;

        if (mFocusHandler->getModalMouseInputFocused() != NULL
            && !widget->isModalMouseInputFocused())
            return;

        while (parent != NULL)
        {
            // If the widget has been removed due to input
            // cancel the distribution.
            if (!Widget::widgetExists(widget))
                break;

            parent = widget->getParent();

            if (widget->isEnabled())
            {
                keyEvent.mDistributor = widget;
                std::list<KeyListener*> keyListeners = widget->_getKeyListeners();
            
                // Send the event to all key listeners of the source widget.
                for (std::list<KeyListener*>::iterator it = keyListeners.begin();
                     it != keyListeners.end();
                     ++it)
                {
                    switch (keyEvent.getType())
                    {
                      case KeyEvent::Pressed:
                          (*it)->keyPressed(keyEvent);
                          break;
                      case KeyEvent::Released:
                          (*it)->keyReleased(keyEvent);
                          break;
                      default:
                          throw GCN_EXCEPTION("Unknown key event type.");
                    }                
                }
            }

            Widget* swap = widget;
            widget = parent;
            parent = swap->getParent();

            // If a non modal focused widget has been reach
            // and we have modal focus cancel the distribution.
            if (mFocusHandler->getModalFocused() != NULL
                && !widget->isModalFocused())
                break;
        }
    }

    void Gui::distributeKeyEventToGlobalKeyListeners(KeyEvent& keyEvent)
    {
        KeyListenerListIterator it;

        for (it = mKeyListeners.begin(); it != mKeyListeners.end(); it++)
        {
            switch (keyEvent.getType())
            {
              case KeyEvent::Pressed:
                  (*it)->keyPressed(keyEvent);
                  break;
              case KeyEvent::Released:
                  (*it)->keyReleased(keyEvent);
                  break;
              default:
                  throw GCN_EXCEPTION("Unknown key event type.");
            }

            if (keyEvent.isConsumed())
                break;
        }
    }

    void Gui::handleModalMouseInputFocus()
    {
        // Check if modal mouse input focus has been gained by a widget.
        if ((mFocusHandler->getLastWidgetWithModalMouseInputFocus() 
                != mFocusHandler->getModalMouseInputFocused())
             && (mFocusHandler->getLastWidgetWithModalMouseInputFocus() == NULL))
        {
            handleModalFocusGained();
            mFocusHandler->setLastWidgetWithModalMouseInputFocus(mFocusHandler->getModalMouseInputFocused());
        }
        // Check if modal mouse input focus has been released.
        else if ((mFocusHandler->getLastWidgetWithModalMouseInputFocus()
                    != mFocusHandler->getModalMouseInputFocused())
                    && (mFocusHandler->getLastWidgetWithModalMouseInputFocus() != NULL))
        {
            handleModalFocusReleased();
            mFocusHandler->setLastWidgetWithModalMouseInputFocus(NULL);
        }
    }

     void Gui::handleModalFocus()
    {
        // Check if modal focus has been gained by a widget.
        if ((mFocusHandler->getLastWidgetWithModalFocus() 
                != mFocusHandler->getModalFocused())
             && (mFocusHandler->getLastWidgetWithModalFocus() == NULL))
        {
            handleModalFocusGained();
            mFocusHandler->setLastWidgetWithModalFocus(mFocusHandler->getModalFocused());
        }
        // Check if modal focus has been released.
        else if ((mFocusHandler->getLastWidgetWithModalFocus()
                    != mFocusHandler->getModalFocused())
                    && (mFocusHandler->getLastWidgetWithModalFocus() != NULL))
        {
            handleModalFocusReleased();
            mFocusHandler->setLastWidgetWithModalFocus(NULL);
        }
    }

    void Gui::handleModalFocusGained()
    {
        // Get all widgets at the last known mouse position
        // and send them a mouse exited event.
        std::set<Widget*> mWidgetsWithMouse = getWidgetsAt(mLastMouseX, mLastMouseY);
       
        std::set<Widget*>::const_iterator iter;
        for (iter = mWidgetsWithMouse.begin(); 
             iter != mWidgetsWithMouse.end();
             iter++)
        {
            distributeMouseEvent((*iter),
                                 MouseEvent::Exited,
                                 mLastMousePressButton,
                                 mLastMouseX,
                                 mLastMouseY,
                                 true,
                                 true);   
        }

        mFocusHandler->setLastWidgetWithModalMouseInputFocus(mFocusHandler->getModalMouseInputFocused());
    }

    void Gui::handleModalFocusReleased()
    {
        // Get all widgets at the last known mouse position
        // and send them a mouse entered event.
        std::set<Widget*> mWidgetsWithMouse = getWidgetsAt(mLastMouseX, mLastMouseY);
       
        std::set<Widget*>::const_iterator iter;
        for (iter = mWidgetsWithMouse.begin(); 
             iter != mWidgetsWithMouse.end();
             iter++)
        {
            distributeMouseEvent((*iter),
                                 MouseEvent::Entered,
                                 mLastMousePressButton,
                                 mLastMouseX,
                                 mLastMouseY,
                                 false,
                                 true);   
        }
    }
}
