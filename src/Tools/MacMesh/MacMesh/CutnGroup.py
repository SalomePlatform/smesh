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



# This module allows cutting and grouping geometries by defining plane sections, with level of cutting as well as customizable Prefixes.

import math, Config

from salome.geom import geomBuilder
geompy = geomBuilder.New( Config.theStudy )

def Go(GeoObj, CutPlnLst, OutLvlLst, PrefixLst, Publish):

	"""
	This function cuts any geometry (with infinite trim !) into several subgeometries that are cleanly saved inside the navigation tree. (Check GoTrim for the same functionality with custom trim size)
	  - GeoObj is the geometrical object to be cut and grouped
	  - CutPlnLst is a list of plane definitions. Each plane is a 6-tuple (contains 6 values). The first three represent the coordinates of the origin point and the second three represent the coordinates of the normal vector to the plane
		Example 1: [(0,0,0,1,0,0)]: cut along a plane passing through the origin and normal to the x-axis
		Example 2: [(0,0,0,1,0,0),(50,0,0,0,1,0)]: in addition to the first plane cut, cut through a plane passing by (50,0,0) and normal to the y axis.
		Note that the plane size us determined automatically from the size of the geometry in question (using a very big trim size = 100 x length of geometry!)
	  - OutLvlLst is a list containing integers that represent the inner sectioning level with respect to the original geometry type
		A value of 1 means that the section will provide elements of one level lower than the original type. For exemple a solid sectioned at level 1 will produce faces. A Face sectioned at level 1 will produce edges.
		A value of 2 means that a deeper sectioning will be applied. A solid sectioned with level 2 will give faces and edges. A face will give edges and vertices. An edge will give only vertices
		The number of elements in this list should be (this is verified in the code) equal to the number of elements in the plane cut list. This is logical.
		Example 1: [1]
		Example 2: [1, 2], This means that the cut over the second plane will produce two types of elements unlike the first cut which will only output the first level objects.
	  - PrefixLst is a list of strings that contains the naming Prefixes that are used by the script to generate the subshape names. This is very useful for relating the results to the sectioning requested.
		Example 1: ['Entry']
		Example 2: ['Entry','Exit']	The resulting groups from the sectioning with plane no.1 will then be saved as "Entry_FACE" and/or "Entry_EDGE" according to the original geometry object type and the cutting level
	
	Imagine that we have a solid called ExampleSolid, an example command will be:
	CutnGroup.Go(ExampleSolid,[(0,0,0,1,0,0),(50,0,0,0,1,0)],[1, 2],['Entry','Exit'])
	"""

	NumCuts = CheckInput(CutPlnLst, OutLvlLst, PrefixLst, 1)
	OrigType = FindStandType(GeoObj,0)
	InvDictionary = dict((v,k) for k, v in geompy.ShapeType.iteritems())	# Give geometry type name as a function of standard type numbering, ex: 4=FACE, 6=EDGE, 7=VERTEX
	TrimSize = geompy.BasicProperties(GeoObj)[0]*100
	CutPlane = [] ;	Sections = [] ; Parts = []
	
	if NumCuts:
		for i in range(0, NumCuts):		# Loop over the cutting planes to create them one by one
			CutPlane.append(CreatePlane(CutPlnLst[i],TrimSize))
		OutFather = geompy.MakePartition([GeoObj],CutPlane, [], [],FindStandType(GeoObj,1), 0, [], 0)	#Creating the partition object
		if Publish: geompy.addToStudy(OutFather,'SectionedObject')
		for i in range(0, NumCuts):
			for j in range(OrigType+1+2, OrigType+1+2*(OutLvlLst[i]+1),2):
				if j == 8 : j = 7;	# Exception for the vertex case (=7)
				PossSubShapesID = geompy.SubShapeAllIDs(OutFather,j)	# List of subshape IDs than correspond to the required cutting level (section type : face/wire/vertex)
				PossSubShapes =	geompy.ExtractShapes(OutFather,j)		# and the corresponding objects
				Accepted = []
				for k in range(0,len(PossSubShapesID)): 	# Loop over all the subshapes checking if they belong to the cutting plane! if yes add them to current list
					if  IsOnPlane(PossSubShapes[k], CutPlnLst[i], 1e-7):			
						Accepted.append(PossSubShapesID[k])
				if Accepted :					# If some element is found, save it as a group with the prescribed Prefix
					dummyObj = geompy.CreateGroup(OutFather, j)
					geompy.UnionIDs(dummyObj, Accepted)
					Sections.append(dummyObj)
					if Publish:geompy.addToStudyInFather(OutFather, dummyObj, PrefixLst[i]+"_"+InvDictionary[j][0:2])
				else :
					print "Warning: For the section no.", i, ", No intersection of type " + InvDictionary[j] + " was found. Hence, no corresponding groups were created"
		
		SubShapesID = geompy.SubShapeAllIDs(OutFather,OrigType+1)		# Saving also the groups corresponding to the sectioned item of the same type: ex. A solid into n sub-solids due to the sections
		for i in range(0,len(SubShapesID)):
			dummyObj = geompy.CreateGroup(OutFather, OrigType+1)
			geompy.UnionIDs(dummyObj, [SubShapesID[i]])
			Parts.append(dummyObj)
			if Publish: geompy.addToStudyInFather(OutFather, dummyObj, "SB"+"_"+InvDictionary[OrigType+1][0:3]+"_"+str(i+1))

		return OutFather, Sections, Parts
	else:
		print("Fatal error, the routine cannot continue any further, check your input variables")
		return -1

