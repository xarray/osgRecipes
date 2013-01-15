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

#include "guichan/widgets/textfield.hpp"

#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseinput.hpp"
#include "guichan/text.hpp"

namespace gcn
{
    TextField::TextField():
        mEditable(true),
        mXScroll(0)
    {
        mText = new Text();

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
    }

    TextField::TextField(const std::string& text):
        mEditable(true),
        mXScroll(0)
    {
        mText = new Text(text);

        adjustSize();

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
    }

    void TextField::setText(const std::string& text)
    {
        mText->setRow(0, text);
    }

    void TextField::draw(Graphics* graphics)
    {
        Color faceColor = getBaseColor();
        Color highlightColor, shadowColor;
        int alpha = getBaseColor().a;
        highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;

        // Draw a border.
        graphics->setColor(shadowColor);
        graphics->drawLine(0, 0, getWidth() - 1, 0);
        graphics->drawLine(0, 1, 0, getHeight() - 2);
        graphics->setColor(highlightColor);
        graphics->drawLine(getWidth() - 1, 1, getWidth() - 1, getHeight() - 1);
        graphics->drawLine(0, getHeight() - 1, getWidth() - 1, getHeight() - 1);

        // Push a clip area so the other drawings don't need to worry
        // about the border.
        graphics->pushClipArea(Rectangle(1, 1, getWidth() - 2, getHeight() - 2));

        graphics->setColor(getBackgroundColor());
        graphics->fillRectangle(0, 0, getWidth(), getHeight());

        if (isFocused())
        {
            graphics->setColor(getSelectionColor());
            graphics->drawRectangle(0, 0, getWidth() - 2, getHeight() - 2);
            graphics->drawRectangle(1, 1, getWidth() - 4, getHeight() - 4);
        }

        if (isFocused() && isEditable())
        {
            drawCaret(graphics, mText->getCaretX(getFont()) - mXScroll);
        }

        graphics->setColor(getForegroundColor());
        graphics->setFont(getFont());

        const Rectangle& dim = mText->getCaretDimension(getFont());
        if (mText->getNumberOfRows() != 0)
            graphics->drawText(mText->getRow(0), 1 - mXScroll, 1);

        graphics->popClipArea();
    }

    void TextField::drawCaret(Graphics* graphics, int x)
    {
        // Check the current clip area as a clip area with a different
        // size than the widget might have been pushed (which is the
        // case in the draw method when we push a clip area after we have
        // drawn a border).
        const Rectangle clipArea = graphics->getCurrentClipArea();

        graphics->setColor(getForegroundColor());
        graphics->drawLine(x, clipArea.height - 2, x, 1);
    }

    void TextField::mousePressed(MouseEvent& mouseEvent)
    {
        if (mouseEvent.getButton() == MouseEvent::Left)
        {
            mText->setCaretPosition(mouseEvent.getX() + mXScroll, mouseEvent.getY(), getFont());
            fixScroll();
        }
    }

    void TextField::mouseDragged(MouseEvent& mouseEvent)
    {
        mouseEvent.consume();
    }

    void TextField::keyPressed(KeyEvent& keyEvent)
    {
        Key key = keyEvent.getKey();

        if (key.getValue() == Key::Left)
            mText->setCaretPosition(mText->getCaretPosition() - 1);

        else if (key.getValue() == Key::Right)
            mText->setCaretPosition(mText->getCaretPosition() + 1);

        else if (key.getValue() == Key::Delete && mEditable)
            mText->remove(1);

        else if (key.getValue() == Key::Backspace && mEditable)
            mText->remove(-1);

        else if (key.getValue() == Key::Enter)
            distributeActionEvent();

        else if (key.getValue() == Key::Home)
            mText->setCaretColumn(0);

        else if (key.getValue() == Key::End)
            mText->setCaretColumn(mText->getNumberOfCharacters(0));

        else if (key.isCharacter()
                 && key.getValue() != Key::Tab
                 && mEditable)
            mText->insert(key.getValue());

        if (key.getValue() != Key::Tab)
            keyEvent.consume();

        fixScroll();
    }

    void TextField::adjustSize()
    {
        const Rectangle& dim = mText->getDimension(getFont());
        setWidth(dim.width + 8);
        adjustHeight();

        fixScroll();
    }

    void TextField::adjustHeight()
    {
        setHeight(getFont()->getHeight() + 4);
    }

    void TextField::fixScroll()
    {
        if (isFocused())
        {
            int caretX = mText->getCaretDimension(getFont()).x;

            if (caretX - mXScroll >= getWidth() - 4)
            {
                mXScroll = caretX - getWidth() + 4;
            }
            else if (caretX - mXScroll <= 0)
            {
                mXScroll = caretX - getWidth() / 2;

                if (mXScroll < 0)
                {
                    mXScroll = 0;
                }
            }
        }
    }

    void TextField::setCaretPosition(unsigned int position)
    {
        mText->setCaretPosition(position);
    }

    unsigned int TextField::getCaretPosition() const
    {
        return  mText->getCaretPosition();
    }

    std::string TextField::getText() const
    {
        return mText->getContent();
    }

    bool TextField::isEditable() const
    {
        return mEditable;
    }

    void TextField::setEditable(bool editable)
    {
        mEditable = editable;
    }
}
