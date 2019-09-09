#include "stdafx.h"


#include <Python.h>

#ifdef WIN32

#include "windows.h"
#include <GL/gl.h>
#include <GL/glu.h>

#else
#include </usr/include/GL/gl.h>
#include </usr/include/GL/glu.h>
#endif


#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

#include "../Geom/geometry.h"
#include "Viewport.h"
#include "ViewPoint.h"
#include "Observer.h"
#include "strconv.h"
#include "StlSolid.h"
#include "Property.h"
#include "HXml.h"
#include "Material.h"
#include "MarkedList.h"
#include "PropertyChange.h"
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#include "HPoint.h"
#include "InputMode.h"
#include "SelectMode.h"
#include "MagDragWindow.h"
#include "ViewRotating.h"
#include "ViewZooming.h"
#include "ViewPanning.h"
#include "KeyEvent.h"
#include "LineArcDrawing.h"
#include "MarkedObject.h"
#include "ConversionTools.h"
#include "PyWrapper.h"
#include "PyBaseObject.h"

void OnExit()
{
	theApp->OnExit();
}

void AddPropertyToPythonList(Property* p, boost::python::list& list)
{
	if (PropertyCheck* o = dynamic_cast<PropertyCheck*>(p)){ list.append(boost::python::ptr<Property*>(p)); return;	}
	if (PropertyChoice* o = dynamic_cast<PropertyChoice*>(p)){ list.append(o); return; }
	if (PropertyColor* o = dynamic_cast<PropertyColor*>(p)){ list.append(o); return; }
	if (PropertyDoubleScaled* o = dynamic_cast<PropertyDoubleScaled*>(p)){ list.append(o); return; }
	if (PropertyDoubleLimited* o = dynamic_cast<PropertyDoubleLimited*>(p)){ list.append(o); return; }
	if (PropertyString* o = dynamic_cast<PropertyString*>(p)){ list.append(o); return; }
	if (PropertyStringReadOnly* o = dynamic_cast<PropertyStringReadOnly*>(p)){ list.append(o); return; }
	if (PropertyFile* o = dynamic_cast<PropertyFile*>(p)){ list.append(o); return; }
	if (PropertyInt* o = dynamic_cast<PropertyInt*>(p)){ list.append(o); return; }
	if (PropertyLength* o = dynamic_cast<PropertyLength*>(p)){ list.append(o); return; }
	if (PropertyLengthWithKillGLLists* o = dynamic_cast<PropertyLengthWithKillGLLists*>(p)){ list.append(o); return; }
	if (PropertyList* o = dynamic_cast<PropertyList*>(p)){ list.append(o); return; }
	if (PropertyObjectTitle* o = dynamic_cast<PropertyObjectTitle*>(p)){ list.append(o); return; }
	if (PropertyObjectColor* o = dynamic_cast<PropertyObjectColor*>(p)){ list.append(o); return; }
}

void AddObjectToPythonList(HeeksObj* object, boost::python::list& list)
{
	switch (object->GetType())
	{
	case SketchType:
		list.append(boost::python::pointer_wrapper<CSketch*>((CSketch*)object));
		break;
	case StlSolidType:
		list.append(boost::python::pointer_wrapper<CStlSolid*>((CStlSolid*)object));
		break;
#if 0
	case CircleType:
		list.append(boost::python::pointer_wrapper<HCircle*>((HCircle*)object));
		break;
#endif
	default:
		list.append(boost::python::pointer_wrapper<HeeksObj*>((HeeksObj*)object));
		break;
	}
}

class ObserverWrap : public Observer, public cad_wrapper<Observer>
{
public:
	ObserverWrap() :Observer(){}

	void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)override
	{
		//		if (added && added->size() > 0)
		{
			boost::python::list added_list;
			boost::python::list removed_list;
			boost::python::list modified_list;
			if (added && added->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *added) {
					AddObjectToPythonList(o, added_list);
				}
				CallVoidReturn("OnAdded", added_list);
			}
			if (removed && removed->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *removed) {
					AddObjectToPythonList(o, removed_list);
				}
				CallVoidReturn("OnRemoved", removed_list);
			}
			if (modified && modified->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *modified) {
					AddObjectToPythonList(o, modified_list);
				}
				CallVoidReturn("OnModified", modified_list);
			}
		}
	}

	void WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed)
	{
		boost::python::list added_list;
		boost::python::list removed_list;
		if (added && added->size() > 0)
		{
			BOOST_FOREACH(HeeksObj* o, *added) {
				AddObjectToPythonList(o, added_list);
			}
		}
		if (removed && removed->size() > 0)
		{
			BOOST_FOREACH(HeeksObj* o, *removed) {
				AddObjectToPythonList(o, removed_list);
			}
		}
		CallVoidReturn("OnSelectionChanged", added_list, removed_list);

	}
};

static std::wstring str_for_input_mode;

class InputModeWrap : public CInputMode, public boost::python::wrapper<CInputMode>
{
public:
	InputModeWrap() :CInputMode(){}

	const wchar_t* GetTitle()override
	{
		if (boost::python::override f = this->get_override("GetTitle"))
		{
			std::string s = f();
			str_for_input_mode = Ctt(s.c_str());
			return str_for_input_mode.c_str();
		}
		return NULL;
	}

	void OnKeyDown(KeyEvent& e)override
	{
		if (boost::python::override f = this->get_override("OnKeyDown"))
		{
			f(e);
		}
		else
			CInputMode::OnKeyDown(e);
	}

	void OnKeyUp(KeyEvent& e)override
	{
		if (boost::python::override f = this->get_override("OnKeyUp"))
		{
			f(e);
		}
		else
			CInputMode::OnKeyUp(e);
	}

};

class DrawingWrap : public Drawing, public cad_wrapper<Drawing>
{
public:
	DrawingWrap() :Drawing(){}

	void AddPoint()override
	{
		if (boost::python::override f = this->get_override("AddPoint"))
		{
			f();
		}
		else
			Drawing::AddPoint();
	}

	bool calculate_item(DigitizedPoint &end)override
	{
		if (boost::python::override f = this->get_override("CalculateItem"))
		{
			bool result = f(end);
			return result;
		}
		else
			return Drawing::calculate_item(end);
	}

	bool is_an_add_level(int level)override
	{
		if (boost::python::override f = this->get_override("IsAnAddLevel"))
		{
			bool result = f(level);
			return result;
		}
		return Drawing::is_an_add_level(level);
	}

	int number_of_steps()override
	{
		if (boost::python::override f = this->get_override("NumberOfSteps"))
		{
			int result = f();
			return result;
		}
		return Drawing::number_of_steps();
	}

	HeeksObj* GetOwnerForDrawingObjects()override
	{
		std::pair<bool, HeeksObj*> result = CallReturnHeeksObj("GetOwnerForDrawingObjects");
		if (result.first)
			return result.second;
		return Drawing::GetOwnerForDrawingObjects();
	}

	HeeksObj* TempObject(){
		return Drawing::TempObject();
	}

	void ClearObjectsMade(){
		Drawing::ClearObjectsMade();
	}

	void AddToTempObjects(HeeksObj* object){
		return Drawing::AddToTempObjects(object);
	}

};

void CadReset()
{
	theApp->Reset();
}

bool CadOpenFile(std::wstring fp)
{
	return theApp->OpenFile(fp.c_str(), false);
}

void CadImport(const std::wstring &filepath, HeeksObj* paste_into = NULL)
{
	theApp->OpenFile(filepath.c_str(), true, paste_into);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(CadImportOverloads, CadImport, 1, 2)

bool CadSaveFile(std::wstring fp)
{
	return theApp->SaveFile(fp.c_str());
}

bool SaveObjects(std::wstring fp, boost::python::list &list)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	return theApp->SaveFile(fp.c_str(), &o_list);
}

void RegisterObserver(Observer* observer)
{
	theApp->RegisterObserver(observer);
}


std::list<PyObject*>  repaint_callbacks;

void PythonOnRepaint(bool soon = false)
{
	for (std::list<PyObject*>::iterator It = repaint_callbacks.begin(); It != repaint_callbacks.end(); It++)
	{
		CallPythonCallback(*It);
	}
}

BOOST_PYTHON_FUNCTION_OVERLOADS(PythonOnRepaintOverloads, PythonOnRepaint, 0, 1)

void RegisterOnRepaint(PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	repaint_callbacks.push_back(callback);
}

PyObject* message_box_callback = NULL;

void PythonOnMessageBox(const wchar_t* message)
{
	static bool in_message_box = false;
	if (in_message_box)
		return;
	in_message_box = true;
	if (message_box_callback)
		CallPythonCallback(message_box_callback);
	in_message_box = false;
}

void RegisterMessageBoxCallback(PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	message_box_callback = callback;
}

PyObject* context_menu_callback = NULL;

void PythonOnContextMenu()
{
	if (context_menu_callback)
		CallPythonCallback(context_menu_callback);
}

void SetContextMenuCallback(PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	context_menu_callback = callback;
}

std::wstring GetResFolder()
{
	return theApp->m_res_folder;
}

void SetResFolder(std::wstring str)
{
	theApp->m_res_folder = str;
}

void CadMessageBox(std::wstring str)
{
	theApp->DoMessageBox(str.c_str());
}

std::wstring BaseObjectGetIconFilePath(BaseObject& object)
{
	return std::wstring(object.ObjList::GetIconFilePath());
}

