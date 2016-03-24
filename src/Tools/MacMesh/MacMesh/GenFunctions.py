# Copyright (C) 2014-2016  EDF R&D
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



# In this file are all the generation functions for manipulating the different created macro-objects

import math, copy
import Config
import CutnGroup
import CompositeBox

from salome.geom import geomBuilder
geompy = geomBuilder.New( Config.theStudy )

from salome.smesh import smeshBuilder
smesh = smeshBuilder.New( Config.theStudy )

##########################################################################################################

def Box11 (MacObject):
	if Config.debug : print "Generating regular box"

	dummy1 = geompy.MakeScaleAlongAxes( ElemBox11 (), None , MacObject.GeoPar[1][0], MacObject.GeoPar[1][1], 1)
	RectFace = geompy.MakeTranslation(dummy1, MacObject.GeoPar[0][0], MacObject.GeoPar[0][1], 0)

	MacObject.GeoChildren.append(RectFace)
	MacObject.GeoChildrenNames.append("Box_"+ str(len(Config.ListObj)+1))
        
	if Config.debug : Publish (MacObject.GeoChildren,MacObject.GeoChildrenNames)

        if Config.publish :
	        MacObject.Mesh.append(smesh.Mesh(RectFace))			# Creation of a new mesh
	        Quad2D = MacObject.Mesh[0].Quadrangle()			# Applying a quadrangle hypothesis

	        EdgeIDs = geompy.SubShapeAllSorted(RectFace,6)	# List of Edge IDs belonging to RectFace, 6 = Edge in salome dictionary
	        Reg1D = MacObject.Mesh[0].Segment()
	        Reg1D.NumberOfSegments(MacObject.MeshPar[0])

	        MacObject.Mesh[0].Compute()			# Generates the mesh
                
	MacObject.DirectionalMeshParams = [MacObject.MeshPar[0],MacObject.MeshPar[0],MacObject.MeshPar[0],MacObject.MeshPar[0]]

	MacObject.status = 1
        Config.ListObj.append(MacObject)
	return MacObject

##########################################################################################################

def Box42 (MacObject):
	if Config.debug : print "Generating box 4-2 reducer"

	Z_Axis = geompy.MakeVectorDXDYDZ(0., 0., 1.) 
	RotAngle = {'SN' : lambda : 0,
		    'NS' : lambda : math.pi,
		    'EW' : lambda : math.pi/2,
		    'WE' : lambda : -math.pi/2, }[MacObject.MeshPar[1]]()
	dummy0 = geompy.MakeRotation( ElemBox42 () , Z_Axis, RotAngle )
	dummy1 = geompy.MakeScaleAlongAxes( dummy0, None , MacObject.GeoPar[1][0], MacObject.GeoPar[1][1], 1)
	RectFace = geompy.MakeTranslation(dummy1, MacObject.GeoPar[0][0], MacObject.GeoPar[0][1], 0)

	MacObject.GeoChildren.append(RectFace)
	MacObject.GeoChildrenNames.append("Box_"+ str(len(Config.ListObj)+1))
        
	if Config.debug : Publish (MacObject.GeoChildren,MacObject.GeoChildrenNames)

        if Config.publish :
	        MacObject.Mesh.append(smesh.Mesh(RectFace))			# Creation of a new mesh
	        Quad2D = MacObject.Mesh[0].Quadrangle()			# Applying a quadrangle hypothesis

	        EdgeIDs = geompy.SubShapeAllSorted(RectFace,6)	# List of Edge IDs belonging to RectFace, 6 = Edge in salome dictionary
	        Reg1D = MacObject.Mesh[0].Segment()
	        Reg1D.NumberOfSegments(MacObject.MeshPar[0])

	        MacObject.Mesh[0].Compute()			# Generates the mesh

	MacObject.status = 1

	x = MacObject.MeshPar[0]
	MacObject.DirectionalMeshParams = {'SN' : lambda : [3*x, 3*x, 4*x, 2*x],
		    			   'NS' : lambda : [3*x, 3*x, 2*x, 4*x],
		    			   'EW' : lambda : [2*x, 4*x, 3*x, 3*x],
		    			   'WE' : lambda : [4*x, 2*x, 3*x, 3*x], }[MacObject.MeshPar[1]]()

        Config.ListObj.append(MacObject)
	return MacObject

	
##########################################################################################################

def BoxAng32 (MacObject):
	if Config.debug : print "Generating sharp angle"
	Z_Axis = geompy.MakeVectorDXDYDZ(0., 0., 1.) 
	RotAngle = {'NE' : lambda : 0,
		    'NW' : lambda : math.pi/2,
		    'SW' : lambda : math.pi,
		    'SE' : lambda : -math.pi/2, }[MacObject.MeshPar[1]]()
	dummy0 = geompy.MakeRotation( ElemEdge32 () , Z_Axis, RotAngle )
	dummy1 = geompy.MakeScaleAlongAxes( dummy0, None , MacObject.GeoPar[1][0], MacObject.GeoPar[1][1], 1)
	RectFace = geompy.MakeTranslation(dummy1, MacObject.GeoPar[0][0], MacObject.GeoPar[0][1], 0)

	MacObject.GeoChildren.append(RectFace)
	MacObject.GeoChildrenNames.append("Box_"+ str(len(Config.ListObj)+1))
        
	if Config.debug : Publish (MacObject.GeoChildren,MacObject.GeoChildrenNames)
        
        if Config.publish : 
               MacObject.Mesh.append(smesh.Mesh(RectFace))                     # Creation of a new mesh
               Quad2D = MacObject.Mesh[0].Quadrangle()                 # Applying a quadrangle hypothesis

               EdgeIDs = geompy.SubShapeAllSorted(RectFace,6)      # List of Edge IDs belonging to RectFace, 6 = Edge in salome dictionary
               Reg1D = MacObject.Mesh[0].Segment()
               Reg1D.NumberOfSegments(MacObject.MeshPar[0])

               MacObject.Mesh[0].Compute()                     # Generates the mesh

	MacObject.status = 1

	x = MacObject.MeshPar[0]
	MacObject.DirectionalMeshParams = {'NE' : lambda : [3*x, 2*x, 3*x, 2*x],
		    			   'NW' : lambda : [2*x, 3*x, 3*x, 2*x],
		    			   'SW' : lambda : [2*x, 3*x, 2*x, 3*x],
		    			   'SE' : lambda : [3*x, 2*x, 2*x, 3*x], }[MacObject.MeshPar[1]]()

        Config.ListObj.append(MacObject)
	return MacObject
