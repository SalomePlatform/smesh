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

#ifndef MON_CREATEYACS_H
#define MON_CREATEYACS_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Cas)

#include "ui_CreateYACS.h"
#include <QScrollArea>

class HOMARD_EXPORT MonCreateYACS : public QScrollArea, public Ui_CreateYACS
{
    Q_OBJECT

  public:
    MonCreateYACS( bool modal, HOMARD::HOMARD_Gen_var myHomardGen, QString CaseName);
    virtual ~MonCreateYACS();

  protected :
    MonCreateYACS( HOMARD::HOMARD_Gen_var myHomardGen, QString CaseName);

    QString _Name;
    QString _aCaseName;
    QString _aScriptFile;
    QString _aDirName;
    QString _aMeshFile;

    int _Type;
    int _MaxIter;
    int _MaxNode;
    int _MaxElem;

    bool Chgt;

    HOMARD::HOMARD_YACS_var aYACS;
    HOMARD::HOMARD_Gen_var myHomardGen;

    virtual void InitConnect();
    virtual bool CreateOrUpdate();

  public slots:
    virtual void SetNewName();

    virtual void SetCaseName();
    virtual void SetDirName();

    virtual void SetScriptFile();
    virtual void SetMeshFile();

//     virtual void SetType(int Type);
    virtual void SetConstant();
    virtual void SetVariable();

    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();

};

#endif // MON_CREATEYACS_H
