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



# This is an automation of the cylinder-box object, defined with the coordinates of its center, its radius, and the box's 
# boundary size.
# The pitch ratio is calculated automatically from the minimum of the box dimensions on x and y.
# This functions can take a groups input containing the group names of 4 sides in addition to the internal circular boundary
# in the following order : [South,North,West,East,Internal].

import sys, math, commands
CWD = commands.getoutput('pwd')
sys.path.append(CWD)


from MacObject import *
import Config, GenFunctions

def Cylinder (X0 , Y0 , D , DX , DY , LocalMeshing , **args) : 
	if args.__contains__('DLocal') : DLocal = float(args['DLocal']) 
	else : DLocal = float(min(DX,DY))

        # K is the pitch ratio
        K = float(D)/(DLocal-D)
        print "A local pitch ratio of K =", K ," will be used.  "
        NumCuts =  2*GenFunctions.QuarCylParam(K)
        InternalMeshing = int(math.ceil(math.pi*D/(4*NumCuts*LocalMeshing)))
	if InternalMeshing == 0 : InternalMeshing = 1		# This sets a minimum meshing condition in order to avoid an error. The user is notified of the value considered for the local meshing
        print "Possible Local meshing is :", math.pi*D/(4*NumCuts*InternalMeshing), "\nThis value is returned by this function for your convenience.\n"
        if args.__contains__('groups') :
                GroupNames = args['groups']
	else : GroupNames = [None, None, None, None, None]
        
        if DY == DLocal :
                if DX == DLocal:
                        GN1 = [None,GroupNames[1],None,GroupNames[3],GroupNames[4]]
                        GN2 = [None,GroupNames[1],GroupNames[2],None,GroupNames[4]]
                        GN3 = [GroupNames[0],None,GroupNames[2],None,GroupNames[4]]
                        GN4 = [GroupNames[0],None,None,GroupNames[3],GroupNames[4]]
                else :
                        GN1 = [None,GroupNames[1],None,None,GroupNames[4]]
                        GN2 = [None,GroupNames[1],None,None,GroupNames[4]]
                        GN3 = [GroupNames[0],None,None,None,GroupNames[4]]
                        GN4 = [GroupNames[0],None,None,None,GroupNames[4]]
                        
                        GN5 = [GroupNames[0],GroupNames[1],None,GroupNames[3]]
                        GN6 = [GroupNames[0],GroupNames[1],GroupNames[2],None]
        else :
                if DX == DLocal:
                        GN1 = [None,None,None,GroupNames[3],GroupNames[4]]
                        GN2 = [None,None,GroupNames[2],None,GroupNames[4]]
                        GN3 = [None,None,GroupNames[2],None,GroupNames[4]]
                        GN4 = [None,None,None,GroupNames[3],GroupNames[4]]
                        GN7 = [GroupNames[0],None,GroupNames[2],GroupNames[3]]
                        GN8 = [None,GroupNames[1],GroupNames[2],GroupNames[3]]
                else :
                        GN1 = [None,None,None,None,GroupNames[4]]
                        GN2 = [None,None,None,None,GroupNames[4]]
                        GN3 = [None,None,None,None,GroupNames[4]]
                        GN4 = [None,None,None,None,GroupNames[4]]
                        
                        GN5 = [None,None,None,GroupNames[3]]
                        GN6 = [None,None,GroupNames[2],None]
                        
                        GN9  = [GroupNames[0],None,None,GroupNames[3]]
                        GN10 = [GroupNames[0],None,None,None]
                        GN11 = [GroupNames[0],None,GroupNames[2],None]
                        
                        GN12 = [None,GroupNames[1],None,GroupNames[3]]
                        GN13 = [None,GroupNames[1],None,None]
                        GN14 = [None,GroupNames[1],GroupNames[2],None]
                        
	Obj = []

	Obj.append(MacObject('QuartCyl',[(X0+DLocal/4.,Y0+DLocal/4.),(DLocal/2.,DLocal/2.)],[InternalMeshing,'NE',K], groups = GN1))
	Obj.append(MacObject('QuartCyl',[(X0-DLocal/4.,Y0+DLocal/4.),(DLocal/2.,DLocal/2.)],['auto','NW',K], groups = GN2))
	Obj.append(MacObject('QuartCyl',[(X0-DLocal/4.,Y0-DLocal/4.),(DLocal/2.,DLocal/2.)],['auto','SW',K], groups = GN3))
	Obj.append(MacObject('QuartCyl',[(X0+DLocal/4.,Y0-DLocal/4.),(DLocal/2.,DLocal/2.)],['auto','SE',K], groups = GN4))

	if DX > DLocal :
		dX = (DX - DLocal)/2.
 		Obj.append(MacObject('CompBoxF',[(X0+DLocal/2.+dX/2.,Y0),(dX,DLocal)],['auto'], groups = GN5))
 		Obj.append(MacObject('CompBoxF',[(X0-DLocal/2.-dX/2.,Y0),(dX,DLocal)],['auto'], groups = GN6))

 	if DY > DLocal :
		dY = (DY - DLocal)/2.
		if DX > DLocal : 
 		        Obj.append(MacObject('CompBoxF',[(X0+DLocal/2.+dX/2.,Y0-DLocal/2.-dY/2.),(dX,dY)],['auto'], groups = GN9))
 		        Obj.append(MacObject('CompBoxF',[(X0,Y0-DLocal/2.-dY/2.),(DLocal,dY)],['auto'], groups = GN10))
 		        Obj.append(MacObject('CompBoxF',[(X0-DLocal/2.-dX/2.,Y0-DLocal/2.-dY/2.),(dX,dY)],['auto'], groups = GN11))
 		        Obj.append(MacObject('CompBoxF',[(X0+DLocal/2.+dX/2.,Y0+DLocal/2.+dY/2.),(dX,dY)],['auto'], groups = GN12))
 		        Obj.append(MacObject('CompBoxF',[(X0,Y0+DLocal/2.+dY/2.),(DLocal,dY)],['auto'], groups = GN13))
 		        Obj.append(MacObject('CompBoxF',[(X0-DLocal/2.-dX/2.,Y0+DLocal/2.+dY/2.),(dX,dY)],['auto'], groups = GN14))
                else:
 		        Obj.append(MacObject('CompBoxF',[(X0,Y0-DLocal/2.-dY/2.),(DLocal,dY)],['auto'], groups = GN7))
 		        Obj.append(MacObject('CompBoxF',[(X0,Y0+DLocal/2.+dY/2.),(DLocal,dY)],['auto'], groups = GN8))
                        		
	return Obj
