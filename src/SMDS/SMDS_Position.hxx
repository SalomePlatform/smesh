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
//  File   : SMDS_Position.hxx
//  Module : SMESH
//
#ifndef _SMDS_Position_HeaderFile
#define _SMDS_Position_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_TypeOfPosition.hxx"
#include <memory>

//class SMDS_Position;
//typedef boost::shared_ptr<SMDS_Position> SMDS_PositionPtr;
//typedef SMDS_Position* SMDS_PositionPtr;

class SMDS_EXPORT SMDS_Position
{

 public:
  virtual SMDS_TypeOfPosition GetTypeOfPosition() const = 0;
  int GetDim() const { return GetTypeOfPosition(); }
  virtual const double* GetParameters() const = 0;
  virtual ~SMDS_Position() {}
};

/*!
 * \brief Replace "typedef SMDS_Position* SMDS_PositionPtr" by a smart
 *        pointer allowing implicit casting to derived types; e.g.
 *        if ( SMDS_FacePositionPtr fPos = node->GetPosition() )
 *          fPos->SetUParameter(0);
 */

template<class T>
class SMDS_Ptr : public std::unique_ptr< T >
{
  bool myIsOwner;

 public:
  SMDS_Ptr( T * pos = (T *) 0, bool isOwner=true ):
    std::unique_ptr< T >( pos ), myIsOwner( isOwner ) {}

  SMDS_Ptr( const SMDS_Ptr& from ) : std::unique_ptr< T >(), myIsOwner( from.myIsOwner )
  { this->swap( const_cast<SMDS_Ptr&>( from )); }

  SMDS_Ptr& operator=( const SMDS_Ptr& from  )
  {
    myIsOwner = from.myIsOwner;
    this->swap( const_cast<SMDS_Ptr&>( from ));
    return *this;
  }

  template<class Y>
    SMDS_Ptr( const SMDS_Ptr< Y >& base ): myIsOwner( true )
  {
    if ( const T* p = dynamic_cast<const T*>( base.get() ))
    {
      this->reset( const_cast<T*>( p ));
      this->myIsOwner = base.IsOwner();
      const_cast< SMDS_Ptr< Y >& >( base ).release();
    }
  }
  ~SMDS_Ptr() { if ( !myIsOwner ) this->release(); }

  operator bool () const { return bool( this->get() ); }
  bool IsOwner() const { return myIsOwner; }
};

class SMDS_EdgePosition;
class SMDS_FacePosition;
typedef SMDS_Ptr< SMDS_Position >     SMDS_PositionPtr;
typedef SMDS_Ptr< SMDS_EdgePosition > SMDS_EdgePositionPtr;
typedef SMDS_Ptr< SMDS_FacePosition > SMDS_FacePositionPtr;

#endif
