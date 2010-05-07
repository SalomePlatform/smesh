//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
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
#include <CORBA.h>

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
                                         QWidget*              parent);
  StdMeshersGUI_ObjectReferenceParamWdg( MeshObjectType objType,
                                         QWidget*       parent);
  ~StdMeshersGUI_ObjectReferenceParamWdg();

  void SetObject(CORBA::Object_ptr obj);

  template<class TInterface> 
    typename TInterface::_var_type GetObject() const {
    if ( IsObjectSelected() ) return TInterface::_narrow(myObject);
    return TInterface::_nil();
  }

  QString GetValue() const { return myParamValue; }

  bool IsObjectSelected() const { return !CORBA::is_nil(myObject); }

  void AvoidSimultaneousSelection( StdMeshersGUI_ObjectReferenceParamWdg* other);

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
  
private slots:
  void onSelectionDone(); 

private:
  void init();
  
private:
 CORBA::Object_var      myObject;
 SUIT_SelectionFilter*  myFilter;
 bool                   mySelectionActivated;

 SMESHGUI*              mySMESHGUI;
 LightApp_SelectionMgr* mySelectionMgr;

 QLineEdit*             myObjNameLineEdit;
 QPushButton*           mySelButton;
 QString                myParamValue;
};

#endif // STDMESHERSGUI_OBJECTREFERENCEPARAMWDG_H