std::wstring GetTitleFromHeeksObj(const HeeksObj* object)
{
	const wchar_t* s = object->GetShortStringOrTypeString();
	if (s == NULL)return L"";
	return std::wstring(s);
}

std::wstring BaseObjectGetTitle(const BaseObject& object)
{
	return object.ObjList::GetShortString();
}

HeeksColor BaseObjectGetColor(const BaseObject& object)
{
	return *(object.ObjList::GetColor());
}

int PropertyGetInt(Property& property)
{
	return property.GetInt();
}


std::wstring HeeksObjGetTitle(const HeeksObj& object)
{
	return GetTitleFromHeeksObj(&object);
	//return object.GetShortString();
}

std::wstring HeeksObjGetTypeString(HeeksObj& object)
{
	return std::wstring(object.GetTypeString());
}

std::wstring HeeksObjGetIconFilePath(HeeksObj& object)
{
	return std::wstring(object.GetIconFilePath());
}

void HeeksObjReadFromXML(HeeksObj& object)
{
	object.ReadFromXML(BaseObject::m_cur_element);
}

void HeeksObjReadObjectXml(HeeksObj& object)
{
	object.HeeksObj::ReadFromXML(BaseObject::m_cur_element);
}

void HeeksObjWriteToXML(HeeksObj& object)
{
	object.WriteToXML(BaseObject::m_cur_element);
}

void HeeksObjWriteObjectToXML(HeeksObj& object)
{
	object.HeeksObj::WriteToXML(BaseObject::m_cur_element);
}

boost::python::list HeeksObjGetProperties(HeeksObj& object) {
	boost::python::list return_list;
	std::list<Property*> p_list;
	object.GetProperties(&p_list);
	for (std::list<Property*>::iterator It = p_list.begin(); It != p_list.end(); It++)
	{
		Property* property = *It;
		//AddPropertyToPythonList(property, return_list);
		return_list.append(boost::python::ptr<Property*>(property));
	}
	return return_list;
}

boost::python::list HeeksObjGetBaseProperties(BaseObject& object) {
	boost::python::list return_list;
	std::list<Property*> p_list;
	object.ObjList::GetProperties(&p_list);
	for (std::list<Property*>::iterator It = p_list.begin(); It != p_list.end(); It++)
	{
		Property* property = *It;
		//AddPropertyToPythonList(property, return_list);
		return_list.append(boost::python::ptr<Property*>(property));
	}
	return return_list;
}

boost::python::list BaseObjectGetChildren(BaseObject& object) {
	boost::python::list return_list;
	HeeksObj* obj = object.GetFirstChild();
	while (obj)
	{
		return_list.append(boost::python::ptr<HeeksObj*>(obj));
		obj = object.GetNextChild();
	}

	return return_list;
}


static double GetLines_pixels_per_mm = 0.0;
static boost::python::list return_list_ForGetLines;


static void CallbackForGetLines(const double *p, bool start)
{
	return_list_ForGetLines.append(boost::python::make_tuple(start, p[0], p[1], p[2]));
}

void SetGetLinesPixelsPerMm(double pixels_per_mm)
{
	GetLines_pixels_per_mm = pixels_per_mm;
}

CInputMode* GetSelectMode()
{
	return theApp->m_select_mode;
}

CInputMode* GetMagnification()
{
	return theApp->magnification;
}

CInputMode* GetViewRotating()
{
	return theApp->viewrotating;
}

CInputMode* GetViewZooming()
{
	return theApp->viewzooming;
}

CInputMode* GetViewPanning()
{
	return theApp->viewpanning;
}

void SetInputMode(CInputMode* input_mode)
{
	theApp->SetInputMode(input_mode);
}

CInputMode* GetInputMode()
{
	return theApp->input_mode_object;
}

void SetLineArcDrawing()
{
	line_strip.drawing_mode = LineDrawingMode;
	theApp->SetInputMode(&line_strip);
}

void SetCircles3pDrawing()
{
	line_strip.drawing_mode = CircleDrawingMode;
	line_strip.circle_mode = ThreePointsCircleMode;
	theApp->SetInputMode(&line_strip);
}

void SetCircles2pDrawing()
{
	line_strip.drawing_mode = CircleDrawingMode;
	line_strip.circle_mode = CentreAndPointCircleMode;
	theApp->SetInputMode(&line_strip);
}

void SetCircle1pDrawing()
{
	line_strip.drawing_mode = CircleDrawingMode;
	line_strip.circle_mode = CentreAndRadiusCircleMode;
	theApp->SetInputMode(&line_strip);
}

void SetEllipseDrawing()
{
	line_strip.drawing_mode = EllipseDrawingMode;
	theApp->SetInputMode(&line_strip);
}

void SetILineDrawing()
{
	line_strip.drawing_mode = ILineDrawingMode;
	theApp->SetInputMode(&line_strip);
}

HeeksObj* NewPoint(const Point3d& p)
{
	HPoint* point = new HPoint(p, &theApp->current_color);
	return point;
}

boost::python::list HeeksObjGetLines(HeeksObj& object)
{
	return_list_ForGetLines = boost::python::list();
	object.GetSegments(CallbackForGetLines, GetLines_pixels_per_mm);
	return return_list_ForGetLines;
}

Point3d HeeksObjGetStartPoint(HeeksObj& object)
{
	Point3d p(0,0,0);
	object.GetStartPoint(p);
	return p;
}

Point3d HeeksObjGetEndPoint(HeeksObj& object)
{
	Point3d p(0, 0, 0);
	object.GetEndPoint(p);
	return p;
}

std::wstring PropertyGetShortString(Property& p)
{
	return std::wstring(p.GetShortString());
}

std::wstring PropertyGetString(Property& p)
{
	return std::wstring(p.GetString());
}

boost::python::list PropertyGetProperties(Property& p) {
	boost::python::list return_list;
	std::list<Property*> p_list;
	p.GetList(p_list);
	for (std::list<Property*>::iterator It = p_list.begin(); It != p_list.end(); It++)
	{
		Property* property = *It;
		return_list.append(boost::python::ptr<Property*>(property));
	}
	return return_list;
}

void ObjListClear(ObjList& objlist)
{
	objlist.Clear();
}

void ObjListAdd(ObjList& objlist, HeeksObj* object)
{
	objlist.Add(object, NULL);
}

void ObjListReadFromXML(ObjList& objlist)
{
	TiXmlElement* save_element = BaseObject::m_cur_element;
	objlist.ObjList::ReadFromXML(BaseObject::m_cur_element);
	BaseObject::m_cur_element = save_element;
}

void ObjListWriteToXML(ObjList& objlist)
{
	TiXmlElement* save_element = BaseObject::m_cur_element;
	objlist.ObjList::WriteToXML(BaseObject::m_cur_element);
	BaseObject::m_cur_element = save_element;
}

void ObjListCopyFrom(ObjList& objlist, HeeksObj* object)
{
	objlist.ObjList::CopyFrom(object);
}


HeeksColor PropertyGetColor(const Property& p)
{
	return p.GetColor();
}

boost::python::list PropertyGetChoices(const Property& p)
{
	boost::python::list return_list;
	std::list< std::wstring > choices;
	p.GetChoices(choices);
	for (std::list<std::wstring>::iterator It = choices.begin(); It != choices.end(); It++)
	{
		std::wstring& choice = *It;
		return_list.append(choice);
	}
	return return_list;
}

HeeksColor HeeksObjGetColor(const HeeksObj& object)
{
	return *(object.GetColor());
}

bool HeeksObjHasEdit(const HeeksObj& object)
{
	return false;
}

boost::python::tuple SketchGetStartPoint(CSketch &sketch)
{
	Point3d s(0.0, 0.0, 0.0);

	HeeksObj* last_child = NULL;
	HeeksObj* child = sketch.GetFirstChild();
	child->GetStartPoint(s);
	return boost::python::make_tuple(s.x, s.y, s.z);
}

boost::python::tuple SketchGetEndPoint(CSketch &sketch)
{
	Point3d s(0.0, 0.0, 0.0);

	HeeksObj* last_child = NULL;
	HeeksObj* child = sketch.GetFirstChild();
	child->GetEndPoint(s);
	return boost::python::make_tuple(s.x, s.y, s.z);
}

boost::python::list SketchSplit(CSketch& sketch) {
	boost::python::list olist;
	std::list<HeeksObj*> new_separate_sketches;
	sketch.ExtractSeparateSketches(new_separate_sketches, true);
	for (std::list<HeeksObj*>::iterator It = new_separate_sketches.begin(); It != new_separate_sketches.end(); It++)
	{
		HeeksObj* object = *It;
		AddObjectToPythonList(object, olist);
	}
	return olist;
}

double SketchGetCircleDiameter(CSketch& sketch)
{
	HeeksObj* span = sketch.GetFirstChild();
	if (span == NULL)
		return 0.0;

	if (span->GetType() == ArcType)
	{
		HArc* arc = (HArc*)span;
		return arc->m_radius * 2;
	}
	else if (span->GetType() == CircleType)
	{
#if 0 // to do
		HCircle* circle = (HCircle*)span;
		return circle->m_radius * 2;
#endif
	}
	return 0.0;
}