def GoTrim(GeoObj, CutPlnLst, OutLvlLst, PrefixLst, Publish):

	"""
	This function cuts any geometry into several subgeometries that are cleanly saved inside the navigation tree with a fully customizable trim size.
	  - GeoObj is the geometrical object to be cut and grouped
	  - CutPlnLst is a list of plane definitions. Each plane is a 7-tuple (contains 7 values). The first three represent the coordinates of the origin point and the second three represent the coordinates of the normal vector to the plane, the last value corresponds to the trim size of the planes
		Example 1: [(0,0,0,1,0,0,5)]: cut along a plane passing through the origin and normal to the x-axis with a trim size of 5
		Example 2: [(0,0,0,1,0,0,5),(50,0,0,0,1,0,10)]: in addition to the first plane cut, cut through a plane passing by (50,0,0) and normal to the y axis with a trim size of 10
	  - OutLvlLst is a list containing integers that represent the inner sectioning level with respect to the original geometry type
		A value of 1 means that the section will provide elements of one level lower than the original type. For exemple a solid sectioned at level 1 will produce faces. A Face sectioned at level 1 will produce edges.
		A value of 2 means that a deeper sectioning will be applied. A solid sectioned with level 2 will give faces and edges. A face will give edges and vertices. An edge will give only vertices
		The number of elements in this list should be (this is verified in the code) equal to the number of elements in the plane cut list. This is logical.
		Example 1: [1]
		Example 2: [1, 2], This means that the cut over the second plane will produce two types of elements unlike the first cut which will only output the first level objects.
	  - PrefixLst is a list of strings that contains the naming Prefixes that are used by the script to generate the subshape names. This is very useful for relating the results to the sectioning requested.
		Example 1: ['Entry']
		Example 2: ['Entry','Exit']	The resulting groups from the sectioning with plane no.1 will then be saved as "Entry_FACE" and/or "Entry_EDGE" according to the original geometry object type and the cutting level
	
	Imagine that we have a solid called ExampleSolid, an example command will be:
	CutnGroup.Go(ExampleSolid,[(0,0,0,1,0,0,5),(50,0,0,0,1,0,10)],[1, 2],['Entry','Exit'])
	"""

	NumCuts = CheckInput(CutPlnLst, OutLvlLst, PrefixLst, 0)
	OrigType = FindStandType(GeoObj,0)
	InvDictionary = dict((v,k) for k, v in geompy.ShapeType.iteritems())	# Give geometry type name as a function of standard type numbering, ex: 4=FACE, 6=EDGE, 7=VERTEX
	CutPlane = [] ;	Sections = [] ; Parts = []
	if NumCuts:
		for i in range(0, NumCuts):		# Loop over the cutting planes to create them one by one
			CutPlane.append(CreatePlane(CutPlnLst[i][0:6],CutPlnLst[i][6]))
		OutFather = geompy.MakePartition([GeoObj],CutPlane, [], [],FindStandType(GeoObj,1), 0, [], 0)	#Creating the partition object
		if Publish: geompy.addToStudy(OutFather,'SectionedObject')
		for i in range(0, NumCuts):
			for j in range(OrigType+1+2, OrigType+1+2*(OutLvlLst[i]+1),2):
				if j == 8 : j = 7;	# Exception for the vertex case (=7)
				PossSubShapesID = geompy.SubShapeAllIDs(OutFather,j)	# List of subshape IDs than correspond to the required cutting level (section type : face/wire/vertex)
				PossSubShapes =	geompy.ExtractShapes(OutFather,j)		# and the corresponding objects
				Accepted = []
				for k in range(0,len(PossSubShapesID)): 	# Loop over all the subshapes checking if they belong to the cutting plane WITH THE TRIM SIZE CONDITION! if yes add them to current list
					if  IsOnPlane(PossSubShapes[k], CutPlnLst[i], 1e-7) and Distance2Pt(geompy.PointCoordinates(geompy.MakeCDG(PossSubShapes[k])),CutPlnLst[i][0:3])<=CutPlnLst[i][-1]:			
						Accepted.append(PossSubShapesID[k])
				if Accepted :					# If some element is found, save it as a group with the prescribed Prefix
					dummyObj = geompy.CreateGroup(OutFather, j)
					geompy.UnionIDs(dummyObj, Accepted)
					Sections.append(dummyObj)
					if Publish: geompy.addToStudyInFather(OutFather, dummyObj, PrefixLst[i]+"_"+InvDictionary[j][0:2])
				else :
					print "Warning: For the section no.", i, ", No intersection of type " + InvDictionary[j] + " was found. Hence, no corresponding groups were created"
		
		SubShapesID = geompy.SubShapeAllIDs(OutFather,OrigType+1)		# Saving also the groups corresponding to the sectioned item of the same type: ex. A solid into n sub-solids due to the sections
		for i in range(0,len(SubShapesID)):
			dummyObj = geompy.CreateGroup(OutFather, OrigType+1)
			geompy.UnionIDs(dummyObj, [SubShapesID[i]])
			Parts.append(dummyObj)
			if Publish: geompy.addToStudyInFather(OutFather, dummyObj, "SB"+"_"+InvDictionary[OrigType+1][0:3]+"_"+str(i+1))

		return OutFather, Sections, Parts
	else:
		print("Fatal error, the routine cannot continue any further, check your input variables")
		return -1
