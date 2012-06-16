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

#include "TuioBlob.h"
using namespace TUIO;

TuioBlob::TuioBlob (TuioTime ttime, long si, int bi, float xp, float yp, float a, float w, float h, float f):TuioContainer(ttime, si, xp, yp) {
	blob_id = bi;
	angle = a;
	width = w;
	height = h;
	area = f;
	rotation_speed = 0.0f;
	rotation_accel = 0.0f;
}

TuioBlob::TuioBlob (long si, int bi, float xp, float yp, float a, float  w, float h, float f):TuioContainer(si, xp, yp) {
	blob_id = bi;
	angle = a;
	width = w;
	height = h; 
	area = f;
	rotation_speed = 0.0f;
	rotation_accel = 0.0f;
}

TuioBlob::TuioBlob (TuioBlob *tblb):TuioContainer(tblb) {
	blob_id = tblb->getBlobID();
	angle = tblb->getAngle();
	width = tblb->getWidth();
	height = tblb->getHeight();
	area = tblb->getArea();
	rotation_speed = 0.0f;
	rotation_accel = 0.0f;
}

int TuioBlob::getBlobID() const{
	return blob_id;
}

void TuioBlob::update (TuioTime ttime, float xp, float yp, float a, float w, float h, float f, float xs, float ys, float rs, float ma, float ra) {
	TuioContainer::update(ttime,xp,yp,xs,ys,ma);
	angle = a;
	width = w;
	height = h;
	area = f;
	rotation_speed = rs;
	rotation_accel = ra;
	if ((rotation_accel!=0) && (state==TUIO_STOPPED)) state = TUIO_ROTATING;
}

void TuioBlob::update (float xp, float yp, float a, float w, float h, float f, float xs, float ys, float rs, float ma, float ra) {
	TuioContainer::update(xp,yp,xs,ys,ma);
	angle = a;
	width = w;
	height = h;
	area = f;
	rotation_speed = rs;
	rotation_accel = ra;
	if ((rotation_accel!=0) && (state==TUIO_STOPPED)) state = TUIO_ROTATING;
}

void TuioBlob::update (TuioTime ttime, float xp, float yp, float a, float w, float h, float f) {
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
	
	width = w;
	height = h;
	area = f;
	
	rotation_speed = (float)da/dt;
	rotation_accel =  (rotation_speed - last_rotation_speed)/dt;
	
	if ((rotation_accel!=0) && (state==TUIO_STOPPED)) state = TUIO_ROTATING;
}

void TuioBlob::stop (TuioTime ttime) {
	update(ttime,xpos,ypos,angle,width,height,area);
}

void TuioBlob::update (TuioBlob *tblb) {
	TuioContainer::update(tblb);
	angle = tblb->getAngle();
	width = tblb->getWidth();
	height = tblb->getHeight();
	area = tblb->getArea();
	rotation_speed = tblb->getRotationSpeed();
	rotation_accel = tblb->getRotationAccel();
	if ((rotation_accel!=0) && (state==TUIO_STOPPED)) state = TUIO_ROTATING;
}

float TuioBlob::getWidth() const{ 
	return width;
}

float TuioBlob::getHeight() const{ 
	return height;
}

int TuioBlob::getScreenWidth(int w) const{ 
	return (int)(w*width);
}

int TuioBlob::getScreenHeight(int h) const{ 
	return (int)(h*height);
}

float TuioBlob::getArea() const{ 
	return area;
}

float TuioBlob::getAngle() const{
	return angle;
}

float TuioBlob::getAngleDegrees() const{ 
	return (float)(angle/M_PI*180);
}

float TuioBlob::getRotationSpeed() const{ 
	return rotation_speed;
}

float TuioBlob::getRotationAccel() const{
	return rotation_accel;
}

bool TuioBlob::isMoving() const{ 
	if ((state==TUIO_ACCELERATING) || (state==TUIO_DECELERATING) || (state==TUIO_ROTATING)) return true;
	else return false;
}
