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

#include "TuioObject.h"

using namespace TUIO;

TuioObject::TuioObject (TuioTime ttime, long si, int sym, float xp, float yp, float a):TuioContainer(ttime, si, xp, yp) {
	symbol_id = sym;
	angle = a;
	rotation_speed = 0.0f;
	rotation_accel = 0.0f;
}

TuioObject::TuioObject (long si, int sym, float xp, float yp, float a):TuioContainer(si, xp, yp) {
	symbol_id = sym;
	angle = a;
	rotation_speed = 0.0f;
	rotation_accel = 0.0f;
}

TuioObject::TuioObject (TuioObject *tobj):TuioContainer(tobj) {
	symbol_id = tobj->getSymbolID();
	angle = tobj->getAngle();
	rotation_speed = 0.0f;
	rotation_accel = 0.0f;
}

void TuioObject::update (TuioTime ttime, float xp, float yp, float a, float xs, float ys, float rs, float ma, float ra) {
	TuioContainer::update(ttime,xp,yp,xs,ys,ma);
	angle = a;
	rotation_speed = rs;
	rotation_accel = ra;
	if ((rotation_accel!=0) && (state==TUIO_STOPPED)) state = TUIO_ROTATING;
}


void TuioObject::update (float xp, float yp, float a, float xs, float ys, float rs, float ma, float ra) {
	TuioContainer::update(xp,yp,xs,ys,ma);
	angle = a;
	rotation_speed = rs;
	rotation_accel = ra;
	if ((rotation_accel!=0) && (state==TUIO_STOPPED)) state = TUIO_ROTATING;
}

void TuioObject::update (TuioTime ttime, float xp, float yp, float a) {
	TuioPoint lastPoint = path.back();
	TuioContainer::update(ttime,xp,yp);
	
	TuioTime diffTime = currentTime - lastPoint.getTuioTime();
	float dt = diffTime.getTotalMilliseconds()/1000.0f;
	float last_angle = angle;
	float last_rotation_speed = rotation_speed;
	angle = a;
	
	double da = (angle-last_angle)/(2*M_PI);
	if (da > 0.75f) da-=1.0f;
	else if (da < -0.75f) da+=1.0f;
	
	rotation_speed = (float)da/dt;
	rotation_accel =  (rotation_speed - last_rotation_speed)/dt;
	
	if ((rotation_accel!=0) && (state==TUIO_STOPPED)) state = TUIO_ROTATING;
}

void TuioObject::stop (TuioTime ttime) {
	update(ttime,xpos,ypos,angle);
}

void TuioObject::update (TuioObject *tobj) {
	TuioContainer::update(tobj);
	angle = tobj->getAngle();
	rotation_speed = tobj->getRotationSpeed();
	rotation_accel = tobj->getRotationAccel();
	if ((rotation_accel!=0) && (state==TUIO_STOPPED)) state = TUIO_ROTATING;
}

int TuioObject::getSymbolID() const{ 
	return symbol_id;
}

float TuioObject::getAngle() const{
	return angle;
}

float TuioObject::getAngleDegrees() const{ 
	return (float)(angle/M_PI*180);
}

float TuioObject::getRotationSpeed() const{ 
	return rotation_speed;
}

float TuioObject::getRotationAccel() const{
	return rotation_accel;
}

bool TuioObject::isMoving() const{ 
	if ((state==TUIO_ACCELERATING) || (state==TUIO_DECELERATING) || (state==TUIO_ROTATING)) return true;
	else return false;
}
