#  Copyright (C) 2004  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#
#
#  File   : SMESH_control.py
#  Author : Sergey LITONIN
#  Module : SMESH


import smesh
import SMESH_mechanic

salome = smesh.salome
mesh   = SMESH_mechanic.mesh


# ---- Criterion : AREA > 100

# create group
aGroup = mesh.MakeGroup("Area > 100", smesh.FACE, smesh.FT_Area, smesh.FT_MoreThan, 100)

# print result
anIds = aGroup.GetIDs()
print "Criterion: Area > 100 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]


# ----  Criterion : Taper > 3e-15

# create group
aGroup = mesh.MakeGroup("Taper > 3e-15", smesh.FACE, smesh.FT_Taper, smesh.FT_MoreThan, 3e-15)

# print result
anIds = aGroup.GetIDs()
print "Criterion: Taper > 3e-15 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]


# ----  Criterion : ASPECT RATIO > 1.3

# create group
aGroup = mesh.MakeGroup("Aspect Ratio > 1.3", smesh.FACE, smesh.FT_AspectRatio, smesh.FT_MoreThan, 1.3)

# print result
anIds = aGroup.GetIDs()
print "Criterion: Aspect Ratio > 1.3 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]


# ----  Criterion : MINIMUM ANGLE < 30

# create group
aGroup = mesh.MakeGroup("Minimum Angle < 30", smesh.FACE, smesh.FT_MinimumAngle, smesh.FT_LessThan, 30)

# print result
anIds = aGroup.GetIDs()
print "Criterion: Minimum Angle < 30 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]


# ---- Criterion : Warp > 2e-13

# create group
aGroup = mesh.MakeGroup("Warp > 2e-13", smesh.FACE, smesh.FT_Warping, smesh.FT_MoreThan, 2e-13 )

# print result
anIds = aGroup.GetIDs()
print "Criterion: Warp > 2e-13 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]


# ---- Criterion : Skew > 18

# create group
aGroup = mesh.MakeGroup("Skew > 18", smesh.FACE, smesh.FT_Skew, smesh.FT_MoreThan, 18 )

# print result
anIds = aGroup.GetIDs()
print "Criterion: Skew > 18 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]


# Criterion : Length > 10

# create group
aGroup = mesh.MakeGroup("Length > 10", smesh.FACE, smesh.FT_Length, smesh.FT_MoreThan, 10 )

# print result
anIds = aGroup.GetIDs()
print "Criterion: Length > 10 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]


# Criterion : Borders at multi-connections = 2

# create group
aGroup = mesh.MakeGroup("Borders at multi-connections = 2", smesh.EDGE, smesh.FT_MultiConnection, smesh.FT_EqualTo, 2)

# print result
anIds = aGroup.GetIDs()
print "Criterion: Borders at multi-connections = 2 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]


salome.sg.updateObjBrowser(1)
