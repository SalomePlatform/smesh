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

#ifndef UNV2412_Structure_HeaderFile
#define UNV2412_Structure_HeaderFile

#include <map>
#include <vector>
#include <fstream>	


namespace UNV2412{
  
  typedef std::vector<int> TNodeLabels; // Nodal connectivities

  struct TRecord{
    TRecord();

    int fe_descriptor_id;  // FE descriptor id
    int phys_prop_tab_num;  // physical property table number
    int mat_prop_tab_num;  // material property table number
    int color;  // color
    TNodeLabels node_labels;  // node labels defining element

    //FOR BEAM ELEMENTS ONLY
    int beam_orientation;  // beam orientation node number
    int beam_fore_end;  // beam fore-end cross section number
    int beam_aft_end;  // beam  aft-end cross section number
  };
  
  typedef int TElementLab; // type of element label
  typedef std::map<TElementLab,TRecord> TDataSet;

  void Read(std::ifstream& in_stream, TDataSet& theDataSet);

  void Write(std::ofstream& out_stream, const TDataSet& theDataSet);

  bool IsBeam(int theFeDescriptorId);
  bool IsFace(int theFeDescriptorId);
  bool IsVolume(int theFeDescriptorId);

};


#endif
