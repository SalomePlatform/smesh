//  File   : SMESH_NumberFilter.hxx
//  Module : SMESH

#ifndef _SMESH_NumberFilter_HeaderFile
#define _SMESH_NumberFilter_HeaderFile

#include "SUIT_SelectionFilter.h"

#include <TopAbs_ShapeEnum.hxx>
#include <TColStd_MapOfInteger.hxx>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)

class SUIT_DataOwner;

/*!
 *  Class       : SMESH_NumberFilter
 *  Description : Filter for geom or smesh objects.
 *                Filter geom objects by number of subshapes of the given type
 *                Parameters of constructor:
 *                * theSubShapeType - Type of subshape
 *                * theNumber       - Number of subshapes. Object is selected if it contains theNumber of
 *                                    theSubShapeType sub-shapes
 *                * theShapeType    - This map specifies types of object to be selected
 *                * theMainObject   - Sub-shapes of this object is selected only
 *                * theIsClosedOnly - Closed shapes is selected if this parameter is true
 */
class SMESH_NumberFilter : public SUIT_SelectionFilter
{
 public:
  SMESH_NumberFilter (const char*            theKind,
                      const TopAbs_ShapeEnum theSubShapeType,
                      const int              theNumber,
                      const TopAbs_ShapeEnum theShapeType = TopAbs_SHAPE,
                      GEOM::GEOM_Object_ptr  theMainObj = GEOM::GEOM_Object::_nil(),
                      const bool             theIsClosedOnly = false );

  SMESH_NumberFilter (const char*                 theKind,
                      const TopAbs_ShapeEnum      theSubShapeType,
                      const int                   theNumber,
                      const TColStd_MapOfInteger& theShapeTypes,
                      GEOM::GEOM_Object_ptr       theMainObj = GEOM::GEOM_Object::_nil(),
                      const bool                  theIsClosedOnly = false );

  virtual ~SMESH_NumberFilter();

  virtual bool isOk (const SUIT_DataOwner*) const;

  void SetSubShapeType (const TopAbs_ShapeEnum);
  void SetNumber       (const int);
  void SetClosedOnly   (const bool);
  void SetShapeType    (const TopAbs_ShapeEnum);
  void SetShapeTypes   (const TColStd_MapOfInteger&);
  void SetMainShape    (GEOM::GEOM_Object_ptr);

 private:
  GEOM::GEOM_Object_ptr getGeom (const SUIT_DataOwner*) const;

 private:
  char*                 myKind;
  TopAbs_ShapeEnum      mySubShapeType;
  int                   myNumber;
  bool                  myIsClosedOnly;
  TColStd_MapOfInteger  myShapeTypes;
  GEOM::GEOM_Object_var myMainObj;
};

#endif
