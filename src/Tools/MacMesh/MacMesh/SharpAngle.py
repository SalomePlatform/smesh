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



# This is an automation of the sharp angle object, with a corner at (X0,Y0), side length : Extension and a fine local meshing : LocalMeshing
# The corner orientation is defined as NE (North-East) , NW (North-West), SE, or SW. The object's "arm" is 8/14 of Extension
#	     	    	|      |     8      6
#	   	 -------	---------
#	---->	|			 |   <----
#	   	|   NW		   NE	 |  oo
#	   _____|			 |_____

import sys, math, commands
CWD = commands.getoutput('pwd')
sys.path.append(CWD)

from MacObject import *
from CompositeBox import *
import Config, GenFunctions

def SharpAngleOut (X0 , Y0 , DX , DY , DLocal, LocalMeshing , CornerOrientation , NLevels, **args) : 
	if DLocal == 'auto' : DLocal = float(min(DX,DY))

	BoxSide = DLocal/(2.**(NLevels+1))
	InternalMeshing = int(math.ceil(BoxSide/(3*LocalMeshing)))
	InternalMeshing = InternalMeshing+InternalMeshing%2	# An even number is needed, otherwise the objects would not be compatible once created
	if InternalMeshing == 0 : InternalMeshing = 2		# This sets a minimum meshing condition in order to avoid an error. The user is notified of the value considered for the local meshing
	print "Possible Local meshing is :", BoxSide/(3*InternalMeshing), "\nThis value is returned by this function for your convenience"

	DirPar =     {'NE' : lambda : ['NE', 'NW', 'SE', 'EW', 'NW', 'SN', 'SN', 'NE', 'WE', 'WE', 'SE', 'NS'],
	              'NW' : lambda : ['NW', 'NE', 'SW', 'WE', 'NE', 'SN', 'SN', 'NW', 'EW', 'EW', 'SW', 'NS'],
	              'SE' : lambda : ['SE', 'SW', 'NE', 'EW', 'SW', 'NS', 'NS', 'SE', 'WE', 'WE', 'NE', 'SN'],
	              'SW' : lambda : ['SW', 'SE', 'NW', 'WE', 'SE', 'NS', 'NS', 'SW', 'EW', 'EW', 'NW', 'SN'], }[CornerOrientation]()

	CoefVer =   {'NE' : lambda :  1,
	             'NW' : lambda :  1,
	             'SE' : lambda : -1,
	             'SW' : lambda : -1, }[CornerOrientation]()

	CoefHor =   {'NE' : lambda :  1,
	             'NW' : lambda : -1,
	             'SE' : lambda :  1,
	             'SW' : lambda : -1, }[CornerOrientation]()
                     
	ToLook  = {'NE' : lambda : [0,2,1,3],
		   'NW' : lambda : [0,3,1,2],
		   'SE' : lambda : [1,2,0,3],
		   'SW' : lambda : [1,3,0,2], }[CornerOrientation]()        
       
        if args.__contains__('groups') :
                GroupNames = args['groups']
	else : GroupNames = [None, None, None, None, None, None]
        
        GN00 = GroupArray(ToLook[0],GroupNames[0])
        GN01 = GroupArray(ToLook[1],GroupNames[1])
        
        GN1 = GroupArray([ToLook[0],ToLook[1]],[GroupNames[0],GroupNames[5]])
        GN7 = GroupArray([ToLook[0],ToLook[1]],[GroupNames[4],GroupNames[1]])
        
        if DY == DLocal :
                GN2 = GroupArray([ToLook[1],ToLook[2]],[GroupNames[5],GroupNames[2]])
                GN3 = GroupArray(ToLook[2],GroupNames[2])
                if DX == DLocal:
                        GN4 = GroupArray([ToLook[2],ToLook[3]],[GroupNames[2],GroupNames[3]])
                        GN5 = GroupArray(ToLook[3],GroupNames[3])
                        GN6 = GroupArray([ToLook[3],ToLook[0]],[GroupNames[3],GroupNames[4]])
                else :
                        GN4 = GroupArray(ToLook[2],GroupNames[2])
                        GN5 = [None,None,None,None]
                        GN6 = GroupArray(ToLook[0],GroupNames[4])
                        GN21 = GroupArray([ToLook[3],ToLook[0],ToLook[2]],[GroupNames[3],GroupNames[4],GroupNames[2]])
        else :
                GN2 = GroupArray(ToLook[1],GroupNames[5])
                GN3 = [None,None,None,None]
                if DX == DLocal:
                        GN4 = GroupArray(ToLook[3],GroupNames[3])
                        GN5 = GroupArray(ToLook[3],GroupNames[3])
                        GN6 = GroupArray([ToLook[3],ToLook[0]],[GroupNames[3],GroupNames[4]])
                        GN22 = GroupArray([ToLook[1],ToLook[2],ToLook[3]],[GroupNames[5],GroupNames[2],GroupNames[3]])
                else :
                        GN4 = [None,None,None,None]
                        GN5 = [None,None,None,None]
                        GN6 = GroupArray(ToLook[0],GroupNames[4])
                        GN21 = GroupArray([ToLook[3],ToLook[0]],[GroupNames[3],GroupNames[4]])
                        GN22 = GroupArray([ToLook[1],ToLook[2]],[GroupNames[5],GroupNames[2]])
                        GN23 = GroupArray([ToLook[2],ToLook[3]],[GroupNames[2],GroupNames[3]])
                        
	Obj = []

	Obj.append(MacObject('BoxAng32',[(X0+CoefHor*BoxSide/2,Y0+CoefVer*BoxSide/2),(BoxSide,BoxSide)],[InternalMeshing,DirPar[0]]))
	Obj.append(MacObject('BoxAng32',[(X0-CoefHor*BoxSide/2,Y0+CoefVer*BoxSide/2),(BoxSide,BoxSide)],['auto',DirPar[1]], groups = GroupArray(ToLook[0],GroupNames[0])))
	Obj.append(MacObject('BoxAng32',[(X0+CoefHor*BoxSide/2,Y0-CoefVer*BoxSide/2),(BoxSide,BoxSide)],['auto',DirPar[2]], groups = GroupArray(ToLook[1],GroupNames[1])))
	
	for N in range (1,NLevels+1):
		n = N-1
                if N < NLevels : 
 	               Obj.append(MacObject('Box42',[(X0-CoefHor*BoxSide*(2**n)*3/2,Y0+CoefVer*(2**n)*BoxSide/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[3]]      , groups = GN00))
	               Obj.append(MacObject('BoxAng32',[(X0-CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[4]]               ))
	               Obj.append(MacObject('Box42',[(X0-CoefHor*(2**n)*BoxSide/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[5]]                    ))
	               Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[6]]                    ))
	               Obj.append(MacObject('BoxAng32',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[7]]               ))
	               Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[8]]                    ))
	               Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0-CoefVer*(2**n)*BoxSide/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[9]]                    ))
	               Obj.append(MacObject('BoxAng32',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0-CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[10]]              ))
	               Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide/2,Y0-CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[11]]     , groups = GN01))
                else :
 	               Obj.append(MacObject('Box42',[(X0-CoefHor*BoxSide*(2**n)*3/2,Y0+CoefVer*(2**n)*BoxSide/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[3]]      , groups = GN1))
	               Obj.append(MacObject('BoxAng32',[(X0-CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[4]] , groups = GN2))
	               Obj.append(MacObject('Box42',[(X0-CoefHor*(2**n)*BoxSide/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[5]]      , groups = GN3))
	               Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[6]]      , groups = GN3))
	               Obj.append(MacObject('BoxAng32',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[7]] , groups = GN4))
	               Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[8]]      , groups = GN5))
	               Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0-CoefVer*(2**n)*BoxSide/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[9]]      , groups = GN5))
	               Obj.append(MacObject('BoxAng32',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0-CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[10]], groups = GN6))
	               Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide/2,Y0-CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[11]]     , groups = GN7))
                       
	OuterMeshing = (3/2)*InternalMeshing*2**(NLevels-1)
	OuterSegLength = (DLocal/OuterMeshing)

	if DX > DLocal :
		dX = DX - DLocal
 		Obj.append(MacObject('CompBoxF',[(X0+CoefHor*(DX)/2.,Y0),(dX,DLocal)],['auto'], groups = GN21))
 	if DY > DLocal :
		dY = DY - DLocal
		if DX > DLocal : 
			Obj.append(MacObject('CompBoxF',[(X0+CoefHor*DX/2.,Y0+CoefVer*(DY)/2.),(DX-DLocal,dY)],['auto'], groups = GN23))

		Obj.append(MacObject('CompBoxF',[(X0,Y0+CoefVer*(DY)/2.),(DLocal,dY)],['auto'], groups = GN22))
		
	return Obj

