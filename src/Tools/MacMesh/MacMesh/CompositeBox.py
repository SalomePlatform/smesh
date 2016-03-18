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

def CompositeBox (X0 , Y0 , DX , DY , **args ) : 
                                     
        if args.__contains__('groups') :
                GroupNames = args['groups']
	else : GroupNames = [None, None, None, None]
        # Create a full Box just to inherit, globally, the mesh parameters of bounding objects
     	MacObject('CompBoxF',[(X0,Y0),(DX,DY)],['auto'],publish=0)
        
        # Save the existing number of segments on each direction
	ExistingSegments = Config.ListObj[-1].DirectionalMeshParams
        
        # Sort the connection list for the full Box
        ObjIDLists = SortObjLists(Config.Connections[-1],X0 , Y0 , DX , DY )
        RemoveLastObj()
        
        print "ObjIDLists: ", ObjIDLists
        
        RealSegments = []
        Direction = []
        flag = 0
        if not(args.__contains__('recursive')) : Config.Count = 0
        print "Config.Count : ", Config.Count
        Config.Criterion = GetCriterion(ObjIDLists)
        for index, ObjList in enumerate(ObjIDLists) :
                if not (ObjList[0] == -1 or Config.Count >= Config.Criterion):
                        if len(ObjList)>1 : flag = 1
                        else : flag = 0
                        for ObjID in ObjList:
                           ToLook0 = [2,2,0,0][index]
                           ToLook1 = [3,2,1,0][index]
                           CommonSide =  FindCommonSide(Config.ListObj[ObjID].DirBoundaries(ToLook1),[X0-DX/2.,X0+DX/2.,Y0-DY/2.,Y0+DY/2.][ToLook0:ToLook0+2])
                           ToLook2 = [1,0,3,2][index]
                           RealSegments.append(Config.ListObj[ObjID].DirectionalMeshParams[ToLook2]*IntLen(CommonSide)/IntLen(Config.ListObj[ObjID].DirBoundaries(ToLook1)))
                           Direction.append(ToLook0/2)
                           
                           if flag and Config.Count < Config.Criterion:
                                if index < 2 :
                                        if abs(CommonSide[0] - (Y0-DY/2.))<1e-7 : SouthGR = GroupNames[0]
                                        else : SouthGR = None
                                        if abs(CommonSide[1] - (Y0+DY/2.))<1e-7 : NorthGR = GroupNames[1]
                                        else : NorthGR = None
                                        CompositeBox (X0, CommonSide[0]+IntLen(CommonSide)/2., DX,IntLen(CommonSide), recursive=1, groups = [SouthGR,NorthGR]+GroupNames[2:4])
                                else : 
                                        if abs(CommonSide[0] - (X0-DX/2.))<1e-7 : EastGR = GroupNames[2]
                                        else : EastGR = None
                                        if abs(CommonSide[1] - (X0+DX/2.))<1e-7 : WestGR = GroupNames[3]
                                        else : WestGR = None
                                        CompositeBox (CommonSide[0]+IntLen(CommonSide)/2., Y0, IntLen(CommonSide),DY, recursive=1, groups = GroupNames[0:2]+[EastGR,WestGR])

                           if Config.Count >= Config.Criterion :
                                break
        if flag == 0 and Config.Count < Config.Criterion:
                #print "Dir : ", Direction
                #print "RealSegments : ", RealSegments
                
                #Xind = Direction.index(0)
                #Yind = Direction.index(1)
                #MacObject('CompBoxF',[(X0,Y0),(DX,DY)] ,[(RealSegments[Xind],RealSegments[Yind])], groups = GroupNames)
                MacObject('CompBoxF',[(X0,Y0),(DX,DY)] ,['auto'], groups = GroupNames)
                
                Config.Count += 1

                           
def FindCommonSide (Int1, Int2) :
        if abs(min(Int1[1],Int2[1])-max(Int1[0],Int2[0])) < 1e-5: return [0,0]
        else : return [max(Int1[0],Int2[0]), min(Int1[1],Int2[1])]
        
def IntLen (Interval) :
        return abs(Interval[1]-Interval[0])     
           
def RemoveLastObj() : 
        Config.ListObj = Config.ListObj[:-1]
        Config.Connections = Config.Connections[:-1]
        
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


            
