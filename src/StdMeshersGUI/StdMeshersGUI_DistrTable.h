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

// File   : StdMeshersGUI_DistrTable.h
// Author : Open CASCADE S.A.S.
//
#ifndef STDMESHERSGUI_DISTRTABLE_H
#define STDMESHERSGUI_DISTRTABLE_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

// Qt includes
#include <QWidget>
#include <QList>
#include <QMap>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QPushButton;

class StdMeshersGUI_DistrTableFrame : public QWidget
{
  Q_OBJECT

  enum { ArgColumn, FuncColumn };

  class Table;
  class SpinBoxDelegate;

  typedef SMESH::double_array DataArray;

public:
  typedef enum { InsertRowBtn, RemoveRowBtn } TableButton;
  
  StdMeshersGUI_DistrTableFrame( QWidget* = 0 );
  ~StdMeshersGUI_DistrTableFrame();

  void         showButton( const TableButton, const bool );
  bool         isButtonShown( const TableButton ) const;

  void         data( DataArray& ) const;
  void         setData( const DataArray& );

  void         setFuncMinValue( double );

private:
  QPushButton* button( const TableButton ) const;

private slots:
  void onInsert();
  void onRemove();

signals:
  void         valueChanged( int, int );
  void         currentChanged( int, int );

private:
  QMap<int, QPushButton*>   myButtons;
  Table*                    myTable;
};

#endif // STDMESHERSGUI_DISTRTABLE_H
