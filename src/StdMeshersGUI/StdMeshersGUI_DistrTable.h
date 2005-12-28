//  SMESH StdMeshersGUI
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : StdMeshersGUI_DistrTable.h
//  Module : SMESH
//  $Header$

#ifndef StdMesherGUI_DistrTable_Header
#define StdMesherGUI_DistrTable_Header

#include <qtable.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButton;
class QDoubleValidator;

/*!
 *  \brief Values corresponding to buttons for table resize
 */
typedef enum { INSERT_ROW, REMOVE_ROW } TableButton;

/*!
 *  \brief This class represents custom table. It has only double values and
    editor for every cell has validator
 */
class StdMeshersGUI_DistrTable : public QTable
{
  Q_OBJECT
  
public:
  StdMeshersGUI_DistrTable( const int rows, QWidget* = 0, const char* = 0 );
  virtual ~StdMeshersGUI_DistrTable();

/*!
 *  \brief Hides current editor of cell
 */
  void stopEditing( const bool accept );
 
  virtual QSize sizeHint() const;

  static void sortData( SMESH::double_array& );

  void data( SMESH::double_array& );
  void setData( const SMESH::double_array& );

protected:
  virtual QWidget* createEditor( int, int, bool ) const;
  virtual bool     eventFilter( QObject*, QEvent* );
  virtual void     keyPressEvent( QKeyEvent* );
  virtual void     edit( const int, const int );

private slots:
  void onEdit( TableButton, int );

private:
  QDoubleValidator   *myArgV, *myFuncV;
};


/*!
 *  \brief This class represents frame for table and buttons
 */
class StdMeshersGUI_DistrTableFrame : public QFrame
{
  Q_OBJECT

public:
  StdMeshersGUI_DistrTableFrame( QWidget* );
  ~StdMeshersGUI_DistrTableFrame();

  StdMeshersGUI_DistrTable* table() const;

/*!
 *  \brief Changes shown state of some button for table resize
 */   
  void setShown( const TableButton, const bool );

/*!
 *  \brief Returns shown state of some button for table resize
 */
  bool isShown( const TableButton ) const;
  
private:
  QButton* button( const TableButton ) const;

private slots:
  void onButtonClicked();
  
signals:
/*!
 *  \brief This signal is emitted if some of button for table resize is clicked
 *         Second parameter is current row. Take into account that
 *         this object resize table ( returned by table() ) automatically
 */
  void toEdit( TableButton, int );
  void valueChanged( int, int );

private:
  QButton                   *myInsertRow, *myRemoveRow;
  StdMeshersGUI_DistrTable  *myTable;
};


#endif

