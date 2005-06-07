//  File   : SMESH_TypeFilter.hxx
//  Module : SMESH

#ifndef _SMESH_TypeFilter_HeaderFile
#define _SMESH_TypeFilter_HeaderFile

#include "SMESH_Type.h"
#include "SUIT_SelectionFilter.h"

class SUIT_DataOwner;

class SMESH_TypeFilter : public SUIT_SelectionFilter
{
public:
  SMESH_TypeFilter (MeshObjectType theType);
  ~SMESH_TypeFilter();

  virtual bool isOk (const SUIT_DataOwner*) const;
  MeshObjectType type() const;

protected:
  MeshObjectType myType;
};

#endif
