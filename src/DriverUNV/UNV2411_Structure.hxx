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

#ifndef UNV2411_Structure_HeaderFile
#define UNV2411_Structure_HeaderFile

#include <map>
#include <fstream>	

namespace UNV2411{
  
  struct TRecord{
    TRecord();
    int exp_coord_sys_num;  // export coordinate system number
    int disp_coord_sys_num;  // displacement coordinate system number
    int color;  // color                                
    double coord[3];  // node coordinates in the part coordinate system
  };
  
  typedef int TNodeLab; // type of node label
  typedef std::map<TNodeLab,TRecord> TDataSet;

  void Read(std::ifstream& in_stream, TDataSet& theDataSet);

  void Write(std::ofstream& out_stream, const TDataSet& theDataSet);

};


#endif
