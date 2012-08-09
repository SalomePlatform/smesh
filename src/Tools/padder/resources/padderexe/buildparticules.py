#!/usr/bin/env python
# Copyright (C) 2011-2012  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

import geompy
import salome
salome.salome_init()
theStudy = salome.myStudy
geompy.init_geom(theStudy)

filename="/home/gboulant/development/projets/salome/SPADDER/spadder/resources/padderexe/REF_spheres.dat.xyz"
file=open(filename,'rb')

import csv
datalines = csv.reader(file, delimiter=' ')
i=0
for row in datalines:
    x=float(row[0])
    y=float(row[1])
    z=float(row[2])
    rayon=float(row[3])/2.

    centre = geompy.MakeVertex(x, y, z)
    particule = geompy.MakeSpherePntR(centre, rayon)
    geompy.addToStudy( particule, 'p'+str(i) )
    i+=1