def FindStandType(GeoObj, method):
	"""
	Find the standard index for the Geometrical object/compound type input, see dictionary in geompy.ShapeType
	"""
	TopType = GeoObj.GetMaxShapeType().__str__()
	UnModType = geompy.ShapeType[TopType]
	if method == 0 :
		StandType = UnModType-int(not(UnModType%2))		# So that wires and edges and considered the same, faces and shells, and so on
	else :
		StandType = UnModType

	return(StandType)

def CreatePlane(CutPlnVar,Trim):
	"""
	Creates a temporary point and vector in salome in order to build the sectioning planes needed
	"""
	Temp_Vtx = geompy.MakeVertex(CutPlnVar[0], CutPlnVar[1], CutPlnVar[2])
	Temp_Vec = geompy.MakeVectorDXDYDZ(CutPlnVar[3], CutPlnVar[4], CutPlnVar[5])
	CutPlane = geompy.MakePlane(Temp_Vtx, Temp_Vec, Trim)
	return(CutPlane)

def CheckInput(CutPlnLst, OutLvlLst, PrefixLst, AutoTrim):
	"""
	Checks the user input specifically if all needed parameters are provided
	"""
	if not ((len(CutPlnLst) == len(OutLvlLst)) and (len(CutPlnLst) == len(PrefixLst))):
		print("Missing information about one or more of the cut planes") 
		return 0
	elif not ((len(CutPlnLst[0]) == 6+int(not AutoTrim))):
		print("For each cutting plane you need to specify 6 parameters = 2 x 3 coordinates") 
		return 0
	else:
		return len(CutPlnLst)

