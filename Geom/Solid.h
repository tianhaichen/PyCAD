// Solid.h
// Copyright 2015, Dan Heeks

#ifdef OPEN_CASCADE_INCLUDED

#pragma once

#include <sstream>

#include <Standard.hxx>
#include <Standard_TypeDef.hxx>

#include <Bnd_Box.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgo_Fuse.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <BRepGProp.hxx>
#include <BRepMesh.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRepOffsetAPI_MakeEvolved.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_Sewing.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GC_MakeSegment.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Geom_Axis1Placement.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <GeomAPI_IntCS.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomAPI_IntSS.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GeomLProp_SLProps.hxx>
#include <GProp_GProps.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <Circle.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <Point3d.hxx>
#include <gp_Elips.hxx>
#include <gp_GTrsf.hxx>
#include <Line.hxx>
#include <Plane.hxx>
#include <Point3d.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <Matrix.hxx>
#include <Point3d.hxx>
#include <Handle_Geom_TrimmedCurve.hxx>
#include <IGESControl_Controller.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <IntTools_FaceFace.hxx>
#include "math_BFGS.hxx"
#include "math_MultipleVarFunctionWithGradient.hxx"
#include <Poly_Connect.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <ShapeFix_Wire.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Version.hxx>
#include <StdPrs_ToolShadedShape.hxx>
#include <STEPControl_Controller.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <TColgp_Array1OfDir.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopOpeBRep_FacesIntersector.hxx>
#include <TopOpeBRepBuild_FuseFace.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapIteratorOfMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <UnitsAPI.hxx>

#include "geometry.h"

class CArea;
class CCurve;
class CBox;

class CSolid
{
	std::wstring m_title;
	TopoDS_Shape m_shape;

	enum SOLID_BOOLEAN_TYPE
	{
		SOLID_BOOLEAN_UNION,
		SOLID_BOOLEAN_CUT,
		SOLID_BOOLEAN_COMMON,
		SOLID_BOOLEAN_XOR,
	};

	void read_from_file(const std::wstring& step_file_path);
	static CSolid* DoBoolean(const CSolid& solid1, const CSolid& solid2, SOLID_BOOLEAN_TYPE boolean_type);
	bool GetExtents(double* extents, const double* orig, const double* xdir, const double* ydir, const double* zdir)const;

public:
	CSolid(){}
	CSolid(const CArea& area, double thickness);
	CSolid(const CCurve& curve, double thickness);
	CSolid(const TopoDS_Shape& shape);
	CSolid(const std::wstring& step_file_path){ read_from_file(step_file_path); }

	CSolid* BooleanCut(const CSolid& solid)const;
	CSolid* BooleanUnion(const CSolid& solid)const;
	CSolid* BooleanCommon(const CSolid& solid)const;
	void WriteStep(const std::wstring& step_file_path);
	void Translate(const Point3d& v);
	void Transform(const Matrix& mat);
	void GetBox(CBox& box)const;
	CSolid* Extrusion(const Point3d& v)const;
	const TopoDS_Shape& GetShape()const{ return m_shape; }
	bool IsNull()const{ return m_shape.IsNull(); }
};

#endif