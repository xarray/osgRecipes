/*
 TUIO C++ Library - part of the reacTIVision project
 http://reactivision.sourceforge.net/
 
 Copyright (c) 2005-2009 Martin Kaltenbrunner <martin@tuio.org>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "TuioCursor.h"

using namespace TUIO;


TuioCursor::TuioCursor (TuioTime ttime, long si, int ci, float xp, float yp):TuioContainer(ttime,si,xp,yp) {
	cursor_id = ci;
}

TuioCursor::TuioCursor (long si, int ci, float xp, float yp):TuioContainer(si,xp,yp) {
	cursor_id = ci;
}

TuioCursor::TuioCursor (TuioCursor *tcur):TuioContainer(tcur) {
	cursor_id = tcur->getCursorID();
}


int TuioCursor::getCursorID() const{
	return cursor_id;
};

