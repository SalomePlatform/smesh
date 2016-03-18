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


# INTRODUCTION HERE

import sys, math, copy, commands
CWD = commands.getoutput('pwd')
sys.path.append(CWD)

from MacObject import *
import Config, GenFunctions

def CompositeBoxF (Pt1 , Pt2 , Pt3 , Pt4 , **args ) : 
        [Pt1 , Pt2 , Pt3 , Pt4] = GenFunctions.SortPoints([Pt1 , Pt2 , Pt3 , Pt4])
        if args.__contains__('groups') :
                GroupNames = args['groups']
	else : GroupNames = [None, None, None, None]
        # Create a full NonOrtho box just to inherit, globally, the mesh parameters of bounding objects
     	dummy = MacObject('NonOrtho',[Pt1,Pt2,Pt3,Pt4],['auto'],publish=0)
        # Save the existing number of segments on each direction
        ExistingSeg0 = Config.ListObj[-1].DirectionalMeshParams
        Convention = [2,3,0,1]
        ExistingSegments = [ExistingSeg0[Convention[i]] for i in range(4)]
        # Save Boundary lengths on each direction
        BoundaryLengths = [IntLen(dummy.DirBoundaries(i)) for i in range(4) ]
        # Calculate global mesh element size on each direction
        GlobalDelta = [1.*BoundaryLengths[i]/ExistingSegments[i] for i in range(4) ]
        print "GlobalDelta :",GlobalDelta
        # Sort the connection list for the full Box
        [(X0,Y0),(DX,DY)] = dummy.GeoPar
        ObjIDLists = SortObjLists(Config.Connections[-1],X0 , Y0 , DX , DY )
        [Xmin,Xmax,Ymin,Ymax] = dummy.Boundaries() # Used for groups determination
        RemoveLastObj()
               
        RealSegments = []
        Direction = []
        flag = 0
        if not(args.__contains__('recursive')) : 
                Config.Count = 0

        Config.Criterion = GetCriterion(ObjIDLists)
        for index, ObjList in enumerate(ObjIDLists) :
                if not (ObjList[0] == -1 or Config.Count >= Config.Criterion):
                        if not(args.__contains__('recursive')) : 
                                Config.DirIndex = index
                                if index > 1 : Config.RefPts = [Pt2, Pt3]
                                elif index == 0 : Config.RefPts = [Pt1, Pt2]
                                else : Config.RefPts = [Pt4, Pt3]
                                
                        if len(ObjList)>1 : flag = 1
                        else : flag = 0
                        for ObjID in ObjList:
                           ToLook0 = [2,3,0,1][index]
                           ToLook1 = [3,2,1,0][index]
                           CommonSide =  FindCommonSide(Config.ListObj[ObjID].DirBoundaries(ToLook1),dummy.DirBoundaries(ToLook0))
                           ToLook2 = [1,0,3,2][index]
                           RealSegments = Config.ListObj[ObjID].DirectionalMeshParams[ToLook2]*IntLen(CommonSide)/IntLen(Config.ListObj[ObjID].DirBoundaries(ToLook1))
                           LocalDelta = 1.*IntLen(CommonSide)/RealSegments
                           print "Direction:", ["West","East","South","North"][ToLook2]
                           print "IntLen(CommonSide):",IntLen(CommonSide) 
                           print "RealSegments:",RealSegments    
                           print "LocalDelta:",LocalDelta                                                    
                           if flag and Config.Count < Config.Criterion:
                                if index ==0 :
                                        if abs(CommonSide[0] - Ymin)<1e-7 : SouthGR = GroupNames[0]
                                        else : SouthGR = None
                                        if abs(CommonSide[1] - Ymax)<1e-7 : NorthGR = GroupNames[1]
                                        else : NorthGR = None
                                        
                                        NDelta = Config.ListObj[ObjID].DirectionalMeshParams[ToLook2]* (LocalDelta-GlobalDelta[Convention[index]])
                                        [Pt1,Pt2] = Config.RefPts
                                        Coef = [1.,-1.][index] 
                                        Vref1 = [Coef*(Pt2[0]-Pt1[0]),Coef*(Pt2[1]-Pt1[1])]
                                        Vref2 = NormalizeVector([Pt2[0]-Pt3[0],Pt2[1]-Pt3[1]])
                                        Ptref = Config.ListObj[ObjID].PtCoor[[2,3][index]]
                                        NewPt = ExtrapPoint (Ptref,Vref1,Vref2,NDelta)
                                        CompositeBoxF (Pt1, Pt2, NewPt, Ptref, recursive=1, groups = [SouthGR,NorthGR]+GroupNames[2:4])
                                elif index == 1:
                                        if abs(CommonSide[0] - Ymin)<1e-7 : SouthGR = GroupNames[0]
                                        else : SouthGR = None
                                        if abs(CommonSide[1] - Ymax)<1e-7 : NorthGR = GroupNames[1]
                                        else : NorthGR = None
                                        
                                        NDelta = Config.ListObj[ObjID].DirectionalMeshParams[ToLook2]* (LocalDelta-GlobalDelta[Convention[index]])
                                        [Pt4,Pt3] = Config.RefPts
                                        Coef = 1.
                                        Vref1 = [Coef*(Pt4[0]-Pt3[0]),Coef*(Pt4[1]-Pt3[1])]
                                        Vref2 = NormalizeVector([Pt1[0]-Pt4[0],Pt1[1]-Pt4[1]])
                                        Ptref = Config.ListObj[ObjID].PtCoor[0]
                                        NewPt = ExtrapPoint (Ptref,Vref1,Vref2,NDelta)
                                        CompositeBoxF (NewPt, Ptref, Pt3, Pt4, recursive=1, groups = [SouthGR,NorthGR]+GroupNames[2:4])                                        
                                else : 
                                        if abs(CommonSide[0] - Xmin)<1e-7 : WestGR = GroupNames[2]
                                        else : WestGR = None
                                        if abs(CommonSide[1] - Xmax)<1e-7 : EastGR = GroupNames[3]
                                        else : EastGR = None
                                        
                                        NDelta = Config.ListObj[ObjID].DirectionalMeshParams[ToLook2]* (LocalDelta-GlobalDelta[Convention[index]])
                                        [Pt2,Pt3] = Config.RefPts
                                        Coef = [1.,-1.][index-2] 
                                        Vref1 = [Coef*(Pt3[0]-Pt2[0]),Coef*(Pt3[1]-Pt2[1])]
                                        Vref2 = NormalizeVector([Pt3[0]-Pt4[0],Pt3[1]-Pt4[1]])
                                        Ptref = Config.ListObj[ObjID].PtCoor[[3,0][index-2]]
                                        NewPt = ExtrapPoint (Ptref,Vref1,Vref2,NDelta)
                                        CompositeBoxF (Ptref, Pt2, Pt3, NewPt, recursive=1, groups = GroupNames[0:2] + [WestGR,EastGR])

                           if Config.Count >= Config.Criterion :
                                break
        if flag == 0 and Config.Count < Config.Criterion:
                print "Creating NonOrtho object with the points:", Pt1,Pt2,Pt3,Pt4
                MacObject('NonOrtho',[Pt1,Pt2,Pt3,Pt4] ,['auto'], groups = GroupNames)
                
                Config.Count += 1
                if Config.DirIndex > 1 : Config.RefPts = [Pt1, Pt4]
                elif Config.DirIndex==0 : Config.RefPts = [Pt4, Pt3]
                else : Config.RefPts = [Pt1, Pt2]
                           
