// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
#ifndef STDMESHERSGUI_RadioButtonsGrpWdg_H
#define STDMESHERSGUI_RadioButtonsGrpWdg_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

// Qt includes
#include <QGroupBox>

class QButtonGroup;
class QStringList;

/*!
 * \brief A QGroupBox holding several radio buttons
 */
class STDMESHERSGUI_EXPORT StdMeshersGUI_RadioButtonsGrpWdg : public QGroupBox
{
  Q_OBJECT

public:
  StdMeshersGUI_RadioButtonsGrpWdg (const QString& title);

  void setButtonLabels( const QStringList& buttonLabels,
                        const QStringList& buttonIcons=QStringList());

  void setChecked(int id);

  int checkedId() const;

  QButtonGroup* getButtonGroup() { return myButtonGrp; }

private:
  QButtonGroup* myButtonGrp;
};

#endif // STDMESHERSGUI_RadioButtonsGrpWdg_H
