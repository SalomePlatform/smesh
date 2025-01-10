// Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#ifndef _SMESH_CONTROLSCLASSIFIER_HXX_
#define _SMESH_CONTROLSCLASSIFIER_HXX_

#include "SMESH_Controls.hxx"

#include <TopoDS_Shape.hxx>
#include <Bnd_B3d.hxx>

class BRepClass3d_SolidClassifier;
class GeomAPI_ProjectPointOnSurf;
class GeomAPI_ProjectPointOnCurve;

namespace SMESH
{
    namespace Controls
    {
        struct SMESHCONTROLS_EXPORT Classifier
        {
            Classifier(): mySolidClfr(0), myProjFace(0), myProjEdge(0), myFlags(0) { myU = myV = 1e100; }
            ~Classifier();
            void Init(const TopoDS_Shape& s, double tol, const Bnd_B3d* box = 0 );
            bool IsOut(const gp_Pnt& p)        { return SetChecked( true ), (this->*myIsOutFun)( p ); }
            TopAbs_ShapeEnum ShapeType() const { return myShape.ShapeType(); }
            const TopoDS_Shape& Shape() const  { return myShape; }
            const Bnd_B3d* GetBndBox() const   { return & myBox; }
            double Tolerance() const           { return myTol; }
            bool IsChecked()                   { return myFlags & theIsCheckedFlag; }
            bool IsSetFlag( int flag ) const   { return myFlags & flag; }
            void SetChecked( bool is ) { is ? SetFlag( theIsCheckedFlag ) : UnsetFlag( theIsCheckedFlag ); }
            void SetFlag  ( int flag ) { myFlags |= flag; }
            void UnsetFlag( int flag ) { myFlags &= ~flag; }
            void GetParams( double & u, double & v ) const { u = myU; v = myV; }

            private:
            bool isOutOfSolid (const gp_Pnt& p);
            bool isOutOfBox   (const gp_Pnt& p);
            bool isOutOfFace  (const gp_Pnt& p);
            bool isOutOfEdge  (const gp_Pnt& p);
            bool isOutOfVertex(const gp_Pnt& p);
            bool isOutOfNone  (const gp_Pnt& /*p*/) { return true; }
            bool isBox        (const TopoDS_Shape& s);

            TopoDS_Shape prepareSolid( const TopoDS_Shape& theSolid );

            bool (Classifier::*          myIsOutFun)(const gp_Pnt& p);
            BRepClass3d_SolidClassifier* mySolidClfr;
            Bnd_B3d                      myBox;
            GeomAPI_ProjectPointOnSurf*  myProjFace;
            GeomAPI_ProjectPointOnCurve* myProjEdge;
            gp_Pnt                       myVertexXYZ;
            TopoDS_Shape                 myShape;
            double                       myTol;
            double                       myU, myV; // result of isOutOfFace() and isOutOfEdge()
            int                          myFlags;

            static const int theIsCheckedFlag = 0x0000100;
        };
    }
}

#endif