def IsOnPlane(GeoSubObj, CutPlnVar, tolerance):
	"""
	Checks whether a geometry (vertex, segment, or face) belongs *completely* to the plane defined as a point and a normal vector
	"""
	# lambda function that represents the plane equation, function = 0 <=> Pt defined with Coor belongs to plane
	PlaneEq = lambda Coor: CutPlnVar[3]*(Coor[0]-CutPlnVar[0])+CutPlnVar[4]*(Coor[1]-CutPlnVar[1])+CutPlnVar[5]*(Coor[2]-CutPlnVar[2])
	OrigType = FindStandType(GeoSubObj,0)
	if (OrigType >= 7):		# Vertex
		NonTrimDecision = abs(PlaneEq(geompy.PointCoordinates(GeoSubObj))) < tolerance
                if len(CutPlnVar) == 6 : return NonTrimDecision # No trim condition used
                else : return (NonTrimDecision and Distance2Pt(CutPlnVar[0:3],geompy.PointCoordinates(GeoSubObj))<=CutPlnVar[6]/2)
	elif (OrigType >= 5):		# Line, decompose into two points then call recursively IsOnPlane function!
		Verdict = True
		for i in range(0,2):
			Verdict = Verdict and IsOnPlane(geompy.GetVertexByIndex(GeoSubObj,i), CutPlnVar, tolerance)
		return Verdict
	elif (OrigType >= 3):		# Face, decompose into three points then call recursively IsOnPlane function!
		if IsOnPlane(geompy.MakeCDG(GeoSubObj),CutPlnVar, tolerance) : # Center of gravity belongs to plane, check if normal is parallel to plane
			NormalP1Coor = geompy.PointCoordinates(geompy.GetVertexByIndex(geompy.GetNormal(GeoSubObj),0))
			NormalP2Coor = geompy.PointCoordinates(geompy.GetVertexByIndex(geompy.GetNormal(GeoSubObj),1))
			Normal = [NormalP1Coor[0]-NormalP2Coor[0], NormalP1Coor[1]-NormalP2Coor[1], NormalP1Coor[2]-NormalP2Coor[2]]
			CrossP = CrossProd(CutPlnVar[3:6],Normal)		# Checks whether normals (of section plane and of face) are parallel or not
			if (abs(CrossP[0])<tolerance and abs(CrossP[1])<tolerance and abs(CrossP[2])<tolerance):	# meaning zero cross product => parallel
				return True
			else :
				return False
		else :
			return False


def CrossProd(V1,V2):
	"""
	Determines the cross product of two 3D vectors
	"""
	return ([V1[1]*V2[2]-V1[2]*V2[1], V1[2]*V2[0]-V1[0]*V2[2], V1[0]*V2[1]-V1[1]*V2[0]])

def Distance2Pt(P1,P2):
	"""
	Returns the distance between two points
	"""
	return (math.sqrt((P1[0]-P2[0])**2+(P1[1]-P2[1])**2+(P1[2]-P2[2])**2))
