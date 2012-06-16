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

#include "TuioPoint.h"

using namespace TUIO;

TuioPoint::TuioPoint (float xp, float yp) {
	xpos = xp;
	ypos = yp;
	currentTime = TuioTime::getSessionTime();
	startTime = currentTime;
}

TuioPoint::TuioPoint (TuioTime ttime, float xp, float yp) {
	xpos = xp;
	ypos = yp;
	currentTime = ttime;
	startTime = currentTime;
}

TuioPoint::TuioPoint (TuioPoint *tpoint) {
	xpos = tpoint->getX();
	ypos = tpoint->getY();
	currentTime = TuioTime::getSessionTime();
	startTime = currentTime;
}

void TuioPoint::update (TuioPoint *tpoint) {
	xpos = tpoint->getX();
	ypos = tpoint->getY();
}

void TuioPoint::update (float xp, float yp) {
	xpos = xp;
	ypos = yp;
}		

void TuioPoint::update (TuioTime ttime, float xp, float yp) {
	xpos = xp;
	ypos = yp;
	currentTime = ttime;
}


float TuioPoint::getX() const{ 
	return xpos;
}

float TuioPoint::getY() const{
	return ypos;
}

float TuioPoint::getDistance(float xp, float yp) const{
	float dx = xpos-xp;
	float dy = ypos-yp;
	return sqrtf(dx*dx+dy*dy);
}

float TuioPoint::getScreenDistance(float xp, float yp, int w, int h) const{
	float dx = w*xpos-w*xp;
	float dy = h*ypos-h*yp;
	return sqrtf(dx*dx+dy*dy);
}

float TuioPoint::getDistance(TuioPoint *tpoint) const{
	return getDistance(tpoint->getX(),tpoint->getY());
}


float TuioPoint::getAngle(float xp, float yp) const{
	float side = xpos-xp;
	float height = ypos-yp;
	float distance = getDistance(xp,yp);
	
	float angle = (float)(asin(side/distance)+M_PI/2);
	if (height<0) angle = 2.0f*(float)M_PI-angle;
	
	return angle;
}

float TuioPoint::getAngle(TuioPoint *tpoint) const{
	return getAngle(tpoint->getX(),tpoint->getY());
}

float TuioPoint::getAngleDegrees(float xp, float yp) const{
	return ((getAngle(xp,yp)/(float)M_PI)*180.0f);
}

float TuioPoint::getAngleDegrees(TuioPoint *tpoint) const{
	return ((getAngle(tpoint)/(float)M_PI)*180.0f);
}

int TuioPoint::getScreenX(int width) const{ 
	return (int)floor(xpos*width+0.5f);
}

int TuioPoint::getScreenY(int height) const{
	return (int)floor(ypos*height+0.5f);
}

TuioTime TuioPoint::getTuioTime() const{ 
	return currentTime;
}

TuioTime TuioPoint::getStartTime() const{
	return startTime;
}

