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



class MacObject:
	""" 
	This represents a python class definition which contains 
	all necessary information about the macro object being created
	in Salome 
	"""

	def __init__( self, ObjectType, GeoParameters, MeshParameters, **args ):
		"""
		Initializes the macro object to be created, saves parameters inside of it, checks for neighboring objects,
		determines meshing parameters if necessary and finally launches the generation process.
		"""
		import Config,GenFunctions
		if Config.debug : print "Initializing object No. " + str(len(Config.ListObj)+1)

                if 'publish' in args :
        		if args['publish']==0 : Config.publish = 0
        		else : Config.publish = 1
		else : Config.publish = 1
                
                if 'groups' in args :
        	        self.GroupNames = args['groups']
                        for group in args['groups'] :
                                if not(group in Config.Groups) and group : Config.Groups.append(group)
		else : self.GroupNames = [None, None, None, None]
                
                if ObjectType == 'NonOrtho':
                        if not(len(GeoParameters)==4): print "Error: trying to construct a non-ortho object but the 4 constitutive vertices are not given!"
                        else :
                                Xmin = min([GeoParameters[i][0] for i in range(4)])
                                Xmax = max([GeoParameters[i][0] for i in range(4)])
                                Ymin = min([GeoParameters[i][1] for i in range(4)])
                                Ymax = max([GeoParameters[i][1] for i in range(4)])                        
                                self.GeoPar = [(0.5*(Xmin+Xmax),0.5*(Ymin+Ymax)),(Xmax-Xmin,Ymax-Ymin)]
                                self.PtCoor = GenFunctions.SortPoints(GeoParameters)
                else:
                        self.GeoPar = GeoParameters
                        [Xmin,Ymin,Xmax,Ymax] = [ self.GeoPar[0][0]-0.5*self.GeoPar[1][0], self.GeoPar[0][1]-0.5*self.GeoPar[1][1] ] + [ self.GeoPar[0][0]+0.5*self.GeoPar[1][0], self.GeoPar[0][1]+0.5*self.GeoPar[1][1] ]
                        self.PtCoor = [(Xmin,Ymin),(Xmax,Ymin),(Xmax,Ymax),(Xmin,Ymax)]      
                
		self.Type = ObjectType
		self.LowBound = [ self.GeoPar[0][0]-0.5*self.GeoPar[1][0], self.GeoPar[0][1]-0.5*self.GeoPar[1][1] ]
		self.UpperBound = [ self.GeoPar[0][0]+0.5*self.GeoPar[1][0], self.GeoPar[0][1]+0.5*self.GeoPar[1][1] ]
		self.MeshPar = MeshParameters
		self.GeoChildren = []
		self.GeoChildrenNames = []
		self.Mesh = []
		self.MeshGroups = []
		self.CheckInterfaces()
		if 'auto' in MeshParameters : self.AutoParam()
		if not(self.MeshPar[0]<0): self.Generate()
		else : 
			Config.ListObj.append(self)
			print("Aborting object creation\n ")

	def Generate(self) :
		"""
		This method generates the geometrical object with the corresponding mesh once all verifications (CheckInterfaces and AutoParam) 
		have been accomplished
		"""
		import GenFunctions, Alarms, Config
		self = {'Box11'    : lambda : GenFunctions.Box11(self),
		 	'Box42'    : lambda : GenFunctions.Box42(self),
		 	'BoxAng32' : lambda : GenFunctions.BoxAng32(self),
			'CompBox'  : lambda : GenFunctions.CompBox(self),
			'CompBoxF' : lambda : GenFunctions.CompBoxF(self),
			'NonOrtho' : lambda : GenFunctions.NonOrtho(self),
		 	'QuartCyl' : lambda : GenFunctions.QuartCyl(self) }[self.Type]()

		if Config.debug : Alarms.Message(self.status)	# notification on the result of the generation algorithm
                

	def CheckInterfaces(self):
		"""
		This method searches for neighbours for the object being created and saves them inside the Config.Connections
		array. This array contains 4 entries per object corresponding to West, East, South, and North neighbours.
		Note that an object may have more than one neighbour for a given direction. 
		"""
		import Alarms, Config
                from GenFunctions import AddIfDifferent
                from CompositeBox import FindCommonSide
                
		Config.Connections.append([(-1,),(-1,),(-1,),(-1,)])
		itemID = len(Config.ListObj)
                # In all cases except non ortho, PrincipleBoxes is unitary and contains the box in question
                # In the non-ortho case it contains all possible combinations of boxes with 3 vertices 
                PrincipleBoxes = self.PrincipleBoxes()
		for i, TestObj in enumerate(Config.ListObj): 
                        SecondaryBoxes = TestObj.PrincipleBoxes()                               
			ConnX = 0
			ConnY = 0
                        for Box0 in PrincipleBoxes:
                                for Box1 in SecondaryBoxes:
			                # Along X
			                CenterDis = abs(Box1[0][0]-Box0[0][0])
			                Extension = 0.5*(Box1[1][0]+Box0[1][0])
			                if CenterDis - Extension < -1e-7 : 
				                ConnX = -1
			                elif CenterDis - Extension < 1e-7 :
                                                if not(FindCommonSide(self.DirBoundaries(2),TestObj.DirBoundaries(3))==[0,0]) and Box1[0][0] < Box0[0][0] : ConnX = 1
				                elif not(FindCommonSide(self.DirBoundaries(3),TestObj.DirBoundaries(2))==[0,0]) and Box1[0][0] >= Box0[0][0]: ConnX = 2
                                                else : ConnX = 0
                                                
			                # Along Y
			                CenterDis = abs(Box1[0][1]-Box0[0][1])
			                Extension = 0.5*(Box1[1][1]+Box0[1][1])
			                if CenterDis - Extension < -1e-7 : 
				                ConnY = -1
			                elif CenterDis - Extension < 1e-7 :
				                if not(FindCommonSide(self.DirBoundaries(0),TestObj.DirBoundaries(1))==[0,0]) and Box1[0][1] < Box0[0][1] : ConnY = 1
				                elif not(FindCommonSide(self.DirBoundaries(1),TestObj.DirBoundaries(0))==[0,0]) and Box1[0][1] >= Box0[0][1]: ConnY = 2
                                                else : ConnY = 0

			                if not (ConnX*ConnY == 0) :
				                if max(ConnX,ConnY) == -1 and not('NonOrtho' in [self.Type,TestObj.Type]) : Alarms.Message(3)
				                else:
					                if ConnX == 1 and ConnY == -1:
						                if Config.Connections[i][1] == (-1,) : Config.Connections[i][1] = (itemID,)
						                else : Config.Connections[i][1] = AddIfDifferent(Config.Connections[i][1],itemID)
						                if Config.Connections[itemID][0] == (-1,) : Config.Connections[itemID][0] = (i,)
						                else : Config.Connections[itemID][0] = AddIfDifferent(Config.Connections[itemID][0],i)
					                elif ConnX == 2 and ConnY == -1:
						                if Config.Connections[i][0] == (-1,) : Config.Connections[i][0] = (itemID,)
						                else : Config.Connections[i][0] = AddIfDifferent(Config.Connections[i][0],itemID)
						                if Config.Connections[itemID][1] == (-1,) : Config.Connections[itemID][1] = (i,)
						                else : Config.Connections[itemID][1] = AddIfDifferent(Config.Connections[itemID][1],i)
					                elif ConnY == 1 and ConnX == -1:
						                if Config.Connections[i][3] == (-1,) : Config.Connections[i][3] = (itemID,)
						                else : Config.Connections[i][3] = AddIfDifferent(Config.Connections[i][3],itemID)
						                if Config.Connections[itemID][2] == (-1,) : Config.Connections[itemID][2] = (i,)
						                else : Config.Connections[itemID][2] = AddIfDifferent(Config.Connections[itemID][2],i)
					                elif ConnY ==2 and ConnX == -1:
						                if Config.Connections[i][2] == (-1,) : Config.Connections[i][2] = (itemID,)
						                else : Config.Connections[i][2] = AddIfDifferent(Config.Connections[i][2],itemID)
						                if Config.Connections[itemID][3] == (-1,) : Config.Connections[itemID][3] = (i,)
						                else : Config.Connections[itemID][3] = AddIfDifferent(Config.Connections[itemID][3],i)

	def AutoParam (self):
		"""
		This method is called only if the 'auto' keyword is used inside the meshing algorithm. It is based on the 
		connection results per object and tries to find the correct parameters for obtaining a final compatible mesh
		between the objects already present and the one being created. If this is not possible, the method gives an error
		message.
		"""
		import Alarms, Config, GenFunctions, CompositeBox
		MeshPar = [0,0,0,0]	# initialize the mesh parameter value to be used to -1
                [(X0,Y0),(DX,DY)] = self.GeoPar
		ObjectsInvolved = []
		for i, Conn in enumerate(Config.Connections[-1]):
			if not ( Conn == (-1,) ):   # Meaning that there is one or more neighbors on this direction
				for ObjID in Conn : 
                                        ToLook0 = [2,3,0,1][i]
                                        ToLook1 = [3,2,1,0][i]
                                        CommonSide =  CompositeBox.FindCommonSide(Config.ListObj[ObjID].DirBoundaries(ToLook1),self.DirBoundaries(ToLook0))
                                        #print "Common Side is:", CommonSide
                                        ToLook2 = [1,0,3,2][i]
                                        #print "Full Side is:", CompositeBox.IntLen(Config.ListObj[ObjID].DirBoundaries(ToLook1))
                                        #print "Full Segments on this direction are:", Config.ListObj[ObjID].DirectionalMeshParams[ToLook2]
                                        RealSegments = round(Config.ListObj[ObjID].DirectionalMeshParams[ToLook2]*CompositeBox.IntLen(CommonSide)/CompositeBox.IntLen(Config.ListObj[ObjID].DirBoundaries(ToLook1)))
                                        #print "RealSegments :", RealSegments
                                        
					MeshPar[i] = MeshPar[i] + RealSegments
					ObjectsInvolved.append(ObjID+1)
		self.DirectionalMeshParams =  MeshPar
		self.MeshPar[0] = GenFunctions.CompatibilityTest(self)

		if self.MeshPar[0] < 0 : 
			Alarms.Message(4)
			if self.MeshPar[0] == -1 : print ("Problem encountered with object(s) no. "+str(ObjectsInvolved))
			elif self.MeshPar[0] == -2 : print ("This object has no neighbours !!!")

	def Boundaries (self):
                """
                This method returns the global boundaries of the MacObject. [Xmin,Xmax,Ymin,Ymax]
                """
                Xmin = min([self.DirBoundaries(i)[0] for i in [0,1]])
                Xmax = max([self.DirBoundaries(i)[1] for i in [0,1]])
                Ymin = min([self.DirBoundaries(i)[0] for i in [2,3]])
                Ymax = max([self.DirBoundaries(i)[1] for i in [2,3]])
                
                return [Xmin,Xmax,Ymin,Ymax]
                
	def DirBoundaries (self, Direction):
                """
                This method returns a single interval giving [Xmin,Xmax] or [Ymin,Ymax] according to the required direction.
                This works particularly well for nonorthogonal objects.
                Direction : [0,1,2,3] <=> [South, North, West, East]
                """
                PtCoor = self.PtCoor
                PtCoor.append(self.PtCoor[0])
                if type(Direction) is str :
                        Dir = { 'South'  : lambda : 0,
		    		'North'  : lambda : 1,
		    		'West'   : lambda : 2,
                                'East'   : lambda : 3,}[Direction]()
                else : Dir = int(Direction)
                         
                PtIndex  = [0,2,3,1][Dir]
                DirIndex = [0,0,1,1][Dir]
                             
		return sorted([PtCoor[PtIndex][DirIndex],PtCoor[PtIndex+1][DirIndex]])
        def DirVectors (self, Direction):
                """
                This method returns for a given object, the real vectors which define a given direction
                The interest in using this method is for non-orthogonal objects where the sides can be 
                deviated from the orthogonal basis vectors
                """
                if type(Direction) is str :
                        Dir = { 'South'  : lambda : 0,
		    		'North'  : lambda : 1,
		    		'West'   : lambda : 2,
                                'East'   : lambda : 3,}[Direction]()
                else : Dir = int(Direction)
                PtCoor = self.PtCoor
                PtCoor.append(self.PtCoor[0])
                PtIndex  = [0,2,3,1][Dir]
                return [PtCoor[PtIndex+1][0]-PtCoor[PtIndex][0],PtCoor[PtIndex+1][1]-PtCoor[PtIndex][1],0.]
                                
        def GetBorder (self, Criterion):
                import GenFunctions, Config

                from salome.geom import geomBuilder
                geompy = geomBuilder.New( Config.theStudy )
                
                if type(Criterion) is str :
                        Crit = {'South'  : lambda : 0,
		    		'North'  : lambda : 1,
		    		'West'   : lambda : 2,
                                'East'   : lambda : 3,}[Criterion]()
                else : Crit = int(Criterion)
                
                AcceptedObj = []
                if Crit < 4 :
                        Boundaries = self.Boundaries()
                        Research = {0 : lambda : [self.DirVectors(0),1,Boundaries[2]],
		    	            1 : lambda : [self.DirVectors(1),1,Boundaries[3]],
		    	            2 : lambda : [self.DirVectors(2),0,Boundaries[0]],
		    	            3 : lambda : [self.DirVectors(3),0,Boundaries[1]], }[Crit]()
                                                            
                        for i,ElemObj in enumerate(self.GeoChildren):
	                        EdgeIDs = geompy.ExtractShapes(ElemObj,6)# List of Edge IDs belonging to ElemObj
                                for Edge in EdgeIDs:
                                        if GenFunctions.IsParallel(Edge,Research[0]):
                                                if abs( geompy.PointCoordinates(geompy.GetVertexByIndex(Edge,0))[Research[1]] - Research[2] )< 1e-6 or abs( geompy.PointCoordinates(geompy.GetVertexByIndex(Edge,1))[Research[1]] - Research[2] )< 1e-6 :
                                                        AcceptedObj.append(Edge)
                else :
                	CenterSrchPar = {'NE' : lambda : [-1., -1.],
		    	                 'NW' : lambda : [ 1., -1.],
		    	                 'SW' : lambda : [ 1.,  1.],
		      	                 'SE' : lambda : [-1.,  1.], }[self.MeshPar[1]]()
                        Radius = self.GeoPar[1][1]*float(self.MeshPar[2])/(self.MeshPar[2]+1)
                        Center = (self.GeoPar[0][0]+CenterSrchPar[0]*self.GeoPar[1][0]/2.,self.GeoPar[0][1]+CenterSrchPar[1]*self.GeoPar[1][1]/2.,0.)
                        for i,ElemObj in enumerate(self.GeoChildren):
	                        EdgeIDs = geompy.ExtractShapes(ElemObj,6)# List of Edge IDs belonging to ElemObj
                                for Edge in EdgeIDs:
                                        if GenFunctions.IsOnCircle(Edge,Center,Radius):
                                                AcceptedObj.append(Edge)
                return AcceptedObj

        def PrincipleBoxes (self):
                """
                This function returns all possible combination rectangular shape objects that can contain at least 3 of the principle vertices
                constituting the MacObject. This is indispensible for the Non-ortho types and shall return a number of 24 possible combinations
                """
                from itertools import combinations
                Boxes = []
                if self.Type == 'NonOrtho':
                        for combi in combinations(range(4),3):
                                Xmin = min([self.PtCoor[i][0] for i in combi])
                                Xmax = max([self.PtCoor[i][0] for i in combi])
                                Ymin = min([self.PtCoor[i][1] for i in combi])
                                Ymax = max([self.PtCoor[i][1] for i in combi])                        
                                Boxes.append([(0.5*(Xmin+Xmax),0.5*(Ymin+Ymax)),(Xmax-Xmin,Ymax-Ymin)])
                else :
                        Boxes = [self.GeoPar]
                
                return Boxes     
                        
        
