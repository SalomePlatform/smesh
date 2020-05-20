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

#ifndef MON_ITERINFO_H
#define MON_ITERINFO_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Cas)

#include "ui_IterInfo.h"
#include <QDialog>
#include <QWidget>

class QListBox;

class HOMARD_EXPORT MonIterInfo : public QDialog, public Ui_IterInfo
{
    Q_OBJECT

public:
    MonIterInfo( QWidget* parent, bool modal, HOMARD::HOMARD_Gen_var myHomardGen, QString IterName);
    ~MonIterInfo();

protected :
    MonIterInfo( QWidget* parent, HOMARD::HOMARD_Gen_var myHomardGen, QString IterName );

    QString _IterName;
    QString _aCaseName;

    int _BlockSize;
    int _Connection;
    int _Diametre;
    int _Entanglement;
    int _Quality;
    int _Option;

    HOMARD::HOMARD_Iteration_var aIter ;
    HOMARD::HOMARD_Gen_var myHomardGen;

    virtual void InitConnect();

public slots:

    virtual void SetBlockSize();
    virtual void SetConnection();
    virtual void SetEntanglement();
    virtual void SetDiametre();
    virtual void SetQuality();

    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();

};

#endif // MON_ITERINFO_H
