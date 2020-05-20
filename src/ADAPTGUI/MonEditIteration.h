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

#ifndef MON_EDITITERATION_H
#define MON_EDITITERATION_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)

#include <MonCreateIteration.h>

class HOMARD_EXPORT MonEditIteration : public MonCreateIteration
{
    Q_OBJECT
public:
    MonEditIteration( QWidget* parent, bool modal,
                      HOMARD::HOMARD_Gen_var myHomardGen,
                      QString IterParentName, QString Name );
    virtual ~MonEditIteration();

protected :
    QString _FieldFile ;
    QString _IterationName ;
    int _aTypeAdap ;
    int _rank ;
    int _step ;

    virtual void InitValEdit();
    virtual void InitValEdit0(int etat);
    virtual bool PushOnApply();

public slots:

};

#endif
