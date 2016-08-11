// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_Filter_i.cxx
//  Author : Alexey Petrov, OCC
//  Module : SMESH

#include "SMESH_Filter_i.hxx"

#include "SMDS_ElemIterator.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMESHDS_GroupBase.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_PythonDump.hxx"

#include <SALOMEDS_wrap.hxx>
#include <GEOM_wrap.hxx>

#include <Basics_OCCTVersion.hxx>

#include <BRep_Tool.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Plane.hxx>
#include <LDOMParser.hxx>
#include <LDOMString.hxx>
#include <LDOM_Document.hxx>
#include <LDOM_Element.hxx>
#include <LDOM_Node.hxx>
#include <LDOM_XmlWriter.hxx>
#include <Precision.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>
#include <TColStd_ListIteratorOfListOfReal.hxx>
#include <TColStd_ListOfInteger.hxx>
#include <TColStd_ListOfReal.hxx>
#include <TColStd_SequenceOfHAsciiString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>

using namespace SMESH;
using namespace SMESH::Controls;


namespace SMESH
{
  Predicate_i*
  GetPredicate( Predicate_ptr thePredicate )
  {
    return DownCast<Predicate_i*>(thePredicate);
  }
}

/*
                            AUXILIARY METHODS
*/

inline
const SMDS_Mesh*
MeshPtr2SMDSMesh( SMESH_Mesh_ptr theMesh )
{
  SMESH_Mesh_i* anImplPtr = DownCast<SMESH_Mesh_i*>(theMesh);
  return anImplPtr ? anImplPtr->GetImpl().GetMeshDS() : 0;
}

inline
SMESH::long_array*
toArray( const TColStd_ListOfInteger& aList )
{
  SMESH::long_array_var anArray = new SMESH::long_array;
  anArray->length( aList.Extent() );
  TColStd_ListIteratorOfListOfInteger anIter( aList );
  int i = 0;
  for( ; anIter.More(); anIter.Next() )
    anArray[ i++ ] = anIter.Value();

  return anArray._retn();
}

inline
SMESH::double_array*
toArray( const TColStd_ListOfReal& aList )
{
  SMESH::double_array_var anArray = new SMESH::double_array;
  anArray->length( aList.Extent() );
  TColStd_ListIteratorOfListOfReal anIter( aList );
  int i = 0;
  for( ; anIter.More(); anIter.Next() )
    anArray[ i++ ] = anIter.Value();

  return anArray._retn();
}

static SMESH::Filter::Criterion createCriterion()
{
  SMESH::Filter::Criterion aCriterion;

  aCriterion.Type          = FT_Undefined;
  aCriterion.Compare       = FT_Undefined;
  aCriterion.Threshold     = 0;
  aCriterion.UnaryOp       = FT_Undefined;
  aCriterion.BinaryOp      = FT_Undefined;
  aCriterion.ThresholdStr  = "";
  aCriterion.ThresholdID   = "";
  aCriterion.Tolerance     = Precision::Confusion();
  aCriterion.TypeOfElement = SMESH::ALL;
  aCriterion.Precision     = -1;

  return aCriterion;
}

static TopoDS_Shape getShapeByName( const char* theName )
{
  if ( theName != 0 )
  {
    SMESH_Gen_i* aSMESHGen     = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var aStudy = aSMESHGen->GetCurrentStudy();
    if ( !aStudy->_is_nil() )
    {
      SALOMEDS::Study::ListOfSObject_var aList = aStudy->FindObjectByName( theName, "GEOM" );
      if ( aList->length() > 0 )
      {
        CORBA::Object_var        anObj = aList[ 0 ]->GetObject();
        GEOM::GEOM_Object_var aGeomObj = GEOM::GEOM_Object::_narrow( anObj );
        TopoDS_Shape             shape = aSMESHGen->GeomObjectToShape( aGeomObj );
        SALOME::UnRegister( aList ); // UnRegister() objects in aList
        return shape;
      }
    }
  }
  return TopoDS_Shape();
}

static TopoDS_Shape getShapeByID (const char* theID)
{
  if ( theID && strlen( theID ) > 0 ) {
    SMESH_Gen_i*     aSMESHGen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var aStudy = aSMESHGen->GetCurrentStudy();
    if ( !aStudy->_is_nil() ) {
      SALOMEDS::SObject_wrap aSObj = aStudy->FindObjectID(theID);
      if ( !aSObj->_is_nil() ) {
        CORBA::Object_var          obj = aSObj->GetObject();
        GEOM::GEOM_Object_var aGeomObj = GEOM::GEOM_Object::_narrow(obj);
        return aSMESHGen->GeomObjectToShape( aGeomObj );
      }
    }
  }
  return TopoDS_Shape();
}

// static std::string getShapeNameByID (const char* theID)
// {
//   if ( theID && strlen( theID ) > 0 ) {
//     SMESH_Gen_i*     aSMESHGen = SMESH_Gen_i::GetSMESHGen();
//     SALOMEDS::Study_var aStudy = aSMESHGen->GetCurrentStudy();
//     if ( !aStudy->_is_nil() ) {
//       SALOMEDS::SObject_wrap aSObj = aStudy->FindObjectID(theID);
//       if ( !aSObj->_is_nil() ) {
//         CORBA::String_var name = aSObj->GetName();
//         return name.in();
//       }
//     }
//   }
//   return "";
// }

/*
                                FUNCTORS
*/

/*
  Class       : Functor_i
  Description : An abstact class for all functors
*/
Functor_i::Functor_i():
  SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  //Base class Salome_GenericObject do it inmplicitly by overriding PortableServer::POA_ptr _default_POA() method  
  //PortableServer::ObjectId_var anObjectId =
  //  SMESH_Gen_i::GetPOA()->activate_object( this );
}

Functor_i::~Functor_i()
{
  //TPythonDump()<<this<<".UnRegister()";
}

void Functor_i::SetMesh( SMESH_Mesh_ptr theMesh )
{
  myFunctorPtr->SetMesh( MeshPtr2SMDSMesh( theMesh ) );
  TPythonDump()<<this<<".SetMesh("<<theMesh<<")";
}

ElementType Functor_i::GetElementType()
{
  return ( ElementType )myFunctorPtr->GetType();
}


/*
  Class       : NumericalFunctor_i
  Description : Base class for numerical functors
*/
CORBA::Double NumericalFunctor_i::GetValue( CORBA::Long theId )
{
  return myNumericalFunctorPtr->GetValue( theId );
}

SMESH::Histogram* NumericalFunctor_i::GetHistogram(CORBA::Short nbIntervals, CORBA::Boolean isLogarithmic)
{
  std::vector<int> nbEvents;
  std::vector<double> funValues;
  std::vector<int> elements;
  myNumericalFunctorPtr->GetHistogram(nbIntervals,nbEvents,funValues,elements,0,isLogarithmic);

  SMESH::Histogram_var histogram = new SMESH::Histogram;

  nbIntervals = CORBA::Short( Min( int( nbEvents.size()),
                                   int( funValues.size() - 1 )));
  if ( nbIntervals > 0 )
  {
    histogram->length( nbIntervals );
    for ( int i = 0; i < nbIntervals; ++i )
    {
      HistogramRectangle& rect = histogram[i];
      rect.nbEvents = nbEvents[i];
      rect.min = funValues[i];
      rect.max = funValues[i+1];
    }
  }
  return histogram._retn();
}

SMESH::Histogram* NumericalFunctor_i::GetLocalHistogram(CORBA::Short              nbIntervals,
                                                        CORBA::Boolean            isLogarithmic,
                                                        SMESH::SMESH_IDSource_ptr object)
{
  SMESH::Histogram_var histogram = new SMESH::Histogram;

  std::vector<int>    nbEvents;
  std::vector<double> funValues;
  std::vector<int>    elements;

  SMDS_ElemIteratorPtr elemIt;
  if ( SMESH::DownCast< SMESH_GroupOnFilter_i* >( object ) ||
       SMESH::DownCast< SMESH::Filter_i* >( object ))
  {
    elemIt = SMESH_Mesh_i::GetElements( object, GetElementType() );
    if ( !elemIt ) return histogram._retn();
  }
  else
  {
    SMESH::SMESH_Mesh_var        mesh = object->GetMesh();
    SMESH::long_array_var  objNbElems = object->GetNbElementsByType();
    SMESH::long_array_var meshNbElems = mesh->  GetNbElementsByType();
    if ( meshNbElems[ GetElementType() ] !=
         objNbElems [ GetElementType() ] )
    {
      elements.reserve( objNbElems[ GetElementType() ]);
      elemIt = SMESH_Mesh_i::GetElements( object, GetElementType() );
    }
  }
  if ( elemIt )
  {
    while ( elemIt->more() )
      elements.push_back( elemIt->next()->GetID() );
    if ( elements.empty() ) return histogram._retn();
  }

  myNumericalFunctorPtr->GetHistogram(nbIntervals,nbEvents,funValues,elements,0,isLogarithmic);

  nbIntervals = CORBA::Short( Min( int( nbEvents.size()),
                                   int( funValues.size() - 1 )));
  if ( nbIntervals > 0 )
  {
    histogram->length( nbIntervals );
    for ( int i = 0; i < nbIntervals; ++i )
    {
      HistogramRectangle& rect = histogram[i];
      rect.nbEvents = nbEvents[i];
      rect.min = funValues[i];
      rect.max = funValues[i+1];
    }
  }
  return histogram._retn();
}

void NumericalFunctor_i::SetPrecision( CORBA::Long thePrecision )
{
  myNumericalFunctorPtr->SetPrecision( thePrecision );
  TPythonDump()<<this<<".SetPrecision("<<thePrecision<<")";
}

CORBA::Long NumericalFunctor_i::GetPrecision()
{
 return myNumericalFunctorPtr->GetPrecision();
}

Controls::NumericalFunctorPtr NumericalFunctor_i::GetNumericalFunctor()
{
  return myNumericalFunctorPtr;
}


