// Solid.h
// Copyright 2015, Dan Heeks

#pragma once

#include <sstream>
#include <list>
#include "geometry.h"
#include "Curve.h"
#include "Box.h"
#include "MachiningArea.h"

class CArea;

class LineOrPoint
{
public:
	geoff_geometry::Point3d m_p;
	geoff_geometry::Line m_line;
	bool m_is_a_line;
	LineOrPoint() :m_p(0.0, 0.0, 0.0), m_is_a_line(false){}
	LineOrPoint(const geoff_geometry::Line& line) :m_line(line), m_is_a_line(true){}
	LineOrPoint(const geoff_geometry::Point3d& p) :m_p(p), m_is_a_line(false){}
	double MinX()const;
	geoff_geometry::Point3d MostLeftPoint()const;
};

class CTri
{
public:
	CBox m_box;
	float x[3][3];

	CTri(){}

	geoff_geometry::Triangle3d GeoffTri()const;
	void Transform(const geoff_geometry::Matrix &mat);
	bool IntersectsXY(Span &span);
	void SetBox();
	bool On(const geoff_geometry::Point3d &p)const;
	bool Intof(const geoff_geometry::Line& l, LineOrPoint& intof)const;
};

class CTris
{
	std::wstring m_title;

	enum SOLID_BOOLEAN_TYPE
	{
		SOLID_BOOLEAN_UNION,
		SOLID_BOOLEAN_CUT,
		SOLID_BOOLEAN_COMMON,
		SOLID_BOOLEAN_XOR,
	};

	void read_from_file(const std::wstring& stl_file_path);
	static CTris* DoBoolean(const CTris& solid1, const CTris& solid2, SOLID_BOOLEAN_TYPE boolean_type);

public:
	CBox m_box; // built up while adding triangles
	std::list<CTri> m_tris;
	CTris(){}
	CTris(const std::wstring& stl_file_path){ read_from_file(stl_file_path); }

	void MakeSection(const Point& s, const Point& e, const std::string& dxf_file_path);
	void Project(const CArea& area, std::list<geoff_geometry::Line>& lines)const;
	void ProjectSpan(const Span& span, std::list<geoff_geometry::Line>& lines)const;

	CTris* BooleanCut(const CTris& solid)const;
	CTris* BooleanUnion(const CTris& solid)const;
	CTris* BooleanCommon(const CTris& solid)const;
	void AddTri(const float* x);
	void WriteStl(const std::string& stl_file_path);
	void SplitTriangles(const CTris& solid);
	CArea Shadow()const;
	CArea Shadow2(bool just_up_allowed = true)const;
	CArea Shadow2Mat(const geoff_geometry::Matrix &tm, bool just_up_allowed = true)const;
	void Transform(const geoff_geometry::Matrix& tm);
	void GetBox(CBox& box)const;
	const CTris& operator+=(const CTris &t);
	void ClipSpans(std::list<Span>& spans, double length)const;
	void GetMachiningAreas(std::list<CMachiningArea>& areas)const;
	CTris* GetFlattenedSurface()const;
};
