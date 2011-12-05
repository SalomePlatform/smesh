#!/usr/bin/env python

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