##########################################################################################################
def CompBox (MacObject):
	if Config.debug : print "Generating composite box"

	dummy1 = geompy.MakeScaleAlongAxes( ElemBox11 (), None , MacObject.GeoPar[1][0], MacObject.GeoPar[1][1], 1)
	RectFace = geompy.MakeTranslation(dummy1, MacObject.GeoPar[0][0], MacObject.GeoPar[0][1], 0)

	MacObject.GeoChildren.append(RectFace)
	MacObject.GeoChildrenNames.append("Box_"+ str(len(Config.ListObj)+1))
        
        if Config.debug : Publish (MacObject.GeoChildren,MacObject.GeoChildrenNames)

        if Config.publish : 
              MacObject.Mesh.append(smesh.Mesh(RectFace))                     # Creation of a new mesh
              Quad2D = MacObject.Mesh[0].Quadrangle()                 # Applying a quadrangle hypothesis

              EdgeIDs = geompy.SubShapeAllSorted(RectFace,6)      # List of Edge IDs belonging to RectFace, 6 = Edge in salome dictionary

              ReducedRatio = ReduceRatio(MacObject.GeoPar[1][0],MacObject.GeoPar[1][1])

              Reference = [0,0,0]
              Vec = [(1,0,0),(0,1,0)]
              for Edge in EdgeIDs:
                              for i in range(0,2):
                                      if IsParallel(Edge,Vec[i]):
                                              if not Reference[i]:    # If this is the first found edge to be parallel to this direction, apply user preferences for meshing
                                                      Reference[i] = Edge
                                                      ApplyConstant1DMesh(MacObject.Mesh[0],Edge,int(round(ReducedRatio[i]*MacObject.MeshPar[0])))
                                                      break
                                              else:                   # If there already exists an edge parallel to this direction, then use a 1D projection
                                                      Apply1DProjMesh(MacObject.Mesh[0],Edge,Reference[i])
                                                      break

              MacObject.Mesh[0].Compute()                     # Generates the mesh
	
        MacObject.DirectionalMeshParams = [MacObject.MeshPar[0]*ReducedRatio[1],MacObject.MeshPar[0]*ReducedRatio[1],MacObject.MeshPar[0]*ReducedRatio[0],MacObject.MeshPar[0]*ReducedRatio[0]]

	MacObject.status = 1
        Config.ListObj.append(MacObject)
	return MacObject

##########################################################################################################

def CompBoxF (MacObject):
	if Config.debug : print "Generating composite box"

	dummy1 = geompy.MakeScaleAlongAxes( ElemBox11 (), None , MacObject.GeoPar[1][0], MacObject.GeoPar[1][1], 1)
	RectFace = geompy.MakeTranslation(dummy1, MacObject.GeoPar[0][0], MacObject.GeoPar[0][1], 0)

	MacObject.GeoChildren.append(RectFace)
	MacObject.GeoChildrenNames.append("Box_"+ str(len(Config.ListObj)+1))
        
	if Config.debug : Publish (MacObject.GeoChildren,MacObject.GeoChildrenNames)
        
        if Config.publish : 
               MacObject.Mesh.append(smesh.Mesh(RectFace))                     # Creation of a new mesh
               Quad2D = MacObject.Mesh[0].Quadrangle()                 # Applying a quadrangle hypothesis

               EdgeIDs = geompy.SubShapeAllSorted(RectFace,6)      # List of Edge IDs belonging to RectFace, 6 = Edge in salome dictionary

               #ReducedRatio = ReduceRatio(MacObject.GeoPar[1][0],MacObject.GeoPar[1][1])

               Reference = [0,0,0]
               Vec = [(1,0,0),(0,1,0)]
               for Edge in EdgeIDs:
                               for i in range(0,2):
                                       if IsParallel(Edge,Vec[i]):
                                               if not Reference[i]:    # If this is the first found edge to be parallel to this direction, apply user preferences for meshing
                                                       Reference[i] = Edge
                                                       ApplyConstant1DMesh(MacObject.Mesh[0],Edge,int(round(MacObject.MeshPar[0][i])))
                                                       break
                                               else:                   # If there already exists an edge parallel to this direction, then use a 1D projection
                                                       Apply1DProjMesh(MacObject.Mesh[0],Edge,Reference[i])
                                                       break

               MacObject.Mesh[0].Compute()                 # Generates the mesh
	
        MacObject.DirectionalMeshParams = [MacObject.MeshPar[0][1],MacObject.MeshPar[0][1],MacObject.MeshPar[0][0],MacObject.MeshPar[0][0]]

	MacObject.status = 1
        Config.ListObj.append(MacObject)
	return MacObject