def SharpAngleIn (X0 , Y0 , DX , DY , DLocal, LocalMeshing , CornerOrientation , NLevels, **args) : 
	if DLocal == 'auto' : DLocal = float(min(DX,DY))

	BoxSide = DLocal/(2.**(NLevels))
	InternalMeshing = int(math.ceil(BoxSide/(3*LocalMeshing)))
	InternalMeshing = InternalMeshing+InternalMeshing%2	# An even number is needed, otherwise the objects would not be compatible once created
	if InternalMeshing == 0 : InternalMeshing = 2		# This sets a minimum meshing condition in order to avoid an error. The user is notified of the value considered for the local meshing
	print "Possible Local meshing is :", BoxSide/(3*InternalMeshing), "\nThis value is returned by this function for your convenience..."

	DirPar =     {'NE' : lambda : ['NE', 'SN', 'NE', 'WE'],
	              'NW' : lambda : ['NW', 'SN', 'NW', 'EW'],
	              'SE' : lambda : ['SE', 'NS', 'SE', 'WE'],
	              'SW' : lambda : ['SW', 'NS', 'SW', 'EW'], }[CornerOrientation]()

	CoefVer =   {'NE' : lambda :  1,
	             'NW' : lambda :  1,
	             'SE' : lambda : -1,
	             'SW' : lambda : -1, }[CornerOrientation]()

	CoefHor =   {'NE' : lambda :  1,
	             'NW' : lambda : -1,
	             'SE' : lambda :  1,
	             'SW' : lambda : -1, }[CornerOrientation]()
                     
	ToLook  = {'NE' : lambda : [0,2,1,3],
		   'NW' : lambda : [0,3,1,2],
		   'SE' : lambda : [1,2,0,3],
		   'SW' : lambda : [1,3,0,2], }[CornerOrientation]()
                                               
        if args.__contains__('groups') :
                GroupNames = args['groups']
	else : GroupNames = [None, None, None, None]
        
        GN01 = GroupArray([ToLook[0],ToLook[1]],[GroupNames[ToLook[0]],GroupNames[ToLook[1]]])
        GN02 = GroupArray(ToLook[1],GroupNames[ToLook[1]])        
        GN03 = [None, None, None, None]
        GN04 = GroupArray(ToLook[0],GroupNames[ToLook[0]])  

        if DY == DLocal :
                GN05 = GroupArray([ToLook[1],ToLook[2]],[GroupNames[ToLook[1]],GroupNames[ToLook[2]]])
                GN08 = GroupArray([ToLook[0],ToLook[2],ToLook[3]],[GroupNames[ToLook[0]],GroupNames[ToLook[2]],GroupNames[ToLook[3]]])
                if DX == DLocal:
                        GN06 = GroupArray([ToLook[2],ToLook[3]],[GroupNames[ToLook[2]],GroupNames[ToLook[3]]])
                        GN07 = GroupArray([ToLook[0],ToLook[3]],[GroupNames[ToLook[0]],GroupNames[ToLook[3]]])
                else :
                        GN06 = GroupArray(ToLook[2],GroupNames[ToLook[2]])
                        GN07 = GroupArray(ToLook[0],GroupNames[ToLook[0]])
        else :
                GN05 = GroupArray(ToLook[1],GroupNames[ToLook[1]])
                if DX == DLocal : 
                        GN06 = GroupArray(ToLook[3],GroupNames[ToLook[3]])
                        GN07 = GroupArray([ToLook[0],ToLook[3]],[GroupNames[ToLook[0]],GroupNames[ToLook[3]]])
                        GN10 = GroupArray([ToLook[1],ToLook[2],ToLook[3]],[GroupNames[ToLook[1]],GroupNames[ToLook[2]],GroupNames[ToLook[3]]])
                else :
                        GN06 = [None, None, None, None]
                        GN07 = GroupArray(ToLook[0],GroupNames[ToLook[0]])
                        GN08 = GroupArray([ToLook[0],ToLook[3]],[GroupNames[ToLook[0]],GroupNames[ToLook[3]]])
                        GN09 = GroupArray([ToLook[2],ToLook[3]],[GroupNames[ToLook[2]],GroupNames[ToLook[3]]])
                        GN10 = GroupArray([ToLook[1],ToLook[2]],[GroupNames[ToLook[1]],GroupNames[ToLook[2]]])
         
	Obj = []

	Obj.append(MacObject('BoxAng32',[(X0+CoefHor*BoxSide/2,Y0+CoefVer*BoxSide/2),(BoxSide,BoxSide)],[InternalMeshing,DirPar[0]],groups = GN01))
	
	for N in range (1,NLevels+1):
		n = N-1
                if N < NLevels : 
		        Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[1]],groups = GN02))
		        Obj.append(MacObject('BoxAng32',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[2]],groups = GN03))
		        Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[3]],groups = GN04))
                else :
		        Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[1]],groups = GN05))
		        Obj.append(MacObject('BoxAng32',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide*3/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[2]],groups = GN06))
		        Obj.append(MacObject('Box42',[(X0+CoefHor*(2**n)*BoxSide*3/2,Y0+CoefVer*(2**n)*BoxSide/2),((2**n)*BoxSide,(2**n)*BoxSide)],['auto',DirPar[3]],groups = GN07))
 
 	OuterMeshing = (3/2)*InternalMeshing*2**(NLevels-1)
	OuterSegLength = (DLocal/OuterMeshing)

	if DX > DLocal :
		dX = DX - DLocal
 		Obj = Obj + CompositeBox(X0+CoefHor*(DLocal+dX/2.),Y0+CoefVer*(DLocal)/2.,dX,DLocal, groups = GN08)
 	if DY > DLocal :
		dY = DY - DLocal

		if DX > DLocal : 
			Obj = Obj + CompositeBox(X0+CoefHor*(DLocal+(DX-DLocal)/2.),Y0+CoefVer*(DLocal+dY/2.),DX-DLocal,dY, groups = GN09)

		Obj = Obj + CompositeBox(X0+CoefHor*DLocal/2,Y0+CoefVer*(DLocal+dY/2.),DLocal,dY,groups = GN10)

	return Obj

def GroupArray(indices, GroupNames) :
        if type(indices) is int : 
                indices = [indices]
                GroupNames = [GroupNames]
        Output = [None,None,None,None]
        for i, ind in enumerate(indices) : 
                Output[ind] = GroupNames[i]
        return Output