def FindCommonSide (Int1, Int2) :
        if max(Int1[0],Int2[0])<min(Int1[1],Int2[1]): return [max(Int1[0],Int2[0]), min(Int1[1],Int2[1])]
        else : 
                print "Can not find interval intersection, returning [0,0]..."
                return [0,0]
        
def IntLen (Interval) :
        return float(abs(Interval[1]-Interval[0]))     
           
def RemoveLastObj() : 
        Config.ListObj = Config.ListObj[:-1]
        Config.Connections = Config.Connections[:-1]
        
def NormalizeVector(V):
        Magnitude = math.sqrt(GenFunctions.DotProd(V,V))
        return [ V[i]/Magnitude for i in range(len(V))]
        
def GetCriterion (ObjListIDs):
        return max(Config.Criterion, max(len(ObjListIDs[0]),len(ObjListIDs[1]))*max(len(ObjListIDs[2]),len(ObjListIDs[3])))

def SortObjLists (List,X0,Y0,DX,DY) :
        """ 
        This function sorts the list of neighbouring objects on each side, according to their intersection
        with the object being created. From South to North and from East to West
        """
        Output = List
        # First find the directions where no neighbour exists
        # Important : Here we assume that exactly two directions have no neighbours !!!
        #             Should we change this to allow a more general case ????
        dummy = IndexMultiOcc(List,(-1,))
        
        # dummy[0] is either 0, meaning there is no neighbour on X- (West)
        #                 or 1, meaning there is no neighbour on X+ (East)
        # Similarly dummy[1] can be either 2 or 3 (South and North respectively)
        # In order to get back to the formalism of groups (SNWE) 
        #  => we do the following to define Sense of no neighbours and then the Direction list
        #       is calculated as to include uniquely the directions where we DO have neighbours
        if len(dummy) == 1 :
                # This adds a second direction where neighbours are not regarded, it is either 0 or 2
                dummy.append(2*(dummy[0]+2<4))
                print("Careful, you have neighbours on 3 or more sides of the box, we will not check if on two parallel sides the boxes are compatible !!!")
        if len(dummy) == 2 or len(dummy) == 1 :
                # Sense contains : Vertical then Horizontal
                Sense = [dummy[1]%2,dummy[0]]
                DirList = [[1,0][dummy[0]],[3,2][dummy[1]%2]]
                for index,Direction in enumerate(DirList) :
                        ObjList = List[Direction]
                        RankMin = []
                        ToLook0 = [2,2,0,0][Direction]
                        ToLook1 = [3,2,1,0][Direction]
                        for index1,ObjID in enumerate(ObjList) : 
                                RankMin.append([-1.,1.][Sense[index]] * FindCommonSide(Config.ListObj[ObjID].DirBoundaries(ToLook1),[X0-DX/2.,X0+DX/2.,Y0-DY/2.,Y0+DY/2.][ToLook0:ToLook0+2])[Sense[index]])
                        Output[Direction] = SortList(ObjList,RankMin)
                        
        elif len(dummy) == 3 :
                # We find the direction where we do have neighbours and then we sort the object list along it
                Sense = dummy[0]%2
                Direction = [ i not in dummy for i in range(4) ].index(True)
                ObjList = List[Direction]
                RankMin = []
                ToLook0 = [2,2,0,0][Direction]
                ToLook1 = [3,2,1,0][Direction]
                for index1,ObjID in enumerate(ObjList) : 
                        RankMin.append([-1.,1.][Sense] * FindCommonSide(Config.ListObj[ObjID].DirBoundaries(ToLook1),[X0-DX/2.,X0+DX/2.,Y0-DY/2.,Y0+DY/2.][ToLook0:ToLook0+2])[Sense])
                Output[Direction] = SortList(ObjList,RankMin)
        else :
                print ("Error : the composite box being created has no neighbours, how on earth do you want us to inherit its mesh parameters!!!")
                
        
        return Output
        
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
                index = CritList.index(SortedCritList[i])
                Output.append(ValList[index])
        return Output

def ExtrapPoint (Ptref,Vref1,Vref2,Delta):
        """
        This function allows determining the absolute coordinates of an extrapolation point
        as shown in the following :
        
        
        ExtrapPoint x---Vref2->--------o
                   /       delta_glob  |Vref1 
                  /                    |
           Ptref x---------------------+
                        delta_loc * Nseg
                        
             Delta = (delta_loc - delta_glob) * Nseg    
        """
        
        X = Ptref[0] + Vref1[0] + Delta*Vref2[0]
        Y = Ptref[1] + Vref1[1] + Delta*Vref2[1]
        return (X,Y,)
        