##########################################################################################################

def NonOrtho (MacObject):
	if Config.debug : print "Generating Non-orthogonal quadrangle"

	RectFace = Quadrangler (MacObject.PtCoor)

	MacObject.GeoChildren.append(RectFace)
	MacObject.GeoChildrenNames.append("Quad_"+ str(len(Config.ListObj)+1))
        
        
	if Config.debug : Publish (MacObject.GeoChildren,MacObject.GeoChildrenNames)
        
        if Config.publish : 
               MacObject.Mesh.append(smesh.Mesh(RectFace))                     # Creation of a new mesh
               Quad2D = MacObject.Mesh[0].Quadrangle()                 # Applying a quadrangle hypothesis

               EdgeIDs = geompy.SubShapeAllSorted(RectFace,6)      # List of Edge IDs belonging to RectFace, 6 = Edge in salome dictionary

               #ReducedRatio = ReduceRatio(MacObject.GeoPar[1][0],MacObject.GeoPar[1][1])

               Vec = [MacObject.DirVectors(i) for i in range(4)]
               for Edge in EdgeIDs:
                        Dir = [IsParallel(Edge,Vec[j]) for j in range(4)].index(True)
                        DirConv = [0,0,1,1][Dir]
                        ApplyConstant1DMesh(MacObject.Mesh[0],Edge,int(round(MacObject.MeshPar[0][DirConv])))

               MacObject.Mesh[0].Compute()                 # Generates the mesh
	
        MacObject.DirectionalMeshParams = [MacObject.MeshPar[0][1],MacObject.MeshPar[0][1],MacObject.MeshPar[0][0],MacObject.MeshPar[0][0]]       
        
        MacObject.status = 1
	Config.ListObj.append(MacObject)
        return MacObject

##########################################################################################################

def QuartCyl (MacObject):
	if Config.debug : print "Generating quarter cylinder"
	Z_Axis = geompy.MakeVectorDXDYDZ(0., 0., 1.) 
	RotAngle = {'NE' : lambda : 0,
		    'NW' : lambda : math.pi/2,
		    'SW' : lambda : math.pi,
		    'SE' : lambda : -math.pi/2, }[MacObject.MeshPar[1]]()
	dummy0 = geompy.MakeRotation( ElemQuartCyl(MacObject.MeshPar[2]) , Z_Axis, RotAngle )
	dummy1 = geompy.MakeScaleAlongAxes( dummy0, None , MacObject.GeoPar[1][0]/10., MacObject.GeoPar[1][1]/10., 1)
	RectFace = geompy.MakeTranslation(dummy1, MacObject.GeoPar[0][0], MacObject.GeoPar[0][1], 0)

	MacObject.GeoChildren.append(RectFace)
	MacObject.GeoChildrenNames.append("Box_"+ str(len(Config.ListObj)+1))
        
	if Config.debug : Publish (MacObject.GeoChildren,MacObject.GeoChildrenNames)
        
        if Config.publish : 
               MacObject.Mesh.append(smesh.Mesh(RectFace))                     # Creation of a new mesh
               Quad2D = MacObject.Mesh[0].Quadrangle()                 # Applying a quadrangle hypothesis

               EdgeIDs = geompy.SubShapeAllSorted(RectFace,6)      # List of Edge IDs belonging to RectFace, 6 = Edge in salome dictionary
               Reg1D = MacObject.Mesh[0].Segment()
                            
               #if MacObject.MeshPar[0] == 2 and MacObject.MeshPar[2] <= 2.:
               #         print("Due to a bug in Salome 6.3, we are forced to either increase or decrease the local refinement by 50%, we choose in this case to increase the model's refinement.")
               #         MacObject.MeshPar[0] = 3

               Reg1D.NumberOfSegments(MacObject.MeshPar[0])

               MacObject.Mesh[0].Compute()                     # Generates the mesh

	MacObject.status = 1

	x = MacObject.MeshPar[0]
        N = QuarCylParam(MacObject.MeshPar[2])+1
        
	MacObject.DirectionalMeshParams = {'NE' : lambda : [2*x, N*x, 2*x, N*x],
		    			   'NW' : lambda : [N*x, 2*x, 2*x, N*x],
		    			   'SW' : lambda : [N*x, 2*x, N*x, 2*x],
		    			   'SE' : lambda : [2*x, N*x, N*x, 2*x], }[MacObject.MeshPar[1]]()

        Config.ListObj.append(MacObject)
	return MacObject
        
##########################################################################################################
# Below this are the elementary calculation/visualization functions 
##########################################################################################################

def Publish (ObjToPublish,NamesToPublish):
	i = 0
	for GeoObj in ObjToPublish :
		geompy.addToStudy(GeoObj,NamesToPublish[i])
		i = i+1

def IsParallel (Edge, Vector):
        """
        Function checks whether a given edge object is parallel to a reference vector. 
        Output can be 0 (not parallel) or 1 (parallel and same sense) or 2 (parallel and opposite sense).
        If the reference vector is null, the function returns 0
        """
        if Vector == (0,0,0) : return 0
	else :
                P1 = geompy.PointCoordinates(geompy.GetVertexByIndex(Edge,0))
	        P2 = geompy.PointCoordinates(geompy.GetVertexByIndex(Edge,1))
	        V0 = [ P1[0] - P2[0], P1[1] - P2[1], P1[2] - P2[2] ]
	        if Distance2Pt((0,0,0),CrossProd(V0,Vector))<1e-7 and DotProd(V0,Vector) > 0 : return 1
	        elif Distance2Pt((0,0,0),CrossProd(V0,Vector))<1e-7 and DotProd(V0,Vector) < 0 : return 2
	        else : return 0

