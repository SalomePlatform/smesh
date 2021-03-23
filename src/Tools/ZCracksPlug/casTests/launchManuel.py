# Copyright (C) 2016-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

import os, tempfile

directory=tempfile.mktemp(prefix='tmpZcracks')
print("directory=", tmpdir)

# Tous les cas
listCas=['1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','18','19','20','21']

# Cas sans les aretes ou faces sur la fissure :
listCas=['1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','20']

synthese={}

for cas in listCas:
  result=os.path.join(directory,'cracked'+cas+'.geo')
  if os.path.isfile(result):  os.remove(result)
  try:
    os.remove(os.path.join(directory,'cracked'+cas+'.geo'))
  except:
    pass
  os.system('cd '+directory+';Zrun -zp insert'+cas+'.z7p')

  synthese[cas]= os.path.isfile(result)

print(synthese)