boost::python::tuple SketchGetCircleCentre(CSketch& sketch)
{
	HeeksObj* span = sketch.GetFirstChild();
	if (span == NULL)
		return boost::python::make_tuple(NULL);

	if (span->GetType() == ArcType)
	{
		HArc* arc = (HArc*)span;
		Point3d& C = arc->C;
		return boost::python::make_tuple(C.x, C.y, C.z);
	}
	else if (span->GetType() == CircleType)
	{
#if 0 // to do
		HCircle* circle = (HCircle*)span;
		const Point3d& C = circle->m_axis.Location();
		return boost::python::make_tuple(C.X(), C.Y(), C.Z());
#endif
	}

	return boost::python::make_tuple(NULL);
}

void SketchWriteDXF(CSketch& sketch, std::wstring filepath)
{
	std::list<HeeksObj*> objects;
	objects.push_back(&sketch);
	theApp->SaveDXFFile(objects, filepath.c_str());
}

CCurve SketchGetCurve(CSketch& sketch)
{
	// this makes a Curve object, compatible with the geom library
	CArea area = ObjectToArea(&sketch);
	if (area.m_curves.size() == 0)
		return CCurve();
	return area.m_curves.front();
}

CArea SketchGetArea(CSketch& sketch)
{
	return ObjectToArea(&sketch);
}

void BeginTriangles()
{
	if (!BaseObject::triangles_begun)
	{
		if (BaseObject::lines_begun)
		{
			glEnd();
			BaseObject::lines_begun = false;
		}
		glBegin(GL_TRIANGLES);
		BaseObject::triangles_begun = true;
	}
}

void BeginLines()
{
	if (!BaseObject::lines_begun)
	{
		if (BaseObject::triangles_begun)
		{
			glEnd();
			BaseObject::triangles_begun = false;
		}
		glBegin(GL_LINE_STRIP);
		BaseObject::lines_begun = true;
	}
}

void EndLinesOrTriangles()
{
	if (BaseObject::triangles_begun)
	{
		glEnd();
		BaseObject::triangles_begun = false;
	}
	else if (BaseObject::lines_begun)
	{
		glEnd();
		BaseObject::lines_begun = false;
	}
}

void DrawTriangle(double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7, double x8)
{
	BeginTriangles();

	Point3d p0(x0, x1, x2);
	Point3d p1(x3, x4, x5);
	Point3d p2(x6, x7, x8);
	Point3d v1(p0, p1);
	Point3d v2(p0, p2);
	try
	{
		Point3d norm = (v1 ^ v2).Normalized();
		glNormal3d(norm.x, norm.y, norm.z);
	}
	catch (...)
	{
	}
	glVertex3d(x0, x1, x2);
	glVertex3d(x3, x4, x5);
	glVertex3d(x6, x7, x8);

}

void DrawEnableLighting()
{
	glEnable(GL_LIGHTING);
}

void DrawDisableLighting()
{
	glDisable(GL_LIGHTING);
}

void DrawLine(double x0, double x1, double x2, double x3, double x4, double x5)
{
	BeginLines();
	glVertex3d(x0, x1, x2);
	glVertex3d(x3, x4, x5);
}

void DrawColor(const HeeksColor& col)
{
	col.glColor();
}

static unsigned char cross16[32] = { 0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01 };
static unsigned char bmp16[10] = { 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x07, 0xe0, 0x03, 0xc0 };

void DrawSymbol(int type, double x, double y, double z)
{
	glRasterPos3d(x, y, z);
	switch (type)
	{
	case 0:
		glBitmap(16, 16, 8, 8, 10.0, 0.0, cross16);
		break;
	case 1:
		glBitmap(16, 5, 8, 3, 10.0, 0.0, bmp16);
		break;
	}
}

void glVertexPoint3d(const Point3d& p)
{
	glVertex3dv(p.getBuffer());
}

void GlLineWidth(int width)
{
	glLineWidth(width);
}

static unsigned int current_display_list_started = 0;

unsigned int DrawNewList()
{
	if (current_display_list_started != 0)
		return 0;
	current_display_list_started = glGenLists(1);
	glNewList(current_display_list_started, GL_COMPILE_AND_EXECUTE);
	return current_display_list_started;
}


void DrawEndList()
{
	if (current_display_list_started != 0)
		glEndList();
	current_display_list_started = 0;
}

void DrawCallList(unsigned int display_list)
{
	if (display_list != 0)
		glCallList(display_list);
}

void DrawDeleteList(unsigned int display_list)
{
	glDeleteLists(display_list, 1);
}

boost::python::object GetObjectFromId(int type, int id) {
	// to do
	// this returns a list with the object in, because that works
	// but it should just return an object
	boost::python::list olist;
	HeeksObj* object = theApp->GetIDObject(type, id);
	if (object != NULL)
	{
		AddObjectToPythonList(object, olist);
		if (boost::python::len(olist) > 0)
		{
			return olist[0];
		}
	}

	return boost::python::object(); // None
}

void SetXmlValue(const std::wstring &name, PyObject* value)
{
	const char* sname = Ttc(name.c_str());
	if (PyLong_Check(value))
	{
		BaseObject::m_cur_element->SetAttribute(sname, PyLong_AsLong(value));
	}
	else if (PyFloat_Check(value))
	{
		BaseObject::m_cur_element->SetDoubleAttribute(sname, PyFloat_AsDouble(value));
	}
	else if (PyBool_Check(value))
	{
		BaseObject::m_cur_element->SetAttribute(sname, PyObject_IsTrue(value) ? 1:0);
	}
	else if (PyBytes_Check(value))
	{
		BaseObject::m_cur_element->SetAttribute(sname, PyBytes_AsString(value));
	}
	else if (PyUnicode_Check(value))
	{
		BaseObject::m_cur_element->SetAttribute(sname, PyBytes_AsString(PyUnicode_AsASCIIString(value)));
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "invalid value type");
		boost::python::throw_error_already_set();
	}
}

void BeginXmlChild(const std::wstring &child_name)
{
	std::string svalue(Ttc(child_name.c_str()));

	TiXmlElement *element = new TiXmlElement(Ttc(child_name.c_str()));
	BaseObject::m_cur_element->LinkEndChild(element);
	BaseObject::m_cur_element = element;
}

void EndXmlChild()
{
	if (BaseObject::m_cur_element)BaseObject::m_cur_element = BaseObject::m_cur_element->Parent()->ToElement();
}

boost::python::object GetXmlObject() {
	if (BaseObject::m_cur_element)
	{
		HeeksObj* object = theApp->ReadXMLElement(BaseObject::m_cur_element);
		if (object != NULL)
		{
			boost::python::list olist;
			AddObjectToPythonList(object, olist);
			if (boost::python::len(olist) > 0)
			{
				return olist[0];
			}
		}
	}

	return boost::python::object(); // None
}


std::wstring GetXmlValue(const std::wstring &name, const std::wstring &default_value = L"")
{
	if (BaseObject::m_cur_element != NULL)
	{
		const char* value = BaseObject::m_cur_element->Attribute(Ttc(name.c_str()));
		if (value != NULL)
			return std::wstring(Ctt(value));
	}
	return default_value;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlValueOverloads, GetXmlValue, 1, 2)

std::wstring GetXmlText()
{
	if (BaseObject::m_cur_element == NULL)return L"";
	const char* text = BaseObject::m_cur_element->GetText();
	if (text == NULL)return L"";
	return Ctt(text);
}

void SetXmlText(const std::wstring& str)
{
	if (BaseObject::m_cur_element == NULL)return;

	// add actual text as a child object
	TiXmlText* text = new TiXmlText(Ttc(str.c_str()));
	BaseObject::m_cur_element->LinkEndChild(text);
}

bool GetXmlBool(const std::wstring &name, bool default_value = false)
{
	if (BaseObject::m_cur_element == NULL)
		return default_value;

	const char* cname = Ttc(name.c_str());
	if (BaseObject::m_cur_element->Attribute(cname) == NULL)
		return default_value;
	int value;
	BaseObject::m_cur_element->Attribute(cname, &value);
	return value != 0;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlBoolOverloads, GetXmlBool, 1, 2)

int GetXmlInt(const std::wstring &name, int default_value = 0)
{
	if (BaseObject::m_cur_element == NULL)
		return default_value;

	const char* cname = Ttc(name.c_str());
	if (BaseObject::m_cur_element->Attribute(cname) == NULL)
		return default_value;
	int value;
	BaseObject::m_cur_element->Attribute(cname, &value);
	return value;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlIntOverloads, GetXmlInt, 1, 2)

double GetXmlFloat(const std::wstring &name, double default_value = 0)
{
	if (BaseObject::m_cur_element == NULL)
		return default_value;

	const char* cname = Ttc(name.c_str());
	if (BaseObject::m_cur_element->Attribute(cname) == NULL)
		return default_value;
	double value;
	BaseObject::m_cur_element->Attribute(cname, &value);
	return value;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlFloatOverloads, GetXmlFloat, 1, 2)

void ReturnFromXmlChild()
{
	if(BaseObject::m_cur_element)BaseObject::m_cur_element = BaseObject::m_cur_element->Parent()->ToElement();
}

boost::python::object GetFirstXmlChild()
{
	if (BaseObject::m_cur_element)
	{
		TiXmlElement* first_child = BaseObject::m_cur_element->FirstChildElement();
		if (first_child == NULL)
		{
			// leave current object as it is, but return None
			return boost::python::object(); // None
		}
		else
		{
			// set current to the first child and return it
			BaseObject::m_cur_element = first_child;
			return boost::python::object(std::wstring(Ctt(BaseObject::m_cur_element->Value())));
		}
	}
	return boost::python::object(); // None
}