def IsOnCircle (Edge, Center, Radius):
        """
        Function checks whether a given edge object belong to the periphery of a circle defined by its 
        center and radius. 
        Output can be 0 (does not belong) or 1 (belongs).
        If the reference Radius is null, the function returns 0
        Note that this function is basic in the sense that it only checks if the two border points of a 
        given edge belong to the arc of reference.
        """
        if Radius == 0 : return 0
	else :
                P1 = geompy.PointCoordinates(geompy.GetVertexByIndex(Edge,0))
	        P2 = geompy.PointCoordinates(geompy.GetVertexByIndex(Edge,1))
	        if abs(Distance2Pt(Center,P1)-Radius) < 1e-6 and abs(Distance2Pt(Center,P2)-Radius) < 1e-6:
                        return 1
                else :
	                return 0
                
def CrossProd(V1,V2):
	"""
	Determines the cross product of two 3D vectors
	"""
	return ([V1[1]*V2[2]-V1[2]*V2[1], V1[2]*V2[0]-V1[0]*V2[2], V1[0]*V2[1]-V1[1]*V2[0]])

def QuarCylParam(PitchRatio):
        R = float(PitchRatio)/(PitchRatio+1)
        Eps = 1. - R
        X = (R+Eps/2.)*math.sin(math.pi/4)+Eps/2.
        N = int(math.floor((math.pi*R/4.)/(Eps/2.)))
        return N

def DotProd(V1,V2):
	"""
	Determines the dot product of two 3D vectors
	"""
        if len(V1)==2 : V1.append(0)
        if len(V2)==2 : V2.append(0)
        
	return (V1[0]*V2[0]+V1[1]*V2[1]+V1[2]*V2[2])

def Distance2Pt(P1,P2):
	"""
	Returns the distance between two points
	"""
	return (math.sqrt((P1[0]-P2[0])**2+(P1[1]-P2[1])**2+(P1[2]-P2[2])**2))

def ApplyConstant1DMesh (ParentMsh, Edge, Nseg):
	Reg1D = ParentMsh.Segment(geom=Edge)
	Len = Reg1D.NumberOfSegments(Nseg)

def Apply1DProjMesh (ParentMsh, Edge, Ref):
	Proj1D = ParentMsh.Projection1D(geom=Edge)
	SrcEdge = Proj1D.SourceEdge(Ref,None,None,None)

def EdgeLength (Edge):
        """
        This function returns the edge object length.
        """
	P1 = geompy.PointCoordinates(geompy.GetVertexByIndex(Edge,0))
	P2 = geompy.PointCoordinates(geompy.GetVertexByIndex(Edge,1))
	return Distance2Pt(P1,P2)


def D2R (Angle):
	return Angle*math.pi/180

def R2D (Angle):
	return Angle*180/math.pi

def F2D (FloatNumber):
	return round(FloatNumber*100.)/100.

def BezierGen (PointA, PointB, AngleA, AngleB):

	if AngleA == 0 and AngleB == 0 : return (geompy.MakeEdge(PointA, PointB))
 	else : 
		A = geompy.PointCoordinates(PointA)
		B = geompy.PointCoordinates(PointB)
		dAB = Distance2Pt(A,B)
		dAC = dAB * (math.tan(AngleA)*math.tan(AngleB)) / (math.sin(AngleA) * ( math.tan(AngleA)+math.tan(AngleB) ) )
		AngleOX_AB = math.acos((B[0]-A[0])/dAB)
		PointC = geompy.MakeVertex(A[0]+math.cos(AngleA+AngleOX_AB)*dAC,A[1]+math.sin(AngleA+AngleOX_AB)*dAC,0)
		CurveACB = geompy.MakeBezier([PointA,PointC,PointB])
		return CurveACB

def GetSideAngleForBezier (PointA , PointB):
	"""
	This function takes for input two points A and B where the bezier line is needed. It calculates the incident 
	angle needed at point A so that the final curve is either at 0 or 90 degrees from the x'Ox axis
	"""
	A = geompy.PointCoordinates(PointA)
	B = geompy.PointCoordinates(PointB)
	ABx = B[0]-A[0]
	dAB = Distance2Pt(A,B)
	Alpha = math.acos(ABx/dAB)
	#print "New angle request"
	#print ABx, dAB, R2D(Alpha)
	if Alpha < math.pi/4 :
		#print "returning", R2D(-Alpha)
		return -Alpha
	elif Alpha < 3*math.pi/4 :
		#print "returning", R2D(-(Alpha-math.pi/2))
		return -(Alpha-math.pi/2)
	else : 
		#print "returning", R2D(-(Alpha-math.pi))
		return -(Alpha-math.pi)

def VecDivRatio (Vec1, Vec2):
	"""
	This function tries to find the ratio of Vec1 on Vec2 while neglecting any zero term in Vec1. This is used afterwards
	for determining the global mesh parameter from automatically detected directional mesh params. If no compatibility is
	possible, the function returns -1
	"""
	Vec3 = []
	for i in range(len(Vec1)) :
		Vec3.append(float(Vec1[i])/Vec2[i])
	Ratio=[]
	for i in Vec3 : 
		if not (abs(i)<1e-7) : Ratio.append(i)
	if Ratio :
		if min(Ratio) == max(Ratio) and min(Ratio)==int(min(Ratio)) : return(min(Ratio))
		else : return -1		
	else :
		return -2
			
			
