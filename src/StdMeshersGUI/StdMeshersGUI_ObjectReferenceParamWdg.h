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

// File   : StdMeshersGUI_ObjectReferenceParamWdg.h
// Author : Open CASCADE S.A.S.
//
#ifndef STDMESHERSGUI_OBJECTREFERENCEPARAMWDG_H
#define STDMESHERSGUI_OBJECTREFERENCEPARAMWDG_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

#include <SMESH_Type.h>

// Qt includes
#include <QWidget>

// CORBA includes
#include <omniORB4/CORBA.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SUIT_SelectionFilter;
class SMESHGUI;
class LightApp_SelectionMgr;
class QLineEdit;
class QPushButton;

/*!
 *  \brief Widget controlling hypothesis parameter that is an object reference
 */
class STDMESHERSGUI_EXPORT StdMeshersGUI_ObjectReferenceParamWdg : public QWidget
{
  Q_OBJECT

public:
  StdMeshersGUI_ObjectReferenceParamWdg( SUIT_SelectionFilter* filter, 
                                         QWidget*              parent,
                                         bool                  multiSelection=false
                                         /* ,bool                  stretch=true*/);
  StdMeshersGUI_ObjectReferenceParamWdg( SMESH::MeshObjectType objType,
                                         QWidget*       parent,
                                         bool           multiSelection=false);
  ~StdMeshersGUI_ObjectReferenceParamWdg();

  void SetObject(CORBA::Object_ptr obj);

  void SetObjects(SMESH::string_array_var& objEntries);

  template<class TInterface> 
    typename TInterface::_var_type GetObject(unsigned i=0) const {
    if ( IsObjectSelected(i) ) return TInterface::_narrow(myObjects[i]);
    return TInterface::_nil();
  }

  int NbObjects() const { return myObjects.size(); }

  QString GetValue() const { return myParamValue; }

  bool IsObjectSelected(unsigned i=0) const
  { return i < myObjects.size() && !CORBA::is_nil(myObjects[i]); }

  /*!
   * \brief Get the selection status
    *
    * Useful to know which Object Reference param widget is activated
    * to be able to activate the next one when the content of this
    * one has been modified
   */
  bool IsSelectionActivated() const { return mySelectionActivated; }

  void AvoidSimultaneousSelection( StdMeshersGUI_ObjectReferenceParamWdg* other);
  
  void SetDefaultText(QString defaultText="", QString styleSheet="");

public slots:
  /*!
   * \brief Activates selection (if not yet done), emits selectionActivated()
    *
    * Useful to deactivate one Object Reference param widget when an other
    * one is activated
   */
  void activateSelection();
  void deactivateSelection();

signals:
  /*!
   * \brief Emitted when selection is activated
    *
    * Useful to deactivate one Object Reference param widget when an other
    * one is activated
   */
  void selectionActivated();
  void contentModified();
  
private slots:
  void onSelectionDone(); 

private:
  void init();
  
private:

  bool                                myMultiSelection;
  std::vector<CORBA::Object_var>      myObjects;

 SUIT_SelectionFilter*  myFilter;
 bool                   mySelectionActivated;
 bool                   myStretchActivated;

 SMESHGUI*              mySMESHGUI;
 LightApp_SelectionMgr* mySelectionMgr;

 QLineEdit*             myObjNameLineEdit;
 QPushButton*           mySelButton;
 QString                myParamValue;
 QString                myEmptyText;
 QString                myEmptyStyleSheet;
};

#endif // STDMESHERSGUI_OBJECTREFERENCEPARAMWDG_H
