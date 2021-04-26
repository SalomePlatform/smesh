// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMDS : implementation of Salome mesh data structure
//
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "SMDS_MeshNode.hxx"

#include "SMDS_ElementFactory.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMDS_SpacePosition.hxx"

#include <utilities.h>
#include <Utils_SALOME_Exception.hxx>
#include <cassert>
#include <smIdType.hxx>

#include <boost/make_shared.hpp>

void SMDS_MeshNode::init(double x, double y, double z)
{
  SMDS_UnstructuredGrid * grid = getGrid();
  vtkPoints *points = grid->GetPoints();
  points->InsertPoint( GetVtkID(), x, y, z );
  if ( grid->HasLinks() )
    grid->GetLinks()->ResizeForPoint( GetVtkID() );
}

//=======================================================================
//function : RemoveInverseElement
//purpose  :
//=======================================================================

void SMDS_MeshNode::RemoveInverseElement(const SMDS_MeshElement * elem)
{
  if ( getGrid()->HasLinks() )
    getGrid()->RemoveReferenceToCell( GetVtkID(), elem->GetVtkID());
}

//=======================================================================
//function : Print
//purpose  :
//=======================================================================

void SMDS_MeshNode::Print(ostream & OS) const
{
  OS << "Node <" << GetID() << "> : X = " << X() << " Y = "
     << Y() << " Z = " << Z() << endl;
}

//=======================================================================
//function : SetPosition
//purpose  :
//=======================================================================

void SMDS_MeshNode::SetPosition(const SMDS_PositionPtr& aPos, int shapeID)
{
  myHolder->SetPosition( this, aPos, shapeID );
}

//=======================================================================
//function : GetPosition
//purpose  : Return a position of this node on shape
//warning  : result is std::unique_ptr !
//=======================================================================

SMDS_PositionPtr SMDS_MeshNode::GetPosition() const
{
  return myHolder->GetPosition( this );
}

//=======================================================================
/*!
 * \brief Iterator on list of elements
 */
//=======================================================================

namespace
{
  struct InverseIterator: public SMDS_ElemIterator
  {
    const SMDS_Mesh*       myMesh;
    size_t                 myIter;
    std::vector<vtkIdType> myCellList;

    InverseIterator(const SMDS_Mesh *   mesh = 0,
                    const vtkIdType*    cells = 0,
                    const int           ncells = 0,
                    SMDSAbs_ElementType type = SMDSAbs_All)
      : myMesh(mesh), myIter(0)
    {
      if ( ncells )
      {
        myCellList.reserve( ncells );
        if (type == SMDSAbs_All)
        {
          myCellList.assign( cells, cells + ncells );
        }
        else
        {
          for (int i = 0; i < ncells; i++)
          {
            vtkIdType vtkId = cells[i];
            smIdType smdsId = myMesh->FromVtkToSmds( vtkId );
            const SMDS_MeshElement* elem = myMesh->FindElement( smdsId );
            if ( elem->GetType() == type )
            {
              myCellList.push_back(vtkId);
            }
          }
        }
      }
    }

    bool more()
    {
      return ( myIter < myCellList.size() );
    }

    const SMDS_MeshElement* next()
    {
      vtkIdType vtkId = myCellList[ myIter++ ];
      smIdType smdsId = myMesh->FromVtkToSmds( vtkId );
      const SMDS_MeshElement* elem = myMesh->FindElement(smdsId);
      if (!elem)
      {
        MESSAGE("InverseIterator problem Null element");
        throw SALOME_Exception("InverseIterator problem Null element");
      }
      return elem;
    }
  };

  //=======================================================================
  /*!
   * \brief Iterator on a node
   */
  //=======================================================================

  template< class ELEM_ITERATOR >
  struct Iterator : public ELEM_ITERATOR
  {
    typedef typename ELEM_ITERATOR::value_type element_type;
    const SMDS_MeshNode* myNode;

    Iterator( const SMDS_MeshNode* n ): myNode( n ) {}

    virtual bool more()
    {
      return myNode;
    }
    virtual element_type next()
    {
      element_type res = static_cast<element_type>( myNode );
      myNode = 0;
      return res;
    }
  };
}

SMDS_ElemIteratorPtr SMDS_MeshNode::GetInverseElementIterator(SMDSAbs_ElementType type) const
{
  if ( GetMesh()->NbElements() > 0 ) // avoid building links
  {
    vtkCellLinks::Link& l = getGrid()->GetLinks()->GetLink( GetVtkID() );
    return boost::make_shared< InverseIterator >( GetMesh(), l.cells, l.ncells, type );
  }
  else
  {
    return boost::make_shared< InverseIterator >();
  }
}

SMDS_ElemIteratorPtr SMDS_MeshNode::nodesIterator() const
{
  return boost::make_shared< Iterator< SMDS_ElemIterator > >( this );
}

SMDS_NodeIteratorPtr SMDS_MeshNode::nodeIterator() const
{
  return boost::make_shared< Iterator< SMDS_NodeIterator > >( this );
}

const SMDS_MeshNode* SMDS_MeshNode::GetNode(const int ind) const
{
  return ind == 0 ? this : 0;
}

double* SMDS_MeshNode::getCoord() const
{
  return getGrid()->GetPoint( GetVtkID() );
}

double SMDS_MeshNode::X() const
{
  double *coord = getCoord();
  return coord[0];
}

double SMDS_MeshNode::Y() const
{
  double *coord = getCoord();
  return coord[1];
}

double SMDS_MeshNode::Z() const
{
  double *coord = getCoord();
  return coord[2];
}

//================================================================================
/*!
 * \brief thread safe getting coords
 */
//================================================================================

void SMDS_MeshNode::GetXYZ(double xyz[3]) const
{
  return getGrid()->GetPoint( GetVtkID(), xyz );
}

//================================================================================
void SMDS_MeshNode::setXYZ( double x, double y, double z )
{
  vtkPoints *points = getGrid()->GetPoints();
  points->InsertPoint( GetVtkID(), x, y, z );
  //GetMesh()->adjustBoundingBox(x, y, z);
  GetMesh()->setMyModified();
}

//=======================================================================
//function : AddInverseElement
//purpose  :
//=======================================================================
void SMDS_MeshNode::AddInverseElement( const SMDS_MeshElement* elem )
{
  SMDS_UnstructuredGrid* grid = getGrid();
  if ( grid->HasLinks() )
  {
    vtkCellLinks *Links = grid->GetLinks();
    Links->ResizeCellList( GetVtkID(), 1 );
    Links->AddCellReference( elem->GetVtkID(), GetVtkID() );
  }
}

//=======================================================================
//function : ClearInverseElements
//purpose  :
//=======================================================================
void SMDS_MeshNode::ClearInverseElements()
{
  getGrid()->ResizeCellList( GetVtkID(), 0);
}

//================================================================================
/*!
 * \brief Count inverse elements of given type
 */
//================================================================================

int SMDS_MeshNode::NbInverseElements(SMDSAbs_ElementType type) const
{
  int nb = 0;
  SMDS_Mesh *mesh = GetMesh();
  if ( mesh->NbElements() > 0 ) // avoid building links
  {
    vtkCellLinks::Link& l = mesh->GetGrid()->GetLinks()->GetLink( GetVtkID() );

    if ( type == SMDSAbs_All )
      return l.ncells;

    for ( int i = 0; i < l.ncells; i++ )
    {
      const SMDS_MeshElement* elem = mesh->FindElement( mesh->FromVtkToSmds( l.cells[i] ));
      nb += ( elem->GetType() == type );
    }
  }
  return nb;
}
