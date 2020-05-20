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

#ifndef MON_EDITFILE_H
#define MON_EDITFILE_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include "SALOME_Selection.h"
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Cas)

#include "ui_EditFile.h"
#include <QWidget>

class QListBox;
class QDialog;

class HOMARD_EXPORT MonEditFile : public QWidget, public Ui_EditFile
{
    Q_OBJECT

public:
    MonEditFile( QWidget* parent,  bool modal,
                 HOMARD::HOMARD_Gen_var myHomardGen,
                 QString FileName, int option );
    ~MonEditFile();
    int _codret ;

protected :

    HOMARD::HOMARD_Gen_var myHomardGen;
    QString _aFileName ;
    int _option ;

    virtual void InitConnect();
    virtual void EditText();

public slots:
    virtual void PushOnPrint();

};

#endif // MON_EDITFILE_H
