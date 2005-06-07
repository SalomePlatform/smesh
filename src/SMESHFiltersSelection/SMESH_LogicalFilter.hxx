//  File   : SMESH_LogicalFilter.hxx
//  Module : SMESH

#ifndef _SMESH_LogicalFilter_HeaderFile
#define _SMESH_LogicalFilter_HeaderFile

#include <SUIT_SelectionFilter.h>

#include <qptrlist.h>

class SMESH_LogicalFilter : public SUIT_SelectionFilter
{
 public:
  enum { LO_OR, LO_AND, LO_NOT, LO_UNDEFINED };

 public:
  SMESH_LogicalFilter (const QPtrList<SUIT_SelectionFilter>&,
                       const int);
  virtual ~SMESH_LogicalFilter();

  virtual bool isOk (const SUIT_DataOwner*) const;

  void                                 setFilters (const QPtrList<SUIT_SelectionFilter>&);
  void                                 setOperation (const int);
  const QPtrList<SUIT_SelectionFilter> getFilters() const;
  int                                  getOperation() const;

private:
  QPtrList<SUIT_SelectionFilter>  myFilters;
  int                             myOperation;
};

#endif
