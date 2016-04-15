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



# This object allows unrefining from a central point (actually, a line) to the exterior
# X0 and Y0 are the center points of the origin point and not those of the center of the generated block



import sys, math, commands
CWD = commands.getoutput('pwd')
sys.path.append(CWD)

from MacObject import *
import Config, GenFunctions

def CentralUnrefine (X0 , Y0 , DX , DY , Orientation, **args ) : 

	DirPar =     {'SN' : lambda : ['NW', 'NE', 'EW', 'NW', 'SN', 'SN', 'NE', 'WE'],
	              'NS' : lambda : ['SE', 'SW', 'WE', 'SE', 'NS', 'NS', 'SW', 'EW'],
	              'EW' : lambda : ['NW', 'SW', 'SN', 'NW', 'EW', 'EW', 'SW', 'NS'],
	              'WE' : lambda : ['SE', 'NE', 'NS', 'SE', 'WE', 'WE', 'NE', 'SN'], }[Orientation]()

	CoefVer =   {'SN' : lambda :  1.,
	             'NS' : lambda : -1.,
	             'EW' : lambda :  0.,
	             'WE' : lambda :  0., }[Orientation]()

	CoefHor =   {'SN' : lambda :  0.,
	             'NS' : lambda :  0.,
	             'EW' : lambda : -1.,
	             'WE' : lambda :  1., }[Orientation]()


	MacObject('CompBoxF',[(X0+CoefHor*DX/2,Y0+CoefVer*DY/2),(DX,DY)],['auto'],publish=0)
	ToLook1 = {'SN' : lambda : 2,
		   'NS' : lambda : 3,
		   'EW' : lambda : 1,
		   'WE' : lambda : 0, }[Orientation]()

	ToLook2 = {'SN' : lambda : 0,
		   'NS' : lambda : 0,
		   'EW' : lambda : 2,
		   'WE' : lambda : 2, }[Orientation]()
                   
	ToLook3 = {'SN' : lambda : [0,1,2,3],
		   'NS' : lambda : [1,0,3,2],
		   'EW' : lambda : [3,2,1,0],
		   'WE' : lambda : [2,3,0,1], }[Orientation]() 
                                     
        if args.__contains__('groups') :
                GroupNames = args['groups']
	else : GroupNames = [None, None, None, None, None, None]
                
	ExistingSegments = Config.ListObj[-1].DirectionalMeshParams[ToLook1]
	ObjIDs = Config.Connections[-1][ToLook1]
        RemoveLastObj()
        
	ExtensionSegments = math.ceil(ExistingSegments/12.)*12.
	Dmin = 1.E50
	Dmax = -1.E50
	for ObjID in ObjIDs : 
		Boundaries = Config.ListObj[ObjID].Boundaries()
		if Boundaries[ToLook2] < Dmin : Dmin = Boundaries[ToLook2]
		if Boundaries[ToLook2+1] > Dmax : Dmax = Boundaries[ToLook2+1]
        dx = 0                    
	if ExtensionSegments > ExistingSegments :
		dn = (ExtensionSegments-ExistingSegments)/2.
		dx = dn*(Dmax-Dmin)/ExistingSegments
		#MacObject('CompBoxF',[(X0-CoefHor*dx/2+CoefVer*(-X0+Dmin-dx/2),Y0-CoefVer*dx/2+CoefHor*(-Y0+Dmin-dx/2)),(dx,dx)],[(dn,dn)],publish=0)
		#MacObject('CompBoxF',[(X0-CoefHor*dx/2+CoefVer*(-X0+Dmax+dx/2),Y0-CoefVer*dx/2+CoefHor*(-Y0+Dmax+dx/2)),(dx,dx)],[(dn,dn)],publish=0)
        
        BoxSide = (Dmax-Dmin+2*dx)/2.
        
        Obj = []
        Obj.append(MacObject('BoxAng32',[(X0+CoefHor*(BoxSide/2)+CoefVer*(-BoxSide/2),Y0+CoefVer*(BoxSide/2)+CoefHor*(-BoxSide/2)),(BoxSide,BoxSide)],[int(ExtensionSegments/6),DirPar[0]],groups=GroupArray(ToLook3[0],GroupNames[0])))       
        Obj.append(MacObject('BoxAng32',[(X0+CoefHor*(BoxSide/2)+CoefVer*(BoxSide/2),Y0+CoefVer*(BoxSide/2)+CoefHor*(BoxSide/2)),(BoxSide,BoxSide)],[int(ExtensionSegments/6),DirPar[1]],groups=GroupArray(ToLook3[0],GroupNames[0])))        

        NLevOpt = 0
        for NLevels in range (1,100) : 
                DX1 = abs(CoefVer)*BoxSide*2.**(NLevels+1)+abs(CoefHor)*BoxSide*2.**(NLevels)
                DY1 = abs(CoefHor)*BoxSide*2.**(NLevels+1)+abs(CoefVer)*BoxSide*2.**(NLevels)
                if DX1 > DX or DY1 > DY :
                        NLevOpt = NLevels-1
                        DXinner = DX1/2.
                        DYinner = DY1/2.
                        break
                        
        dummyArray = [DXinner,DYinner,DYinner,DXinner]                 
        D1inner = dummyArray[ToLook2]           # = DXinner for SN and NS orientations
        D2inner = dummyArray[ToLook2+1]         # = DYinner for SN and NS orientations
         
        dummyArray = [DX,DY,DY,DX]                 
        D1 = dummyArray[ToLook2]                # = DX for SN and NS orientations
        D2 = dummyArray[ToLook2+1]              # = DY for SN and NS orientations
                       
        if D1inner < D1 : 
                GN0a = GroupArray(ToLook3[0],GroupNames[1])
                GN0b = GroupArray(ToLook3[0],GroupNames[2])              
                GN01 = GroupArray(ToLook3[0],GroupNames[1])
                GN02 = GroupArray(ToLook3[0],GroupNames[2])
                if D2inner < D2 : 
                        GN10 = [None,None,None,None]
                        GN11 = [None,None,None,None]
                        GN20 = [None,None,None,None]
                else : 
                        GN10 = GroupArray(ToLook3[1],GroupNames[3])
                        GN11 = GroupArray(ToLook3[1],GroupNames[3])
                        GN20 = GroupArray(ToLook3[1],GroupNames[3]) 
        else : 
                GN0a = GroupArray(ToLook3[0],GroupNames[1])
                GN0b = GroupArray(ToLook3[0],GroupNames[2])
                GN01 = GroupArray([ToLook3[0],ToLook3[2]],[GroupNames[1],GroupNames[4]]) 
                GN02 = GroupArray([ToLook3[0],ToLook3[3]],[GroupNames[2],GroupNames[5]]) 
                if D2inner < D2 : 
                        GN10 = GroupArray(ToLook3[2],GroupNames[4])
                        GN11 = GroupArray(ToLook3[3],GroupNames[5])
                        GN20 = [None,None,None,None]
                else : 
                        GN10 = GroupArray([ToLook3[1],ToLook3[2]],[GroupNames[3],GroupNames[4]])
                        GN11 = GroupArray([ToLook3[1],ToLook3[3]],[GroupNames[3],GroupNames[5]])
                        GN20 = GroupArray(ToLook3[1],GroupNames[3]) 
                                              
        for N in range (1,NLevOpt+1):
		n=N-1
                D = BoxSide*(2.**n)
                if N < NLevOpt : 
		        Obj.append(MacObject('Box42'   ,[(X0+D*(CoefHor*1/2-CoefVer*3/2)  , Y0+D*(CoefVer*1/2-CoefHor*3/2) )          , (D,D)],['auto',DirPar[2]], groups=GN0a))
		        Obj.append(MacObject('BoxAng32',[(X0+D*(CoefHor*3/2-CoefVer*3/2)  , Y0+D*(CoefVer*3/2-CoefHor*3/2) )          , (D,D)],['auto',DirPar[3]]))
		        Obj.append(MacObject('Box42'   ,[(X0+D*(CoefHor*3/2-CoefVer*1/2)  , Y0+D*(CoefVer*3/2-CoefHor*1/2) )          , (D,D)],['auto',DirPar[4]]))
		        Obj.append(MacObject('Box42'   ,[(X0+D*(CoefHor*3/2+CoefVer*1/2)  , Y0+D*(CoefHor*1/2+CoefVer*3/2) )          , (D,D)],['auto',DirPar[5]]))
		        Obj.append(MacObject('BoxAng32',[(X0+D*(CoefVer*3/2+CoefHor*3/2)  , Y0+D*(CoefVer*3/2+CoefHor*3/2) )          , (D,D)],['auto',DirPar[6]]))
		        Obj.append(MacObject('Box42'   ,[(X0+D*(CoefVer*3/2+CoefHor*1/2)  , Y0+D*(CoefHor*3/2+CoefVer*1/2) )          , (D,D)],['auto',DirPar[7]], groups=GN0b))
                else :
		        Obj.append(MacObject('Box42'   ,[(X0+D*(CoefHor*1/2-CoefVer*3/2)  , Y0+D*(CoefVer*1/2-CoefHor*3/2) )          , (D,D)],['auto',DirPar[2]], groups=GN01))
		        Obj.append(MacObject('BoxAng32',[(X0+D*(CoefHor*3/2-CoefVer*3/2)  , Y0+D*(CoefVer*3/2-CoefHor*3/2) )          , (D,D)],['auto',DirPar[3]], groups=GN10))
		        Obj.append(MacObject('Box42'   ,[(X0+D*(CoefHor*3/2-CoefVer*1/2)  , Y0+D*(CoefVer*3/2-CoefHor*1/2) )          , (D,D)],['auto',DirPar[4]], groups=GN20))
		        Obj.append(MacObject('Box42'   ,[(X0+D*(CoefHor*3/2+CoefVer*1/2)  , Y0+D*(CoefHor*1/2+CoefVer*3/2) )          , (D,D)],['auto',DirPar[5]], groups=GN20))
		        Obj.append(MacObject('BoxAng32',[(X0+D*(CoefVer*3/2+CoefHor*3/2)  , Y0+D*(CoefVer*3/2+CoefHor*3/2) )          , (D,D)],['auto',DirPar[6]], groups=GN11))
		        Obj.append(MacObject('Box42'   ,[(X0+D*(CoefVer*3/2+CoefHor*1/2)  , Y0+D*(CoefHor*3/2+CoefVer*1/2) )          , (D,D)],['auto',DirPar[7]], groups=GN02))

        
	if CoefVer and DX>DXinner : 
                Obj.append(MacObject('CompBoxF',[(X0-CoefVer*0.25*(DX+DXinner),Y0+CoefVer*DYinner/2),((DX-DXinner)/2,DYinner)],['auto'], groups = GroupArray([ToLook3[0],ToLook3[2]],[GroupNames[1],GroupNames[4]])))
                Obj.append(MacObject('CompBoxF',[(X0+CoefVer*0.25*(DX+DXinner),Y0+CoefVer*DYinner/2),((DX-DXinner)/2,DYinner)],['auto'], groups = GroupArray([ToLook3[0],ToLook3[3]],[GroupNames[2],GroupNames[5]])))
                if DY>DYinner : 
                        Obj.append(MacObject('CompBoxF',[(X0-CoefVer*0.25*(DX+DXinner),Y0+CoefVer*(DY+DYinner)/2.),((DX-DXinner)/2,DY-DYinner)],['auto'], groups = GroupArray([ToLook3[1],ToLook3[2]],[GroupNames[3],GroupNames[4]])))
                        Obj.append(MacObject('CompBoxF',[(X0+CoefVer*0.25*(DX+DXinner),Y0+CoefVer*(DY+DYinner)/2.),((DX-DXinner)/2,DY-DYinner)],['auto'], groups = GroupArray([ToLook3[1],ToLook3[3]],[GroupNames[3],GroupNames[5]])))
                        Obj.append(MacObject('CompBoxF',[(X0,Y0+CoefVer*(DY+DYinner)/2.),(DXinner,DY-DYinner)],['auto'], groups = GroupArray(ToLook3[1],GroupNames[3])))
        elif CoefHor and DY>DYinner : 
                Obj.append(MacObject('CompBoxF',[(X0+CoefHor*DXinner/2,Y0-CoefHor*0.25*(DY+DYinner)),(DXinner,(DY-DYinner)/2)],['auto'], groups = GroupArray([ToLook3[0],ToLook3[2]],[GroupNames[1],GroupNames[4]])))
                Obj.append(MacObject('CompBoxF',[(X0+CoefHor*DXinner/2,Y0+CoefHor*0.25*(DY+DYinner)),(DXinner,(DY-DYinner)/2)],['auto'], groups = GroupArray([ToLook3[0],ToLook3[3]],[GroupNames[2],GroupNames[5]])))
                if DX>DXinner : 
                        Obj.append(MacObject('CompBoxF',[(X0+CoefHor*(DX+DXinner)/2.,Y0-CoefHor*0.25*(DY+DYinner)),(DX-DXinner,(DY-DYinner)/2)],['auto'], groups = GroupArray([ToLook3[1],ToLook3[2]],[GroupNames[3],GroupNames[4]])))
                        Obj.append(MacObject('CompBoxF',[(X0+CoefHor*(DX+DXinner)/2.,Y0+CoefHor*0.25*(DY+DYinner)),(DX-DXinner,(DY-DYinner)/2)],['auto'], groups = GroupArray([ToLook3[1],ToLook3[3]],[GroupNames[3],GroupNames[5]])))
                        Obj.append(MacObject('CompBoxF',[(X0+CoefHor*(DX+DXinner)/2.,Y0),(DX-DXinner,DYinner)],['auto'], groups = GroupArray(ToLook3[1],GroupNames[3])))
        return Obj

def RemoveLastObj() : 
        Config.ListObj = Config.ListObj[:-1]
        Config.Connections = Config.Connections[:-1]
 
def GroupArray(indices, GroupNames) :
        if type(indices) is int : 
                indices = [indices]
                GroupNames = [GroupNames]
        Output = [None,None,None,None]
        for i, ind in enumerate(indices) : 
                Output[ind] = GroupNames[i]
        return Output