boost::python::object GetNextXmlChild()
{
	if (BaseObject::m_cur_element)
	{
		TiXmlElement* next_sibling = BaseObject::m_cur_element->NextSiblingElement();
		if (next_sibling == NULL)
		{
			// set current element to be the parent, but return None
			BaseObject::m_cur_element = BaseObject::m_cur_element->Parent()->ToElement();
			return boost::python::object(); // None
		}
		else
		{
			// set current to the next sibling and return it
			BaseObject::m_cur_element = next_sibling;
			return boost::python::object(std::wstring(Ctt(BaseObject::m_cur_element->Value())));
		}
	}
	return boost::python::object(); // None
}

void OpenXmlFile(const std::wstring &filepath, HeeksObj* paste_into = NULL, HeeksObj* paste_before = NULL)
{
	theApp->OpenXMLFile(filepath.c_str(), paste_into, paste_before, true);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(OpenXMLFileOverloads, OpenXmlFile, 1, 3)

class PropertyWrap : public Property, public cad_wrapper<Property>
{
	int m_type;
public:
	PropertyWrap() :Property(), m_type(InvalidPropertyType){}
	PropertyWrap(int type, const std::wstring& title, HeeksObj* object) :Property(object, title.c_str()), m_type(type){ m_editable = true; }
	int get_property_type()const{ return m_type; }
	int GetInt()const override
	{
		std::pair<bool, int> result = CallReturnInt("GetInt");
		if (result.first)
			return result.second;
		return Property::GetInt();
	}
	bool GetBool()const override
	{
		std::pair<bool, bool> result = CallReturnBool("GetBool");
		if (result.first)
			return result.second;
		return Property::GetBool();
	}
	double GetDouble()const override
	{
		std::pair<bool, double> result = CallReturnDouble("GetFloat");
		if (result.first)
			return result.second;
		return Property::GetDouble();
	}
	const wchar_t* GetString()const override
	{
		std::pair<bool, std::wstring> result = CallReturnWString("GetString");
		if (result.first)
			return result.second.c_str();
		return Property::GetString();
	}
	void Set(bool value)override
	{
		CallVoidReturn("SetBool", value);
	}
	void Set(int value)override
	{
		CallVoidReturn("SetInt", value);
	}
	void Set(double value)override
	{
		CallVoidReturn("SetFloat", value);
	}
	void Set(const wchar_t* value)override
	{
		CallVoidReturn("SetString", std::wstring(value));
	}
	Property *MakeACopy(void)const{ return new PropertyWrap(*this); }
};


int PropertyWrapGetInt(PropertyWrap& property)
{
	return property.GetInt();
}


class UndoableWrap : public Undoable, public cad_wrapper<Undoable>
{
public:
	void Run(bool redo) override
	{
		CallVoidReturn("Run", redo);
	}
	const wchar_t* GetTitle() override
	{
		std::pair<bool, std::wstring> result = CallReturnWString("GetTitle");
		if (result.first)
			return result.second.c_str();
		return L"";
	}
	void RollBack()
	{
		CallVoidReturn("RollBack");
	}
};



void StlSolidWriteSTL(CStlSolid& solid, double tolerance, std::wstring filepath)
{
	std::list<HeeksObj*> list;
	list.push_back(&solid);
	theApp->SaveSTLFileAscii(list, filepath.c_str(), tolerance);
}

int StlSolidNumTriangles(CStlSolid& solid)
{
	return solid.m_list.size();
}

static boost::shared_ptr<CStlSolid> initStlSolid(const std::wstring& title, const HeeksColor* color)
{
	return boost::shared_ptr<CStlSolid>(new CStlSolid(title.c_str(), color));
}

static boost::shared_ptr<PropertyStringReadOnly> initPropertyStringReadOnly(const std::wstring& title, const std::wstring& value)
{
	return boost::shared_ptr<PropertyStringReadOnly>(new PropertyStringReadOnly(title.c_str(), value.c_str()));
}

static boost::shared_ptr<HPoint> initHPoint(const Point3d& p)
{
	return boost::shared_ptr<HPoint>(new HPoint(p, &theApp->current_color));
}

boost::python::list GetSelectedObjects() {
	boost::python::list slist;
	for (std::list<HeeksObj *>::iterator It = theApp->m_marked_list->list().begin(); It != theApp->m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		AddObjectToPythonList(object, slist);
	}
	return slist;
}

boost::python::list GetSelectionProperties()
{
	std::list<Property *> list;
	theApp->m_marked_list->GetProperties(&list);
	boost::python::list return_list;
	for (std::list<Property *>::iterator It = list.begin(); It != list.end(); It++)
	{
		Property* prop = *It;
		return_list.append(boost::python::ptr<Property*>(prop));
	}
	return return_list;
}



unsigned int GetNumSelected()
{
	return theApp->m_marked_list->list().size();
}

boost::python::list GetObjects() {
	boost::python::list olist;
	for (HeeksObj *object = theApp->GetFirstChild(); object; object = theApp->GetNextChild())
	{
		AddObjectToPythonList(object, olist);
	}
	return olist;
}


boost::python::list GetClickedObjects(int x, int y) {
	// for context menu, we want one object of each type, including child objects, if there are more than one the smae type, we want the one nearest to the camera

	MarkedObjectOneOfEach marked_object;
	theApp->FindMarkedObject(IPoint(x, y), &marked_object);

	std::list<MarkedObject*> stack;
	stack.push_back(&marked_object);
	std::map<int, MarkedObject*> types;

	while (stack.size() > 0)
	{
		MarkedObject* m = stack.front();
		stack.pop_front();

		for (std::map<HeeksObj*, MarkedObject*>::iterator It = m->m_map.begin(); It != m->m_map.end(); It++)
		{
			stack.push_back(It->second);
		}

		for (std::map<int, MarkedObject*>::iterator It = m->m_types.begin(); It != m->m_types.end(); It++)
		{
			int type = It->first;
			MarkedObject* object = It->second;
			std::map<int, MarkedObject*>::iterator FindIt = types.find(type);

			if (FindIt == types.end())
			{
				types.insert(std::make_pair(type, object));
			}
			else
			{
				MarkedObject* existing_object = FindIt->second;
				if (object->GetDepth() < existing_object->GetDepth())
				{
					types.erase(type);
					types.insert(std::make_pair(type, object));
				}
			}
		}
	}

	boost::python::list olist;

	if (types.size() > 0)
	{
		for (std::map<int, MarkedObject*>::iterator It = types.begin(); It != types.end(); It++)
		{
			MarkedObject* object = It->second;
			AddObjectToPythonList(object->GetHeeksObj(), olist);
		}
	}
	else
	{
		HeeksObj* object = marked_object.GetHeeksObj();
		if (object)AddObjectToPythonList(object, olist);
	}

	return olist;
}

bool ObjectMarked(HeeksObj* object)
{
	return theApp->m_marked_list->ObjectMarked(object);
}

void Select(HeeksObj* object, bool call_OnChanged = true)
{
	theApp->m_marked_list->Add(object, call_OnChanged);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(SelectOverloads, Select, 1, 2)


void Unselect(HeeksObj* object, bool call_OnChanged)
{
	theApp->m_marked_list->Remove(object, call_OnChanged);
}

void ClearSelection(bool call_OnChanged)
{
	theApp->m_marked_list->Clear(call_OnChanged);
}

int PickObjects(const std::wstring& str, long marking_filter, bool just_one)
{
	return theApp->PickObjects(str.c_str(), marking_filter, just_one);
}

double GetViewUnits()
{
	return theApp->m_view_units;
}

void SetViewUnits(double units)
{
	theApp->m_view_units = units;
}

void StartHistory()
{
	theApp->StartHistory();
}

void EndHistory()
{
	theApp->EndHistory();
}

void ClearHistory()
{
	theApp->ClearHistory();
}

bool IsModified()
{
	return theApp->IsModified();
}

void SetLikeNewFile()
{
	theApp->SetLikeNewFile();
}

void RollBack()
{
	theApp->RollBack();
}

void RollForward()
{
	theApp->RollForward();
}

void DeleteUndoably(HeeksObj *object)
{
	theApp->DeleteUndoably(object);
}

void CopyUndoably(HeeksObj* object, HeeksObj* copy_object)
{
	theApp->CopyUndoably(object, copy_object);
}

void AddUndoably(HeeksObj *object, HeeksObj* owner = NULL, HeeksObj* prev_object = NULL)
{
	theApp->AddUndoably(object, owner, prev_object);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(AddUndoablyOverloads, AddUndoably, 1, 3)



void DoUndoable(Undoable* undoable)
{
	theApp->DoUndoable(undoable);
}

void WasModified(HeeksObj *object)
{
	theApp->WasModified(object);
}

bool ShiftSelect(HeeksObj *object, bool control_down)
{
	// mark a list of siblings

	bool waiting_until_left_up = false;
	HeeksObj* parent = object->m_owner;
	std::set<HeeksObj*> sibling_set;
	std::list<HeeksObj*> sibling_list;
	for (HeeksObj* sibling = parent->GetFirstChild(); sibling; sibling = parent->GetNextChild())
	{
		sibling_set.insert(sibling);
		sibling_list.push_back(sibling);
	}
	// find most recently marked sibling
	std::list<HeeksObj*> &marked = theApp->m_marked_list->list();
	HeeksObj* recently_marked_sibling = NULL;
	bool recent_first = false;
	for (std::list<HeeksObj*>::reverse_iterator It = marked.rbegin(); It != marked.rend(); It++)
	{
		if (*It == object)recent_first = true;
		if (sibling_set.find(*It) != sibling_set.end())
		{
			recently_marked_sibling = *It;
			break;
		}
	}

	if (recently_marked_sibling)
	{
		if (!control_down)
		{
			theApp->m_marked_list->Clear(false);
		}

		bool marking = false;
		std::list<HeeksObj*> list_to_mark;
		bool finish_marking = false;
		for (std::list<HeeksObj*>::iterator It = sibling_list.begin(); !finish_marking && It != sibling_list.end(); It++)
		{
			HeeksObj* sibling = *It;
			if (sibling == object || sibling == recently_marked_sibling)
			{
				if (marking)finish_marking = true;
				else marking = true;
			}

			if (marking)
			{
				list_to_mark.push_back(sibling);
			}
		}

		theApp->m_marked_list->Add(list_to_mark, true);
	}
	else
	{
		if (control_down)
		{
			if (theApp->m_marked_list->ObjectMarked(object))
			{
				theApp->m_marked_list->Remove(object, true);
			}
			else{
				theApp->m_marked_list->Add(object, true);
			}
		}
		else
		{
			if (theApp->m_marked_list->ObjectMarked(object))
			{
				waiting_until_left_up = true;
			}
			else
			{
				theApp->m_marked_list->Clear(false);
				theApp->m_marked_list->Add(object, true);
			}
		}
	}

	return waiting_until_left_up;
}

void ChangePropertyString(const std::wstring& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeString(value, property));
}

void ChangePropertyDouble(const double& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeDouble(value, property));
}

void ChangePropertyLength(const double& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeLength(value, property));
}

