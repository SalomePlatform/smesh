// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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

#include "SMESH_ControlsClassifier.hxx"

#include <GEOMUtils.hxx>
#include <Utils_SALOME_Exception.hxx>

#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRep_Tool.hxx>
#include <BRepBndLib.hxx>
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>

namespace SMESH
{
    namespace Controls
    {
        void Classifier::Init( const TopoDS_Shape& theShape,
                                                double              theTol,
                                                const Bnd_B3d*      theBox )
        {
          myShape = theShape;
          myTol   = theTol;
          myFlags = 0;

          bool isShapeBox = false;
          switch ( myShape.ShapeType() )
          {
          case TopAbs_SOLID:
          {
            if (( isShapeBox = isBox( theShape )))
            {
              myIsOutFun = & Classifier::isOutOfBox;
            }
            else
            {
              mySolidClfr = new BRepClass3d_SolidClassifier( prepareSolid( theShape ));
              myIsOutFun = & Classifier::isOutOfSolid;
            }
            break;
          }
          case TopAbs_FACE:
          {
            Standard_Real u1,u2,v1,v2;
            Handle(Geom_Surface) surf = BRep_Tool::Surface( TopoDS::Face( theShape ));
            if ( surf.IsNull() )
              myIsOutFun = & Classifier::isOutOfNone;
            else
            {
              surf->Bounds( u1,u2,v1,v2 );
              myProjFace = new GeomAPI_ProjectPointOnSurf;
              myProjFace->Init( surf, u1,u2, v1,v2, myTol );
              myIsOutFun = & Classifier::isOutOfFace;
            }
            break;
          }
          case TopAbs_EDGE:
          {
            Standard_Real u1, u2;
            Handle(Geom_Curve) curve = BRep_Tool::Curve( TopoDS::Edge( theShape ), u1, u2);
            if ( curve.IsNull() )
              myIsOutFun = & Classifier::isOutOfNone;
            else
            {
              myProjEdge = new GeomAPI_ProjectPointOnCurve;
              myProjEdge->Init( curve, u1, u2 );
              myIsOutFun = & Classifier::isOutOfEdge;
            }
            break;
          }
          case TopAbs_VERTEX:
          {
            myVertexXYZ = BRep_Tool::Pnt( TopoDS::Vertex( theShape ) );
            myIsOutFun = & Classifier::isOutOfVertex;
            break;
          }
          default:
            throw SALOME_Exception("Programmer error in usage of Classifier");
          }

          if ( !isShapeBox )
          {
            if ( theBox )
            {
              myBox = *theBox;
            }
            else
            {
              Bnd_Box box;
              if ( myShape.ShapeType() == TopAbs_FACE )
              {
                BRepAdaptor_Surface SA( TopoDS::Face( myShape ), /*useBoundaries=*/false );
                if ( SA.GetType() == GeomAbs_BSplineSurface )
                  BRepBndLib::AddOptimal( myShape, box,
                                          /*useTriangulation=*/true, /*useShapeTolerance=*/true );
              }
              if ( box.IsVoid() )
                BRepBndLib::Add( myShape, box );
              myBox.Clear();
              myBox.Add( box.CornerMin() );
              myBox.Add( box.CornerMax() );
              gp_XYZ halfSize = 0.5 * ( box.CornerMax().XYZ() - box.CornerMin().XYZ() );
              for ( int iDim = 1; iDim <= 3; ++iDim )
              {
                double x = halfSize.Coord( iDim );
                halfSize.SetCoord( iDim, x + Max( myTol, 1e-2 * x ));
              }
              myBox.SetHSize( halfSize );
            }
          }
        }

        Classifier::~Classifier()
        {
          delete mySolidClfr; mySolidClfr = 0;
          delete myProjFace;  myProjFace = 0;
          delete myProjEdge;  myProjEdge = 0;
        }

        TopoDS_Shape Classifier::prepareSolid( const TopoDS_Shape& theSolid )
        {
          // try to limit tolerance of theSolid down to myTol (issue #19026)

          // check if tolerance of theSolid is more than myTol
          bool tolIsOk = true; // max tolerance is at VERTEXes
          for ( TopExp_Explorer exp( theSolid, TopAbs_VERTEX ); exp.More() &&  tolIsOk; exp.Next() )
            tolIsOk = ( myTol >= BRep_Tool::Tolerance( TopoDS::Vertex( exp.Current() )));
          if ( tolIsOk )
            return theSolid;

          // make a copy to prevent the original shape from changes
          TopoDS_Shape resultShape = BRepBuilderAPI_Copy( theSolid );

          if ( !GEOMUtils::FixShapeTolerance( resultShape, TopAbs_SHAPE, myTol ))
            return theSolid;
          return resultShape;
        }

        bool Classifier::isOutOfSolid( const gp_Pnt& p )
        {
          if ( isOutOfBox( p )) return true;
          mySolidClfr->Perform( p, myTol );
          return ( mySolidClfr->State() != TopAbs_IN && mySolidClfr->State() != TopAbs_ON );
        }

        bool Classifier::isOutOfBox( const gp_Pnt& p )
        {
          return myBox.IsOut( p.XYZ() );
        }

        bool Classifier::isOutOfFace( const gp_Pnt& p )
        {
          if ( isOutOfBox( p )) return true;
          myProjFace->Perform( p );
          if ( myProjFace->IsDone() && myProjFace->LowerDistance() <= myTol )
          {
            // check relatively to the face
            myProjFace->LowerDistanceParameters( myU, myV );
            gp_Pnt2d aProjPnt( myU, myV );
            BRepClass_FaceClassifier aClsf ( TopoDS::Face( myShape ), aProjPnt, myTol );
            if ( aClsf.State() == TopAbs_IN || aClsf.State() == TopAbs_ON )
              return false;
          }
          return true;
        }

        bool Classifier::isOutOfEdge( const gp_Pnt& p )
        {
          if ( isOutOfBox( p )) return true;
          myProjEdge->Perform( p );
          bool isOn = ( myProjEdge->NbPoints() > 0 && myProjEdge->LowerDistance() <= myTol );
          if ( isOn )
            myU = myProjEdge->LowerDistanceParameter();
          return !isOn;
        }

        bool Classifier::isOutOfVertex( const gp_Pnt& p )
        {
          return ( myVertexXYZ.Distance( p ) > myTol );
        }

        bool Classifier::isBox(const TopoDS_Shape& theShape )
        {
          TopTools_IndexedMapOfShape vMap;
          TopExp::MapShapes( theShape, TopAbs_VERTEX, vMap );
          if ( vMap.Extent() != 8 )
            return false;

          myBox.Clear();
          for ( int i = 1; i <= 8; ++i )
            myBox.Add( BRep_Tool::Pnt( TopoDS::Vertex( vMap( i ))).XYZ() );

          gp_XYZ pMin = myBox.CornerMin(), pMax = myBox.CornerMax();
          for ( int i = 1; i <= 8; ++i )
          {
            gp_Pnt p = BRep_Tool::Pnt( TopoDS::Vertex( vMap( i )));
            for ( int iC = 1; iC <= 3; ++ iC )
            {
              double d1 = Abs( pMin.Coord( iC ) - p.Coord( iC ));
              double d2 = Abs( pMax.Coord( iC ) - p.Coord( iC ));
              if ( Min( d1, d2 ) > myTol )
                return false;
            }
          }
          myBox.Enlarge( myTol );
          return true;
        }
    }
}