/*
  Class       : SMESH_MinimumAngle
  Description : Functor for calculation of minimum angle
*/
MinimumAngle_i::MinimumAngle_i()
{
  myNumericalFunctorPtr.reset( new Controls::MinimumAngle() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType MinimumAngle_i::GetFunctorType()
{
  return SMESH::FT_MinimumAngle;
}


/*
  Class       : AspectRatio
  Description : Functor for calculating aspect ratio
*/
AspectRatio_i::AspectRatio_i()
{
  myNumericalFunctorPtr.reset( new Controls::AspectRatio() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType AspectRatio_i::GetFunctorType()
{
  return SMESH::FT_AspectRatio;
}


/*
  Class       : AspectRatio3D
  Description : Functor for calculating aspect ratio 3D
*/
AspectRatio3D_i::AspectRatio3D_i()
{
  myNumericalFunctorPtr.reset( new Controls::AspectRatio3D() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType AspectRatio3D_i::GetFunctorType()
{
  return SMESH::FT_AspectRatio3D;
}


/*
  Class       : Warping_i
  Description : Functor for calculating warping
*/
Warping_i::Warping_i()
{
  myNumericalFunctorPtr.reset( new Controls::Warping() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType Warping_i::GetFunctorType()
{
  return SMESH::FT_Warping;
}


/*
  Class       : Taper_i
  Description : Functor for calculating taper
*/
Taper_i::Taper_i()
{
  myNumericalFunctorPtr.reset( new Controls::Taper() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType Taper_i::GetFunctorType()
{
  return SMESH::FT_Taper;
}

/*
  Class       : Skew_i
  Description : Functor for calculating skew in degrees
*/
Skew_i::Skew_i()
{
  myNumericalFunctorPtr.reset( new Controls::Skew() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType Skew_i::GetFunctorType()
{
  return SMESH::FT_Skew;
}

/*
  Class       : Area_i
  Description : Functor for calculating area
*/
Area_i::Area_i()
{
  myNumericalFunctorPtr.reset( new Controls::Area() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType Area_i::GetFunctorType()
{
  return SMESH::FT_Area;
}

/*
  Class       : Volume3D_i
  Description : Functor for calculating volume of 3D element
*/
Volume3D_i::Volume3D_i()
{
  myNumericalFunctorPtr.reset( new Controls::Volume() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType Volume3D_i::GetFunctorType()
{
  return SMESH::FT_Volume3D;
}

/*
  Class       : MaxElementLength2D_i
  Description : Functor for calculating maximum length of 2D element
*/
MaxElementLength2D_i::MaxElementLength2D_i()
{
  myNumericalFunctorPtr.reset( new Controls::MaxElementLength2D() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType MaxElementLength2D_i::GetFunctorType()
{
  return SMESH::FT_MaxElementLength2D;
}

/*
  Class       : MaxElementLength3D_i
  Description : Functor for calculating maximum length of 3D element
*/
MaxElementLength3D_i::MaxElementLength3D_i()
{
  myNumericalFunctorPtr.reset( new Controls::MaxElementLength3D() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType MaxElementLength3D_i::GetFunctorType()
{
  return SMESH::FT_MaxElementLength3D;
}

/*
  Class       : Length_i
  Description : Functor for calculating length off edge
*/
Length_i::Length_i()
{
  myNumericalFunctorPtr.reset( new Controls::Length() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType Length_i::GetFunctorType()
{
  return SMESH::FT_Length;
}

/*
  Class       : Length2D_i
  Description : Functor for calculating length of edge
*/
Length2D_i::Length2D_i()
{
  myNumericalFunctorPtr.reset( new Controls::Length2D() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType Length2D_i::GetFunctorType()
{
  return SMESH::FT_Length2D;
}

SMESH::Length2D::Values* Length2D_i::GetValues()
{
  SMESH::Controls::Length2D::TValues aValues;
  (dynamic_cast<SMESH::Controls::Length2D*>(myFunctorPtr.get()))->GetValues( aValues );

  long i = 0, iEnd = aValues.size();

  SMESH::Length2D::Values_var aResult = new SMESH::Length2D::Values(iEnd);
  aResult->length(iEnd);

  SMESH::Controls::Length2D::TValues::const_iterator anIter;
  for ( anIter = aValues.begin() ; anIter != aValues.end(); anIter++, i++ )
  {
    const SMESH::Controls::Length2D::Value&  aVal = *anIter;
    SMESH::Length2D::Value &aValue = aResult[ i ];

    aValue.myLength = aVal.myLength;
    aValue.myPnt1 = aVal.myPntId[ 0 ];
    aValue.myPnt2 = aVal.myPntId[ 1 ];
  }

  return aResult._retn();
}

/*
  Class       : MultiConnection_i
  Description : Functor for calculating number of faces conneted to the edge
*/
MultiConnection_i::MultiConnection_i()
{
  myNumericalFunctorPtr.reset( new Controls::MultiConnection() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType MultiConnection_i::GetFunctorType()
{
  return SMESH::FT_MultiConnection;
}

/*
  Class       : BallDiameter_i
  Description : Functor returning diameter of a ball element
*/
BallDiameter_i::BallDiameter_i()
{
  myNumericalFunctorPtr.reset( new Controls::BallDiameter() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType BallDiameter_i::GetFunctorType()
{
  return SMESH::FT_BallDiameter;
}

/*
  Class       : NodeConnectivityNumber_i
  Description : Functor returning diameter of a ball element
*/
NodeConnectivityNumber_i::NodeConnectivityNumber_i()
{
  myNumericalFunctorPtr.reset( new Controls::NodeConnectivityNumber() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType NodeConnectivityNumber_i::GetFunctorType()
{
  return SMESH::FT_NodeConnectivityNumber;
}

/*
  Class       : MultiConnection2D_i
  Description : Functor for calculating number of faces conneted to the edge
*/
MultiConnection2D_i::MultiConnection2D_i()
{
  myNumericalFunctorPtr.reset( new Controls::MultiConnection2D() );
  myFunctorPtr = myNumericalFunctorPtr;
}

FunctorType MultiConnection2D_i::GetFunctorType()
{
  return SMESH::FT_MultiConnection2D;
}

SMESH::MultiConnection2D::Values* MultiConnection2D_i::GetValues()
{
  SMESH::Controls::MultiConnection2D::MValues aValues;
  (dynamic_cast<SMESH::Controls::MultiConnection2D*>(myFunctorPtr.get()))->GetValues( aValues );
  
  long i = 0, iEnd = aValues.size();

  SMESH::MultiConnection2D::Values_var aResult = new SMESH::MultiConnection2D::Values(iEnd);
  aResult->length(iEnd);

  SMESH::Controls::MultiConnection2D::MValues::const_iterator anIter;
  for ( anIter = aValues.begin() ; anIter != aValues.end(); anIter++, i++ )
  {
    const SMESH::Controls::MultiConnection2D::Value&  aVal = (*anIter).first;
    SMESH::MultiConnection2D::Value &aValue = aResult[ i ];

    aValue.myPnt1 = aVal.myPntId[ 0 ];
    aValue.myPnt2 = aVal.myPntId[ 1 ];
    aValue.myNbConnects = (*anIter).second;
  }

  return aResult._retn();
}

/*
                            PREDICATES
*/


/*
  Class       : Predicate_i
  Description : Base class for all predicates
*/
CORBA::Boolean Predicate_i::IsSatisfy( CORBA::Long theId )
{
  return myPredicatePtr->IsSatisfy( theId );
}

CORBA::Long Predicate_i::NbSatisfying( SMESH::SMESH_IDSource_ptr obj )
{
  SMESH::SMESH_Mesh_var meshVar = obj->GetMesh();
  const SMDS_Mesh*       meshDS = MeshPtr2SMDSMesh( meshVar );
  if ( !meshDS )
    return 0;
  myPredicatePtr->SetMesh( meshDS );

  SMDSAbs_ElementType elemType = SMDSAbs_ElementType( GetElementType() );

  int nb = 0;
  SMDS_ElemIteratorPtr elemIt =
    SMESH::DownCast<SMESH_Mesh_i*>( meshVar )->GetElements( obj, GetElementType() );
  if ( elemIt )
    while ( elemIt->more() )
    {
      const SMDS_MeshElement* e = elemIt->next();
      if ( e && e->GetType() == elemType )
        nb += myPredicatePtr->IsSatisfy( e->GetID() );
    }
  return nb;
}

Controls::PredicatePtr Predicate_i::GetPredicate()
{
  return myPredicatePtr;
}

/*
  Class       : BadOrientedVolume_i
  Description : Verify whether a mesh volume is incorrectly oriented from
                the point of view of MED convention
*/
BadOrientedVolume_i::BadOrientedVolume_i()
{
  Controls::PredicatePtr control( new Controls::BadOrientedVolume() );
  myFunctorPtr = myPredicatePtr = control;
};

FunctorType BadOrientedVolume_i::GetFunctorType()
{
  return SMESH::FT_BadOrientedVolume;
}

/*
  Class       : BareBorderVolume_i
  Description : Verify whether a mesh volume has a free facet without a face on it
*/
BareBorderVolume_i::BareBorderVolume_i()
{
  Controls::PredicatePtr control( new Controls::BareBorderVolume() );
  myFunctorPtr = myPredicatePtr = control;
};

FunctorType BareBorderVolume_i::GetFunctorType()
{
  return SMESH::FT_BareBorderVolume;
}

/*
  Class       : BareBorderFace_i
  Description : Verify whether a mesh face has a free border without an edge on it
*/
BareBorderFace_i::BareBorderFace_i()
{
  Controls::PredicatePtr control( new Controls::BareBorderFace() );
  myFunctorPtr = myPredicatePtr = control;
};

FunctorType BareBorderFace_i::GetFunctorType()
{
  return SMESH::FT_BareBorderFace;
}

/*
  Class       : OverConstrainedVolume_i
  Description : Verify whether a mesh volume has only one facet shared with other volumes
*/
OverConstrainedVolume_i::OverConstrainedVolume_i()
{
  Controls::PredicatePtr control( new Controls::OverConstrainedVolume() );
  myFunctorPtr = myPredicatePtr = control;
};

FunctorType OverConstrainedVolume_i::GetFunctorType()
{
  return SMESH::FT_OverConstrainedVolume;
}

/*
  Class       : OverConstrainedFace_i
  Description : Verify whether a mesh face has only one border shared with other faces
*/
OverConstrainedFace_i::OverConstrainedFace_i()
{
  Controls::PredicatePtr control( new Controls::OverConstrainedFace() );
  myFunctorPtr = myPredicatePtr = control;
};

FunctorType OverConstrainedFace_i::GetFunctorType()
{
  return SMESH::FT_OverConstrainedFace;
}

/*
  Class       : BelongToMeshGroup_i
  Description : Verify whether a mesh element is included into a mesh group
*/
BelongToMeshGroup_i::BelongToMeshGroup_i()
{
  myBelongToMeshGroup = Controls::BelongToMeshGroupPtr( new Controls::BelongToMeshGroup() );
  myFunctorPtr = myPredicatePtr = myBelongToMeshGroup;
}

BelongToMeshGroup_i::~BelongToMeshGroup_i()
{
  SetGroup( SMESH::SMESH_GroupBase::_nil() );
}

void BelongToMeshGroup_i::SetGroup( SMESH::SMESH_GroupBase_ptr theGroup )
{
  if ( myGroup->_is_equivalent( theGroup ))
    return;

  if ( ! myGroup->_is_nil() )
    myGroup->UnRegister();

  myGroup = SMESH_GroupBase::_duplicate( theGroup );

  myBelongToMeshGroup->SetGroup( 0 );
  if ( SMESH_GroupBase_i* gr_i = SMESH::DownCast< SMESH_GroupBase_i* >( myGroup ))
  {
    myBelongToMeshGroup->SetGroup( gr_i->GetGroupDS() );
    myGroup->Register();
  }
}

void BelongToMeshGroup_i::SetGroupID( const char* theID ) // IOR or StoreName
{
  myID = theID;
  if ( strncmp( "IOR:", myID.c_str(), 4 ) == 0 ) // transient mode, no GUI
  {
    CORBA::Object_var obj = SMESH_Gen_i::GetORB()->string_to_object( myID.c_str() );
    SetGroup( SMESH::SMESH_GroupBase::_narrow( obj ));
  }
  else if ( strncmp( "0:", myID.c_str(), 2 ) == 0 ) // transient mode + GUI
  {
    SMESH_Gen_i* aSMESHGen     = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var aStudy = aSMESHGen->GetCurrentStudy();
    if ( !aStudy->_is_nil() ) {
      SALOMEDS::SObject_wrap aSObj = aStudy->FindObjectID( myID.c_str() );
      if ( !aSObj->_is_nil() ) {
        CORBA::Object_var obj = aSObj->GetObject();
        SetGroup( SMESH::SMESH_GroupBase::_narrow( obj ));
      }
    }
  }
  else if ( !myID.empty() ) // persistent mode
  {
    myBelongToMeshGroup->SetStoreName( myID );
  }
}

std::string BelongToMeshGroup_i::GetGroupID()
{
  if ( myGroup->_is_nil() )
    SMESH::SMESH_GroupBase_var( GetGroup() );

  if ( !myGroup->_is_nil() )
    myID = SMESH_Gen_i::GetORB()->object_to_string( myGroup );

  return myID;
}

SMESH::SMESH_GroupBase_ptr BelongToMeshGroup_i::GetGroup()
{
  if ( myGroup->_is_nil() && myBelongToMeshGroup->GetGroup() )
  {
    // search for a group in a current study
    SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
    if ( StudyContext*  sc = aSMESHGen->GetCurrentStudyContext() )
    {
      int id = 1;
      std::string ior;
      while (true)
      {
        ior = sc->getIORbyId( id++ );
        if ( ior.empty() ) break;
        CORBA::Object_var obj = aSMESHGen->GetORB()->string_to_object( ior.c_str() );
        if ( SMESH_GroupBase_i* g_i = SMESH::DownCast<SMESH_GroupBase_i*>( obj ))
          if ( g_i->GetGroupDS() == myBelongToMeshGroup->GetGroup() )
          {
            SetGroup( g_i->_this() );
            break;
          }
      }
    }
  }
  return SMESH::SMESH_GroupBase::_duplicate( myGroup );
}

FunctorType BelongToMeshGroup_i::GetFunctorType()
{
  return SMESH::FT_BelongToMeshGroup;
}

/*
  Class       : BelongToGeom_i
  Description : Predicate for selection on geometrical support
*/
BelongToGeom_i::BelongToGeom_i()
{
  myBelongToGeomPtr.reset( new Controls::BelongToGeom() );
  myFunctorPtr = myPredicatePtr = myBelongToGeomPtr;
  myShapeName = 0;
  myShapeID   = 0;
}

BelongToGeom_i::~BelongToGeom_i()
{
  CORBA::string_free( myShapeName );
  CORBA::string_free( myShapeID );
}

void BelongToGeom_i::SetGeom( GEOM::GEOM_Object_ptr theGeom )
{
  if ( theGeom->_is_nil() )
    return;
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  GEOM::GEOM_Gen_ptr aGEOMGen = SMESH_Gen_i::GetGeomEngine();
  TopoDS_Shape aLocShape = aSMESHGen->GetShapeReader()->GetShape( aGEOMGen, theGeom );
  myBelongToGeomPtr->SetGeom( aLocShape );
  TPythonDump()<<this<<".SetGeom("<<theGeom<<")";
}

void BelongToGeom_i::SetGeom( const TopoDS_Shape& theShape )
{
  myBelongToGeomPtr->SetGeom( theShape );
}

void BelongToGeom_i::SetElementType(ElementType theType)
{
  myBelongToGeomPtr->SetType(SMDSAbs_ElementType(theType));
  TPythonDump()<<this<<".SetElementType("<<theType<<")";
}

FunctorType BelongToGeom_i::GetFunctorType()
{
  return SMESH::FT_BelongToGeom;
}

void BelongToGeom_i::SetShapeName( const char* theName )
{
  CORBA::string_free( myShapeName );
  myShapeName = CORBA::string_dup( theName );
  myBelongToGeomPtr->SetGeom( getShapeByName( myShapeName ) );
  TPythonDump()<<this<<".SetShapeName('"<<theName<<"')";
}

void BelongToGeom_i::SetShape( const char* theID, const char* theName )
{
  CORBA::string_free( myShapeName );
  CORBA::string_free( myShapeID );
  myShapeName = CORBA::string_dup( theName );
  myShapeID   = CORBA::string_dup( theID );
  bool hasName = ( theName && theName[0] );
  bool hasID   = ( theID   && theID[0] );

  TopoDS_Shape S;
  if ( hasName && hasID )
  {
    S = getShapeByID( myShapeID );
    if ( S.IsNull() )
      S = getShapeByName( myShapeName );
  }
  else
  {
    S = hasID ? getShapeByID( myShapeID ) : getShapeByName( myShapeName );
  }
  myBelongToGeomPtr->SetGeom( S );
}

char* BelongToGeom_i::GetShapeName()
{
  return CORBA::string_dup( myShapeName );
}

char* BelongToGeom_i::GetShapeID()
{
  return CORBA::string_dup( myShapeID );
}

void BelongToGeom_i::SetTolerance( CORBA::Double theToler )
{
  myBelongToGeomPtr->SetTolerance( theToler );
  TPythonDump()<<this<<".SetTolerance("<<theToler<<")";
}

CORBA::Double BelongToGeom_i::GetTolerance()
{
  return myBelongToGeomPtr->GetTolerance();
}

/*
  Class       : BelongToSurface_i
  Description : Predicate for selection on geometrical support
*/
BelongToSurface_i::BelongToSurface_i( const Handle(Standard_Type)& theSurfaceType )
{
  myElementsOnSurfacePtr.reset( new Controls::ElementsOnSurface() );
  myFunctorPtr = myPredicatePtr = myElementsOnSurfacePtr;
  myShapeName = 0;
  myShapeID   = 0;
  mySurfaceType = theSurfaceType;
}

BelongToSurface_i::~BelongToSurface_i()
{
  CORBA::string_free( myShapeName );
  CORBA::string_free( myShapeID );
}

void BelongToSurface_i::SetSurface( GEOM::GEOM_Object_ptr theGeom, ElementType theType )
{
  if ( theGeom->_is_nil() )
    return;
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  GEOM::GEOM_Gen_ptr aGEOMGen = SMESH_Gen_i::GetGeomEngine();
  TopoDS_Shape aLocShape = aSMESHGen->GetShapeReader()->GetShape( aGEOMGen, theGeom );

  if ( aLocShape.ShapeType() == TopAbs_FACE )
  {
    Handle(Geom_Surface) aSurf = BRep_Tool::Surface( TopoDS::Face( aLocShape ) );
    if ( !aSurf.IsNull() && aSurf->DynamicType() == mySurfaceType )
    {
      myElementsOnSurfacePtr->SetSurface( aLocShape, (SMDSAbs_ElementType)theType );
      return;
    }
  }

  myElementsOnSurfacePtr->SetSurface( TopoDS_Shape(), (SMDSAbs_ElementType)theType );
}

void BelongToSurface_i::SetShapeName( const char* theName, ElementType theType )
{
  CORBA::string_free( myShapeName );
  myShapeName = CORBA::string_dup( theName );
  myElementsOnSurfacePtr->SetSurface( getShapeByName( myShapeName ), (SMDSAbs_ElementType)theType );
  TPythonDump()<<this<<".SetShapeName('"<<theName<<"',"<<theType<<")";
}

void BelongToSurface_i::SetShape( const char* theID,  const char* theName, ElementType theType )
{
  CORBA::string_free( myShapeName );
  CORBA::string_free( myShapeID );
  myShapeName = CORBA::string_dup( theName );
  myShapeID   = CORBA::string_dup( theID );
  bool hasName = ( theName && theName[0] );
  bool hasID   = ( theID   && theID[0] );

  TopoDS_Shape S;
  if ( hasName && hasID )
  {
    S = getShapeByID( myShapeID );
    if ( S.IsNull() )
      S = getShapeByName( myShapeName );
  }
  else
  {
    S = hasID ? getShapeByID( myShapeID ) : getShapeByName( myShapeName );
  }
  myElementsOnSurfacePtr->SetSurface( S, (SMDSAbs_ElementType)theType );
}

char* BelongToSurface_i::GetShapeName()
{
  return CORBA::string_dup( myShapeName );
}

char* BelongToSurface_i::GetShapeID()
{
  return CORBA::string_dup( myShapeID );
}

void BelongToSurface_i::SetTolerance( CORBA::Double theToler )
{
  myElementsOnSurfacePtr->SetTolerance( theToler );
  TPythonDump()<<this<<".SetTolerance("<<theToler<<")";
}

CORBA::Double BelongToSurface_i::GetTolerance()
{
  return myElementsOnSurfacePtr->GetTolerance();
}

void BelongToSurface_i::SetUseBoundaries( CORBA::Boolean theUseBndRestrictions )
{
  myElementsOnSurfacePtr->SetUseBoundaries( theUseBndRestrictions );
  TPythonDump()<<this<<".SetUseBoundaries( " << theUseBndRestrictions << " )";
}

CORBA::Boolean BelongToSurface_i::GetUseBoundaries()
{
  return myElementsOnSurfacePtr->GetUseBoundaries();
}


/*
  Class       : BelongToPlane_i
  Description : Verify whether mesh element lie in pointed Geom planar object
*/

BelongToPlane_i::BelongToPlane_i()
: BelongToSurface_i( STANDARD_TYPE( Geom_Plane ) )
{
}

void BelongToPlane_i::SetPlane( GEOM::GEOM_Object_ptr theGeom, ElementType theType )
{
  BelongToSurface_i::SetSurface( theGeom, theType );
  TPythonDump()<<this<<".SetPlane("<<theGeom<<","<<theType<<")";
}

FunctorType BelongToPlane_i::GetFunctorType()
{
  return FT_BelongToPlane;
}

/*
  Class       : BelongToCylinder_i
  Description : Verify whether mesh element lie in pointed Geom planar object
*/

BelongToCylinder_i::BelongToCylinder_i()
: BelongToSurface_i( STANDARD_TYPE( Geom_CylindricalSurface ) )
{
}

void BelongToCylinder_i::SetCylinder( GEOM::GEOM_Object_ptr theGeom, ElementType theType )
{
  BelongToSurface_i::SetSurface( theGeom, theType );
  TPythonDump()<<this<<".SetCylinder("<<theGeom<<","<<theType<<")";
}

FunctorType BelongToCylinder_i::GetFunctorType()
{
  return FT_BelongToCylinder;
}

/*
  Class       : BelongToGenSurface_i
  Description : Verify whether mesh element lie in pointed Geom planar object
*/

BelongToGenSurface_i::BelongToGenSurface_i()
: BelongToSurface_i( STANDARD_TYPE( Geom_CylindricalSurface ) )
{
}

void BelongToGenSurface_i::SetSurface( GEOM::GEOM_Object_ptr theGeom, ElementType theType )
{
  if ( theGeom->_is_nil() )
    return;
  TopoDS_Shape aLocShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( theGeom );
  if ( !aLocShape.IsNull() && aLocShape.ShapeType() != TopAbs_FACE )
    aLocShape.Nullify();
  
  BelongToSurface_i::myElementsOnSurfacePtr->SetSurface( aLocShape, (SMDSAbs_ElementType)theType );
  TPythonDump()<<this<<".SetGenSurface("<<theGeom<<","<<theType<<")";
}

FunctorType BelongToGenSurface_i::GetFunctorType()
{
  return FT_BelongToGenSurface;
}

/*
  Class       : LyingOnGeom_i
  Description : Predicate for selection on geometrical support
*/
LyingOnGeom_i::LyingOnGeom_i()
{
  myLyingOnGeomPtr.reset( new Controls::LyingOnGeom() );
  myFunctorPtr = myPredicatePtr = myLyingOnGeomPtr;
  myShapeName = 0;
  myShapeID = 0;
}

LyingOnGeom_i::~LyingOnGeom_i()
{
  CORBA::string_free( myShapeName );
  CORBA::string_free( myShapeID );
}

void LyingOnGeom_i::SetGeom( GEOM::GEOM_Object_ptr theGeom )
{
  if ( theGeom->_is_nil() )
    return;
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  GEOM::GEOM_Gen_ptr aGEOMGen = SMESH_Gen_i::GetGeomEngine();
  TopoDS_Shape aLocShape = aSMESHGen->GetShapeReader()->GetShape( aGEOMGen, theGeom );
  myLyingOnGeomPtr->SetGeom( aLocShape );
  TPythonDump()<<this<<".SetGeom("<<theGeom<<")";
}

void LyingOnGeom_i::SetGeom( const TopoDS_Shape& theShape )
{
  myLyingOnGeomPtr->SetGeom( theShape );
}

void LyingOnGeom_i::SetElementType(ElementType theType){
  myLyingOnGeomPtr->SetType(SMDSAbs_ElementType(theType));
  TPythonDump()<<this<<".SetElementType("<<theType<<")";
}

FunctorType LyingOnGeom_i::GetFunctorType()
{
  return SMESH::FT_LyingOnGeom;
}

void LyingOnGeom_i::SetShapeName( const char* theName )
{
  CORBA::string_free( myShapeName );
  myShapeName = CORBA::string_dup( theName );
  myLyingOnGeomPtr->SetGeom( getShapeByName( myShapeName ) );
  TPythonDump()<<this<<".SetShapeName('"<<theName<<"')";
}

void LyingOnGeom_i::SetShape( const char* theID, const char* theName )
{
  CORBA::string_free( myShapeName );
  CORBA::string_free( myShapeID   );
  myShapeName = CORBA::string_dup( theName );
  myShapeID   = CORBA::string_dup( theID   );
  bool hasName = ( theName && theName[0] );
  bool hasID   = ( theID   && theID[0]   );

  TopoDS_Shape S;
  if ( hasName && hasID )
  {
    S = getShapeByID( myShapeID );
    if ( S.IsNull() )
      S = getShapeByName( myShapeName );
  }
  else
  {
    S = hasID ? getShapeByID( myShapeID ) : getShapeByName( myShapeName );
  }
  myLyingOnGeomPtr->SetGeom( S );
}

char* LyingOnGeom_i::GetShapeName()
{
  return CORBA::string_dup( myShapeName );
}

char* LyingOnGeom_i::GetShapeID()
{
  return CORBA::string_dup( myShapeID );
}

void LyingOnGeom_i::SetTolerance( CORBA::Double theToler )
{
  myLyingOnGeomPtr->SetTolerance( theToler );
  TPythonDump()<<this<<".SetTolerance("<<theToler<<")";
}

CORBA::Double LyingOnGeom_i::GetTolerance()
{
  return myLyingOnGeomPtr->GetTolerance();
}

/*
  Class       : FreeBorders_i
  Description : Predicate for free borders
*/
FreeBorders_i::FreeBorders_i()
{
  myPredicatePtr.reset(new Controls::FreeBorders());
  myFunctorPtr = myPredicatePtr;
}

FunctorType FreeBorders_i::GetFunctorType()
{
  return SMESH::FT_FreeBorders;
}

/*
  Class       : FreeEdges_i
  Description : Predicate for free borders
*/
FreeEdges_i::FreeEdges_i()
: myFreeEdgesPtr( new Controls::FreeEdges() )
{
  myFunctorPtr = myPredicatePtr = myFreeEdgesPtr;
}

SMESH::FreeEdges::Borders* FreeEdges_i::GetBorders()
{
  SMESH::Controls::FreeEdges::TBorders aBorders;
  myFreeEdgesPtr->GetBoreders( aBorders );

  long i = 0, iEnd = aBorders.size();

  SMESH::FreeEdges::Borders_var aResult = new SMESH::FreeEdges::Borders;
  aResult->length(iEnd);

  SMESH::Controls::FreeEdges::TBorders::const_iterator anIter;
  for ( anIter = aBorders.begin() ; anIter != aBorders.end(); anIter++, i++ )
  {
    const SMESH::Controls::FreeEdges::Border&  aBord = *anIter;
    SMESH::FreeEdges::Border &aBorder = aResult[ i ];

    aBorder.myElemId = aBord.myElemId;
    aBorder.myPnt1 = aBord.myPntId[ 0 ];
    aBorder.myPnt2 = aBord.myPntId[ 1 ];
  }
  return aResult._retn();
}

FunctorType FreeEdges_i::GetFunctorType()
{
  return SMESH::FT_FreeEdges;
}

/*
  Class       : FreeFaces_i
  Description : Predicate for free faces
*/
FreeFaces_i::FreeFaces_i()
{
  myPredicatePtr.reset(new Controls::FreeFaces());
  myFunctorPtr = myPredicatePtr;
}

FunctorType FreeFaces_i::GetFunctorType()
{
  return SMESH::FT_FreeFaces;
}

/*
  Class       : FreeNodes_i
  Description : Predicate for free nodes
*/
FreeNodes_i::FreeNodes_i()
{
  myPredicatePtr.reset(new Controls::FreeNodes());
  myFunctorPtr = myPredicatePtr;
}

FunctorType FreeNodes_i::GetFunctorType()
{
  return SMESH::FT_FreeNodes;
}

/*
  Class       : EqualNodes_i
  Description : Predicate for Equal nodes
*/
EqualNodes_i::EqualNodes_i()
{
  myCoincidentNodesPtr.reset(new Controls::CoincidentNodes());
  myFunctorPtr = myPredicatePtr = myCoincidentNodesPtr;
}

FunctorType EqualNodes_i::GetFunctorType()
{
  return SMESH::FT_EqualNodes;
}

void EqualNodes_i::SetTolerance( double tol )
{
  myCoincidentNodesPtr->SetTolerance( tol );
}

double EqualNodes_i::GetTolerance()
{
  return myCoincidentNodesPtr->GetTolerance();
}

/*
  Class       : EqualEdges_i
  Description : Predicate for Equal Edges
*/
EqualEdges_i::EqualEdges_i()
{
  myPredicatePtr.reset(new Controls::CoincidentElements1D());
  myFunctorPtr = myPredicatePtr;
}

FunctorType EqualEdges_i::GetFunctorType()
{
  return SMESH::FT_EqualEdges;
}

/*
  Class       : EqualFaces_i
  Description : Predicate for Equal Faces
*/
EqualFaces_i::EqualFaces_i()
{
  myPredicatePtr.reset(new Controls::CoincidentElements2D());
  myFunctorPtr = myPredicatePtr;
}

FunctorType EqualFaces_i::GetFunctorType()
{
  return SMESH::FT_EqualFaces;
}

/*
  Class       : EqualVolumes_i
  Description : Predicate for Equal Volumes
*/
EqualVolumes_i::EqualVolumes_i()
{
  myPredicatePtr.reset(new Controls::CoincidentElements3D());
  myFunctorPtr = myPredicatePtr;
}

FunctorType EqualVolumes_i::GetFunctorType()
{
  return SMESH::FT_EqualVolumes;
}


/*
  Class       : RangeOfIds_i
  Description : Predicate for Range of Ids.
                Range may be specified with two ways.
                1. Using AddToRange method
                2. With SetRangeStr method. Parameter of this method is a string
                   like as "1,2,3,50-60,63,67,70-"
*/

RangeOfIds_i::RangeOfIds_i()
{
  myRangeOfIdsPtr.reset( new Controls::RangeOfIds() );
  myFunctorPtr = myPredicatePtr = myRangeOfIdsPtr;
}

void RangeOfIds_i::SetRange( const SMESH::long_array& theIds )
{
  CORBA::Long iEnd = theIds.length();
  for ( CORBA::Long i = 0; i < iEnd; i++ )
    myRangeOfIdsPtr->AddToRange( theIds[ i ] );
  TPythonDump()<<this<<".SetRange("<<theIds<<")";
}

CORBA::Boolean RangeOfIds_i::SetRangeStr( const char* theRange )
{
  TPythonDump()<<this<<".SetRangeStr('"<<theRange<<"')";
  return myRangeOfIdsPtr->SetRangeStr(
    TCollection_AsciiString( (Standard_CString)theRange ) );
}

char* RangeOfIds_i::GetRangeStr()
{
  TCollection_AsciiString aStr;
  myRangeOfIdsPtr->GetRangeStr( aStr );
  return CORBA::string_dup( aStr.ToCString() );
}

void RangeOfIds_i::SetElementType( ElementType theType )
{
  myRangeOfIdsPtr->SetType( SMDSAbs_ElementType( theType ) );
  TPythonDump()<<this<<".SetElementType("<<theType<<")";
}

FunctorType RangeOfIds_i::GetFunctorType()
{
  return SMESH::FT_RangeOfIds;
}

/*
  Class       : LinearOrQuadratic_i
  Description : Predicate to verify whether a mesh element is linear
*/
LinearOrQuadratic_i::LinearOrQuadratic_i()
{
  myLinearOrQuadraticPtr.reset(new Controls::LinearOrQuadratic());
  myFunctorPtr = myPredicatePtr = myLinearOrQuadraticPtr;
}

void LinearOrQuadratic_i::SetElementType(ElementType theType)
{
  myLinearOrQuadraticPtr->SetType(SMDSAbs_ElementType(theType));
  TPythonDump()<<this<<".SetElementType("<<theType<<")";
}

FunctorType LinearOrQuadratic_i::GetFunctorType()
{
  return SMESH::FT_LinearOrQuadratic;
}

/*
  Class       : GroupColor_i
  Description : Functor for check color of group to whic mesh element belongs to
*/
GroupColor_i::GroupColor_i()
{
  myGroupColorPtr.reset(new Controls::GroupColor());
  myFunctorPtr = myPredicatePtr = myGroupColorPtr;
}

FunctorType GroupColor_i::GetFunctorType()
{
  return SMESH::FT_GroupColor;
}

void GroupColor_i::SetColorStr( const char* theColor )
{
  myGroupColorPtr->SetColorStr(
    TCollection_AsciiString( (Standard_CString)theColor ) );
  TPythonDump()<<this<<".SetColorStr('"<<theColor<<"')";
}

char* GroupColor_i::GetColorStr()
{
  TCollection_AsciiString aStr;
  myGroupColorPtr->GetColorStr( aStr );
  return CORBA::string_dup( aStr.ToCString() );
}

void GroupColor_i::SetElementType(ElementType theType)
{
  myGroupColorPtr->SetType(SMDSAbs_ElementType(theType));
  TPythonDump()<<this<<".SetElementType("<<theType<<")";
}

/*
  Class       : ElemGeomType_i
  Description : Predicate check is element has indicated geometry type
*/
ElemGeomType_i::ElemGeomType_i()
{
  myElemGeomTypePtr.reset(new Controls::ElemGeomType());
  myFunctorPtr = myPredicatePtr = myElemGeomTypePtr;
}

void ElemGeomType_i::SetElementType(ElementType theType)
{
  myElemGeomTypePtr->SetType(SMDSAbs_ElementType(theType));
  TPythonDump()<<this<<".SetElementType("<<theType<<")";
}

void ElemGeomType_i::SetGeometryType(GeometryType theType)
{
  myElemGeomTypePtr->SetGeomType(SMDSAbs_GeometryType(theType));
  TPythonDump()<<this<<".SetGeometryType("<<theType<<")";
}

GeometryType ElemGeomType_i::GetGeometryType() const
{
  return (GeometryType)myElemGeomTypePtr->GetGeomType();
}

FunctorType ElemGeomType_i::GetFunctorType()
{
  return SMESH::FT_ElemGeomType;
}

/*
  Class       : ElemEntityType_i
  Description : Predicate check is element has indicated entity type
*/
ElemEntityType_i::ElemEntityType_i()
{
  myElemEntityTypePtr.reset(new Controls::ElemEntityType());
  myFunctorPtr = myPredicatePtr = myElemEntityTypePtr;
}

void ElemEntityType_i::SetElementType(ElementType theType)
{
  myElemEntityTypePtr->SetType(SMDSAbs_ElementType(theType));
  TPythonDump()<<this<<".SetElementType("<<theType<<")";
}

void ElemEntityType_i::SetEntityType(EntityType theEntityType)
{
  myElemEntityTypePtr->SetElemEntityType(SMDSAbs_EntityType (theEntityType));
  TPythonDump()<<this<<".SetEntityType("<<theEntityType<<")";
}
EntityType ElemEntityType_i::GetEntityType() const
{
 return (EntityType) myElemEntityTypePtr->GetElemEntityType();
}

FunctorType ElemEntityType_i::GetFunctorType()
{
  return SMESH::FT_EntityType;
}

/*
  Class       : CoplanarFaces_i
  Description : Returns true if a mesh face is a coplanar neighbour to a given one
*/
CoplanarFaces_i::CoplanarFaces_i()
{
  myCoplanarFacesPtr.reset(new Controls::CoplanarFaces());
  myFunctorPtr = myPredicatePtr = myCoplanarFacesPtr;
}

void CoplanarFaces_i::SetFace ( CORBA::Long theFaceID )
{
  myCoplanarFacesPtr->SetFace(theFaceID);
  TPythonDump()<<this<<".SetFace("<<theFaceID<<")";
}

void CoplanarFaces_i::SetTolerance( CORBA::Double theToler )
{
  myCoplanarFacesPtr->SetTolerance(theToler);
  TPythonDump()<<this<<".SetTolerance("<<theToler<<")";
}

CORBA::Long CoplanarFaces_i::GetFace () const
{
  return myCoplanarFacesPtr->GetFace();
}

char* CoplanarFaces_i::GetFaceAsString () const
{
  TCollection_AsciiString str(Standard_Integer(myCoplanarFacesPtr->GetFace()));
  return CORBA::string_dup( str.ToCString() );
}

CORBA::Double CoplanarFaces_i::GetTolerance() const
{
  return myCoplanarFacesPtr->GetTolerance();
}

FunctorType CoplanarFaces_i::GetFunctorType()
{
  return SMESH::FT_CoplanarFaces;
}

/*
 * Class       : ConnectedElements_i
 * Description : Returns true if an element is connected via other elements to the element
 *               located at a given point.
 */
ConnectedElements_i::ConnectedElements_i()
{
  myConnectedElementsPtr.reset(new Controls::ConnectedElements());
  myFunctorPtr = myPredicatePtr = myConnectedElementsPtr;
}

FunctorType ConnectedElements_i::GetFunctorType()
{
  return FT_ConnectedElements;
}

void ConnectedElements_i::SetElementType( ElementType theType )
{
  myConnectedElementsPtr->SetType( SMDSAbs_ElementType( theType ));
  TPythonDump() << this << ".SetElementType( " << theType << " )";
}

void ConnectedElements_i::SetPoint( CORBA::Double x, CORBA::Double y, CORBA::Double z )
{
  myConnectedElementsPtr->SetPoint( x,y,z );
  myVertexID.clear();
  TPythonDump() << this << ".SetPoint( " << x << ", " << y << ", " << z << " )";
}

void ConnectedElements_i::SetVertex( GEOM::GEOM_Object_ptr vertex )
  throw (SALOME::SALOME_Exception)
{
  TopoDS_Shape shape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( vertex );
  if ( shape.IsNull() )
    THROW_SALOME_CORBA_EXCEPTION( "ConnectedElements_i::SetVertex(): NULL Vertex",
                                  SALOME::BAD_PARAM );

  TopExp_Explorer v( shape, TopAbs_VERTEX );
  if ( !v.More() )
    THROW_SALOME_CORBA_EXCEPTION( "ConnectedElements_i::SetVertex(): empty vertex",
                                  SALOME::BAD_PARAM );

  gp_Pnt p = BRep_Tool::Pnt( TopoDS::Vertex( v.Current() ));
  myConnectedElementsPtr->SetPoint( p.X(), p.Y(), p.Z() );
  //
  CORBA::String_var id = vertex->GetStudyEntry();
  myVertexID = id.in();

  TPythonDump() << this << ".SetVertex( " << vertex << " )";
}

void ConnectedElements_i::SetNode ( CORBA::Long nodeID )
  throw (SALOME::SALOME_Exception)
{
  if ( nodeID < 1 )
    THROW_SALOME_CORBA_EXCEPTION( "ConnectedElements_i::SetNode(): nodeID must be > 0",
                                  SALOME::BAD_PARAM );

  myConnectedElementsPtr->SetNode( nodeID );
  myVertexID.clear();
  TPythonDump() << this << ".SetNode( " << nodeID << " )";
}

/*!
 * \brief This is a comfort method for Filter dialog
 */
void ConnectedElements_i::SetThreshold ( const char*                             threshold,
                                         SMESH::ConnectedElements::ThresholdType type )
  throw (SALOME::SALOME_Exception)
{
  if ( !threshold )
    THROW_SALOME_CORBA_EXCEPTION( "ConnectedElements_i::SetThreshold(): NULL threshold",
                                  SALOME::BAD_PARAM );
  switch ( type )
  {
  case SMESH::ConnectedElements::POINT: // read 3 node coordinates ///////////////////
    {
      std::vector< double > xyz;
      char* endptr;
      do
      {
        // skip a separator
        while ( *threshold &&
                *threshold != '+' &&
                *threshold != '-' &&
                !isdigit( *threshold ))
          ++threshold;
        if ( !*threshold )
          break;
        // read a coordinate
        xyz.push_back( strtod( threshold, &endptr ));
        if ( threshold == endptr )
        {
          xyz.resize( xyz.size() - 1 );
          break;
        }
        threshold = endptr;
      }
      while ( xyz.size() < 3 );

      if ( xyz.size() < 3 )
        THROW_SALOME_CORBA_EXCEPTION
          ( "ConnectedElements_i::SetThreshold(): invalid point coordinates", SALOME::BAD_PARAM );

      SetPoint( xyz[0], xyz[1], xyz[2] );
      break;
    }
  case SMESH::ConnectedElements::VERTEX: // get a VERTEX by its entry /////////////////
    {
      SALOMEDS::Study_var study = SMESH_Gen_i::GetSMESHGen()->GetCurrentStudy();
      if ( study->_is_nil() )
        THROW_SALOME_CORBA_EXCEPTION
          ( "ConnectedElements_i::SetThreshold(): NULL current study", SALOME::BAD_PARAM );
      SALOMEDS::SObject_wrap sobj = study->FindObjectID( threshold );
      if ( sobj->_is_nil() )
        THROW_SALOME_CORBA_EXCEPTION
          ( "ConnectedElements_i::SetThreshold(): invalid vertex study entry", SALOME::BAD_PARAM );
      CORBA::Object_var        obj = sobj->GetObject();
      GEOM::GEOM_Object_var vertex = GEOM::GEOM_Object::_narrow( obj );
      if ( vertex->_is_nil() )
        THROW_SALOME_CORBA_EXCEPTION
          ( "ConnectedElements_i::SetThreshold(): no GEOM_Object in SObject", SALOME::BAD_PARAM );
      SetVertex( vertex );
      break;
    }
  case SMESH::ConnectedElements::NODE: // read a node ID ////////////////////////////
    {
      char* endptr;
      int id = strtol( threshold, &endptr, 10 );
      if ( threshold == endptr )
        THROW_SALOME_CORBA_EXCEPTION
          ( "ConnectedElements_i::SetThreshold(): invalid node ID", SALOME::BAD_PARAM );
      SetNode( id );
      break;
    }
  default:
    THROW_SALOME_CORBA_EXCEPTION
      ( "ConnectedElements_i::SetThreshold(): invalid ThresholdType", SALOME::BAD_PARAM );
  }
}

char* ConnectedElements_i::GetThreshold ( SMESH::ConnectedElements::ThresholdType& type )
{
  std::string threshold;
  if ( !myVertexID.empty() )
  {
    threshold = myVertexID;
    type      = SMESH::ConnectedElements::VERTEX;
  }
  else
  {
    std::vector<double> xyz = myConnectedElementsPtr->GetPoint();
    if ( xyz.size() == 3 )
    {
      threshold = SMESH_Comment( xyz[0] ) << "; " << xyz[1] << "; " << xyz[2];
      type      = SMESH::ConnectedElements::POINT;
    }
    else
    {
      threshold = SMESH_Comment( myConnectedElementsPtr->GetNode() );
      type      = SMESH::ConnectedElements::NODE;
    }
  }
  return CORBA::string_dup( threshold.c_str() );
}

/*
  Class       : Comparator_i
  Description : Base class for comparators
*/
Comparator_i::Comparator_i():
  myNumericalFunctor( NULL )
{}

Comparator_i::~Comparator_i()
{
  if ( myNumericalFunctor )
    myNumericalFunctor->UnRegister();
}

void Comparator_i::SetMargin( CORBA::Double theValue )
{
  myComparatorPtr->SetMargin( theValue );
  TPythonDump()<<this<<".SetMargin("<<theValue<<")";
}

CORBA::Double Comparator_i::GetMargin()
{
  return myComparatorPtr->GetMargin();
}

void Comparator_i::SetNumFunctor( NumericalFunctor_ptr theFunct )
{
  if ( myNumericalFunctor )
    myNumericalFunctor->UnRegister();

  myNumericalFunctor = DownCast<NumericalFunctor_i*>(theFunct);

  if ( myNumericalFunctor )
  {
    myComparatorPtr->SetNumFunctor( myNumericalFunctor->GetNumericalFunctor() );
    myNumericalFunctor->Register();
    TPythonDump()<<this<<".SetNumFunctor("<<myNumericalFunctor<<")";
  }
}

Controls::ComparatorPtr Comparator_i::GetComparator()
{
  return myComparatorPtr;
}

NumericalFunctor_i* Comparator_i::GetNumFunctor_i()
{
  return myNumericalFunctor;
}


/*
  Class       : LessThan_i
  Description : Comparator "<"
*/
LessThan_i::LessThan_i()
{
  myComparatorPtr.reset( new Controls::LessThan() );
  myFunctorPtr = myPredicatePtr = myComparatorPtr;
}

FunctorType LessThan_i::GetFunctorType()
{
  return SMESH::FT_LessThan;
}


/*
  Class       : MoreThan_i
  Description : Comparator ">"
*/
MoreThan_i::MoreThan_i()
{
  myComparatorPtr.reset( new Controls::MoreThan() );
  myFunctorPtr = myPredicatePtr = myComparatorPtr;
}

FunctorType MoreThan_i::GetFunctorType()
{
  return SMESH::FT_MoreThan;
}


/*
  Class       : EqualTo_i
  Description : Comparator "="
*/
EqualTo_i::EqualTo_i()
: myEqualToPtr( new Controls::EqualTo() )
{
  myFunctorPtr = myPredicatePtr = myComparatorPtr = myEqualToPtr;
}

void EqualTo_i::SetTolerance( CORBA::Double theToler )
{
  myEqualToPtr->SetTolerance( theToler );
  TPythonDump()<<this<<".SetTolerance("<<theToler<<")";
}

CORBA::Double EqualTo_i::GetTolerance()
{
  return myEqualToPtr->GetTolerance();
}

FunctorType EqualTo_i::GetFunctorType()
{
  return SMESH::FT_EqualTo;
}

/*
  Class       : LogicalNOT_i
  Description : Logical NOT predicate
*/
LogicalNOT_i::LogicalNOT_i():
  myLogicalNOTPtr( new Controls::LogicalNOT() ),
  myPredicate( NULL )
{
  myFunctorPtr = myPredicatePtr = myLogicalNOTPtr;
}

LogicalNOT_i::~LogicalNOT_i()
{
  if ( myPredicate )
    myPredicate->UnRegister();
}

void LogicalNOT_i::SetPredicate( Predicate_ptr thePredicate )
{
  if ( myPredicate )
    myPredicate->UnRegister();

  myPredicate = SMESH::GetPredicate(thePredicate);

  if ( myPredicate ){
    myLogicalNOTPtr->SetPredicate(myPredicate->GetPredicate());
    myPredicate->Register();
    TPythonDump()<<this<<".SetPredicate("<<myPredicate<<")";
  }
}

FunctorType LogicalNOT_i::GetFunctorType()
{
  return SMESH::FT_LogicalNOT;
}

Predicate_i* LogicalNOT_i::GetPredicate_i()
{
  return myPredicate;
}



/*
  Class       : LogicalBinary_i
  Description : Base class for binary logical predicate
*/
LogicalBinary_i::LogicalBinary_i()
: myPredicate1( NULL ),
  myPredicate2( NULL )
{}

LogicalBinary_i::~LogicalBinary_i()
{
  if ( myPredicate1 )
    myPredicate1->UnRegister();

  if ( myPredicate2 )
    myPredicate2->UnRegister();
}

void LogicalBinary_i::SetMesh( SMESH_Mesh_ptr theMesh )
{
  if ( myPredicate1 )
    myPredicate1->SetMesh( theMesh );

  if ( myPredicate2 )
    myPredicate2->SetMesh( theMesh );
}

void LogicalBinary_i::SetPredicate1( Predicate_ptr thePredicate )
{
  if ( myPredicate1 )
    myPredicate1->UnRegister();

  myPredicate1 = SMESH::GetPredicate(thePredicate);

  if ( myPredicate1 ){
    myLogicalBinaryPtr->SetPredicate1(myPredicate1->GetPredicate());
    myPredicate1->Register();
    TPythonDump()<<this<<".SetPredicate1("<<myPredicate1<<")";
  }
}

void LogicalBinary_i::SetPredicate2( Predicate_ptr thePredicate )
{
  if ( myPredicate2 )
    myPredicate2->UnRegister();

  myPredicate2 = SMESH::GetPredicate(thePredicate);

  if ( myPredicate2 ){
    myLogicalBinaryPtr->SetPredicate2(myPredicate2->GetPredicate());
    myPredicate2->Register();
    TPythonDump()<<this<<".SetPredicate2("<<myPredicate2<<")";
  }
}

Controls::LogicalBinaryPtr LogicalBinary_i::GetLogicalBinary()
{
  return myLogicalBinaryPtr;
}

Predicate_i* LogicalBinary_i::GetPredicate1_i()
{
  return myPredicate1;
}
Predicate_i* LogicalBinary_i::GetPredicate2_i()
{
  return myPredicate2;
}


/*
  Class       : LogicalAND_i
  Description : Logical AND
*/
LogicalAND_i::LogicalAND_i()
{
  myLogicalBinaryPtr.reset( new Controls::LogicalAND() );
  myFunctorPtr = myPredicatePtr = myLogicalBinaryPtr;
}

FunctorType LogicalAND_i::GetFunctorType()
{
  return SMESH::FT_LogicalAND;
}


/*
  Class       : LogicalOR_i
  Description : Logical OR
*/
LogicalOR_i::LogicalOR_i()
{
  myLogicalBinaryPtr.reset( new Controls::LogicalOR() );
  myFunctorPtr = myPredicatePtr = myLogicalBinaryPtr;
}

FunctorType LogicalOR_i::GetFunctorType()
{
  return SMESH::FT_LogicalOR;
}


/*
                            FILTER MANAGER
*/

FilterManager_i::FilterManager_i()
: SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  //Base class Salome_GenericObject do it inmplicitly by overriding PortableServer::POA_ptr _default_POA() method
  //PortableServer::ObjectId_var anObjectId =
  //  SMESH_Gen_i::GetPOA()->activate_object( this );
}


FilterManager_i::~FilterManager_i()
{
  //TPythonDump()<<this<<".UnRegister()";
}


MinimumAngle_ptr FilterManager_i::CreateMinimumAngle()
{
  SMESH::MinimumAngle_i* aServant = new SMESH::MinimumAngle_i();
  SMESH::MinimumAngle_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateMinimumAngle()";
  return anObj._retn();
}


AspectRatio_ptr FilterManager_i::CreateAspectRatio()
{
  SMESH::AspectRatio_i* aServant = new SMESH::AspectRatio_i();
  SMESH::AspectRatio_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateAspectRatio()";
  return anObj._retn();
}


AspectRatio3D_ptr FilterManager_i::CreateAspectRatio3D()
{
  SMESH::AspectRatio3D_i* aServant = new SMESH::AspectRatio3D_i();
  SMESH::AspectRatio3D_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateAspectRatio3D()";
  return anObj._retn();
}


Warping_ptr FilterManager_i::CreateWarping()
{
  SMESH::Warping_i* aServant = new SMESH::Warping_i();
  SMESH::Warping_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateWarping()";
  return anObj._retn();
}


Taper_ptr FilterManager_i::CreateTaper()
{
  SMESH::Taper_i* aServant = new SMESH::Taper_i();
  SMESH::Taper_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateTaper()";
  return anObj._retn();
}


Skew_ptr FilterManager_i::CreateSkew()
{
  SMESH::Skew_i* aServant = new SMESH::Skew_i();
  SMESH::Skew_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateSkew()";
  return anObj._retn();
}


Area_ptr FilterManager_i::CreateArea()
{
  SMESH::Area_i* aServant = new SMESH::Area_i();
  SMESH::Area_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateArea()";
  return anObj._retn();
}


Volume3D_ptr FilterManager_i::CreateVolume3D()
{
  SMESH::Volume3D_i* aServant = new SMESH::Volume3D_i();
  SMESH::Volume3D_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateVolume3D()";
  return anObj._retn();
}


MaxElementLength2D_ptr FilterManager_i::CreateMaxElementLength2D()
{
  SMESH::MaxElementLength2D_i* aServant = new SMESH::MaxElementLength2D_i();
  SMESH::MaxElementLength2D_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateMaxElementLength2D()";
  return anObj._retn();
}


MaxElementLength3D_ptr FilterManager_i::CreateMaxElementLength3D()
{
  SMESH::MaxElementLength3D_i* aServant = new SMESH::MaxElementLength3D_i();
  SMESH::MaxElementLength3D_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateMaxElementLength3D()";
  return anObj._retn();
}


Length_ptr FilterManager_i::CreateLength()
{
  SMESH::Length_i* aServant = new SMESH::Length_i();
  SMESH::Length_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLength()";
  return anObj._retn();
}

Length2D_ptr FilterManager_i::CreateLength2D()
{
  SMESH::Length2D_i* aServant = new SMESH::Length2D_i();
  SMESH::Length2D_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLength2D()";
  return anObj._retn();
}

MultiConnection_ptr FilterManager_i::CreateMultiConnection()
{
  SMESH::MultiConnection_i* aServant = new SMESH::MultiConnection_i();
  SMESH::MultiConnection_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateMultiConnection()";
  return anObj._retn();
}

MultiConnection2D_ptr FilterManager_i::CreateMultiConnection2D()
{
  SMESH::MultiConnection2D_i* aServant = new SMESH::MultiConnection2D_i();
  SMESH::MultiConnection2D_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateMultiConnection2D()";
  return anObj._retn();
}

BallDiameter_ptr FilterManager_i::CreateBallDiameter()
{
  SMESH::BallDiameter_i* aServant = new SMESH::BallDiameter_i();
  SMESH::BallDiameter_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBallDiameter()";
  return anObj._retn();
}

NodeConnectivityNumber_ptr FilterManager_i::CreateNodeConnectivityNumber()
{
  SMESH::NodeConnectivityNumber_i* aServant = new SMESH::NodeConnectivityNumber_i();
  SMESH::NodeConnectivityNumber_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateNodeConnectivityNumber()";
  return anObj._retn();
}

BelongToMeshGroup_ptr FilterManager_i::CreateBelongToMeshGroup()
{
  SMESH::BelongToMeshGroup_i* aServant = new SMESH::BelongToMeshGroup_i();
  SMESH::BelongToMeshGroup_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBelongToMeshGroup()";
  return anObj._retn();
}

BelongToGeom_ptr FilterManager_i::CreateBelongToGeom()
{
  SMESH::BelongToGeom_i* aServant = new SMESH::BelongToGeom_i();
  SMESH::BelongToGeom_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBelongToGeom()";
  return anObj._retn();
}

BelongToPlane_ptr FilterManager_i::CreateBelongToPlane()
{
  SMESH::BelongToPlane_i* aServant = new SMESH::BelongToPlane_i();
  SMESH::BelongToPlane_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBelongToPlane()";
  return anObj._retn();
}

BelongToCylinder_ptr FilterManager_i::CreateBelongToCylinder()
{
  SMESH::BelongToCylinder_i* aServant = new SMESH::BelongToCylinder_i();
  SMESH::BelongToCylinder_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBelongToCylinder()";
  return anObj._retn();
}

BelongToGenSurface_ptr FilterManager_i::CreateBelongToGenSurface()
{
  SMESH::BelongToGenSurface_i* aServant = new SMESH::BelongToGenSurface_i();
  SMESH::BelongToGenSurface_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBelongToGenSurface()";
  return anObj._retn();
}

LyingOnGeom_ptr FilterManager_i::CreateLyingOnGeom()
{
  SMESH::LyingOnGeom_i* aServant = new SMESH::LyingOnGeom_i();
  SMESH::LyingOnGeom_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLyingOnGeom()";
  return anObj._retn();
}

CoplanarFaces_ptr FilterManager_i::CreateCoplanarFaces()
{
  SMESH::CoplanarFaces_i* aServant = new SMESH::CoplanarFaces_i();
  SMESH::CoplanarFaces_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateCoplanarFaces()";
  return anObj._retn();
}

ConnectedElements_ptr FilterManager_i::CreateConnectedElements()
{
  SMESH::ConnectedElements_i* aServant = new SMESH::ConnectedElements_i();
  SMESH::ConnectedElements_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateConnectedElements()";
  return anObj._retn();
}

FreeBorders_ptr FilterManager_i::CreateFreeBorders()
{
  SMESH::FreeBorders_i* aServant = new SMESH::FreeBorders_i();
  SMESH::FreeBorders_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateFreeBorders()";
  return anObj._retn();
}

FreeEdges_ptr FilterManager_i::CreateFreeEdges()
{
  SMESH::FreeEdges_i* aServant = new SMESH::FreeEdges_i();
  SMESH::FreeEdges_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateFreeEdges()";
  return anObj._retn();
}

FreeFaces_ptr FilterManager_i::CreateFreeFaces()
{
  SMESH::FreeFaces_i* aServant = new SMESH::FreeFaces_i();
  SMESH::FreeFaces_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateFreeFaces()";
  return anObj._retn();
}

FreeNodes_ptr FilterManager_i::CreateFreeNodes()
{
  SMESH::FreeNodes_i* aServant = new SMESH::FreeNodes_i();
  SMESH::FreeNodes_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateFreeNodes()";
  return anObj._retn();
}

EqualNodes_ptr FilterManager_i::CreateEqualNodes()
{
  SMESH::EqualNodes_i* aServant = new SMESH::EqualNodes_i();
  SMESH::EqualNodes_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateEqualNodes()";
  return anObj._retn();
}

EqualEdges_ptr FilterManager_i::CreateEqualEdges()
{
  SMESH::EqualEdges_i* aServant = new SMESH::EqualEdges_i();
  SMESH::EqualEdges_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateEqualEdges()";
  return anObj._retn();
}
EqualFaces_ptr FilterManager_i::CreateEqualFaces()
{
  SMESH::EqualFaces_i* aServant = new SMESH::EqualFaces_i();
  SMESH::EqualFaces_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateEqualFaces()";
  return anObj._retn();
}
EqualVolumes_ptr FilterManager_i::CreateEqualVolumes()
{
  SMESH::EqualVolumes_i* aServant = new SMESH::EqualVolumes_i();
  SMESH::EqualVolumes_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateEqualVolumes()";
  return anObj._retn();
}

RangeOfIds_ptr FilterManager_i::CreateRangeOfIds()
{
  SMESH::RangeOfIds_i* aServant = new SMESH::RangeOfIds_i();
  SMESH::RangeOfIds_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateRangeOfIds()";
  return anObj._retn();
}

BadOrientedVolume_ptr FilterManager_i::CreateBadOrientedVolume()
{
  SMESH::BadOrientedVolume_i* aServant = new SMESH::BadOrientedVolume_i();
  SMESH::BadOrientedVolume_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBadOrientedVolume()";
  return anObj._retn();
}

BareBorderVolume_ptr FilterManager_i::CreateBareBorderVolume()
{
  SMESH::BareBorderVolume_i* aServant = new SMESH::BareBorderVolume_i();
  SMESH::BareBorderVolume_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBareBorderVolume()";
  return anObj._retn();
}

BareBorderFace_ptr FilterManager_i::CreateBareBorderFace()
{
  SMESH::BareBorderFace_i* aServant = new SMESH::BareBorderFace_i();
  SMESH::BareBorderFace_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateBareBorderFace()";
  return anObj._retn();
}

OverConstrainedVolume_ptr FilterManager_i::CreateOverConstrainedVolume()
{
  SMESH::OverConstrainedVolume_i* aServant = new SMESH::OverConstrainedVolume_i();
  SMESH::OverConstrainedVolume_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateOverConstrainedVolume()";
  return anObj._retn();
}

OverConstrainedFace_ptr FilterManager_i::CreateOverConstrainedFace()
{
  SMESH::OverConstrainedFace_i* aServant = new SMESH::OverConstrainedFace_i();
  SMESH::OverConstrainedFace_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateOverConstrainedFace()";
  return anObj._retn();
}

LessThan_ptr FilterManager_i::CreateLessThan()
{
  SMESH::LessThan_i* aServant = new SMESH::LessThan_i();
  SMESH::LessThan_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLessThan()";
  return anObj._retn();
}

MoreThan_ptr FilterManager_i::CreateMoreThan()
{
  SMESH::MoreThan_i* aServant = new SMESH::MoreThan_i();
  SMESH::MoreThan_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateMoreThan()";
  return anObj._retn();
}

EqualTo_ptr FilterManager_i::CreateEqualTo()
{
  SMESH::EqualTo_i* aServant = new SMESH::EqualTo_i();
  SMESH::EqualTo_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateEqualTo()";
  return anObj._retn();
}

LogicalNOT_ptr FilterManager_i::CreateLogicalNOT()
{
  SMESH::LogicalNOT_i* aServant = new SMESH::LogicalNOT_i();
  SMESH::LogicalNOT_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLogicalNOT()";
  return anObj._retn();
}

LogicalAND_ptr FilterManager_i::CreateLogicalAND()
{
  SMESH::LogicalAND_i* aServant = new SMESH::LogicalAND_i();
  SMESH::LogicalAND_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLogicalAND()";
  return anObj._retn();
}

LogicalOR_ptr FilterManager_i::CreateLogicalOR()
{
  SMESH::LogicalOR_i* aServant = new SMESH::LogicalOR_i();
  SMESH::LogicalOR_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLogicalOR()";
  return anObj._retn();
}

LinearOrQuadratic_ptr FilterManager_i::CreateLinearOrQuadratic()
{
  SMESH::LinearOrQuadratic_i* aServant = new SMESH::LinearOrQuadratic_i();
  SMESH::LinearOrQuadratic_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLinearOrQuadratic()";
  return anObj._retn();
}

GroupColor_ptr FilterManager_i::CreateGroupColor()
{
  SMESH::GroupColor_i* aServant = new SMESH::GroupColor_i();
  SMESH::GroupColor_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateGroupColor()";
  return anObj._retn();
}

ElemGeomType_ptr FilterManager_i::CreateElemGeomType()
{
  SMESH::ElemGeomType_i* aServant = new SMESH::ElemGeomType_i();
  SMESH::ElemGeomType_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateElemGeomType()";
  return anObj._retn();
}

ElemEntityType_ptr FilterManager_i::CreateElemEntityType()
{
  SMESH::ElemEntityType_i* aServant = new SMESH::ElemEntityType_i();
  SMESH::ElemEntityType_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateElemEntityType()";
  return anObj._retn();
}

Filter_ptr FilterManager_i::CreateFilter()
{
  SMESH::Filter_i* aServant = new SMESH::Filter_i();
  SMESH::Filter_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateFilter()";
  return anObj._retn();
}

FilterLibrary_ptr FilterManager_i::LoadLibrary( const char* aFileName )
{
  SMESH::FilterLibrary_i* aServant = new SMESH::FilterLibrary_i( aFileName );
  SMESH::FilterLibrary_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".LoadLibrary('"<<aFileName<<"')";
  return anObj._retn();
}

FilterLibrary_ptr FilterManager_i::CreateLibrary()
{
  SMESH::FilterLibrary_i* aServant = new SMESH::FilterLibrary_i();
  SMESH::FilterLibrary_var anObj = aServant->_this();
  TPythonDump()<<aServant<<" = "<<this<<".CreateLibrary()";
  return anObj._retn();
}

CORBA::Boolean FilterManager_i::DeleteLibrary( const char* aFileName )
{
  TPythonDump()<<this<<".DeleteLibrary("<<aFileName<<")";
  return remove( aFileName ) ? false : true;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateFilterManager
 *
 *  Create filter manager
 */
//=============================================================================

SMESH::FilterManager_ptr SMESH_Gen_i::CreateFilterManager()
{
  SMESH::FilterManager_i* aFilter = new SMESH::FilterManager_i();
  SMESH::FilterManager_var anObj = aFilter->_this();
  return anObj._retn();
}


/*
                              FILTER
*/

//=======================================================================
// name    : Filter_i::Filter_i
// Purpose : Constructor
//=======================================================================
Filter_i::Filter_i()
: myPredicate( NULL )
{}

//=======================================================================
// name    : Filter_i::~Filter_i
// Purpose : Destructor
//=======================================================================
Filter_i::~Filter_i()
{
  if ( myPredicate )
    myPredicate->UnRegister();

  if(!CORBA::is_nil(myMesh))
    myMesh->UnRegister();

  myPredicate = 0;
  FindBaseObjects();
}

//=======================================================================
// name    : Filter_i::SetPredicate
// Purpose : Set predicate
//=======================================================================
void Filter_i::SetPredicate( Predicate_ptr thePredicate )
{
  if ( myPredicate )
    myPredicate->UnRegister();

  myPredicate = SMESH::GetPredicate(thePredicate);

  if ( myPredicate )
  {
    myFilter.SetPredicate( myPredicate->GetPredicate() );
    myPredicate->Register();
    if ( const SMDS_Mesh* aMesh = MeshPtr2SMDSMesh(myMesh))
      myPredicate->GetPredicate()->SetMesh( aMesh );
    TPythonDump()<<this<<".SetPredicate("<<myPredicate<<")";
  }
  NotifyerAndWaiter::Modified();
}

//=======================================================================
// name    : Filter_i::GetElementType
// Purpose : Get entity type
//=======================================================================
SMESH::ElementType Filter_i::GetElementType()
{
  return myPredicate != 0 ? myPredicate->GetElementType() : SMESH::ALL;
}

//=======================================================================
// name    : Filter_i::SetMesh
// Purpose : Set mesh
//=======================================================================
void
Filter_i::
SetMesh( SMESH_Mesh_ptr theMesh )
{
  if(!CORBA::is_nil(theMesh))
    theMesh->Register();

  if(!CORBA::is_nil(myMesh))
    myMesh->UnRegister();

  myMesh = SMESH_Mesh::_duplicate( theMesh );
  TPythonDump()<<this<<".SetMesh("<<theMesh<<")";

  if ( myPredicate )
    if ( const SMDS_Mesh* aMesh = MeshPtr2SMDSMesh(theMesh))
      myPredicate->GetPredicate()->SetMesh( aMesh );
}

SMESH::long_array*
Filter_i::
GetIDs()
{
  return GetElementsId(myMesh);
}

//=======================================================================
// name    : Filter_i::GetElementsId
// Purpose : Get ids of entities
//=======================================================================
void
Filter_i::
GetElementsId( Predicate_i* thePredicate,
               const SMDS_Mesh* theMesh,
               Controls::Filter::TIdSequence& theSequence )
{
  if (thePredicate)
    Controls::Filter::GetElementsId(theMesh,thePredicate->GetPredicate(),theSequence);
}

void
Filter_i::
GetElementsId( Predicate_i* thePredicate,
               SMESH_Mesh_ptr theMesh,
               Controls::Filter::TIdSequence& theSequence )
{
  if (thePredicate) 
    if(const SMDS_Mesh* aMesh = MeshPtr2SMDSMesh(theMesh))
      Controls::Filter::GetElementsId(aMesh,thePredicate->GetPredicate(),theSequence);
}

SMESH::long_array*
Filter_i::
GetElementsId( SMESH_Mesh_ptr theMesh )
{
  SMESH::long_array_var anArray = new SMESH::long_array;
  if(!CORBA::is_nil(theMesh) && myPredicate){
    theMesh->Load();
    Controls::Filter::TIdSequence aSequence;
    GetElementsId(myPredicate,theMesh,aSequence);
    long i = 0, iEnd = aSequence.size();
    anArray->length( iEnd );
    for ( ; i < iEnd; i++ )
      anArray[ i ] = aSequence[i];
  }
  return anArray._retn();
}

//=============================================================================
/*!
 * \brief Returns number of mesh elements per each \a EntityType
 */
//=============================================================================

SMESH::long_array* ::Filter_i::GetMeshInfo()
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::Entity_Last);
  for (int i = 0; i < SMESH::Entity_Last; i++)
    aRes[i] = 0;

  if ( !CORBA::is_nil(myMesh) && myPredicate )
  {
    const SMDS_Mesh*  aMesh = MeshPtr2SMDSMesh(myMesh);
    SMDS_ElemIteratorPtr it = aMesh->elementsIterator( SMDSAbs_ElementType( GetElementType() ));
    while ( it->more() )
    {
      const SMDS_MeshElement* anElem = it->next();
      if ( myPredicate->IsSatisfy( anElem->GetID() ) )
        aRes[ anElem->GetEntityType() ]++;
    }
  }

  return aRes._retn();  
}

//=============================================================================
/*!
 * \brief Returns number of mesh elements of each \a ElementType
 */
//=============================================================================

SMESH::long_array* ::Filter_i::GetNbElementsByType()
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::NB_ELEMENT_TYPES);
  for (int i = 0; i < SMESH::NB_ELEMENT_TYPES; i++)
    aRes[i] = 0;

  if ( !CORBA::is_nil(myMesh) && myPredicate ) {
    const SMDS_Mesh*  aMesh = MeshPtr2SMDSMesh(myMesh);
    SMDS_ElemIteratorPtr it = aMesh->elementsIterator( SMDSAbs_ElementType( GetElementType() ));
    CORBA::Long& nbElems = aRes[ GetElementType() ];
    while ( it->more() )
    {
      const SMDS_MeshElement* anElem = it->next();
      if ( myPredicate->IsSatisfy( anElem->GetID() ) )
        nbElems++;
    }
  }

  return aRes._retn();  
}


//================================================================================
/*!
 * \brief Return GetElementType() within an array
 * Implement SMESH_IDSource interface
 */
//================================================================================

SMESH::array_of_ElementType* Filter_i::GetTypes()
{
  SMESH::array_of_ElementType_var types = new SMESH::array_of_ElementType;

  // check if any element passes through the filter
  if ( !CORBA::is_nil(myMesh) && myPredicate )
  {
    const SMDS_Mesh* aMesh = MeshPtr2SMDSMesh(myMesh);
    SMDS_ElemIteratorPtr it = aMesh->elementsIterator( SMDSAbs_ElementType( GetElementType() ));
    bool satisfies = false;
    while ( !satisfies && it->more() )
      satisfies = myPredicate->IsSatisfy( it->next()->GetID() );
    if ( satisfies ) {
      types->length( 1 );
      types[0] = GetElementType();
    }
  }
  return types._retn();
}

//=======================================================================
//function : GetMesh
//purpose  : Returns mesh
//=======================================================================

SMESH::SMESH_Mesh_ptr Filter_i::GetMesh()
{
  return SMESH_Mesh::_duplicate( myMesh );
}

//=======================================================================
//function : GetVtkUgStream
//purpose  : Return data vtk unstructured grid (not implemented)
//=======================================================================

SALOMEDS::TMPFile* Filter_i::GetVtkUgStream()
{
  SALOMEDS::TMPFile_var SeqFile;
  return SeqFile._retn();
}
//=======================================================================
// name    : getCriteria
// Purpose : Retrieve criterions from predicate
//=======================================================================
static inline void getPrediacates( Predicate_i*                thePred,
                                   std::vector<Predicate_i*> & thePredVec )
{
  const int aFType = thePred->GetFunctorType();

  switch ( aFType )
  {
  case FT_LogicalNOT:
  {
    Predicate_i* aPred = ( dynamic_cast<LogicalNOT_i*>( thePred ) )->GetPredicate_i();
    getPrediacates( aPred, thePredVec );
    break;
  }
  case FT_LogicalAND:
  case FT_LogicalOR:
  {
    Predicate_i* aPred1 = ( dynamic_cast<LogicalBinary_i*>( thePred ) )->GetPredicate1_i();
    Predicate_i* aPred2 = ( dynamic_cast<LogicalBinary_i*>( thePred ) )->GetPredicate2_i();
    getPrediacates( aPred1, thePredVec );
    getPrediacates( aPred2, thePredVec );
    break;
   }
  default:;
  }
  thePredVec.push_back( thePred );
}

//=======================================================================
// name    : getCriteria
// Purpose : Retrieve criterions from predicate
//=======================================================================
static inline bool getCriteria( Predicate_i*                thePred,
                                SMESH::Filter::Criteria_out theCriteria )
{
  const int aFType = thePred->GetFunctorType();

  switch ( aFType )
  {
  case FT_LogicalNOT:
    {
      Predicate_i* aPred = ( dynamic_cast<LogicalNOT_i*>( thePred ) )->GetPredicate_i();
      getCriteria( aPred, theCriteria );
      theCriteria[ theCriteria->length() - 1 ].UnaryOp = FT_LogicalNOT;
    }
    return true;

  case FT_LogicalAND:
  case FT_LogicalOR:
    {
      Predicate_i* aPred1 = ( dynamic_cast<LogicalBinary_i*>( thePred ) )->GetPredicate1_i();
      Predicate_i* aPred2 = ( dynamic_cast<LogicalBinary_i*>( thePred ) )->GetPredicate2_i();
      if ( !getCriteria( aPred1, theCriteria ) )
        return false;
      theCriteria[ theCriteria->length() - 1 ].BinaryOp = aFType;
      return getCriteria( aPred2, theCriteria );
    }
  case FT_Undefined:
    return false;
  }

  // resize theCriteria
  CORBA::ULong i = theCriteria->length();
  theCriteria->length( i + 1 );
  theCriteria[ i ] = createCriterion();

  // set members of the added Criterion

  theCriteria[ i ].Type = aFType;
  theCriteria[ i ].TypeOfElement = thePred->GetElementType();

  switch ( aFType )
  {
  case FT_FreeBorders:
  case FT_FreeEdges:
  case FT_FreeFaces:
  case FT_LinearOrQuadratic:
  case FT_FreeNodes:
  case FT_EqualEdges:
  case FT_EqualFaces:
  case FT_EqualVolumes:
  case FT_BadOrientedVolume:
  case FT_BareBorderVolume:
  case FT_BareBorderFace:
  case FT_OverConstrainedVolume:
  case FT_OverConstrainedFace:
    {
      return true;
    }
  case FT_BelongToMeshGroup:
    {
      BelongToMeshGroup_i* aPred = dynamic_cast<BelongToMeshGroup_i*>( thePred );
      SMESH::SMESH_GroupBase_var grp = aPred->GetGroup();
      if ( !grp->_is_nil() )
      {
        theCriteria[ i ].ThresholdStr = grp->GetName();
        theCriteria[ i ].ThresholdID  = aPred->GetGroupID().c_str();
      }
      return true;
    }
  case FT_BelongToGeom:
    {
      BelongToGeom_i* aPred = dynamic_cast<BelongToGeom_i*>( thePred );
      theCriteria[ i ].ThresholdStr  = aPred->GetShapeName();
      theCriteria[ i ].ThresholdID   = aPred->GetShapeID();
      theCriteria[ i ].Tolerance     = aPred->GetTolerance();
      return true;
    }
  case FT_BelongToPlane:
  case FT_BelongToCylinder:
  case FT_BelongToGenSurface:
    {
      BelongToSurface_i* aPred = dynamic_cast<BelongToSurface_i*>( thePred );
      theCriteria[ i ].ThresholdStr  = aPred->GetShapeName();
      theCriteria[ i ].ThresholdID   = aPred->GetShapeID();
      theCriteria[ i ].Tolerance     = aPred->GetTolerance();
      return true;
    }
  case FT_LyingOnGeom:
    {
      LyingOnGeom_i* aPred = dynamic_cast<LyingOnGeom_i*>( thePred );
      theCriteria[ i ].ThresholdStr  = aPred->GetShapeName();
      theCriteria[ i ].ThresholdID   = aPred->GetShapeID();
      theCriteria[ i ].Tolerance     = aPred->GetTolerance();
      return true;
    }
  case FT_CoplanarFaces:
    {
      CoplanarFaces_i* aPred = dynamic_cast<CoplanarFaces_i*>( thePred );
      theCriteria[ i ].ThresholdID   = aPred->GetFaceAsString();
      theCriteria[ i ].Tolerance     = aPred->GetTolerance();
      return true;
    }
  case FT_ConnectedElements:
    {
      ConnectedElements_i* aPred = dynamic_cast<ConnectedElements_i*>( thePred );
      SMESH::ConnectedElements::ThresholdType type;
      CORBA::String_var threshold = aPred->GetThreshold( type );
      switch ( type ) {
      case SMESH::ConnectedElements::POINT:
        theCriteria[ i ].ThresholdStr = threshold; break;
      case SMESH::ConnectedElements::VERTEX:
        theCriteria[ i ].ThresholdID = threshold; break;
      case SMESH::ConnectedElements::NODE:
        theCriteria[ i ].Threshold = atof( threshold.in() ); break;
      default:;
      }
      return true;
    }
  case FT_EqualNodes:
    {
      EqualNodes_i* aPred = dynamic_cast<EqualNodes_i*>( thePred );
      theCriteria[ i ].Tolerance  = aPred->GetTolerance();
      return true;
    }
  case FT_RangeOfIds:
    {
      RangeOfIds_i* aPred = dynamic_cast<RangeOfIds_i*>( thePred );
      theCriteria[ i ].ThresholdStr  = aPred->GetRangeStr();
      return true;
    }
  case FT_LessThan:
  case FT_MoreThan:
  case FT_EqualTo:
    {
      Comparator_i* aCompar = dynamic_cast<Comparator_i*>( thePred );
      theCriteria[ i ].Type      = aCompar->GetNumFunctor_i()->GetFunctorType();
      theCriteria[ i ].Compare   = aFType;
      theCriteria[ i ].Threshold = aCompar->GetMargin();
      if ( aFType == FT_EqualTo )
      {
        EqualTo_i* aCompar = dynamic_cast<EqualTo_i*>( thePred );
        theCriteria[ i ].Tolerance = aCompar->GetTolerance();
      }
      return true;
    }
  case FT_GroupColor:
    {
      GroupColor_i* aPred = dynamic_cast<GroupColor_i*>( thePred );
      theCriteria[ i ].ThresholdStr = aPred->GetColorStr();
      return true;
    }
  case FT_ElemGeomType:
    {
      ElemGeomType_i* aPred = dynamic_cast<ElemGeomType_i*>( thePred );
      theCriteria[ i ].Threshold = (double)aPred->GetGeometryType();
      return true;
    }
  case FT_EntityType:
    {
      ElemEntityType_i* aPred = dynamic_cast<ElemEntityType_i*>( thePred );
      theCriteria[ i ].Threshold = (double)aPred->GetEntityType();
      return true;
    }
  default:
    return false;
  }
}

//=======================================================================
// name    : Filter_i::GetCriteria
// Purpose : Retrieve criterions from predicate
//=======================================================================
CORBA::Boolean Filter_i::GetCriteria( SMESH::Filter::Criteria_out theCriteria )
{
  theCriteria = new SMESH::Filter::Criteria;
  return myPredicate != 0 ? getCriteria( myPredicate, theCriteria ) : true;
}

//=======================================================================
// name    : Filter_i::SetCriteria
// Purpose : Create new predicate and set criterions in it
//=======================================================================
CORBA::Boolean Filter_i::SetCriteria( const SMESH::Filter::Criteria& theCriteria )
{
  SetPredicate( SMESH::Predicate::_nil() );

  SMESH::FilterManager_i* aFilter = new SMESH::FilterManager_i();
  FilterManager_ptr aFilterMgr = aFilter->_this();

  // CREATE two lists ( PREDICATES  and LOG OP )

  // Criterion
  TPythonDump()<<"aCriteria = []";
  std::list<SMESH::Predicate_ptr> aPredicates;
  std::list<int>                  aBinaries;
  for ( int i = 0, n = theCriteria.length(); i < n; i++ )
  {
    int         aCriterion    = theCriteria[ i ].Type;
    int         aCompare      = theCriteria[ i ].Compare;
    double      aThreshold    = theCriteria[ i ].Threshold;
    const char* aThresholdStr = theCriteria[ i ].ThresholdStr;
    const char* aThresholdID  = theCriteria[ i ].ThresholdID;
    int         aUnary        = theCriteria[ i ].UnaryOp;
    int         aBinary       = theCriteria[ i ].BinaryOp;
    double      aTolerance    = theCriteria[ i ].Tolerance;
    ElementType aTypeOfElem   = theCriteria[ i ].TypeOfElement;
    long        aPrecision    = theCriteria[ i ].Precision;

    {
      TPythonDump pd;
      pd << "aCriterion = SMESH.Filter.Criterion("
         << aCriterion    << ", "
         << aCompare      << ", "
         << aThreshold    << ", '"
         << aThresholdStr << "', '"
         << aThresholdID  << "', "
         << aUnary        << ", "
         << aBinary       << ", "
         << aTolerance    << ", "
         << aTypeOfElem   << ", "
         << aPrecision    << ")";
    }
    TPythonDump pd;

    SMESH::Predicate_ptr aPredicate = SMESH::Predicate::_nil();
    SMESH::NumericalFunctor_ptr aFunctor = SMESH::NumericalFunctor::_nil();

    switch ( aCriterion )
    {
      // Functors

      case SMESH::FT_MultiConnection:
        aFunctor = aFilterMgr->CreateMultiConnection();
        break;
      case SMESH::FT_MultiConnection2D:
        aFunctor = aFilterMgr->CreateMultiConnection2D();
        break;
      case SMESH::FT_Length:
        aFunctor = aFilterMgr->CreateLength();
        break;
      case SMESH::FT_Length2D:
        aFunctor = aFilterMgr->CreateLength2D();
        break;
      case SMESH::FT_AspectRatio:
        aFunctor = aFilterMgr->CreateAspectRatio();
        break;
      case SMESH::FT_AspectRatio3D:
        aFunctor = aFilterMgr->CreateAspectRatio3D();
        break;
      case SMESH::FT_Warping:
        aFunctor = aFilterMgr->CreateWarping();
        break;
      case SMESH::FT_MinimumAngle:
        aFunctor = aFilterMgr->CreateMinimumAngle();
        break;
      case SMESH::FT_Taper:
        aFunctor = aFilterMgr->CreateTaper();
        break;
      case SMESH::FT_Skew:
        aFunctor = aFilterMgr->CreateSkew();
        break;
      case SMESH::FT_Area:
        aFunctor = aFilterMgr->CreateArea();
        break;
      case SMESH::FT_Volume3D:
        aFunctor = aFilterMgr->CreateVolume3D();
        break;
      case SMESH::FT_MaxElementLength2D:
        aFunctor = aFilterMgr->CreateMaxElementLength2D();
        break;
      case SMESH::FT_MaxElementLength3D:
        aFunctor = aFilterMgr->CreateMaxElementLength3D();
        break;
      case SMESH::FT_BallDiameter:
        aFunctor = aFilterMgr->CreateBallDiameter();
        break;
      case SMESH::FT_NodeConnectivityNumber:
        aFunctor = aFilterMgr->CreateNodeConnectivityNumber();
        break;

      // Predicates

      case SMESH::FT_FreeBorders:
        aPredicate = aFilterMgr->CreateFreeBorders();
        break;
      case SMESH::FT_FreeEdges:
        aPredicate = aFilterMgr->CreateFreeEdges();
        break;
      case SMESH::FT_FreeFaces:
        aPredicate = aFilterMgr->CreateFreeFaces();
        break;
      case SMESH::FT_FreeNodes:
        aPredicate = aFilterMgr->CreateFreeNodes();
        break;
      case SMESH::FT_EqualNodes:
        {
          SMESH::EqualNodes_ptr pred = aFilterMgr->CreateEqualNodes();
          pred->SetTolerance( aTolerance );
          aPredicate = pred;
          break;
        }
      case SMESH::FT_EqualEdges:
        aPredicate = aFilterMgr->CreateEqualEdges();
        break;
      case SMESH::FT_EqualFaces:
        aPredicate = aFilterMgr->CreateEqualFaces();
        break;
      case SMESH::FT_EqualVolumes:
        aPredicate = aFilterMgr->CreateEqualVolumes();
        break;
      case SMESH::FT_BelongToMeshGroup:
        {
          SMESH::BelongToMeshGroup_ptr tmpPred = aFilterMgr->CreateBelongToMeshGroup();
          tmpPred->SetGroupID( aThresholdID );
          aPredicate = tmpPred;
        }
        break;
      case SMESH::FT_BelongToGeom:
        {
          SMESH::BelongToGeom_ptr tmpPred = aFilterMgr->CreateBelongToGeom();
          tmpPred->SetElementType( aTypeOfElem );
          tmpPred->SetShape( aThresholdID, aThresholdStr );
          tmpPred->SetTolerance( aTolerance );
          aPredicate = tmpPred;
        }
        break;
      case SMESH::FT_BelongToPlane:
      case SMESH::FT_BelongToCylinder:
      case SMESH::FT_BelongToGenSurface:
        {
          SMESH::BelongToSurface_ptr tmpPred;
          switch ( aCriterion ) {
          case SMESH::FT_BelongToPlane:
            tmpPred = aFilterMgr->CreateBelongToPlane(); break;
          case SMESH::FT_BelongToCylinder:
            tmpPred = aFilterMgr->CreateBelongToCylinder(); break;
          default:
            tmpPred = aFilterMgr->CreateBelongToGenSurface();
          }
          tmpPred->SetShape( aThresholdID, aThresholdStr, aTypeOfElem );
          tmpPred->SetTolerance( aTolerance );
          aPredicate = tmpPred;
        }
        break;
      case SMESH::FT_LyingOnGeom:
        {
          SMESH::LyingOnGeom_ptr tmpPred = aFilterMgr->CreateLyingOnGeom();
          tmpPred->SetElementType( aTypeOfElem );
          tmpPred->SetShape( aThresholdID, aThresholdStr );
          tmpPred->SetTolerance( aTolerance );
          aPredicate = tmpPred;
        }
        break;
      case SMESH::FT_RangeOfIds:
        {
          SMESH::RangeOfIds_ptr tmpPred = aFilterMgr->CreateRangeOfIds();
          tmpPred->SetRangeStr( aThresholdStr );
          tmpPred->SetElementType( aTypeOfElem );
          aPredicate = tmpPred;
        }
        break;
      case SMESH::FT_BadOrientedVolume:
        {
          aPredicate = aFilterMgr->CreateBadOrientedVolume();
        }
        break;
      case SMESH::FT_BareBorderVolume:
        {
          aPredicate = aFilterMgr->CreateBareBorderVolume();
        }
        break;
      case SMESH::FT_BareBorderFace:
        {
          aPredicate = aFilterMgr->CreateBareBorderFace();
        }
        break;
      case SMESH::FT_OverConstrainedVolume:
        {
          aPredicate = aFilterMgr->CreateOverConstrainedVolume();
        }
        break;
      case SMESH::FT_OverConstrainedFace:
        {
          aPredicate = aFilterMgr->CreateOverConstrainedFace();
        }
        break;
      case SMESH::FT_LinearOrQuadratic:
        {
          SMESH::LinearOrQuadratic_ptr tmpPred = aFilterMgr->CreateLinearOrQuadratic();
          tmpPred->SetElementType( aTypeOfElem );
          aPredicate = tmpPred;
          break;
        }
      case SMESH::FT_GroupColor:
        {
          SMESH::GroupColor_ptr tmpPred = aFilterMgr->CreateGroupColor();
          tmpPred->SetElementType( aTypeOfElem );
          tmpPred->SetColorStr( aThresholdStr );
          aPredicate = tmpPred;
          break;
        }
      case SMESH::FT_ElemGeomType:
        {
          SMESH::ElemGeomType_ptr tmpPred = aFilterMgr->CreateElemGeomType();
          tmpPred->SetElementType( aTypeOfElem );
          tmpPred->SetGeometryType( (GeometryType)(int)(aThreshold + 0.5) );
          aPredicate = tmpPred;
          break;
        }
      case SMESH::FT_EntityType:
        {
          SMESH::ElemEntityType_ptr tmpPred = aFilterMgr->CreateElemEntityType();
          tmpPred->SetElementType( aTypeOfElem );
          tmpPred->SetEntityType( EntityType( (int (aThreshold + 0.5))));
          aPredicate = tmpPred;
          break;
        }
      case SMESH::FT_CoplanarFaces:
        {
          SMESH::CoplanarFaces_ptr tmpPred = aFilterMgr->CreateCoplanarFaces();
          tmpPred->SetFace( atol (aThresholdID ));
          tmpPred->SetTolerance( aTolerance );
          aPredicate = tmpPred;
          break;
        }
      case SMESH::FT_ConnectedElements:
        {
          SMESH::ConnectedElements_ptr tmpPred = aFilterMgr->CreateConnectedElements();
          if ( strlen( aThresholdID ) > 0 ) // shape ID
            tmpPred->SetThreshold( aThresholdID, SMESH::ConnectedElements::VERTEX );
          else if ( strlen( aThresholdStr ) > 0 ) // point coords
            tmpPred->SetThreshold( aThresholdStr, SMESH::ConnectedElements::POINT );
          else if ( aThreshold >= 1 )
            tmpPred->SetNode( (CORBA::Long) aThreshold ); // node ID
          tmpPred->SetElementType( aTypeOfElem );
          aPredicate = tmpPred;
          break;
        }

      default:
        continue;
    }

    // Comparator
    if ( !aFunctor->_is_nil() && aPredicate->_is_nil() )
    {
      SMESH::Comparator_ptr aComparator = SMESH::Comparator::_nil();

      if ( aCompare == SMESH::FT_LessThan )
        aComparator = aFilterMgr->CreateLessThan();
      else if ( aCompare == SMESH::FT_MoreThan )
        aComparator = aFilterMgr->CreateMoreThan();
      else if ( aCompare == SMESH::FT_EqualTo )
        aComparator = aFilterMgr->CreateEqualTo();
      else
        continue;

      aComparator->SetNumFunctor( aFunctor );
      aComparator->SetMargin( aThreshold );

      if ( aCompare == FT_EqualTo )
      {
        SMESH::EqualTo_var anEqualTo = SMESH::EqualTo::_narrow( aComparator );
        anEqualTo->SetTolerance( aTolerance );
      }

      aPredicate = aComparator;

      aFunctor->SetPrecision( aPrecision );
    }

    // Logical not
    if ( aUnary == FT_LogicalNOT )
    {
      SMESH::LogicalNOT_ptr aNotPred = aFilterMgr->CreateLogicalNOT();
      aNotPred->SetPredicate( aPredicate );
      aPredicate = aNotPred;
    }

    // logical op
    aPredicates.push_back( aPredicate );
    aBinaries.push_back( aBinary );
    pd <<"aCriteria.append(aCriterion)";

  } // end of for
  TPythonDump pd; pd<<this<<".SetCriteria(aCriteria)";

  // CREATE ONE PREDICATE FROM PREVIOUSLY CREATED MAP

  // combine all "AND" operations

  std::list<SMESH::Predicate_ptr> aResList;

  std::list<SMESH::Predicate_ptr>::iterator aPredIter;
  std::list<int>::iterator                  aBinaryIter;

  SMESH::Predicate_ptr aPrevPredicate = SMESH::Predicate::_nil();
  int aPrevBinary = SMESH::FT_Undefined;
  if ( !aBinaries.empty() )
    aBinaries.back() = SMESH::FT_Undefined;

  for ( aPredIter = aPredicates.begin(), aBinaryIter = aBinaries.begin();
        aPredIter != aPredicates.end() && aBinaryIter != aBinaries.end();
        ++aPredIter, ++aBinaryIter )
  {
    int aCurrBinary = *aBinaryIter;

    SMESH::Predicate_ptr aCurrPred = SMESH::Predicate::_nil();

    if ( aPrevBinary == SMESH::FT_LogicalAND )
    {

      SMESH::LogicalBinary_ptr aBinaryPred = aFilterMgr->CreateLogicalAND();
      aBinaryPred->SetPredicate1( aPrevPredicate );
      aBinaryPred->SetPredicate2( *aPredIter );
      aCurrPred = aBinaryPred;
    }
    else
      aCurrPred = *aPredIter;

    if ( aCurrBinary != SMESH::FT_LogicalAND )
      aResList.push_back( aCurrPred );

    aPrevPredicate = aCurrPred;
    aPrevBinary = aCurrBinary;
  }

  // combine all "OR" operations

  SMESH::Predicate_ptr aResPredicate = SMESH::Predicate::_nil();

  if ( aResList.size() == 1 )
    aResPredicate = *aResList.begin();
  else if ( aResList.size() > 1 )
  {
    std::list<SMESH::Predicate_ptr>::iterator anIter = aResList.begin();
    aResPredicate = *anIter;
    anIter++;
    for ( ; anIter != aResList.end(); ++anIter )
    {
      SMESH::LogicalBinary_ptr aBinaryPred = aFilterMgr->CreateLogicalOR();
      aBinaryPred->SetPredicate1( aResPredicate );
      aBinaryPred->SetPredicate2( *anIter );
      aResPredicate = aBinaryPred;
    }
  }

  SetPredicate( aResPredicate );
  if ( !aResPredicate->_is_nil() )
    aResPredicate->UnRegister();

  return !aResPredicate->_is_nil();
}

//=======================================================================
// name    : Filter_i::GetPredicate_i
// Purpose : Get implementation of predicate
//=======================================================================
Predicate_i* Filter_i::GetPredicate_i()
{
  return myPredicate;
}

//=======================================================================
// name    : Filter_i::GetPredicate
// Purpose : Get predicate
//=======================================================================
Predicate_ptr Filter_i::GetPredicate()
{
  if ( myPredicate == 0 )
    return SMESH::Predicate::_nil();
  else
  {
    SMESH::Predicate_var anObj = myPredicate->_this();
    // if ( SMESH::Functor_i* fun = SMESH::DownCast<SMESH::Functor_i*>( anObj ))
    //   TPythonDump() << fun << " = " << this << ".GetPredicate()";
    return anObj._retn();
  }
}

//================================================================================
/*!
 * \brief Find groups it depends on
 */
//================================================================================

void Filter_i::FindBaseObjects()
{
  // release current groups
  for ( size_t i = 0; i < myBaseGroups.size(); ++i )
    if ( myBaseGroups[i] )
    {
      myBaseGroups[i]->RemoveModifWaiter( this );
      myBaseGroups[i]->UnRegister();
    }

  // remember new groups
  myBaseGroups.clear();
  if ( myPredicate )
  {
    std::vector<Predicate_i*> predicates;
    getPrediacates( myPredicate, predicates );
    for ( size_t i = 0; i < predicates.size(); ++i )
      if ( BelongToMeshGroup_i* bmg = dynamic_cast< BelongToMeshGroup_i* >( predicates[i] ))
      {
        SMESH::SMESH_GroupBase_var g = bmg->GetGroup();
        SMESH_GroupBase_i* g_i = SMESH::DownCast< SMESH_GroupBase_i*>( g );
        if ( g_i )
        {
          g_i->AddModifWaiter( this );
          g_i->Register();
          myBaseGroups.push_back( g_i );
        }
      }
  }
}

//================================================================================
/*!
 * \brief When notified on removal of myBaseGroups[i], remove a reference to a
 *        group from a predicate
 */
//================================================================================

void Filter_i::OnBaseObjModified(NotifyerAndWaiter* group, bool removed)
{
  if ( !removed )
    return; // a GroupOnFilter holding this filter is notified automatically

  if ( myPredicate )
  {
    std::vector<Predicate_i*> predicates;
    getPrediacates( myPredicate, predicates );
    for ( size_t i = 0; i < predicates.size(); ++i )
      if ( BelongToMeshGroup_i* bmg = dynamic_cast< BelongToMeshGroup_i* >( predicates[i] ))
      {
        SMESH::SMESH_GroupBase_var g = bmg->GetGroup();
        SMESH_GroupBase_i* g_i = SMESH::DownCast< SMESH_GroupBase_i*>( g );
        if ( g_i == group )
        {
          bmg->SetGroup( SMESH::SMESH_GroupBase::_nil() );
          bmg->SetGroupID( "" );
        }
      }
  }

  FindBaseObjects(); // release and update myBaseGroups;
}

/*
                            FILTER LIBRARY
*/

#define ATTR_TYPE          "type"
#define ATTR_COMPARE       "compare"
#define ATTR_THRESHOLD     "threshold"
#define ATTR_UNARY         "unary"
#define ATTR_BINARY        "binary"
#define ATTR_THRESHOLD_STR "threshold_str"
#define ATTR_TOLERANCE     "tolerance"
#define ATTR_ELEMENT_TYPE  "ElementType"

//=======================================================================
// name    : toString
// Purpose : Convert bool to LDOMString
//=======================================================================
static inline LDOMString toString( CORBA::Boolean val )
{
  return val ? "logical not" : "";
}

//=======================================================================
// name    : toBool
// Purpose : Convert LDOMString to bool
//=======================================================================
static inline bool toBool( const LDOMString& theStr )
{
  return theStr.equals( "logical not" );
}

//=======================================================================
// name    : toString
// Purpose : Convert double to LDOMString
//=======================================================================
static inline LDOMString toString( CORBA::Double val )
{
  char a[ 255 ];
  sprintf( a, "%e", val );
  return LDOMString( a );
}

//=======================================================================
// name    : toDouble
// Purpose : Convert LDOMString to double
//=======================================================================
static inline double toDouble( const LDOMString& theStr )
{
  return atof( theStr.GetString() );
}

//=======================================================================
// name    : toString
// Purpose : Convert functor type to LDOMString
//=======================================================================
static inline LDOMString toString( CORBA::Long theType )
{
  switch ( theType )
  {
    case FT_AspectRatio           : return "Aspect ratio";
    case FT_Warping               : return "Warping";
    case FT_MinimumAngle          : return "Minimum angle";
    case FT_Taper                 : return "Taper";
    case FT_Skew                  : return "Skew";
    case FT_Area                  : return "Area";
    case FT_Volume3D              : return "Volume3D";
    case FT_MaxElementLength2D    : return "Max element length 2D";
    case FT_MaxElementLength3D    : return "Max element length 3D";
    case FT_BelongToMeshGroup     : return "Belong to Mesh Group";
    case FT_BelongToGeom          : return "Belong to Geom";
    case FT_BelongToPlane         : return "Belong to Plane";
    case FT_BelongToCylinder      : return "Belong to Cylinder";
    case FT_BelongToGenSurface    : return "Belong to Generic Surface";
    case FT_LyingOnGeom           : return "Lying on Geom";
    case FT_BadOrientedVolume     : return "Bad Oriented Volume";
    case FT_BareBorderVolume      : return "Volumes with bare border";
    case FT_BareBorderFace        : return "Faces with bare border";
    case FT_OverConstrainedVolume : return "Over-constrained Volumes";
    case FT_OverConstrainedFace   : return "Over-constrained Faces";
    case FT_RangeOfIds            : return "Range of IDs";
    case FT_FreeBorders           : return "Free borders";
    case FT_FreeEdges             : return "Free edges";
    case FT_FreeFaces             : return "Free faces";
    case FT_FreeNodes             : return "Free nodes";
    case FT_EqualNodes            : return "Equal nodes";
    case FT_EqualEdges            : return "Equal edges";
    case FT_EqualFaces            : return "Equal faces";
    case FT_EqualVolumes          : return "Equal volumes";
    case FT_MultiConnection       : return "Borders at multi-connections";
    case FT_MultiConnection2D     :return "Borders at multi-connections 2D";
    case FT_Length                : return "Length";
    case FT_Length2D              : return "Length 2D";
    case FT_LessThan              : return "Less than";
    case FT_MoreThan              : return "More than";
    case FT_EqualTo               : return "Equal to";
    case FT_LogicalNOT            : return "Not";
    case FT_LogicalAND            : return "And";
    case FT_LogicalOR             : return "Or";
    case FT_GroupColor            : return "Color of Group";
    case FT_LinearOrQuadratic     : return "Linear or Quadratic";
    case FT_ElemGeomType          : return "Element geomtry type";
    case FT_EntityType            : return "Entity type";
    case FT_Undefined             : return "";
    default                       : return "";
  }
}

//=======================================================================
// name    : toFunctorType
// Purpose : Convert LDOMString to functor type
//=======================================================================
static inline SMESH::FunctorType toFunctorType( const LDOMString& theStr )
{
  if      ( theStr.equals( "Aspect ratio"                 ) ) return FT_AspectRatio;
  else if ( theStr.equals( "Warping"                      ) ) return FT_Warping;
  else if ( theStr.equals( "Minimum angle"                ) ) return FT_MinimumAngle;
  else if ( theStr.equals( "Taper"                        ) ) return FT_Taper;
  else if ( theStr.equals( "Skew"                         ) ) return FT_Skew;
  else if ( theStr.equals( "Area"                         ) ) return FT_Area;
  else if ( theStr.equals( "Volume3D"                     ) ) return FT_Volume3D;
  else if ( theStr.equals( "Max element length 2D"        ) ) return FT_MaxElementLength2D;
  else if ( theStr.equals( "Max element length 3D"        ) ) return FT_MaxElementLength3D;
  else if ( theStr.equals( "Belong to Mesh Group"         ) ) return FT_BelongToMeshGroup;
  else if ( theStr.equals( "Belong to Geom"               ) ) return FT_BelongToGeom;
  else if ( theStr.equals( "Belong to Plane"              ) ) return FT_BelongToPlane;
  else if ( theStr.equals( "Belong to Cylinder"           ) ) return FT_BelongToCylinder;
  else if ( theStr.equals( "Belong to Generic Surface"    ) ) return FT_BelongToGenSurface;
  else if ( theStr.equals( "Lying on Geom"                ) ) return FT_LyingOnGeom;
  else if ( theStr.equals( "Free borders"                 ) ) return FT_FreeBorders;
  else if ( theStr.equals( "Free edges"                   ) ) return FT_FreeEdges;
  else if ( theStr.equals( "Free faces"                   ) ) return FT_FreeFaces;
  else if ( theStr.equals( "Free nodes"                   ) ) return FT_FreeNodes;
  else if ( theStr.equals( "Equal nodes"                  ) ) return FT_EqualNodes;
  else if ( theStr.equals( "Equal edges"                  ) ) return FT_EqualEdges;
  else if ( theStr.equals( "Equal faces"                  ) ) return FT_EqualFaces;
  else if ( theStr.equals( "Equal volumes"                ) ) return FT_EqualVolumes;
  else if ( theStr.equals( "Borders at multi-connections" ) ) return FT_MultiConnection;
  //  else if ( theStr.equals( "Borders at multi-connections 2D" ) ) return FT_MultiConnection2D;
  else if ( theStr.equals( "Length"                       ) ) return FT_Length;
  //  else if ( theStr.equals( "Length2D"                     ) ) return FT_Length2D;
  else if ( theStr.equals( "Range of IDs"                 ) ) return FT_RangeOfIds;
  else if ( theStr.equals( "Bad Oriented Volume"          ) ) return FT_BadOrientedVolume;
  else if ( theStr.equals( "Volumes with bare border"     ) ) return FT_BareBorderVolume;
  else if ( theStr.equals( "Faces with bare border"       ) ) return FT_BareBorderFace;
  else if ( theStr.equals( "Over-constrained Volumes"     ) ) return FT_OverConstrainedVolume;
  else if ( theStr.equals( "Over-constrained Faces"       ) ) return FT_OverConstrainedFace;
  else if ( theStr.equals( "Less than"                    ) ) return FT_LessThan;
  else if ( theStr.equals( "More than"                    ) ) return FT_MoreThan;
  else if ( theStr.equals( "Equal to"                     ) ) return FT_EqualTo;
  else if ( theStr.equals( "Not"                          ) ) return FT_LogicalNOT;
  else if ( theStr.equals( "And"                          ) ) return FT_LogicalAND;
  else if ( theStr.equals( "Or"                           ) ) return FT_LogicalOR;
  else if ( theStr.equals( "Color of Group"               ) ) return FT_GroupColor;
  else if ( theStr.equals( "Linear or Quadratic"          ) ) return FT_LinearOrQuadratic;
  else if ( theStr.equals( "Element geomtry type"         ) ) return FT_ElemGeomType;
  else if ( theStr.equals( "Entity type"                  ) ) return FT_EntityType;
  else if ( theStr.equals( ""                             ) ) return FT_Undefined;
  else  return FT_Undefined;
}

//=======================================================================
// name    : toFunctorType
// Purpose : Convert LDOMString to value of ElementType enumeration
//=======================================================================
static inline SMESH::ElementType toElementType( const LDOMString& theStr )
{
  if      ( theStr.equals( "NODE"   ) ) return SMESH::NODE;
  else if ( theStr.equals( "EDGE"   ) ) return SMESH::EDGE;
  else if ( theStr.equals( "FACE"   ) ) return SMESH::FACE;
  else if ( theStr.equals( "VOLUME" ) ) return SMESH::VOLUME;
  else                                  return SMESH::ALL;
}

//=======================================================================
// name    : toString
// Purpose : Convert ElementType to string
//=======================================================================
static inline LDOMString toString( const SMESH::ElementType theType )
{
  switch ( theType )
  {
    case SMESH::NODE   : return "NODE";
    case SMESH::EDGE   : return "EDGE";
    case SMESH::FACE   : return "FACE";
    case SMESH::VOLUME : return "VOLUME";
    case SMESH::ALL    : return "ALL";
    default            : return "";
  }
}

//=======================================================================
// name    : findFilter
// Purpose : Find filter in document
//=======================================================================
static LDOM_Element findFilter( const char* theFilterName,
                                const LDOM_Document& theDoc,
                                LDOM_Node* theParent = 0 )
{
  LDOM_Element aRootElement = theDoc.getDocumentElement();
  if ( aRootElement.isNull() || !aRootElement.hasChildNodes() )
    return LDOM_Element();

  for ( LDOM_Node aTypeNode = aRootElement.getFirstChild();
        !aTypeNode.isNull(); aTypeNode = aTypeNode.getNextSibling() )
  {
    for ( LDOM_Node aFilter = aTypeNode.getFirstChild();
          !aFilter.isNull(); aFilter = aFilter.getNextSibling() )
    {
      LDOM_Element* anElem = ( LDOM_Element* )&aFilter;
      if ( anElem->getTagName().equals( LDOMString( "filter" ) ) &&
           anElem->getAttribute( "name" ).equals( LDOMString( theFilterName ) ) )
      {
        if ( theParent != 0  )
          *theParent = aTypeNode;
        return (LDOM_Element&)aFilter;
      }
    }
  }
  return LDOM_Element();
}

//=======================================================================
// name    : getSectionName
// Purpose : Get name of section of filters
//=======================================================================
static const char* getSectionName( const ElementType theType )
{
  switch ( theType )
  {
    case SMESH::NODE   : return "Filters for nodes";
    case SMESH::EDGE   : return "Filters for edges";
    case SMESH::FACE   : return "Filters for faces";
    case SMESH::VOLUME : return "Filters for volumes";
    case SMESH::ALL    : return "Filters for elements";
    default            : return "";
  }
}

//=======================================================================
// name    : getSection
// Purpose : Create section for filters corresponding to the entity type
//=======================================================================
static LDOM_Node getSection( const ElementType theType,
                             LDOM_Document&    theDoc,
                             const bool        toCreate = false )
{
  LDOM_Element aRootElement = theDoc.getDocumentElement();
  if ( aRootElement.isNull() )
    return LDOM_Node();

  // Find section
  bool anExist = false;
  const char* aSectionName = getSectionName( theType );
  if ( strcmp( aSectionName, "" ) == 0 )
    return LDOM_Node();

  LDOM_NodeList aSections = theDoc.getElementsByTagName( "section" );
  LDOM_Node aNode;
  for ( int i = 0, n = aSections.getLength(); i < n; i++ )
  {
    aNode = aSections.item( i );
    LDOM_Element& anItem = ( LDOM_Element& )aNode;
    if ( anItem.getAttribute( "name" ).equals( LDOMString( aSectionName ) ) )
    {
      anExist = true;
      break;
    }
  }

  // Create new section if necessary
  if ( !anExist )
  {
    if ( toCreate )
    {
      LDOM_Element aNewItem = theDoc.createElement( "section" );
      aNewItem.setAttribute( "name", aSectionName );
      aRootElement.appendChild( aNewItem );
      return aNewItem;
    }
    else
      return LDOM_Node();
  }
  return
    aNode;
}

//=======================================================================
// name    : createFilterItem
// Purpose : Create filter item or LDOM document
//=======================================================================
static LDOM_Element createFilterItem( const char*       theName,
                                      SMESH::Filter_ptr theFilter,
                                      LDOM_Document&    theDoc )
{
  // create new filter in document
  LDOM_Element aFilterItem = theDoc.createElement( "filter" );
  aFilterItem.setAttribute( "name", theName );

  // save filter criterions
  SMESH::Filter::Criteria_var aCriteria = new SMESH::Filter::Criteria;

  if ( !theFilter->GetCriteria( aCriteria ) )
    return LDOM_Element();

  for ( CORBA::ULong i = 0, n = aCriteria->length(); i < n; i++ )
  {
    LDOM_Element aCriterionItem = theDoc.createElement( "criterion" );
    
    aCriterionItem.setAttribute( ATTR_TYPE         , toString(  aCriteria[ i ].Type) );
    aCriterionItem.setAttribute( ATTR_COMPARE      , toString(  aCriteria[ i ].Compare ) );
    aCriterionItem.setAttribute( ATTR_THRESHOLD    , toString(  aCriteria[ i ].Threshold ) );
    aCriterionItem.setAttribute( ATTR_UNARY        , toString(  aCriteria[ i ].UnaryOp ) );
    aCriterionItem.setAttribute( ATTR_BINARY       , toString(  aCriteria[ i ].BinaryOp ) );

    aCriterionItem.setAttribute( ATTR_THRESHOLD_STR, (const char*)aCriteria[ i ].ThresholdStr );
    aCriterionItem.setAttribute( ATTR_TOLERANCE    , toString( aCriteria[ i ].Tolerance ) );
    aCriterionItem.setAttribute( ATTR_ELEMENT_TYPE ,
      toString( (SMESH::ElementType)aCriteria[ i ].TypeOfElement ) );

    aFilterItem.appendChild( aCriterionItem );
  }

  return aFilterItem;
}

//=======================================================================
// name    : FilterLibrary_i::FilterLibrary_i
// Purpose : Constructor
//=======================================================================
FilterLibrary_i::FilterLibrary_i( const char* theFileName )
{
  myFileName = CORBA::string_dup( theFileName );
  SMESH::FilterManager_i* aFilterMgr = new SMESH::FilterManager_i();
  myFilterMgr = aFilterMgr->_this();

  LDOMParser aParser;

  // Try to use existing library file
  bool anExists = false;
  if ( !aParser.parse( myFileName ) )
  {
    myDoc = aParser.getDocument();
    anExists = true;
  }
  // Create a new XML document if it doesn't exist
  else
    myDoc = LDOM_Document::createDocument( LDOMString() );

  LDOM_Element aRootElement = myDoc.getDocumentElement();
  if ( aRootElement.isNull() )
  {
    // If the existing document is empty --> try to create a new one
    if ( anExists )
      myDoc = LDOM_Document::createDocument( LDOMString() );
  }
}

//=======================================================================
// name    : FilterLibrary_i::FilterLibrary_i
// Purpose : Constructor
//=======================================================================
FilterLibrary_i::FilterLibrary_i()
{
  myFileName = 0;
  SMESH::FilterManager_i* aFilter = new SMESH::FilterManager_i();
  myFilterMgr = aFilter->_this();

  myDoc = LDOM_Document::createDocument( LDOMString() );
}

FilterLibrary_i::~FilterLibrary_i()
{
  CORBA::string_free( myFileName );
  //TPythonDump()<<this<<".UnRegister()";
}

//=======================================================================
// name    : FilterLibrary_i::Copy
// Purpose : Create filter and initialize it with values from library
//=======================================================================
Filter_ptr FilterLibrary_i::Copy( const char* theFilterName )
{
  Filter_ptr aRes = Filter::_nil();
  LDOM_Node aFilter = findFilter( theFilterName, myDoc );

  if ( aFilter.isNull() )
    return aRes;

  std::list<SMESH::Filter::Criterion> aCriteria;

  for ( LDOM_Node aCritNode = aFilter.getFirstChild();
        !aCritNode.isNull() ; aCritNode = aCritNode.getNextSibling() )
  {
    LDOM_Element* aCrit = (LDOM_Element*)&aCritNode;

    const char* aTypeStr      = aCrit->getAttribute( ATTR_TYPE          ).GetString();
    const char* aCompareStr   = aCrit->getAttribute( ATTR_COMPARE       ).GetString();
    const char* aUnaryStr     = aCrit->getAttribute( ATTR_UNARY         ).GetString();
    const char* aBinaryStr    = aCrit->getAttribute( ATTR_BINARY        ).GetString();
    const char* anElemTypeStr = aCrit->getAttribute( ATTR_ELEMENT_TYPE  ).GetString();

    SMESH::Filter::Criterion aCriterion = createCriterion();

    aCriterion.Type          = toFunctorType( aTypeStr );
    aCriterion.Compare       = toFunctorType( aCompareStr );
    aCriterion.UnaryOp       = toFunctorType( aUnaryStr );
    aCriterion.BinaryOp      = toFunctorType( aBinaryStr );

    aCriterion.TypeOfElement = toElementType( anElemTypeStr );

    LDOMString str = aCrit->getAttribute( ATTR_THRESHOLD );
    int val = 0;
    aCriterion.Threshold = str.Type() == LDOMBasicString::LDOM_Integer && str.GetInteger( val )
      ? val : atof( str.GetString() );

    str = aCrit->getAttribute( ATTR_TOLERANCE );
    aCriterion.Tolerance = str.Type() == LDOMBasicString::LDOM_Integer && str.GetInteger( val )
      ? val : atof( str.GetString() );

    str = aCrit->getAttribute( ATTR_THRESHOLD_STR );
    if ( str.Type() == LDOMBasicString::LDOM_Integer && str.GetInteger( val ) )
    {
      char a[ 255 ];
      sprintf( a, "%d", val );
      aCriterion.ThresholdStr = CORBA::string_dup( a );
    }
    else
      aCriterion.ThresholdStr = str.GetString();

    aCriteria.push_back( aCriterion );
  }

  SMESH::Filter::Criteria_var aCriteriaVar = new SMESH::Filter::Criteria;
  aCriteriaVar->length( aCriteria.size() );

  CORBA::ULong i = 0;
  std::list<SMESH::Filter::Criterion>::iterator anIter = aCriteria.begin();

  for( ; anIter != aCriteria.end(); ++anIter )
    aCriteriaVar[ i++ ] = *anIter;

  aRes = myFilterMgr->CreateFilter();
  aRes->SetCriteria( aCriteriaVar.inout() );

  TPythonDump()<<this<<".Copy('"<<theFilterName<<"')";

  return aRes;
}

//=======================================================================
// name    : FilterLibrary_i::SetFileName
// Purpose : Set file name for library
//=======================================================================
void FilterLibrary_i::SetFileName( const char* theFileName )
{
  CORBA::string_free( myFileName );
  myFileName = CORBA::string_dup( theFileName );
  TPythonDump()<<this<<".SetFileName('"<<theFileName<<"')";
}

//=======================================================================
// name    : FilterLibrary_i::GetFileName
// Purpose : Get file name of library
//=======================================================================
char* FilterLibrary_i::GetFileName()
{
  return CORBA::string_dup( myFileName );
}

//=======================================================================
// name    : FilterLibrary_i::Add
// Purpose : Add new filter to library
//=======================================================================
CORBA::Boolean FilterLibrary_i::Add( const char* theFilterName, Filter_ptr theFilter )
{
  // if filter already in library or entry filter is null do nothing
  LDOM_Node aFilterNode = findFilter( theFilterName, myDoc );
  if ( !aFilterNode.isNull() || theFilter->_is_nil() )
    return false;

  // get section corresponding to the filter type
  ElementType anEntType = theFilter->GetElementType();

  LDOM_Node aSection = getSection( anEntType, myDoc, true );
  if ( aSection.isNull() )
    return false;

  // create filter item
  LDOM_Element aFilterItem = createFilterItem( theFilterName, theFilter, myDoc );
  if ( aFilterItem.isNull() )
    return false;
  else
  {
    aSection.appendChild( aFilterItem );
    if(Filter_i* aFilter = DownCast<Filter_i*>(theFilter))
      TPythonDump()<<this<<".Add('"<<theFilterName<<"',"<<aFilter<<")";
    return true;
  }
}

//=======================================================================
// name    : FilterLibrary_i::Add
// Purpose : Add new filter to library
//=======================================================================
CORBA::Boolean FilterLibrary_i::AddEmpty( const char* theFilterName, ElementType theType )
{
  // if filter already in library or entry filter is null do nothing
  LDOM_Node aFilterNode = findFilter( theFilterName, myDoc );
  if ( !aFilterNode.isNull() )
    return false;

  LDOM_Node aSection = getSection( theType, myDoc, true );
  if ( aSection.isNull() )
    return false;

  // create filter item
  Filter_var aFilter = myFilterMgr->CreateFilter();

  LDOM_Element aFilterItem = createFilterItem( theFilterName, aFilter, myDoc );
  if ( aFilterItem.isNull() )
    return false;
  else
  {
    aSection.appendChild( aFilterItem );
    TPythonDump()<<this<<".AddEmpty('"<<theFilterName<<"',"<<theType<<")";
    return true;
  }
}

//=======================================================================
// name    : FilterLibrary_i::Delete
// Purpose : Delete filter from library
//=======================================================================
CORBA::Boolean FilterLibrary_i::Delete ( const char* theFilterName )
{
  LDOM_Node aParentNode;
  LDOM_Node aFilterNode = findFilter( theFilterName, myDoc, &aParentNode );
  if ( aFilterNode.isNull() || aParentNode.isNull() )
    return false;

  aParentNode.removeChild( aFilterNode );
  TPythonDump()<<this<<".Delete('"<<theFilterName<<"')";
  return true;
}

//=======================================================================
// name      : FilterLibrary_i::Replace
// Purpose   : Replace existing filter with entry filter.
// IMPORTANT : If filter does not exist it is not created
//=======================================================================
CORBA::Boolean FilterLibrary_i::Replace( const char* theFilterName,
                                         const char* theNewName,
                                         Filter_ptr  theFilter )
{
  LDOM_Element aFilterItem = findFilter( theFilterName, myDoc );
  if ( aFilterItem.isNull() || theFilter->_is_nil() )
    return false;

  LDOM_Element aNewItem = createFilterItem( theNewName, theFilter, myDoc );
  if ( aNewItem.isNull() )
    return false;
  else
  {
    aFilterItem.ReplaceElement( aNewItem );
    if(Filter_i* aFilter = DownCast<Filter_i*>(theFilter))
      TPythonDump()<<this<<".Replace('"<<theFilterName<<"','"<<theNewName<<"',"<<aFilter<<")";
    return true;
  }
}

//=======================================================================
// name    : FilterLibrary_i::Save
// Purpose : Save library on disk
//=======================================================================
CORBA::Boolean FilterLibrary_i::Save()
{
  if ( myFileName == 0 || strlen( myFileName ) == 0 )
    return false;

#if OCC_VERSION_MAJOR < 7
  FILE* aOutFile = fopen( myFileName, "wt" );
  if ( !aOutFile )
    return false;

  LDOM_XmlWriter aWriter( aOutFile );
  aWriter.SetIndentation( 2 );
  aWriter << myDoc;
  fclose( aOutFile );
#else
  std::filebuf fb;
  fb.open( myFileName, std::ios::out );

  Standard_OStream os( &fb );

  LDOM_XmlWriter aWriter;
  aWriter.SetIndentation( 2 );
  aWriter.Write( os, myDoc );
  fb.close();
#endif

  TPythonDump()<<this<<".Save()";
  return true;
}

//=======================================================================
// name    : FilterLibrary_i::SaveAs
// Purpose : Save library on disk
//=======================================================================
CORBA::Boolean FilterLibrary_i::SaveAs( const char* aFileName )
{
  myFileName = strdup ( aFileName );
  TPythonDump()<<this<<".SaveAs('"<<aFileName<<"')";
  return Save();
}

//=======================================================================
// name    : FilterLibrary_i::IsPresent
// Purpose : Verify whether filter is in library
//=======================================================================
CORBA::Boolean FilterLibrary_i::IsPresent( const char* theFilterName )
{
  return !findFilter( theFilterName, myDoc ).isNull();
}

//=======================================================================
// name    : FilterLibrary_i::NbFilters
// Purpose : Return amount of filters in library
//=======================================================================
CORBA::Long FilterLibrary_i::NbFilters( ElementType theType )
{
  string_array_var aNames = GetNames( theType );
  return aNames->length();
}

//=======================================================================
// name    : FilterLibrary_i::GetNames
// Purpose : Get names of filters from library
//=======================================================================
string_array* FilterLibrary_i::GetNames( ElementType theType )
{
  string_array_var anArray = new string_array;
  TColStd_SequenceOfHAsciiString aSeq;

  LDOM_Node aSection = getSection( theType, myDoc, false );

  if ( !aSection.isNull() )
  {
    for ( LDOM_Node aFilter = aSection.getFirstChild();
          !aFilter.isNull(); aFilter = aFilter.getNextSibling() )
    {
      LDOM_Element& anElem = ( LDOM_Element& )aFilter;
      aSeq.Append( new TCollection_HAsciiString(
         (Standard_CString)anElem.getAttribute( "name" ).GetString() ) );
    }
  }

  anArray->length( aSeq.Length() );
  for ( int i = 1, n = aSeq.Length(); i <= n; i++ )
    anArray[ i - 1 ] = CORBA::string_dup( aSeq( i )->ToCString() );

  return anArray._retn();
}

//=======================================================================
// name    : FilterLibrary_i::GetAllNames
// Purpose : Get names of filters from library
//=======================================================================
string_array* FilterLibrary_i::GetAllNames()
{
  string_array_var aResArray = new string_array;
  for ( int type = SMESH::ALL; type <= SMESH::VOLUME; type++ )
  {
    SMESH::string_array_var aNames = GetNames( (SMESH::ElementType)type );

    int aPrevLength = aResArray->length();
    aResArray->length( aPrevLength + aNames->length() );
    for ( int i = 0, n = aNames->length(); i < n; i++ )
      aResArray[ aPrevLength + i ] = aNames[ i ];
  }

  return aResArray._retn();
}

//================================================================================
/*!
 * \brief Return an array of strings corresponding to items of enum FunctorType
 */
//================================================================================

static const char** getFunctNames()
{
  static const char* functName[] = {
    // IT's necessary to update this array according to enum FunctorType (SMESH_Filter.idl)
    // The order is IMPORTANT !!!
    "FT_AspectRatio",
    "FT_AspectRatio3D",
    "FT_Warping",
    "FT_MinimumAngle",
    "FT_Taper",
    "FT_Skew",
    "FT_Area",
    "FT_Volume3D",
    "FT_MaxElementLength2D",
    "FT_MaxElementLength3D",
    "FT_FreeBorders",
    "FT_FreeEdges",
    "FT_FreeNodes",
    "FT_FreeFaces",
    "FT_EqualNodes",
    "FT_EqualEdges",
    "FT_EqualFaces",
    "FT_EqualVolumes",
    "FT_MultiConnection",
    "FT_MultiConnection2D",
    "FT_Length",
    "FT_Length2D",
    "FT_NodeConnectivityNumber",
    "FT_BelongToMeshGroup",
    "FT_BelongToGeom",
    "FT_BelongToPlane",
    "FT_BelongToCylinder",
    "FT_BelongToGenSurface",
    "FT_LyingOnGeom",
    "FT_RangeOfIds",
    "FT_BadOrientedVolume",
    "FT_BareBorderVolume",
    "FT_BareBorderFace",
    "FT_OverConstrainedVolume",
    "FT_OverConstrainedFace",
    "FT_LinearOrQuadratic",
    "FT_GroupColor",
    "FT_ElemGeomType",
    "FT_EntityType", 
    "FT_CoplanarFaces",
    "FT_BallDiameter",
    "FT_ConnectedElements",
    "FT_LessThan",
    "FT_MoreThan",
    "FT_EqualTo",
    "FT_LogicalNOT",
    "FT_LogicalAND",
    "FT_LogicalOR",
    "FT_Undefined"};

#ifdef _DEBUG_
  // check if functName is complete, compilation failure means that enum FunctorType changed
  const int nbFunctors = sizeof(functName) / sizeof(const char*);
  int _assert[( nbFunctors == SMESH::FT_Undefined + 1 ) ? 2 : -1 ]; _assert[0]=_assert[1];
#endif

  return functName;
}

//================================================================================
/*!
 * \brief Return a string corresponding to an item of enum FunctorType
 */
//================================================================================

const char* SMESH::FunctorTypeToString(SMESH::FunctorType ft)
{
  if ( ft < 0 || ft > SMESH::FT_Undefined )
    return "FT_Undefined";
  return getFunctNames()[ ft ];
}

//================================================================================
/*!
 * \brief Converts a string to FunctorType. This is reverse of FunctorTypeToString()
 */
//================================================================================

SMESH::FunctorType SMESH::StringToFunctorType(const char* str)
{
  std::string name( str + 3 ); // skip "FT_"
  const char** functNames = getFunctNames();
  int ft = 0;
  for ( ; ft < SMESH::FT_Undefined; ++ft )
    if ( name == ( functNames[ft] + 3 ))
      break;

  //ASSERT( strcmp( str, FunctorTypeToString( SMESH::FunctorType( ft ))) == 0 );

  return SMESH::FunctorType( ft );
}

//================================================================================
/*!
 * \brief calls OnBaseObjModified(), if who != this, and myWaiters[i]->Modified(who)
 */
//================================================================================

void NotifyerAndWaiter::Modified( bool removed, NotifyerAndWaiter* who )
{
  if ( who != 0 && who != this )
    OnBaseObjModified( who, removed );
  else
    who = this;

  std::list<NotifyerAndWaiter*> waiters = myWaiters; // myWaiters can be changed by Modified()
  std::list<NotifyerAndWaiter*>::iterator i = waiters.begin();
  for ( ; i != waiters.end(); ++i )
    (*i)->Modified( removed, who );
}

//================================================================================
/*!
 * \brief Stores an object to be notified on change of predicate
 */
//================================================================================

void NotifyerAndWaiter::AddModifWaiter( NotifyerAndWaiter* waiter )
{
  if ( waiter )
    myWaiters.push_back( waiter );
}

//================================================================================
/*!
 * \brief Removes an object to be notified on change of predicate
 */
//================================================================================

void NotifyerAndWaiter::RemoveModifWaiter( NotifyerAndWaiter* waiter )
{
  myWaiters.remove( waiter );
}

//================================================================================
/*!
 * \brief Checks if a waiter is among myWaiters, maybe nested
 */
//================================================================================

bool NotifyerAndWaiter::ContainModifWaiter( NotifyerAndWaiter* waiter )
{
  bool is = ( waiter == this );

  std::list<NotifyerAndWaiter*>::iterator w = myWaiters.begin();
  for ( ; !is && w != myWaiters.end(); ++w )
    is = (*w)->ContainModifWaiter( waiter );

  return is;
}