def ReduceRatio (dx, dy):
	"""
	This function transforms a decimal ratio into a scale between two integers, for example : [0.2,0.05] --> [4,1] ; 
	"""
	Output = [0,0]
	ratio = float(dy)/dx
	if isinteger(ratio) : return [1,ratio]
	elif dx == 1 :			# when this function is called recursively! 
		for i in range(1,20) : 	# searches over 20 decimals
			if isinteger(ratio * (10**i) ) :
				Output = GetScale((10**i),int(round(ratio * (10**i) ) ) )
				break
			else :
				for n in range(0,i) :
					if isinteger(ratio * ( 10**(i)-10**(n) )) :
						Output = GetScale( 10**(i)-10**(n)  ,  int(round(ratio * ( 10**(i)-10**(n) ) ) ) )
						break
				if not (Output==[0,0]) : break
		return Output		
	else :
		for i in range(1,10) : 	# searches over 10 decimals
			if isinteger(ratio * (10**i) ) :
				Output = GetScale((10**i),int(round(ratio * (10**i) ) ) )
				break
			else :
				for n in range(0,i) :
					if isinteger(ratio * ( 10**(i)-10**(n) )) :
						Output = GetScale( 10**(i)-10**(n)  ,  int(round(ratio * ( 10**(i)-10**(n) ) ) ) )
						break
				if not (Output==[0,0]) : break

		if Output == [0,0] : 
			print "We are having some trouble while interpreting the following ratio: ",ratio, "\nWe will try a recursive method which may in some cases take some time..."
			if dy > dx :
				A = ReduceRatio (dx, dy-dx)
				return ([A[0],A[1]+A[0]])
			else : 
				A = ReduceRatio (dy, dx-dy)
				return ([A[1]+A[0],A[0]])

		else : return Output
		
def GetScale (X,Y):
	"""
	This function is called within ReduceRatio and aims to reduce down two integers X and Y by dividing them with their common divisors;
	Example: 25 and 5 ---> 5 and 1 / 63 and 12 ---> 21 and 4
	"""
	MaxDiv = max(X,Y)
	Divisor = 2		# Initializing the divisor
	while MaxDiv >= Divisor :
		X0 = 0
		Y0 = 0
 		if not(X%Divisor) : 
			X0 = X/Divisor
			MaxDiv = max(MaxDiv,X0)
 		if not(Y%Divisor) : 
			Y0 = Y/Divisor
			MaxDiv = max(MaxDiv,Y0)
		if (X0*Y0) : 
			X = X0
			Y = Y0
		else : 
			Divisor = Divisor + 1 
	return [X,Y]

def isinteger (x) :
	"""
	This functions applies a simple check if the entered value is an integer
	"""
	x = float('%.5f' % (x)) 	#Truncate x to 5 digits after the decimal point
	if math.ceil(x) == math.floor(x) : return True
	else : return False		
##########################################################################################
# Below this are the functions that create the elementary forms for the macro objects
##########################################################################################

def ElemBox11 ():
	"""
	This function returns a simple square face of 1 side length
	""" 
	RectFace = geompy.MakeFaceHW(1, 1, 1)
	return RectFace

def ElemBox42 ():
	"""
	This function returns a square face of 1 side length, partitioned
	according to the elementary 4 to 2 reductor method
	""" 
	OrigRectFace = geompy.MakeFaceHW(1, 1, 1)

	SouthPt1 = geompy.MakeVertex (-.25, -.5, 0)
	SouthPt2 = geompy.MakeVertex (0, -.5, 0)
	SouthPt3 = geompy.MakeVertex (.25, -.5, 0)
	WestPt1 = geompy.MakeVertex (-.5, -.5+1./3, 0)
	WestPt2 = geompy.MakeVertex (-.5, -.5+2./3, 0)
	EastPt1 = geompy.MakeVertex (.5, -.5+1./3, 0)
	EastPt2 = geompy.MakeVertex (.5, -.5+2./3, 0)
	NorthPt = geompy.MakeVertex (0, .5, 0)
	MidPt1 = geompy.MakeVertex (0, .05, 0)
	MidPt2 = geompy.MakeVertex (.2, -.18, 0)
	MidPt3 = geompy.MakeVertex (0, -.28, 0)
	MidPt4 = geompy.MakeVertex (-.2, -.18, 0)

	Cutter = []
	Cutter.append(geompy.MakeEdge(SouthPt2, MidPt3))
	Cutter.append(geompy.MakeEdge(MidPt1, NorthPt))
	Cutter.append(BezierGen(SouthPt1, MidPt4, GetSideAngleForBezier(SouthPt1,MidPt4), D2R(15)))
	Cutter.append(BezierGen(SouthPt3, MidPt2, GetSideAngleForBezier(SouthPt3,MidPt2), D2R(-15)))
	Cutter.append(BezierGen(WestPt1, MidPt4, GetSideAngleForBezier(WestPt1,MidPt4), D2R(-10)))
	Cutter.append(BezierGen(EastPt1, MidPt2, GetSideAngleForBezier(EastPt1,MidPt2), D2R(10)))
	Cutter.append(BezierGen(WestPt2, MidPt1, GetSideAngleForBezier(WestPt2,MidPt1), D2R(-10)))
	Cutter.append(BezierGen(EastPt2, MidPt1, GetSideAngleForBezier(EastPt2,MidPt1), D2R(10)))
	Cutter.append(BezierGen(MidPt2, MidPt1, D2R(-15), D2R(-15)))
	Cutter.append(BezierGen(MidPt3, MidPt2, D2R(10), D2R(15)))
	Cutter.append(BezierGen(MidPt3, MidPt4, D2R(-10), D2R(-15)))
	Cutter.append(BezierGen(MidPt4, MidPt1, D2R(15), D2R(15)))

	RectFace = geompy.MakePartition([OrigRectFace],Cutter, [], [],4, 0, [], 0)	#Creating the partition object
	#i=1
	#for SingleCut in Cutter :
	#	geompy.addToStudy(SingleCut,'Cutter'+str(i))
	#	i = i+1
	#geompy.addToStudy(RectFace,'RectFace')
	return RectFace

