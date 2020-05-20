// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
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

#ifndef MON_MESHINFO_H
#define MON_MESHINFO_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Cas)

#include "ui_MeshInfo.h"
#include <QWidget>

class HOMARD_EXPORT MonMeshInfo : public QDialog, public Ui_MeshInfo
{
    Q_OBJECT

public:
    MonMeshInfo( QWidget* parent, bool modal, HOMARD::HOMARD_Gen_var myHomardGen);
    ~MonMeshInfo();

protected :
    QString _aCaseName;
    QString _aDirName;

    int _BlockSize;
    int _Connection;
    int _Diametre;
    int _Entanglement;
    int _Quality;

    HOMARD::HOMARD_Cas_var aCase ;
    HOMARD::HOMARD_Gen_var myHomardGen;

    virtual void InitConnect();
    virtual void SetNewCaseName();

public slots:
    virtual void SetDirName();
    virtual void SetFileName();

    virtual void SetBlockSize();
    virtual void SetConnection();
    virtual void SetEntanglement();
    virtual void SetDiametre();
    virtual void SetQuality();

    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();

    virtual void CaseNameChanged();
};

#endif // MON_MESHINFO_H