void ChangePropertyInt(const int& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeInt(value, property));
}

void ChangePropertyChoice(const int& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeChoice(value, property));
}

void ChangePropertyColor(const HeeksColor& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeColor(value, property));
}

void ChangePropertyCheck(const bool& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeCheck(value, property));
}

CApp* GetApp()
{
	return theApp;
}

HeeksObj* GetFirstChild()
{
	return theApp->GetFirstChild();
}

HeeksObj* GetNextChild()
{
	return theApp->GetNextChild();
}

HeeksObj* ObjectGetOwner(HeeksObj* object)
{
	return object->m_owner;
}

void ObjectSetOwner(HeeksObj* object, HeeksObj* new_owner)
{
	object->m_owner = new_owner;
}

double GetUnits()
{
	return theApp->m_view_units;
}

void PyIncref(PyObject* object)
{
	Py_IncRef(object);
}

int GetNextID(int id_group_type)
{
	return theApp->GetNextID(id_group_type);
}

bool GetDrawSelect()
{
	return BaseObject::m_select;
}

bool GetDrawMarked()
{
	return BaseObject::m_marked;
}

bool CanUndo()
{
	return theApp->CanUndo();
}

bool CanRedo()
{
	return theApp->CanRedo();
}

int BaseObjectGetIndex(BaseObject& object)
{
	return object.GetIndex();
}