def ElemEdge32 ():
	"""
	This function returns a square face of 1 side length, partitioned
	according to the elementary edge with 3 to 2 reductor
	""" 
	OrigRectFace = geompy.MakeFaceHW(1., 1., 1)

	SouthPt1 = geompy.MakeVertex (-1./6, -0.5, 0.)
	SouthPt2 = geompy.MakeVertex ( 1./6, -0.5, 0.)
	WestPt1 = geompy.MakeVertex  (-0.5, -1./6, 0.)
	WestPt2 = geompy.MakeVertex  (-0.5,  1./6, 0.)
	EastPt = geompy.MakeVertex   ( 0.5, 0., 0.)
	NorthPt = geompy.MakeVertex  (0., 0.5, 0.)

	MidPt1 = geompy.MakeVertex (-0.2, -0.2, 0.)
	MidPt2 = geompy.MakeVertex ( -0.02,  -0.02, 0.)

	Cutter = []
	Cutter.append(BezierGen(SouthPt1,  MidPt1,  GetSideAngleForBezier(SouthPt1,MidPt1) , D2R(-5)))
	Cutter.append(BezierGen( WestPt1,  MidPt1,  GetSideAngleForBezier(WestPt1 ,MidPt1) , D2R(-5)))
	Cutter.append(BezierGen(SouthPt2,  MidPt2,  GetSideAngleForBezier(SouthPt2,MidPt2) , D2R(-10)))
	Cutter.append(BezierGen(  EastPt,  MidPt2,  GetSideAngleForBezier(EastPt  ,MidPt2) , D2R(5)))
	Cutter.append(BezierGen( WestPt2,  MidPt2,  GetSideAngleForBezier(WestPt2 ,MidPt2) , D2R(-10)))
	Cutter.append(BezierGen(  MidPt2, NorthPt,  GetSideAngleForBezier(NorthPt ,MidPt2) , D2R(-5)))

	Cutter.append(geompy.MakeEdge(MidPt1, MidPt2))

	RectFace = geompy.MakePartition([OrigRectFace],Cutter, [], [],4, 0, [], 0)	#Creating the partition object
	#i=1
	#for SingleCut in Cutter :
	#	geompy.addToStudy(SingleCut,'Cutter'+str(i))
	#	i = i+1
	#geompy.addToStudy(RectFace,'RectFace')
	return RectFace

def Quadrangler (Points):
	"""
	This function returns a quadranglar face based on four points, non of which 3 are non-colinear.  
        The points are defined by their 2D [(x1,y1),(x2,y2)..] coordinates.
        Note that the list of points is already arranged upon the creation in MacObject
	""" 
        Pt = []
        for Point in Points: Pt.append(geompy.MakeVertex(Point[0], Point[1], 0))
        # The first point is added at the end of the list in order to facilitate the line creation
        Pt.append(Pt[0])
        #Draw the lines in order to form the 4 side polygon
        Ln=[]
        for i in range(4) : Ln.append(geompy.MakeLineTwoPnt(Pt[i],Pt[i+1]))	
	RectFace = geompy.MakeQuad (Ln[0],Ln[1],Ln[2],Ln[3]) 	
	return RectFace

