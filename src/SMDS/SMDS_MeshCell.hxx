// Copyright (C) 2010-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

#ifndef _SMDS_MESHCELL_HXX_
#define _SMDS_MESHCELL_HXX_

#include "SMDS_MeshElement.hxx"

#include <vector>

/*!
 * \brief Base class for all cells
 */

class SMDS_EXPORT SMDS_MeshCell: public SMDS_MeshElement
{
 protected:

  void init( SMDSAbs_EntityType entityType, int nbNodes, ... );

  void init( SMDSAbs_EntityType entityType, const std::vector<const SMDS_MeshNode*>& nodes );

  void init( SMDSAbs_EntityType entityType, const std::vector<vtkIdType>& vtkNodeIds );

  friend class SMDS_Mesh;

 public:

  virtual int  NbEdges() const;
  virtual int  NbFaces() const;
  virtual int  NbNodes() const;
  virtual int  NbCornerNodes() const;
  virtual bool ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes);
  virtual int  GetNodeIndex( const SMDS_MeshNode* node ) const;
  virtual const SMDS_MeshNode* GetNode(const int ind) const;

  virtual SMDSAbs_ElementType  GetType() const;
  virtual SMDSAbs_EntityType   GetEntityType() const;
  virtual SMDSAbs_GeometryType GetGeomType() const;
  virtual VTKCellType          GetVtkType() const;

  virtual bool IsPoly() const;
  virtual bool IsQuadratic() const;

  virtual SMDS_ElemIteratorPtr nodesIterator() const;
  virtual SMDS_NodeIteratorPtr nodeIterator() const;
  virtual SMDS_NodeIteratorPtr interlacedNodesIterator() const;
  virtual SMDS_NodeIteratorPtr nodesIteratorToUNV() const;


  static void InitStaticMembers();
  static VTKCellType          toVtkType    ( SMDSAbs_EntityType   entityType );
  static SMDSAbs_EntityType   toSmdsType   ( VTKCellType          vtkType );
  static SMDSAbs_ElementType  ElemType     ( SMDSAbs_GeometryType geomType );
  static SMDSAbs_ElementType  ElemType     ( SMDSAbs_EntityType   entityType );
  static SMDSAbs_GeometryType GeomType     ( SMDSAbs_EntityType   entityType );
  static bool                 IsPoly       ( SMDSAbs_EntityType   entityType );
  static bool                 IsQuadratic  ( SMDSAbs_EntityType   entityType );
  static int                  NbCornerNodes( SMDSAbs_EntityType   entityType );
  static int                  NbNodes      ( SMDSAbs_EntityType   entityType );
  static int                  NbEdges      ( SMDSAbs_EntityType   entityType );
  static int                  NbFaces      ( SMDSAbs_EntityType   entityType );

  static const std::vector<int>& toVtkOrder(VTKCellType vtkType);
  static const std::vector<int>& toVtkOrder(SMDSAbs_EntityType smdsType);
  static const std::vector<int>& fromVtkOrder(VTKCellType vtkType);
  static const std::vector<int>& fromVtkOrder(SMDSAbs_EntityType smdsType);

  static const std::vector<int>& reverseSmdsOrder(SMDSAbs_EntityType smdsType,
                                                  const size_t       nbNodes=0);
  static const std::vector<int>& interlacedSmdsOrder(SMDSAbs_EntityType smdsType,
                                                     const size_t       nbNodes=0);


  template< class VECT > // interlacedIDs[i] = smdsIDs[ indices[ i ]]
    static void applyInterlace( const std::vector<int>& interlace, VECT & data)
  {
    if ( interlace.size() < data.size() ) return;
    VECT tmpData( data.size() );
    for ( size_t i = 0; i < data.size(); ++i )
      tmpData[i] = data[ interlace[i] ];
    data.swap( tmpData );
  }
  template< class VECT > // interlacedIDs[ indices[ i ]] = smdsIDs[i]
    static void applyInterlaceRev( const std::vector<int>& interlace, VECT & data)
  {
    if ( interlace.size() < data.size() ) return;
    VECT tmpData( data.size() );
    for ( size_t i = 0; i < data.size(); ++i )
      tmpData[ interlace[i] ] = data[i];
    data.swap( tmpData );
  }

};

#endif
