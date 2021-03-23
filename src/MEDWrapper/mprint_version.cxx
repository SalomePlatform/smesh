// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
  if ( argc < 2 )
    return -1;

  med_idt fid = MEDfileOpen(argv[1], MED_ACC_RDONLY);
  if (fid < 0)
    return 1;

  med_int major, minor, release;
  med_err aRet = MEDfileNumVersionRd(fid, &major, &minor, &release);
  MEDfileClose(fid);
  if (aRet < 0) {
    // VSR: simulate med 2.3.6 behavior, med file version is assumed to be 2.1 or older
    major = 2;
    minor = release = -1;
  }

  printf("%d.%d.%d\n", (int)major, (int)minor, (int)release);
  return 0;
}