def ElemQuartCyl(K):
	"""
	This function returns a quarter cylinder to box relay of 1 side length, partitioned
	with a pitch ratio of K, In other words the side of the box is R*(1+(1/K))
	""" 
        R = 10.*float(K)/(K+1)
        Eps = 10.- R
        
        Config.theStudy.SetReal("R"  , R)
        Config.theStudy.SetReal("minusR"  , -R)				
	Config.theStudy.SetReal("Eps", Eps)
        
	CylWire = geompy.MakeSketcher("Sketcher:F 'R' 0:R 0:L 'Eps':TT 10. 10.0:R 90:L 10.0:R 90:L 'Eps':R 90:C 'minusR' 90.0:WW", [0, 0, 0, 0, 0, 1, 1, 0, -0])	
	CylFace = geompy.MakeFace(CylWire, 1)

	SouthPt = geompy.MakeVertex (R+Eps/2., 0., 0)
	SouthWestPt = geompy.MakeVertex ( 0.,0., 0)   #The origin can be used for practical partionning objectifs
	WestPt = geompy.MakeVertex  (0., R+Eps/2., 0)
        
        N = int(math.floor((math.pi*R/4.)/(Eps/2.)))
        X = 10.*(1.-1./(N+1))
      
         
	EastPt = geompy.MakeVertex  (10.0,  X, 0.)
	NorthPt = geompy.MakeVertex   ( X, 10.0, 0.)

        DivFactor = 8./(F2D(math.log(K))-0.223)
        #MidPt = geompy.MakeVertex ((R+Eps)*math.cos(math.pi/4), (R+Eps)*math.sin(math.pi/4), 0.)
	MidPt = geompy.MakeVertex (X-Eps/DivFactor, X-Eps/DivFactor, 0.)

	Cutter = []
	Cutter.append(BezierGen(SouthWestPt,  MidPt,  GetSideAngleForBezier(SouthWestPt,MidPt) , D2R(-5)))
	Cutter.append(BezierGen( EastPt, MidPt,  GetSideAngleForBezier(EastPt,MidPt) , D2R(5)))
	Cutter.append(BezierGen( MidPt, NorthPt, (-1)**((K<1.25)*1)*D2R(-5), GetSideAngleForBezier(NorthPt,MidPt)))      
	SMBezier = BezierGen( SouthPt,  MidPt,  GetSideAngleForBezier(SouthPt ,MidPt) , D2R((K<1.25)*180-5))
        WMBezier = BezierGen( WestPt,  MidPt,  GetSideAngleForBezier(WestPt, MidPt) , D2R(-5))
	Cutter.append(WMBezier)
        Cutter.append(SMBezier)
        
        for i in range(1,N) :
                # Determining intermediate points on the bezier lines and then performing additional cuts
                
                TempAnglePlus = (math.pi/4)*(1+float(i)/N)
                SectionResult = CutnGroup.Go(WMBezier, [(0,0,0,math.sin(TempAnglePlus),-math.cos(TempAnglePlus),0)], [1], ['Dummy'], 0)
                TempPt1 = SectionResult[1][0]
                TempPt11 = geompy.MakeVertex  ((N-i)*X/N, 10., 0)
                
                TempAngleMinus = (math.pi/4)*(1-float(i)/N)
                SectionResult = CutnGroup.Go(SMBezier, [(0,0,0,math.sin(TempAngleMinus),-math.cos(TempAngleMinus),0)], [1], ['Dummy'], 0)
                TempPt2 = SectionResult[1][0]
                TempPt21 = geompy.MakeVertex  (10.,  (N-i)*X/N, 0)
                
                Cutter.append(geompy.MakeEdge(SouthWestPt, TempPt1))
                Cutter.append(geompy.MakeEdge(SouthWestPt, TempPt2))
                Cutter.append(geompy.MakeEdge(TempPt1, TempPt11))
                Cutter.append(geompy.MakeEdge(TempPt2, TempPt21))                

	CylFace = geompy.MakePartition([CylFace],Cutter, [], [],4, 0, [], 0)	#Creating the partition object
	CylFace = geompy.MakeTranslation(CylFace, -5., -5., 0.0)

	return CylFace
        
def CompatibilityTest(MacObject):
	Type = MacObject.Type
	if Type == 'Box11' :
		BaseDirPar = [1,1,1,1]
		return int(VecDivRatio(MacObject.DirectionalMeshParams, BaseDirPar))
	elif Type == 'Box42' :
		BaseDirPar = {'SN' : lambda : [3, 3, 4, 2],
		    	      'NS' : lambda : [3, 3, 2, 4],
		    	      'EW' : lambda : [2, 4, 3, 3],
		    	      'WE' : lambda : [4, 2, 3, 3], }[MacObject.MeshPar[1]]()
		return int(VecDivRatio(MacObject.DirectionalMeshParams, BaseDirPar))
	elif Type == 'BoxAng32' :
		BaseDirPar = {'NE' : lambda : [3, 2, 3, 2],
		    	      'NW' : lambda : [2, 3, 3, 2],
		    	      'SW' : lambda : [2, 3, 2, 3],
		    	      'SE' : lambda : [3, 2, 2, 3], }[MacObject.MeshPar[1]]()
		return int(VecDivRatio(MacObject.DirectionalMeshParams, BaseDirPar))
	elif Type == 'CompBox' :
		#print "dx is: ", MacObject.GeoPar[1][1], ". dy is: ",MacObject.GeoPar[1][0]
		ReducedRatio = ReduceRatio(MacObject.GeoPar[1][0], MacObject.GeoPar[1][1])
		#print ReducedRatio
		BaseDirPar = [ReducedRatio[1], ReducedRatio[1], ReducedRatio[0], ReducedRatio[0]]
		return int(VecDivRatio(MacObject.DirectionalMeshParams, BaseDirPar))
                
	elif Type == 'QuartCyl' :
                N = QuarCylParam(MacObject.MeshPar[2])+1
	        BaseDirPar = {'NE' : lambda : [2, N, 2, N],
		    	      'NW' : lambda : [N, 2, 2, N],
		    	      'SW' : lambda : [N, 2, N, 2],
		      	      'SE' : lambda : [2, N, N, 2], }[MacObject.MeshPar[1]]()
		return int(VecDivRatio(MacObject.DirectionalMeshParams, BaseDirPar))
	elif Type == 'CompBoxF' : 
		RealRatio = MacObject.GeoPar[1][1]/MacObject.GeoPar[1][0]
		Xd = 0
		Yd = 0
		if MacObject.DirectionalMeshParams[2]+MacObject.DirectionalMeshParams[3] :
			A = int(max(MacObject.DirectionalMeshParams[2:4]))                       
			Xd = int(VecDivRatio([A,0,0,0], [1,1,1,1]))                       
		if MacObject.DirectionalMeshParams[0]+MacObject.DirectionalMeshParams[1] :
			A = int(max(MacObject.DirectionalMeshParams[0:2]))                       
			Yd = int(VecDivRatio([0,0,A,0], [1,1,1,1]))
                        
		if Xd == 0 and Yd : Xd = int(round(Yd/RealRatio))
		elif Yd == 0 : Yd = int(round(RealRatio*Xd))
		
		return [Xd,Yd]
	elif Type == 'NonOrtho' :
                MeanDX = 0.5*(IntLen(MacObject.DirBoundaries(0))+IntLen(MacObject.DirBoundaries(1)))
                MeanDY = 0.5*(IntLen(MacObject.DirBoundaries(2))+IntLen(MacObject.DirBoundaries(3)))
		RealRatio = MeanDY/MeanDX
		Xd = 0
		Yd = 0
		if MacObject.DirectionalMeshParams[2]+MacObject.DirectionalMeshParams[3] :
			A = int(max(MacObject.DirectionalMeshParams[2:4]))                       
			Xd = int(VecDivRatio([A,0,0,0], [1,1,1,1]))                       
		if MacObject.DirectionalMeshParams[0]+MacObject.DirectionalMeshParams[1] :
			A = int(max(MacObject.DirectionalMeshParams[0:2]))                       
			Yd = int(VecDivRatio([0,0,A,0], [1,1,1,1]))
                        
		if Xd == 0 and Yd : Xd = int(round(Yd/RealRatio))
		elif Yd == 0 : Yd = int(round(RealRatio*Xd))
		
		return [Xd,Yd]

