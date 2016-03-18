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
#ifndef STDMESHERSGUI_FieldSelectorWdg_H
#define STDMESHERSGUI_FieldSelectorWdg_H

#include "SMESH_SMESHGUI.hxx"

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(GEOM_Gen)

#include <QGroupBox>

class QTreeWidget;
class QTreeWidgetItem;

/*!
 * \brief Widget listing all fields available for export to MED file
 */
class SMESHGUI_EXPORT SMESHGUI_FieldSelectorWdg : public QGroupBox
{
  Q_OBJECT

 public:
  SMESHGUI_FieldSelectorWdg( QWidget* = 0 );

  bool GetAllFeilds(const QList< QPair< SMESH::SMESH_IDSource_var, QString > >& meshes,
                    QList< QPair< GEOM::ListOfFields_var,          QString > >& fields);

  bool GetSelectedFeilds();

 private slots:
  
  void onItemCheck(QTreeWidgetItem * item, int column);

 private:

  QList< QPair< GEOM::ListOfFields_var, QString > > * myFields;
  QTreeWidget*                                        myTree;
};

#endif // STDMESHERSGUI_FieldSelectorWdg_H