int HeeksObjGetIndex(HeeksObj& object)
{
	return object.GetIndex();
}




	BOOST_PYTHON_MODULE(cad) {
		boost::python::class_<BaseObject, boost::noncopyable >("BaseObject", "derive your custom CAD objects from this")
			.def(boost::python::init<int>())
			.def("GetIDGroupType", &BaseObject::GetIDGroupType)
			.def("GetIconFilePath", &BaseObjectGetIconFilePath)
			.def("GetTitle", &BaseObjectGetTitle)
			.def("GetID", &BaseObject::GetID)
			.def("SetID", &BaseObject::SetID)
			.def("GetIndex", &BaseObjectGetIndex)
			.def("KillGLLists", &BaseObject::KillGLLists)
			.def("GetColor", &BaseObjectGetColor)
			.def("AutoExpand", &BaseObject::AutoExpand)
			.def("GetNumChildren", &BaseObject::GetNumChildren)
			.def("GetOwner", &ObjectGetOwner, boost::python::return_value_policy<boost::python::reference_existing_object>())
			.def("SetOwner", &ObjectSetOwner)
			.def("GetFirstChild", &BaseObject::GetFirstChild, boost::python::return_value_policy<boost::python::reference_existing_object>())
			.def("GetNextChild", &BaseObject::GetNextChild, boost::python::return_value_policy<boost::python::reference_existing_object>())
			.def("GetChildren", &BaseObjectGetChildren)			
			.def("Clear", static_cast< void (BaseObject::*)(void) >(&BaseObject::Clear))
			.def("CanAdd", &HeeksObj::CanAdd)
			.def("CanAddTo", &HeeksObj::CanAddTo)
			.def("CanBeDeleted", &HeeksObj::CanBeRemoved)
			.def("OneOfAKind", &BaseObject::OneOfAKind_default)
			.def("CopyFrom", &ObjListCopyFrom)
			.def("GetProperties", &HeeksObjGetProperties)
			.def("GetBaseProperties", &HeeksObjGetBaseProperties)
			.def("ReadObjectXml", &HeeksObjReadObjectXml)
			.def("Clear", &ObjListClear)
			.def("Add", &ObjListAdd)
			.def("GetCopyFromObject", &BaseObject::GetCopyFromObject, boost::python::return_value_policy<boost::python::reference_existing_object>())
			.def("AddTriangle", &BaseObject::AddTriangle)
			;

		boost::python::class_<HeeksObj, boost::noncopyable>("Object")
			.def(boost::python::init<HeeksObj>())
			.def("GetType", &HeeksObj::GetType)
			.def("GetIDGroupType", &HeeksObj::GetIDGroupType)
			.def("GetTypeString", HeeksObjGetTypeString)
			.def("GetIconFilePath", &HeeksObjGetIconFilePath)
			.def("GetID", &HeeksObj::GetID)
			.def("SetID", &HeeksObj::SetID)
			.def("GetIndex", &HeeksObjGetIndex)
			.def("KillGLLists", &HeeksObj::KillGLLists)
			.def("GetColor", &HeeksObjGetColor)
			.def("HasEdit", &HeeksObjHasEdit)
			.def("GetTitle", &HeeksObjGetTitle)
			.def("GetBox", &HeeksObj::GetBox)
			.def("OnGlCommands", &HeeksObj::glCommands)
			.def("AutoExpand", &HeeksObj::AutoExpand)
			.def("GetNumChildren", &HeeksObj::GetNumChildren)
			.def("GetOwner", &ObjectGetOwner, boost::python::return_value_policy<boost::python::reference_existing_object>())
			.def("SetOwner", &ObjectSetOwner)
			.def("GetFirstChild", &HeeksObj::GetFirstChild, boost::python::return_value_policy<boost::python::reference_existing_object>())
			.def("GetNextChild", &HeeksObj::GetNextChild, boost::python::return_value_policy<boost::python::reference_existing_object>())
			.def("CanAdd", &HeeksObj::CanAdd)
			.def("CanAddTo", &HeeksObj::CanAddTo)
			.def("CanBeDeleted", &HeeksObj::CanBeRemoved)
			.def("OneOfAKind", &HeeksObj::OneOfAKind)
			.def("CopyFrom", &HeeksObj::CopyFrom)
			.def("ReadXml", &HeeksObjReadFromXML)
			.def("ReadObjectXml", &HeeksObjReadObjectXml)
			.def("WriteXml", &HeeksObjWriteToXML)
			.def("WriteObjectXml", &HeeksObjWriteObjectToXML)
			.def("GetProperties", &HeeksObjGetProperties)
			.def("GetLines", &HeeksObjGetLines)
			.def("SetStartPoint", &HeeksObj::SetStartPoint)
			.def("GetStartPoint", &HeeksObjGetStartPoint)
			.def("GetEndPoint", &HeeksObjGetEndPoint)
			.def("MakeACopy", &HeeksObj::MakeACopy, boost::python::return_value_policy<boost::python::reference_existing_object>())
			;

		boost::python::class_<HeeksColor>("Color")
			.def(boost::python::init<HeeksColor>())
			.def(boost::python::init<unsigned char, unsigned char, unsigned char>())
			.def(boost::python::init<long>())
			.def_readwrite("red", &HeeksColor::red)
			.def_readwrite("green", &HeeksColor::green)
			.def_readwrite("blue", &HeeksColor::blue)
			.def("ref", &HeeksColor::COLORREF_color)
			.def("SetGlColor", &HeeksColor::glColor)
			;

		boost::python::class_<Material>("Material")
			.def(boost::python::init<Material>())
			.def(boost::python::init<const HeeksColor&>())
			.def("glMaterial", &Material::glMaterial)
			;

		boost::python::class_<PropertyWrap, boost::noncopyable >("Property")
			.def(boost::python::init<int, std::wstring, HeeksObj*>())
			.def("GetType", &Property::get_property_type)
			.def("GetTitle", &PropertyGetShortString)
			.def("GetString", &PropertyGetString)
			.def("GetDouble", &Property::GetDouble)
			.def("GetInt", &Property::GetInt)
			.def("GetBool", &Property::GetBool)
			.def("GetColor", &PropertyGetColor)
			.def("GetChoices", &PropertyGetChoices)
			.def_readwrite("editable", &PropertyWrap::m_editable)
			.def_readwrite("object", &PropertyWrap::m_object)
			.def("GetProperties", &PropertyGetProperties)
			;

		boost::python::class_<ObjList, boost::python::bases<HeeksObj>, boost::noncopyable>("ObjList")
			.def(boost::python::init<ObjList>())
			.def("Clear", &ObjListClear)
			.def("Add", &ObjListAdd)
			.def("ReadXml", &ObjListReadFromXML)
			.def("WriteXml", &ObjListWriteToXML)
			.def("CopyFrom", &ObjListCopyFrom)
			;

		boost::python::class_<IdNamedObj, boost::python::bases<HeeksObj>, boost::noncopyable>("IdNamedObj")
			.def(boost::python::init<IdNamedObj>())
			;

		boost::python::class_<IdNamedObjList, boost::python::bases<ObjList>, boost::noncopyable>("IdNamedObjList")
			.def(boost::python::init<IdNamedObjList>())
			;

		boost::python::class_<CSketch, boost::python::bases<IdNamedObjList>, boost::noncopyable>("Sketch")
			.def(boost::python::init<CSketch>())
			.def("GetStartPoint", &SketchGetStartPoint)
			.def("GetEndPoint", &SketchGetEndPoint)
			.def("IsCircle", &CSketch::IsCircle)
			.def("IsClosed", &CSketch::IsClosed)
			.def("HasMultipleSketches", &CSketch::HasMultipleSketches)
			.def("Split", &SketchSplit)
			.def("GetCircleDiameter", &SketchGetCircleDiameter)
			.def("GetCircleCentre", &SketchGetCircleCentre)
			.def("WriteDxf", &SketchWriteDXF)
			.def("GetSketchOrder", &CSketch::GetSketchOrder)
			.def("ReOrderSketch", &CSketch::ReOrderSketch)
			.def("GetCurve", &SketchGetCurve)
			.def("GetArea", &SketchGetArea)
			;

		boost::python::class_<HPoint, boost::python::bases<IdNamedObj> >("Point", boost::python::no_init)
			.def("__init__", boost::python::make_constructor(&initHPoint))
//			.def_readwrite("p", &HPoint::m_p)
			;

		boost::python::class_<CStlSolid, boost::python::bases<HeeksObj> >("StlSolid")
			.def(boost::python::init<CStlSolid>())
			.def("__init__", boost::python::make_constructor(&initStlSolid))
			.def(boost::python::init<const std::wstring&>())// load a stl solid from a filepath
			.def("WriteSTL", &StlSolidWriteSTL) ///function WriteSTL///params float tolerance, string filepath///writes an STL file for the body to the given tolerance
			.def("NumTriangles", StlSolidNumTriangles)
			;

		boost::python::class_<PropertyCheck, boost::noncopyable, boost::python::bases<Property> >("PropertyCheck", boost::python::no_init);
		boost::python::class_<PropertyChoice, boost::python::bases<Property> >("PropertyChoice", boost::python::no_init);
		boost::python::class_<PropertyColor, boost::python::bases<Property> >("PropertyColor", boost::python::no_init);
		boost::python::class_<PropertyDouble, boost::python::bases<Property> >("PropertyDouble", boost::python::no_init);
		boost::python::class_<PropertyDoubleScaled, boost::python::bases<Property> >("PropertyDoubleScaled", boost::python::no_init);
		boost::python::class_<PropertyLengthScaled, boost::python::bases<PropertyDoubleScaled> >("PropertyLengthScaled", boost::python::no_init);
		boost::python::class_<PropertyDoubleLimited, boost::python::bases<PropertyDouble> >("PropertyDoubleLimited", boost::python::no_init);
		boost::python::class_<PropertyString, boost::python::bases<Property> >("PropertyString", boost::python::no_init);
		boost::python::class_<PropertyStringReadOnly, boost::python::bases<Property> >("PropertyStringReadOnly", boost::python::no_init)
			.def("__init__", boost::python::make_constructor(&initPropertyStringReadOnly))
			;
		boost::python::class_<PropertyFile, boost::python::bases<PropertyString> >("PropertyFile", boost::python::no_init);
		boost::python::class_<PropertyInt, boost::python::bases<Property> >("PropertyInt", boost::python::no_init);
		boost::python::class_<PropertyLength, boost::python::bases<PropertyDouble> >("PropertyLength", boost::python::no_init);
		boost::python::class_<PropertyLengthWithKillGLLists, boost::python::bases<PropertyLength> >("PropertyLengthWithKillGLLists", boost::python::no_init);
		boost::python::class_<PropertyList, boost::python::bases<Property> >("PropertyList", boost::python::no_init);
		boost::python::class_<PropertyObjectTitle, boost::python::bases<Property> >("PropertyObjectTitle", boost::python::no_init);
		boost::python::class_<PropertyObjectColor, boost::python::bases<Property> >("PropertyObjectColor", boost::python::no_init);

		boost::python::class_<Undoable, boost::noncopyable>("Undoable", boost::python::no_init);

		boost::python::class_<UndoableWrap, boost::noncopyable >("BaseUndoable");

		boost::python::class_<PropertyChangeString, boost::python::bases<Undoable> >("PropertyChangeString", boost::python::no_init).def(boost::python::init<const std::wstring&, Property*>());
		boost::python::class_<PropertyChangeDouble, boost::python::bases<Undoable> >("PropertyChangeDouble", boost::python::no_init).def(boost::python::init<const double&, Property*>());
		boost::python::class_<PropertyChangeLength, boost::python::bases<Undoable> >("PropertyChangeLength", boost::python::no_init).def(boost::python::init<const double&, Property*>());
		boost::python::class_<PropertyChangeInt, boost::python::bases<Undoable> >("PropertyChangeInt", boost::python::no_init).def(boost::python::init<const int&, Property*>());
		boost::python::class_<PropertyChangeColor, boost::python::bases<Undoable> >("PropertyChangeColor", boost::python::no_init).def(boost::python::init<const HeeksColor&, Property*>());
		boost::python::class_<PropertyChangeChoice, boost::python::bases<Undoable> >("PropertyChangeChoice", boost::python::no_init).def(boost::python::init<const int&, Property*>());
		boost::python::class_<PropertyChangeCheck, boost::python::bases<Undoable> >("PropertyChangeCheck", boost::python::no_init).def(boost::python::init<const bool&, Property*>());

		boost::python::class_<CApp, boost::python::bases<ObjList>, boost::noncopyable>("App")
			.def(boost::python::init<CApp>())
			;

		boost::python::class_<CViewPoint>("ViewPoint", boost::python::no_init)
			.def("SetView", &CViewPoint::SetView)
			;

		boost::python::class_<CViewport>("Viewport")
			.def(boost::python::init<int, int>())
			.def("glCommands", &CViewport::glCommands)
			.def("WidthAndHeightChanged", &CViewport::WidthAndHeightChanged)
			.def("OnMouseEvent", &CViewport::OnMouseEvent)
			.def("OnMagExtents", &CViewport::OnMagExtents)
			.def("RestorePreviousViewPoint", &CViewport::RestorePreviousViewPoint)
			.def("ClearViewpoints", &CViewport::ClearViewpoints)
			.def("StoreViewPoint", &CViewport::StoreViewPoint)
			.def_readwrite("m_need_update", &CViewport::m_need_update)
			.def_readwrite("m_need_refresh", &CViewport::m_need_refresh)
			.def_readwrite("m_orthogonal", &CViewport::m_orthogonal)
			.def_readwrite("m_view_point", &CViewport::m_view_point)
			;

		boost::python::class_<MouseEvent>("MouseEvent")
			.def(boost::python::init<MouseEvent>())
			.def_readwrite("m_event_type", &MouseEvent::m_event_type)
			.def_readwrite("m_x", &MouseEvent::m_x)
			.def_readwrite("m_y", &MouseEvent::m_y)
			.def_readwrite("m_leftDown", &MouseEvent::m_leftDown)
			.def_readwrite("m_middleDown", &MouseEvent::m_middleDown)
			.def_readwrite("m_rightDown", &MouseEvent::m_rightDown)
			.def_readwrite("m_controlDown", &MouseEvent::m_controlDown)
			.def_readwrite("m_shiftDown", &MouseEvent::m_shiftDown)
			.def_readwrite("m_altDown", &MouseEvent::m_altDown)
			.def_readwrite("m_metaDown", &MouseEvent::m_metaDown)
			.def_readwrite("m_wheelRotation", &MouseEvent::m_wheelRotation)
			.def_readwrite("m_wheelDelta", &MouseEvent::m_wheelDelta)
			.def_readwrite("m_linesPerAction", &MouseEvent::m_linesPerAction)
			;

		boost::python::class_<ObserverWrap, boost::noncopyable >("Observer")
			.def(boost::python::init<ObserverWrap>())
			;

		boost::python::enum_<KeyCode>("KeyCode")
			.value("None", K_NONE)
			.value("ControlA", K_CONTROL_A)
			.value("ControlB", K_CONTROL_B)
			.value("ControlC", K_CONTROL_C)
			.value("ControlD", K_CONTROL_D)
			.value("ControlE", K_CONTROL_E)
			.value("ControlF", K_CONTROL_F)
			.value("ControlG", K_CONTROL_G)
			.value("ControlH", K_CONTROL_H)
			.value("ControlI", K_CONTROL_I)
			.value("ControlJ", K_CONTROL_J)
			.value("ControlK", K_CONTROL_K)
			.value("ControlL", K_CONTROL_L)
			.value("ControlM", K_CONTROL_M)
			.value("ControlN", K_CONTROL_N)
			.value("ControlO", K_CONTROL_O)
			.value("ControlP", K_CONTROL_P)
			.value("ControlQ", K_CONTROL_Q)
			.value("ControlR", K_CONTROL_R)
			.value("ControlS", K_CONTROL_S)
			.value("ControlT", K_CONTROL_T)
			.value("ControlU", K_CONTROL_U)
			.value("ControlV", K_CONTROL_V)
			.value("ControlW", K_CONTROL_W)
			.value("ControlX", K_CONTROL_X)
			.value("ControlY", K_CONTROL_Y)
			.value("ControlZ", K_CONTROL_Z)
			.value("Back", K_BACK)
			.value("Tab", K_TAB)
			.value("Return", K_RETURN)
			.value("Escape", K_ESCAPE)
			.value("Space", K_SPACE)
			.value("Delete", K_DELETE)
			.value("Start", K_START)
			.value("LButton", K_LBUTTON)
			.value("RButton", K_RBUTTON)
			.value("Cancel", K_CANCEL)
			.value("MButton", K_MBUTTON)
			.value("Clear", K_CLEAR)
			.value("Shift", K_SHIFT)
			.value("Alt", K_ALT)
			.value("Control", K_CONTROL)
			.value("Menu", K_MENU)
			.value("Pause", K_PAUSE)
			.value("Capital", K_CAPITAL)
			.value("End", K_END)
			.value("Home", K_HOME)
			.value("Left", K_LEFT)
			.value("Up", K_UP)
			.value("Right", K_RIGHT)
			.value("Down", K_DOWN)
			.value("Select", K_SELECT)
			.value("Print", K_PRINT)
			.value("Execute", K_EXECUTE)
			.value("Sanpshot", K_SNAPSHOT)
			.value("Insert", K_INSERT)
			.value("Help", K_HELP)
			.value("Numpad0", K_NUMPAD0)
			.value("Numpad1", K_NUMPAD1)
			.value("Numpad2", K_NUMPAD2)
			.value("Numpad3", K_NUMPAD3)
			.value("Numpad4", K_NUMPAD4)
			.value("Numpad5", K_NUMPAD5)
			.value("Numpad6", K_NUMPAD6)
			.value("Numpad7", K_NUMPAD7)
			.value("Numpad8", K_NUMPAD8)
			.value("Numpad9", K_NUMPAD9)
			.value("Multiply", K_MULTIPLY)
			.value("Add", K_ADD)
			.value("Separator", K_SEPARATOR)
			.value("Subtract", K_SUBTRACT)
			.value("Decimal", K_DECIMAL)
			.value("Divide", K_DIVIDE)
			.value("F1", K_F1)
			.value("F2", K_F2)
			.value("F3", K_F3)
			.value("F4", K_F4)
			.value("F5", K_F5)
			.value("F6", K_F6)
			.value("F7", K_F7)
			.value("F8", K_F8)
			.value("F9", K_F9)
			.value("F10", K_F10)
			.value("F11", K_F11)
			.value("F12", K_F12)
			.value("F13", K_F13)
			.value("F14", K_F14)
			.value("F15", K_F15)
			.value("F16", K_F16)
			.value("F17", K_F17)
			.value("F18", K_F18)
			.value("F19", K_F19)
			.value("F20", K_F20)
			.value("F21", K_F21)
			.value("F22", K_F22)
			.value("F23", K_F23)
			.value("F24", K_F24)
			.value("Numlock", K_NUMLOCK)
			.value("Scroll", K_SCROLL)
			.value("PageUp", K_PAGEUP)
			.value("PageDown", K_PAGEDOWN)
			.value("NumpadSpace", K_NUMPAD_SPACE)
			.value("NumpadTab", K_NUMPAD_TAB)
			.value("NumpadEnter", K_NUMPAD_ENTER)
			.value("NumpadF1", K_NUMPAD_F1)
			.value("NumpadF2", K_NUMPAD_F2)
			.value("NumpadF3", K_NUMPAD_F3)
			.value("NumpadF4", K_NUMPAD_F4)
			.value("NumpadHome", K_NUMPAD_HOME)
			.value("NumpadLeft", K_NUMPAD_LEFT)
			.value("NumpadUp", K_NUMPAD_UP)
			.value("NumpadRight", K_NUMPAD_RIGHT)
			.value("NumpadDown", K_NUMPAD_DOWN)
			.value("NumpadPageUp", K_NUMPAD_PAGEUP)
			.value("NumpadPageDown", K_NUMPAD_PAGEDOWN)
			.value("NumpadEnd", K_NUMPAD_END)
			.value("NumpadBegin", K_NUMPAD_BEGIN)
			.value("NumpadInsert", K_NUMPAD_INSERT)
			.value("NumpadDelete", K_NUMPAD_DELETE)
			.value("NumpadEqual", K_NUMPAD_EQUAL)
			.value("NumpadMultiply", K_NUMPAD_MULTIPLY)
			.value("NumpadAdd", K_NUMPAD_ADD)
			.value("NumpadSeparator", K_NUMPAD_SEPARATOR)
			.value("NumpadSubtract", K_NUMPAD_SUBTRACT)
			.value("NumpadDecimal", K_NUMPAD_DECIMAL)
			.value("NumpadDivide", K_NUMPAD_DIVIDE)
			.value("WindowsLeft", K_WINDOWS_LEFT)
			.value("WindowsRight", K_WINDOWS_RIGHT)
			.value("WindowsMenu", K_WINDOWS_MENU)
			.value("RawControl", K_RAW_CONTROL)
			.value("Command", K_COMMAND)
			.value("Special1", K_SPECIAL1)
			.value("Special2", K_SPECIAL2)
			.value("Special3", K_SPECIAL3)
			.value("Special4", K_SPECIAL4)
			.value("Special5", K_SPECIAL5)
			.value("Special6", K_SPECIAL6)
			.value("Special7", K_SPECIAL7)
			.value("Special8", K_SPECIAL8)
			.value("Special9", K_SPECIAL9)
			.value("Special10", K_SPECIAL10)
			.value("Special11", K_SPECIAL11)
			.value("Special12", K_SPECIAL12)
			.value("Special13", K_SPECIAL13)
			.value("Special14", K_SPECIAL14)
			.value("Special15", K_SPECIAL15)
			.value("Special16", K_SPECIAL16)
			.value("Special17", K_SPECIAL17)
			.value("Special18", K_SPECIAL18)
			.value("Special19", K_SPECIAL19)
			.value("Special20", K_SPECIAL20)
			;

		boost::python::class_<KeyEvent>("KeyEvent")
			.def(boost::python::init<KeyEvent>())
			.def_readwrite("m_key_code", &KeyEvent::m_key_code)
			;

		boost::python::class_<InputModeWrap, boost::noncopyable >("InputMode")
			.def(boost::python::init<InputModeWrap>())
			.def("OnKeyDown", &CInputMode::OnKeyDown)
			.def("OnKeyUp", &CInputMode::OnKeyUp)
			;

		boost::python::enum_<DigitizeType>("DigitizeType")
			.value("DIGITIZE_NO_ITEM_TYPE", DigitizeNoItemType)
			.value("DIGITIZE_ENDOF_TYPE", DigitizeEndofType)
			.value("DIGITIZE_INTER_TYPE", DigitizeIntersType)
			.value("DIGITIZE_MIDPOINT_TYPE", DigitizeMidpointType)
			.value("DIGITIZE_CENTRE_TYPE", DigitizeCentreType)
			.value("DIGITIZE_SCREEN_TYPE", DigitizeScreenType)
			.value("DIGITIZE_COORDS_TYPE", DigitizeCoordsType)
			.value("DIGITIZE_NEAREST_TYPE", DigitizeNearestType)
			.value("DIGITIZE_TANGENT_TYPE", DigitizeTangentType)
			.value("DIGITIZE_INPUT_TYPE", DigitizeInputType)
			;

		boost::python::enum_<SketchOrderType>("SketchOrderType")
			.value("SketchOrderTypeUnknown", SketchOrderTypeUnknown)
			.value("SketchOrderTypeEmpty", SketchOrderTypeEmpty)
			.value("SketchOrderTypeOpen", SketchOrderTypeOpen)
			.value("SketchOrderTypeReverse", SketchOrderTypeReverse)
			.value("SketchOrderTypeBad", SketchOrderTypeBad)
			.value("SketchOrderTypeReOrder", SketchOrderTypeReOrder)
			.value("SketchOrderTypeCloseCW", SketchOrderTypeCloseCW)
			.value("SketchOrderTypeCloseCCW", SketchOrderTypeCloseCCW)
			.value("SketchOrderTypeMultipleCurves", SketchOrderTypeMultipleCurves)
			.value("SketchOrderHasCircles", SketchOrderHasCircles)
			.value("MaxSketchOrderTypes", MaxSketchOrderTypes)
			;

		boost::python::class_<DigitizedPoint>("DigitizedPoint")
			.def(boost::python::init<DigitizedPoint>())
			.def_readwrite("point", &DigitizedPoint::m_point)
			.def_readwrite("type", &DigitizedPoint::m_type)
			;

		boost::python::class_<DrawingWrap, boost::python::bases<CInputMode>, boost::noncopyable >("Drawing")
			.def(boost::python::init<DrawingWrap>())
			.def("AddPoint", &Drawing::AddPoint)
			.def("CalculateItem", &DrawingWrap::calculate_item)
			.def("IsAnAddLevel", &DrawingWrap::is_an_add_level)
			.def("NumberOfSteps", &DrawingWrap::number_of_steps)
			.def("TempObject", &DrawingWrap::TempObject, boost::python::return_value_policy<boost::python::reference_existing_object>())
			.def("ClearObjectsMade", &DrawingWrap::ClearObjectsMade)
			.def("AddToTempObjects", &DrawingWrap::AddToTempObjects)
			;

		boost::python::def("OnExit", OnExit);
		boost::python::def("Reset", CadReset);
		boost::python::def("OpenFile", CadOpenFile);
		boost::python::def("Import", &CadImport, CadImportOverloads((boost::python::arg("filepath"), boost::python::arg("paste_into") = NULL)));
		boost::python::def("SaveFile", CadSaveFile);
		boost::python::def("SaveObjects", SaveObjects);		
		boost::python::def("DrawTriangle", &DrawTriangle);
		boost::python::def("DrawEnableLighting", &DrawEnableLighting);
		boost::python::def("DrawDisableLighting", &DrawDisableLighting);
		boost::python::def("DrawLine", &DrawLine);
		boost::python::def("DrawColor", &DrawColor);
		boost::python::def("DrawSymbol", &DrawSymbol, "Use glBitmap to draw a symbol of a limit collection of types at the given position");
		boost::python::def("BeginTriangles", &BeginTriangles);
		boost::python::def("BeginLines", &BeginLines);
		boost::python::def("EndLinesOrTriangles", &EndLinesOrTriangles);
		boost::python::def("GlVertex", &glVertexPoint3d);
		boost::python::def("GlLineWidth", &GlLineWidth);
		boost::python::def("DrawNewList", &DrawNewList);
		boost::python::def("DrawEndList", &DrawEndList);
		boost::python::def("DrawCallList", &DrawCallList); 
		boost::python::def("DrawDeleteList", &DrawDeleteList);
		boost::python::def("DrawEnableLights", &DrawDeleteList);
		boost::python::def("DrawDisableLights", &DrawDeleteList);

		boost::python::def("AddProperty", AddProperty);
		boost::python::def("GetObjectFromId", &GetObjectFromId);
		boost::python::def("RegisterObjectType", RegisterObjectType);
		boost::python::def("SetXmlValue", SetXmlValue);
		boost::python::def("BeginXmlChild", BeginXmlChild);
		boost::python::def("EndXmlChild", EndXmlChild);
		boost::python::def("GetXmlObject", &GetXmlObject);
		boost::python::def("GetXmlValue", &GetXmlValue, GetXmlValueOverloads((boost::python::arg("name"), boost::python::arg("default_value") = std::wstring(L""))));
		boost::python::def("GetXmlText", &GetXmlText);
		boost::python::def("SetXmlText", &SetXmlText);		
		boost::python::def("GetXmlBool", &GetXmlBool, GetXmlBoolOverloads((boost::python::arg("name"), boost::python::arg("default_value") = false)));
		boost::python::def("GetXmlInt", &GetXmlInt, GetXmlIntOverloads((boost::python::arg("name"), boost::python::arg("default_value") = 0)));
		boost::python::def("GetXmlFloat", &GetXmlFloat, GetXmlFloatOverloads((boost::python::arg("name"), boost::python::arg("default_value") = 0.0)));
		boost::python::def("ReturnFromXmlChild", ReturnFromXmlChild);
		boost::python::def("GetFirstXmlChild", GetFirstXmlChild);
		boost::python::def("GetNextXmlChild", GetNextXmlChild);
		boost::python::def("OpenXmlFile", &OpenXmlFile, OpenXMLFileOverloads((boost::python::arg("filepath"), boost::python::arg("paste_into") = NULL, boost::python::arg("paste_before") = NULL)));
		boost::python::def("RegisterObserver", RegisterObserver);
		boost::python::def("RegisterOnRepaint", RegisterOnRepaint);
		boost::python::def("Repaint", &PythonOnRepaint, PythonOnRepaintOverloads((boost::python::arg("soon") = false)));
		boost::python::def("RegisterMessageBoxCallback", RegisterMessageBoxCallback); 
		boost::python::def("SetContextMenuCallback", SetContextMenuCallback);
		boost::python::def("GetResFolder", GetResFolder);
		boost::python::def("SetResFolder", SetResFolder);
		boost::python::def("MessageBox", CadMessageBox);
		boost::python::def("GetSelectedObjects", GetSelectedObjects);
		boost::python::def("GetNumSelected", GetNumSelected);
		boost::python::def("GetObjects", GetObjects);
		boost::python::def("GetClickedObjects", GetClickedObjects);
		boost::python::def("ObjectMarked", ObjectMarked);
		boost::python::def("Select", &Select, SelectOverloads(	(boost::python::arg("object"),	boost::python::arg("CallOnChanged") = NULL)));
		boost::python::def("Unselect", Unselect);
		boost::python::def("ClearSelection", ClearSelection);
		boost::python::def("GetSelectionProperties", GetSelectionProperties);
		boost::python::def("PickObjects", PickObjects);
		boost::python::def("GetViewUnits", GetViewUnits);
		boost::python::def("SetViewUnits", SetViewUnits);
		boost::python::def("GetApp", GetApp, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("StartHistory", StartHistory);
		boost::python::def("EndHistory", EndHistory);
		boost::python::def("ClearHistory", ClearHistory);
		boost::python::def("IsModified", IsModified);
		boost::python::def("SetLikeNewFile", SetLikeNewFile);
		boost::python::def("RollBack", RollBack);
		boost::python::def("RollForward", RollForward);
		boost::python::def("DeleteUndoably", DeleteUndoably);
		boost::python::def("AddUndoably", &AddUndoably, AddUndoablyOverloads((boost::python::arg("object"), boost::python::arg("owner") = NULL, boost::python::arg("prev_object") = NULL)));
		boost::python::def("CopyUndoably", CopyUndoably);
		boost::python::def("DoUndoable", DoUndoable);
		boost::python::def("WasModified", WasModified);		
		boost::python::def("ShiftSelect", ShiftSelect);
		boost::python::def("ChangePropertyString", ChangePropertyString);
		boost::python::def("ChangePropertyDouble", ChangePropertyDouble);
		boost::python::def("ChangePropertyLength", ChangePropertyLength);
		boost::python::def("ChangePropertyInt", ChangePropertyInt);
		boost::python::def("ChangePropertyChoice", ChangePropertyChoice);
		boost::python::def("ChangePropertyColor", ChangePropertyColor);
		boost::python::def("ChangePropertyCheck", ChangePropertyCheck);
		boost::python::def("GetUnits", GetUnits);
		boost::python::def("SetGetLinesPixelsPerMm", SetGetLinesPixelsPerMm);
		boost::python::def("GetSelectMode", GetSelectMode, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("GetMagnification", GetMagnification, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("GetViewRotating", GetViewRotating, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("GetViewZooming", GetViewZooming, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("GetViewPanning", GetViewPanning, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("SetInputMode", SetInputMode);
		boost::python::def("GetInputMode", GetInputMode, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("SetLineArcDrawing", SetLineArcDrawing);
		boost::python::def("SetCircles3pDrawing", SetCircles3pDrawing);
		boost::python::def("SetCircles2pDrawing", SetCircles2pDrawing);
		boost::python::def("SetCircle1pDrawing", SetCircle1pDrawing);
		boost::python::def("SetEllipseDrawing", SetEllipseDrawing);
		boost::python::def("SetILineDrawing", SetILineDrawing);
		boost::python::def("NewPoint", NewPoint, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("PyIncref", PyIncref);
		boost::python::def("GetNextID", GetNextID);
		boost::python::def("GetDrawSelect", GetDrawSelect);
		boost::python::def("GetDrawMarked", GetDrawMarked);
		boost::python::def("CanUndo", CanUndo);
		boost::python::def("CanRedo", CanRedo);
		boost::python::scope().attr("OBJECT_TYPE_UNKNOWN") = (int)UnknownType;
		boost::python::scope().attr("OBJECT_TYPE_SKETCH") = (int)SketchType;
		boost::python::scope().attr("OBJECT_TYPE_SKETCH_LINE") = (int)LineType;
		boost::python::scope().attr("OBJECT_TYPE_SKETCH_ARC") = (int)ArcType;
		boost::python::scope().attr("OBJECT_TYPE_CIRCLE") = (int)CircleType;
		boost::python::scope().attr("OBJECT_TYPE_POINT") = (int)PointType;
		boost::python::scope().attr("PROPERTY_TYPE_INVALID") = (int)InvalidPropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_STRING") = (int)StringPropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_LONG_STRING") = (int)LongStringPropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_DOUBLE") = (int)DoublePropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_LENGTH") = (int)LengthPropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_INT") = (int)IntPropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_CHOICE") = (int)ChoicePropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_COLOR") = (int)ColorPropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_CHECK") = (int)CheckPropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_LIST") = (int)ListOfPropertyType;
		boost::python::scope().attr("PROPERTY_TYPE_FILE") = (int)FilePropertyType;
		boost::python::scope().attr("MARKING_FILTER_SKETCH_GROUP") = (int)MARKING_FILTER_SKETCH_GROUP;
	}