def IntLen (Interval) :
        """
        This function returns the length of a given interval even if the latter is not sorted correctly.
        """
        return abs(Interval[1]-Interval[0])
                        
def NextTo (RefBox, Direction, Extension):
        """
        This functions returns geometrical parameters for easy positioning of neighbouring objects.
        The input (RefBox) and output are in the form :  [(X0,Y0),(DX,DY)]        
        """
        X0_0 = RefBox[0][0]
        Y0_0 = RefBox[0][1]
        DX_0 = RefBox[1][0]
        DY_0 = RefBox[1][1]
        
        DirectionalCoef = {'Above' : lambda : [ 0, 1],
		    	   'Below' : lambda : [ 0,-1],
		    	   'Right' : lambda : [ 1, 0],
		    	   'Left ' : lambda : [-1, 0], }[Direction]()
        
        X0_1 = X0_0+ DirectionalCoef[0] * (DX_0/2.+Extension/2.)
        DX_1 = abs(DirectionalCoef[0]) * (Extension) + abs(DirectionalCoef[1])*DX_0
        Y0_1 = Y0_0+ DirectionalCoef[1] * (DY_0/2.+Extension/2.)
        DY_1 = abs(DirectionalCoef[1]) * (Extension) + abs(DirectionalCoef[0])*DY_0
        
        return [(X0_1,Y0_1),(DX_1,DY_1)]
        
def GeomMinMax (PtA, PtB):
        """
        This function returns geometrical parameters in the format  [(X0,Y0),(DX,DY)]. The input being 
        the coordinates of two points (Xa,Ya), (Xb,Yb).
        """
        # First test that the vector relying the two points is oblique
        AB = [PtB[0]- PtA[0],PtB[1]- PtA[1]]
        if 0 in AB :
                print ("Error: the two points are not correctly defined. In the orthonormal system XOY, it is impossible to define a rectangle with these two points")
                return -1
        else:
                X0 = 0.5*(PtA[0]+PtB[0])
                Y0 = 0.5*(PtA[1]+PtB[1])
                DX = abs(AB[0])
                DY = abs(AB[1])
                return [(X0,Y0),(DX,DY)]

def AddIfDifferent (List, Element):
        if not(Element in List):
                List = List+(Element,)
        return List

def IndexMultiOcc (Array,Element) :
        """
        This functions returns the occurrences indices of Element in Array.
        As opposed to Array.index(Element) method, this allows determining      
        multiple entries rather than just the first one!
        """
        Output = []
        try : Array.index(Element)
        except ValueError : print "No more occurrences"
        else : Output.append(Array.index(Element))
                
        if not(Output == []) and len(Array) > 1 :
                for index, ArrElem in enumerate(Array[Output[0]+1:]) :
                        if ArrElem == Element : Output.append(index+Output[0]+1)
                 
        return Output
        
def SortList (ValList, CritList):
        Output = []
        SortedCritList = copy.copy(CritList)
        SortedCritList.sort()
        for i in range(0,len(ValList)):
                if i > 0 :
                        if not(SortedCritList[i]==SortedCritList[i-1]):
                                index = IndexMultiOcc(CritList,SortedCritList[i])
                                Output= Output + [ValList[j] for j in index]
                else :  
                        index = IndexMultiOcc(CritList,SortedCritList[i])
                        Output= Output + [ValList[j] for j in index]
                        
        return Output

def SortPoints(Points):
        """
        This function sorts a list of the coordinates of N points as to start at 
        an origin that represents Xmin and Xmax and then proceed in a counter
        clock-wise sense
        """
        NbPts = len(Points)
        Xmin = min([Points[i][0] for i in range(NbPts)])
        Ymin = min([Points[i][1] for i in range(NbPts)])
        Xmax = max([Points[i][0] for i in range(NbPts)])
        Ymax = max([Points[i][1] for i in range(NbPts)])        
        Crit = [(abs(Point[0]-Xmin)+0.1*(Xmax-Xmin))*(abs(Point[1]-Ymin)+0.1*(Ymax-Ymin)) for Point in Points]
        #print "Input Points      : ", Points
        #print "Sorting Criterion : ", Crit
        Order = SortList (range(NbPts), Crit)
        #print "Sorted Results    : ", Order
        Output = []
        Output.append(Points[Order[0]])
        
        Point0 = Points[Order[0]]
        #print "Reference point :", Point0
        
        V = [[Point1[0]-Point0[0],Point1[1]-Point0[1]] for Point1 in Points]
        Cosines = [-(vec[0]-1E-10)/(math.sqrt(DotProd(vec,vec)+1e-25)) for vec in V]
        #print "Cosines criterion :", Cosines
        Order = SortList(range(NbPts),Cosines)
        #print "Ordered points:", Order
        for PtIndex in Order[:-1]: Output.append(Points[PtIndex])
        
        return Output

