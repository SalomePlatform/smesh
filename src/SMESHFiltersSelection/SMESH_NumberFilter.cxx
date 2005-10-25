//  File   : SMESH_NumberFilter.cxx
//  Module : SMESH

#include "SMESH_NumberFilter.hxx"

#include "GEOM_Client.hxx"
#include "GeometryGUI.h"

#include "SUIT_Application.h"
#include "SUIT_Session.h"

#include "SalomeApp_Study.h"
#include "SalomeApp_DataOwner.h"

#include "SALOME_InteractiveObject.hxx"
#include "SALOMEDSClient_SObject.hxx"
#include "SALOMEDS_SObject.hxx"

#include <TopTools_MapOfShape.hxx>
#include <TopExp_Explorer.hxx>

/*!
 *  Class       : SMESH_NumberFilter
 *  Description : Filter for geom objects.
 *                Filter geom objects by number of subshapes of the given type
 */

//=======================================================================
// name    : SMESH_NumberFilter::SMESH_NumberFilter
// Purpose : Constructor
//=======================================================================
SMESH_NumberFilter::SMESH_NumberFilter (const char*            theKind,
                                        const TopAbs_ShapeEnum theSubShapeType,
                                        const int              theNumber,
                                        const TopAbs_ShapeEnum theShapeType,
                                        GEOM::GEOM_Object_ptr  theMainObj,
                                        const bool             theIsClosedOnly)
{
  myKind = (char*)theKind;
  mySubShapeType = theSubShapeType;
  myNumber = theNumber;
  myIsClosedOnly = theIsClosedOnly;
  myShapeTypes.Add(theShapeType);
  myMainObj = GEOM::GEOM_Object::_duplicate(theMainObj);
}

//=======================================================================
// name    : SMESH_NumberFilter::SMESH_NumberFilter
// Purpose : Constructor
//=======================================================================
SMESH_NumberFilter::SMESH_NumberFilter (const char*                 theKind,
                                        const TopAbs_ShapeEnum      theSubShapeType,
                                        const int                   theNumber,
                                        const TColStd_MapOfInteger& theShapeTypes,
                                        GEOM::GEOM_Object_ptr       theMainObj,
                                        const bool                  theIsClosedOnly )
{
  myKind = (char*)theKind;
  mySubShapeType = theSubShapeType;
  myNumber = theNumber;
  myIsClosedOnly = theIsClosedOnly;
  myShapeTypes = theShapeTypes;
  myMainObj = GEOM::GEOM_Object::_duplicate(theMainObj);
}

SMESH_NumberFilter::~SMESH_NumberFilter()
{
}

//=======================================================================
// name    : SMESH_NumberFilter::SMESH_NumberFilter
// Purpose : Verify validity of entry object
//=======================================================================
bool SMESH_NumberFilter::isOk (const SUIT_DataOwner* theDataOwner) const
{
  if (!theDataOwner)
    return false;

  // Get geom object from IO
  GEOM::GEOM_Object_var aGeomObj = getGeom(theDataOwner);
  if (aGeomObj->_is_nil())
    return false;

  // Get shape from geom object and verify its parameters
  GEOM_Client aGeomClient;
  TopoDS_Shape aShape = aGeomClient.GetShape(GeometryGUI::GetGeomGen(), aGeomObj);
  if (aShape.IsNull() ||
      !myShapeTypes.Contains(aShape.ShapeType()))
    return false;

  if (myIsClosedOnly && aShape.ShapeType() == TopAbs_SHELL && !aShape.Closed())
    return false;

  // Verify whether shape of entry object is sub-shape of myMainObj
  if (!myMainObj->_is_nil()) {
    TopoDS_Shape aMainShape = aGeomClient.GetShape(GeometryGUI::GetGeomGen(), myMainObj);
    if (aMainShape.IsNull())
      return false;

    bool isFound = false;
    TopAbs_ShapeEnum aShapeType = aShape.ShapeType();
    TopExp_Explorer anExp (aMainShape, aShapeType);
    for (; anExp.More(); anExp.Next()) {
      if (anExp.Current() == aShape) {
        isFound = true;
        break;
      }
    }
    if (!isFound)
      return false;
  }

  // Verify number of sub-shapes
  if (mySubShapeType == TopAbs_SHAPE);
    return true;

  TopExp_Explorer anExp2 (aShape, mySubShapeType);
  TopTools_MapOfShape aMap;
  for (; anExp2.More(); anExp2.Next())
    aMap.Add(anExp2.Current());

  return myNumber == aMap.Extent();
}

//=======================================================================
// name    : SMESH_NumberFilter::getGeom
// Purpose : Retrieve geom object from SALOME_InteractiveObject
//=======================================================================
GEOM::GEOM_Object_ptr SMESH_NumberFilter::getGeom
  (const SUIT_DataOwner* theDataOwner, const bool extractReference ) const
{
  const SalomeApp_DataOwner* owner =
    dynamic_cast<const SalomeApp_DataOwner*>(theDataOwner);
  SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>
    (SUIT_Session::session()->activeApplication()->activeStudy());

  GEOM::GEOM_Object_var anObj;

  if (!owner || !appStudy)
    return GEOM::GEOM_Object::_nil();

  _PTR(Study) study = appStudy->studyDS();
  QString entry = owner->entry();

  _PTR(SObject) aSO( study->FindObjectID( entry.latin1() ) ), aRefSO;
  if( extractReference && aSO && aSO->ReferencedObject( aRefSO ) )
    aSO = aRefSO;

  if (!aSO)
    return GEOM::GEOM_Object::_nil();

  CORBA::Object_var anObject = _CAST(SObject,aSO)->GetObject();
  anObj = GEOM::GEOM_Object::_narrow(anObject);
  if (!CORBA::is_nil(anObj))
    return anObj._retn();

  // Get geom object corresponding to the mesh
  _PTR(ChildIterator) anIter = study->NewChildIterator(aSO);
  for (; anIter->More(); anIter->Next()) {
    _PTR(SObject) aSO = anIter->Value();
    if (!aSO)
      continue;
    _PTR(SObject) aRefSO;
    _PTR(SObject) anObj;
    if (aSO->ReferencedObject(aRefSO))
      anObj = aRefSO;

    if (!anObj)
      anObj = aSO;

    anObject = _CAST(SObject,anObj)->GetObject();
    GEOM::GEOM_Object_var aMeshShape = GEOM::GEOM_Object::_narrow(anObject);

    if (!aMeshShape->_is_nil())
      return aMeshShape._retn();
  }

  return GEOM::GEOM_Object::_nil();
}

void SMESH_NumberFilter::SetSubShapeType (const TopAbs_ShapeEnum theSubShapeType)
{
  mySubShapeType = theSubShapeType;
}

void SMESH_NumberFilter::SetNumber (const int theNumber)
{
  myNumber = theNumber;
}

void SMESH_NumberFilter::SetClosedOnly (const bool theIsClosedOnly)
{
  myIsClosedOnly = theIsClosedOnly;
}

void SMESH_NumberFilter::SetShapeType (const TopAbs_ShapeEnum theShapeType)
{
  myShapeTypes.Add( theShapeType );
}

void SMESH_NumberFilter::SetMainShape (GEOM::GEOM_Object_ptr theMainObj)
{
  myMainObj = GEOM::GEOM_Object::_duplicate(theMainObj);
}
