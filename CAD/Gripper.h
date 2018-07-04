// Gripper.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined Gripper_HEADER
#define Gripper_HEADER

#include "HeeksObj.h"
#include "GripperTypes.h"
#include "GripData.h"

class Gripper: public HeeksObj{
public:
	GripData m_data;
	HeeksObj* m_gripper_parent;

	Gripper(const GripData& data, HeeksObj* parent);
	virtual ~Gripper(){}

	// HeeksObj's virtual functions
	int GetType()const{return GripperType;}
	void glCommands(bool select, bool marked, bool no_color);
	const wchar_t* GetTypeString(void)const{return L"Gripper";}
	void Transform(const geoff_geometry::Matrix &m);

	//Gripper's virtual functions
//	virtual wxCursor* get_gripper_cursor(){return NULL;}
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	virtual bool OnGripperGrabbed(const std::list<HeeksObj*>& list, bool show_grippers_on_drag, double* from){return false;}
	virtual void OnGripperMoved(double* from, const double* to){}
	virtual void OnGripperReleased(const double* from, const double* to){}
};

#endif
