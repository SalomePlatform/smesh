//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 


#include "SMESH_ActorUtils.h"

#include "QAD_Config.h"
#include "utilities.h"

#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>

#ifdef _DEBUG_
static int MYDEBUG = 1;
#else
static int MYDEBUG = 0;
#endif

namespace SMESH{

  float GetFloat(const QString& theValue, float theDefault){
    if(theValue.isEmpty()) return theDefault;
    QString aValue = QAD_CONFIG->getSetting(theValue);
    if(aValue.isEmpty()) return theDefault;
    return aValue.toFloat();
  }

  void WriteUnstructuredGrid(vtkUnstructuredGrid* theGrid, const char* theFileName){
    vtkUnstructuredGridWriter* aWriter = vtkUnstructuredGridWriter::New();
    aWriter->SetFileName(theFileName);
    aWriter->SetInput(theGrid);
    if(theGrid->GetNumberOfCells()){
      aWriter->Write();
    }
    aWriter->Delete();
  }

}
