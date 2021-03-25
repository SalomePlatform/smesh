// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
#ifndef STDMESHERSGUI_NameCheckableGrpWdg_H
#define STDMESHERSGUI_NameCheckableGrpWdg_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

// Qt includes
#include <QGroupBox>

#include <omniORB4/CORBA.h>

class QButtonGroup;
class QLineEdit;

/*!
 * \brief A QGroupBox holding several radio buttons
 */
class STDMESHERSGUI_EXPORT StdMeshersGUI_NameCheckableGrpWdg : public QGroupBox
{
  Q_OBJECT

public:
  StdMeshersGUI_NameCheckableGrpWdg(const QString& groupTitle,
                                    const QString& nameLabel);

  QString getName();

  void    setName( CORBA::String_var name );
  void    setDefaultName( QString name );

private:
  QLineEdit* myNameLineEdit;
};

#endif // STDMESHERSGUI_NameCheckableGrpWdg_H
