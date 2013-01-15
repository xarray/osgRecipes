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

#include "guichan/widgets/textbox.hpp"

#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseinput.hpp"
#include "guichan/text.hpp"

namespace gcn
{
    TextBox::TextBox()
        :mEditable(true),
         mOpaque(true)
    {
        mText = new Text();

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        adjustSize();
    }

    TextBox::TextBox(const std::string& text)
        :mEditable(true),
         mOpaque(true)
    {
        mText = new Text(text);

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        adjustSize();
    }

    void TextBox::setText(const std::string& text)
    {
        mText->setContent(text);
        adjustSize();
    }

    void TextBox::draw(Graphics* graphics)
    {
        if (mOpaque)
        {
            graphics->setColor(getBackgroundColor());
            graphics->fillRectangle(0, 0, getWidth(), getHeight());
        }

        if (isFocused() && isEditable())
        {
            drawCaret(graphics, 
                      mText->getCaretX(getFont()), 
                      mText->getCaretY(getFont()));
        }

        graphics->setColor(getForegroundColor());
        graphics->setFont(getFont());

        unsigned int i;
        for (i = 0; i < mText->getNumberOfRows(); i++)
        {
            // Move the text one pixel so we can have a caret before a letter.
            graphics->drawText(mText->getRow(i), 1, i * getFont()->getHeight());
        }
    }

    void TextBox::drawCaret(Graphics* graphics, int x, int y)
    {
        graphics->setColor(getForegroundColor());
        graphics->drawLine(x, y, x, y + getFont()->getHeight());
    }

    void TextBox::mousePressed(MouseEvent& mouseEvent)
    {
        if (mouseEvent.getButton() == MouseEvent::Left)
        {
            mText->setCaretPosition(mouseEvent.getX(), mouseEvent.getY(), getFont());
            mouseEvent.consume();
        }
    }

    void TextBox::mouseDragged(MouseEvent& mouseEvent)
    {
        mouseEvent.consume();
    }

    void TextBox::keyPressed(KeyEvent& keyEvent)
    {
        Key key = keyEvent.getKey();

        if (key.getValue() == Key::Left)
            mText->setCaretPosition(mText->getCaretPosition() - 1);
        
        else if (key.getValue() == Key::Right)
            mText->setCaretPosition(mText->getCaretPosition() + 1);

        else if (key.getValue() == Key::Down)
            mText->setCaretRow(mText->getCaretRow() + 1);

        else if (key.getValue() == Key::Up)
            mText->setCaretRow(mText->getCaretRow() - 1);

        else if (key.getValue() == Key::Home)
            mText->setCaretColumn(0);

        else if (key.getValue() == Key::End)
            mText->setCaretColumn(mText->getNumberOfCharacters(mText->getCaretRow()));

        else if (key.getValue() == Key::Enter && mEditable)
            mText->insert('\n');

        else if (key.getValue() == Key::Backspace && mEditable)
            mText->remove(-1);

        else if (key.getValue() == Key::Delete && mEditable)
            mText->remove(1);

        else if(key.getValue() == Key::PageUp)
        {
            Widget* par = getParent();

            if (par != NULL)
            {
                int rowsPerPage = par->getChildrenArea().height / getFont()->getHeight();
                mText->setCaretRow(mText->getCaretRow() - rowsPerPage);
            }
        }

        else if(key.getValue() == Key::PageDown)
        {
            Widget* par = getParent();

            if (par != NULL)
            {
                int rowsPerPage = par->getChildrenArea().height / getFont()->getHeight();
                mText->setCaretRow(mText->getCaretRow() + rowsPerPage);
            }
        }

        else if(key.getValue() == Key::Tab && mEditable)
        {
            mText->insert(' ');
            mText->insert(' ');
            mText->insert(' ');
            mText->insert(' ');
        }

        else if (key.isCharacter() && mEditable)
            mText->insert(key.getValue());

        adjustSize();
        scrollToCaret();

        keyEvent.consume();
    }

    void TextBox::adjustSize()
    {
        const Rectangle& dim = mText->getDimension(getFont());
        setSize(dim.width, dim.height);
    }

    void TextBox::setCaretPosition(unsigned int position)
    {
        mText->setCaretPosition(position);
    }

    unsigned int TextBox::getCaretPosition() const
    {
        return mText->getCaretPosition();
    }

    void TextBox::setCaretRowColumn(int row, int column)
    {
        mText->setCaretRow(row);
        mText->setCaretColumn(column);
    }

    void TextBox::setCaretRow(int row)
    {
        mText->setCaretRow(row);
    }

    unsigned int TextBox::getCaretRow() const
    {
        return mText->getCaretRow();
    }

    void TextBox::setCaretColumn(int column)
    {
        mText->setCaretColumn(column);
    }

    unsigned int TextBox::getCaretColumn() const
    {
        return mText->getCaretColumn();
    }

    std::string TextBox::getTextRow(int row) const
    {     
        return mText->getRow(row);
    }

    void TextBox::setTextRow(int row, const std::string& text)
    {
        mText->setRow(row, text);
        adjustSize();
    }

    unsigned int TextBox::getNumberOfRows() const
    {
        return mText->getNumberOfRows();
    }

    std::string TextBox::getText() const
    {
        return mText->getContent();
    }

    void TextBox::fontChanged()
    {
        adjustSize();
    }

    void TextBox::scrollToCaret()
    {
        showPart(mText->getCaretDimension(getFont()));
    }

    void TextBox::setEditable(bool editable)
    {
        mEditable = editable;
    }

    bool TextBox::isEditable() const
    {
        return mEditable;
    }

    void TextBox::addRow(const std::string &row)
    {
        mText->addRow(row);
        adjustSize();
    }

    bool TextBox::isOpaque()
    {
        return mOpaque;
    }

    void TextBox::setOpaque(bool opaque)
    {
        mOpaque = opaque;
    }
}
