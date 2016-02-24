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

// File   : SMESHGUI_SelectionOp.h
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SELECTIONOP_H
#define SMESHGUI_SELECTIONOP_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Operation.h"
#include "SMESHGUI_Dialog.h"

// SALOME GUI includes
#include <SVTK_Selection.h>
#include <SALOME_InteractiveObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

class SUIT_SelectionFilter;
class SVTK_ViewWindow;
class SVTK_Selector;
class SMESH_Actor;

/*
  Class       : SMESHGUI_SelectionOp
  Description : Base operation for all operations using object selection in viewer or objectbrowser
                through common widgets created by LightApp_Dialog::createObject
*/
class SMESHGUI_EXPORT SMESHGUI_SelectionOp : public SMESHGUI_Operation
{ 
  Q_OBJECT

public:
  typedef QList<int> IdList; //! List of node or element ids
  
public:
  SMESHGUI_SelectionOp( const Selection_Mode = ActorSelection );
  virtual ~SMESHGUI_SelectionOp();

  static void                   extractIds( const QStringList&, IdList&, const QChar );  

protected:
  typedef enum
  {
    Object,
    MeshNode,
    MeshElement

  } EntityType;
  /*!
      This enumeration is used in typeById method to distinguish objects, mesh nodes and mesh elements,
      because node end element ids may overlap
  */
  
protected:
  virtual void                  startOperation();
  virtual void                  commitOperation();
  virtual void                  abortOperation();
  virtual void                  selectionDone();

  //! sets the dialog widgets to state just after operation start
  virtual void                  initDialog();

  /*!
   *  Creates filter being used when certain object selection widget is active
   *  If no filter must be used, then function must return 0
   *  if id is negative, then function must return filter for common using independently of active widget
   */
  virtual SUIT_SelectionFilter* createFilter( const int ) const;

  //! Remove only filters set by this operation (they are in map myFilters )
  void                          removeCustomFilters();

  //! Return what selection mode is set in VTK viewer
  Selection_Mode                selectionMode() const;

  //! Set selection mode in VTK viewer
  void                          setSelectionMode( const Selection_Mode );

  //! Hilight object in VTK viewer
  void                          highlight( const Handle( SALOME_InteractiveObject )&,
                                           const bool, const bool = true );
                               
  //! Select some nodes or elements in VTK
  void                          addOrRemoveIndex( const Handle( SALOME_InteractiveObject )&,
                                                  const TColStd_MapOfInteger&, const bool isModeShift);

  SVTK_ViewWindow*              viewWindow() const;
  SVTK_Selector*                selector() const;

  //! Get names, types and ids of selected objects
  virtual void                  selected( QStringList&, 
                                          SMESHGUI_Dialog::TypesList&, QStringList& ) const;

  //! Find type by id
  virtual int                   typeById( const QString&, const EntityType ) const;

  //! Char using to divide <entry> and <id> in string id representation. By default, '#'
  virtual QChar                 idChar() const;

  //! Try to find in certain object selection widget selected node or element ids and return it
  void                          selectedIds( const int, IdList& ) const;

  //! Find in QStringList correct node or element ids representation and append integer(id) to IdList
  void                          extractIds( const QStringList&, IdList& ) const;

  //! Return selected mesh if selection mode isn't ActorSelection and only one object is selected
  SMESH::SMESH_Mesh_var         mesh() const;

  //! Return actor according to selected mesh if selection mode isn't ActorSelection
  SMESH_Actor*                  actor() const;
  
protected slots:
  //! Installs filter corresponding to certain object selection widget
  virtual void                  onActivateObject( int );

  //! Removes filter corresponding to certain object selection widget
  virtual void                  onDeactivateObject( int );

  /*!
    *  Empty default implementation. In successors it may be used for more advanced selection checking.
    *  This slot is connected to signal when the selection changed in some object selection widget
  */
  virtual void                  onSelectionChanged( int );

  /*! Default implementation allowing user to edit selected ids "by hands".
      In order to run default mechanism, you must set for some
      object selection widget the "name indication" to "ListOfNames",
      "read only" state to false and connect the dialog's signal "objectChanged"
      to this slot
      Warning: this mechanism can process only integer ids, NOT MESH OR GROUP NAMES!!!
  */
  virtual void                  onTextChanged( int, const QStringList& );
  
private:
  typedef QMap<int, SUIT_SelectionFilter*> Filters;
  
private:
  Filters         myFilters;
  Selection_Mode  myDefSelectionMode, myOldSelectionMode;
};

#endif // SMESHGUI_SELECTIONOP_H
