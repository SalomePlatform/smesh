// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#include <med.h>

#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
  med_idt aFid = MEDfileOpen(argv[1],MED_ACC_RDONLY);
  if(aFid < 0)
    exit(1);

  med_int aMajor, aMinor, aRelease;
  med_err aRet = MEDfileNumVersionRd(aFid,&aMajor,&aMinor,&aRelease);
  MEDfileClose(aFid);
  if(aRet < 0) {
    // VSR: simulate med 2.3.6 behavior, med file version is assumed to 2.1
    aMajor=2;
    aMinor=aRelease=-1;
  }

  printf("%d.%d.%d\n",aMajor,aMinor,aRelease);
}
