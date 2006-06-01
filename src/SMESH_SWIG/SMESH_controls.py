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


import SMESH
import SMESH_mechanic

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome


aFilterMgr = smesh.CreateFilterManager()

# Criterion : AREA > 100

aFunctor = aFilterMgr.CreateArea()
aPredicate = aFilterMgr.CreateMoreThan()
aPredicate.SetNumFunctor( aFunctor )
aPredicate.SetMargin( 100 )

aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Area > 100 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.FACE, "Area > 100" )
aGroup.Add( anIds )


# Criterion : Taper > 3e-15

aFunctor = aFilterMgr.CreateTaper()
aPredicate = aFilterMgr.CreateMoreThan()
aPredicate.SetNumFunctor( aFunctor )
aPredicate.SetMargin( 3e-15 )

aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Taper > 3e-15 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.FACE, "Taper > 3e-15" )
aGroup.Add( anIds )


# Criterion : ASPECT RATIO > 1.3

aFunctor = aFilterMgr.CreateAspectRatio()
aPredicate = aFilterMgr.CreateMoreThan()
aPredicate.SetNumFunctor( aFunctor )
aPredicate.SetMargin( 1.3 )

aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Aspect Ratio > 1.3 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.FACE, "Aspect Ratio > 1.3" )
aGroup.Add( anIds )


# Criterion : MINIMUM ANGLE < 30

aFunctor = aFilterMgr.CreateMinimumAngle()
aPredicate = aFilterMgr.CreateLessThan()
aPredicate.SetNumFunctor( aFunctor )
aPredicate.SetMargin( 30 )

aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Minimum Angle < 30 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.FACE, "Minimum Angle < 30" )
aGroup.Add( anIds )

# Criterion : Warp > 2e-13

aFunctor = aFilterMgr.CreateWarping()
aPredicate = aFilterMgr.CreateMoreThan()
aPredicate.SetNumFunctor( aFunctor )
aPredicate.SetMargin( 2e-13 )

aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Warp > 2e-13 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.FACE, "Warp > 2e-13" )
aGroup.Add( anIds )

# Criterion : Skew > 18

aFunctor = aFilterMgr.CreateSkew()
aPredicate = aFilterMgr.CreateMoreThan()
aPredicate.SetNumFunctor( aFunctor )
aPredicate.SetMargin( 18 )

aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Skew > 18 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.FACE, "Skew > 18" )
aGroup.Add( anIds )

# Criterion : Length > 10

aFunctor = aFilterMgr.CreateLength()
aPredicate = aFilterMgr.CreateMoreThan()
aPredicate.SetNumFunctor( aFunctor )
aPredicate.SetMargin( 10 )

aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Length > 10 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.EDGE, "Length > 10" )
aGroup.Add( anIds )

# Criterion : Borders at multi-connections = 2

aFunctor = aFilterMgr.CreateMultiConnection()
aPredicate = aFilterMgr.CreateEqualTo()
aPredicate.SetNumFunctor( aFunctor )
aPredicate.SetMargin( 2 )

aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Borders at multi-connections = 2 Nb = ", len( anIds )
#for i in range( len( anIds ) ):
  #print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.EDGE, "Borders at multi-connections = 2" )
aGroup.Add( anIds )


salome.sg.updateObjBrowser(1)
