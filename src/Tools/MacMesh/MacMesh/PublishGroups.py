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

# 
import SMESH
import math
import Config

from salome.geom import geomBuilder
geompy = geomBuilder.New( Config.theStudy )

from salome.smesh import smeshBuilder
smesh = smeshBuilder.New( Config.theStudy )

##########################################################################################################

def PublishGroups ():
        aFilterManager = smesh.CreateFilterManager()

        # Building geometric and mesh compounds and  groups ##############################################
        if Config.debug : print "Searching for geometric groups and publishing final compound"
        
        TempGEOList = []
        TempMESHList = []
        
        for MacroObj in Config.ListObj : 
                TempGEOList += MacroObj.GeoChildren
                TempMESHList += MacroObj.Mesh
                
        FinalCompound = geompy.MakeCompound(TempGEOList)
        geompy.addToStudy (FinalCompound,Config.StudyName)
        MeshCompound = smesh.Concatenate(TempMESHList, 1, 1, 1e-5)
        MeshCompound.SetName(Config.StudyName)
        
        GroupGEO = []
        for group in Config.Groups :
        
                # Geometric groups definition
                TempGEOList = []
                TempNames = []
                for MacroObj in Config.ListObj :
                        if group in MacroObj.GroupNames :
                                Occurences = IndexMultiOcc(MacroObj.GroupNames, group)
                                for Occ in Occurences :
                                        TempGEOList += MacroObj.GetBorder(Occ)
                GroupGEO.append(geompy.MakeCompound(TempGEOList))
                geompy.addToStudyInFather(FinalCompound,GroupGEO[-1],'GR_'+group)
                
                # Mesh groups definition
                Criterion = smesh.GetCriterion(SMESH.EDGE, SMESH.FT_BelongToGeom,'=',GroupGEO[-1],Tolerance=1e-06)
                #Criterion = smesh.Filter.Criterion(18,39,0,'GR_'+group,'GR_'+group,39,39,1e-06,smesh.EDGE,7)
                MeshCompound.MakeGroupByCriterion(group,Criterion)
        
        StudyBuilder = Config.theStudy.NewBuilder()
        for MeshObj in TempMESHList:
                SO = Config.theStudy.FindObjectIOR(Config.theStudy.ConvertObjectToIOR(MeshObj))
                if SO is not None: StudyBuilder.RemoveObjectWithChildren(SO)
        
        return MeshCompound        
                

def IndexMultiOcc (Array,Element) :
        """
        This function returns the occurrences indices of Element in Array.
        As opposed to Array.index(Element) method, this allows determining      
        multiple entries rather than just the first one!
        """
        Output = []
        try : Array.index(Element)
        except ValueError : print "No more occurrences"
        else : Output.append(Array.index(Element))
                
        if not(Output == [-1]) and len(Array) > 1 :
                for index, ArrElem in enumerate(Array[Output[0]+1:]) :
                        if ArrElem is Element : Output.append(index+Output[0]+1)
                 
        return Output
            
def Publish (ObjToPublish):
	for i,GeoObj in enumerate(ObjToPublish) : geompy.addToStudy(GeoObj,"Sub_"+str(i))
        
def RevolveMesh(MainMesh,**args):
        """
        This function premits to revolute and scale a 2D mesh while transforming the edge
        groups into face groups. Moreover, the function automatically creates the face groups 
        corresponding to the symmetry lower and upper faces
        Facultatif arguments are : 
                - Center [X,Y,Z], origin being the default
                - Direction [VX,VY,VZ], x-axis being the default
                - AngleDeg or AngleRad : ALPHA, 10 degrees being the default
                - Scale : BETA, no scaling being default
        """
        ################################################################################
        # Reading input arguments and proceeding to defaults if necessary
        ################################################################################       
        if 'Center' in args : CenterCoor = [float(Coor) for Coor in args['Center']]
        else : 
                print "\nThe coordinates of the center of revolution were not given\nThe origin is used by default."
                CenterCoor = [0.,0.,0.]
        
        if 'Direction' in args : Direction = [float(Dir) for Dir in args['Direction']]
        else :
                print "\nThe axis vector of revolution was not given\nThe x-axis is used by default."
                Direction = [1.,0.,0.]
        
        if 'AngleDeg' in args : Angle = float(args['AngleDeg'])*math.pi/180.
        elif 'AngleRad' in args : Angle = float(args['AngleRad'])
        else :
                print "\nThe revolution angle was not given\nAn angle of 10 degrees is used by default."
                Angle = 10.*math.pi/180.
                
        if 'Scale' in args : Scale = float(args['Scale'])
        else : Scale = 1.
        

        # Creating the lower face group LOFAC
        LOFAC = MainMesh.CreateEmptyGroup( SMESH.FACE, 'LOFAC' )
        LOFAC.AddFrom(MainMesh.GetMesh())

        GR_Names = MainMesh.GetGroupNames()
        GRs = MainMesh.GetGroups()
        Rev3DMeshGroups = MainMesh.RotationSweepObject2D( MainMesh, SMESH.AxisStruct( CenterCoor[0], CenterCoor[1], CenterCoor[2], Direction[0], Direction[1], Direction[2] ), Angle, 1, 1e-05 ,True)

        # Adding an EDGE suffix to the edge groups (to be deleted eventually by the user...)
        for GR in GRs:
                CurrentName = GR.GetName()
                if CurrentName in GR_Names and not(CurrentName=='LOFAC'):  # Meaning that this is an old edge group
                        GR.SetName(CurrentName+'_EDGE')

        # Removing the _rotated prefix from the rotated FACE groups
        for GR in Rev3DMeshGroups:
                CurrentName = GR.GetName()
                if CurrentName.endswith( "_rotated"):
                        if CurrentName.startswith( 'LOFAC_' ):
                                GR.SetName('VOL')
                        else:
                                GR.SetName(CurrentName[:-8])
                elif CurrentName == 'LOFAC_top':
                        GR.SetName('HIFAC')
                        #Index = [ GR_Names[i] in CurrentName for i in range(0,len(GR_Names)) ].index(True)
                        #GR.SetName(GR_Names[Index])

        # Creating the upper face group HIFAC
        ALLFAC = MainMesh.CreateEmptyGroup( SMESH.FACE, 'ALLFAC' )
        ALLFAC.AddFrom(MainMesh.GetMesh())

        #HIFAC = MainMesh.GetMesh().CutListOfGroups( [ ALLFAC ], [LOFAC] + [ MeshGroup for MeshGroup in Rev3DMeshGroups if not(MeshGroup.GetName()=='VOL') ], 'HIFAC' )
        #HIFAC = MainMesh.GetMesh().CutListOfGroups( [ ALLFAC ], [LOFAC] + [ MeshGroup for MeshGroup in Rev3DMeshGroups if ( not(MeshGroup.GetName()=='VOL') and MeshGroup.GetType() == SMESH.FACE )], 'HIFAC' )

        # Scaling down the mesh to meter units
        if not(Scale==1.):
                MeshEditor = MainMesh.GetMeshEditor()
                MeshEditor.Scale( MainMesh.GetMesh(), SMESH.PointStruct( 0, 0, 0 ) ,[ Scale, Scale, Scale ], 0 )
     
                
def ExtrudeMesh(MainMesh,**args):
        """
        This function premits to extrude and scale a 2D mesh while transforming the edge
        groups into face groups. Moreover, the function automatically creates the face groups 
        corresponding to the symmetry lower and upper faces
        Facultatif arguments are : 
                - Direction [VX,VY,VZ], z-axis being default
                - Distance : D, default is 1
                - NSteps : the object will be extruded by NSteps*Distance, default is Nsteps = 1
                - Scale : BETA, no scaling being default
        """
        ################################################################################
        # Reading input arguments and proceeding to defaults if necessary
        ################################################################################              
        if 'Distance' in args : Distance = float(args['Distance'])
        else :
                print "\nThe extrusion distance was not given\nA default value of 1 is used."
                Distance = 1.
                
        if 'Direction' in args : Direction = NormalizeVector([float(Dir) for Dir in args['Direction']],Distance)
        else :
                print "\nThe extrusion vector of revolution was not given\nThe z-axis is used by default."
                Direction = NormalizeVector([0.,0.,1.],Distance)
                                
        if 'Scale' in args : Scale = float(args['Scale'])
        else : Scale = 1.
        
        if 'NSteps' in args : NSteps = int(args['NSteps'])
        else : NSteps = 1
        
        # Creating the lower face group LOFAC
        LOFAC = MainMesh.CreateEmptyGroup( SMESH.FACE, 'LOFAC' )
        LOFAC.AddFrom(MainMesh.GetMesh())

        GR_Names = MainMesh.GetGroupNames()
        GRs = MainMesh.GetGroups()
        Ext3DMeshGroups = MainMesh.ExtrusionSweepObject2D(MainMesh,SMESH.DirStruct(SMESH.PointStruct(Direction[0],Direction[1],Direction[2])), NSteps, True)

        # Adding an EDGE suffix to the edge groups (to be deleted eventually by the user...)
        for GR in GRs:
                CurrentName = GR.GetName()
                if CurrentName in GR_Names and not(CurrentName=='LOFAC'):  # Meaning that this is an old edge group
                        GR.SetName(CurrentName+'_EDGE')

        # Removing the _extruded suffix from the extruded FACE groups
        for GR in Ext3DMeshGroups:
                CurrentName = GR.GetName()
                if CurrentName.endswith( "_extruded"):
                        if CurrentName.startswith( 'LOFAC_' ):
                                GR.SetName('VOL')
                        else:
                                GR.SetName(CurrentName[:-9])
                elif CurrentName == 'LOFAC_top':
                        GR.SetName('HIFAC')

        # Creating the upper face group HIFAC
        ALLFAC = MainMesh.CreateEmptyGroup( SMESH.FACE, 'ALLFAC' )
        ALLFAC.AddFrom(MainMesh.GetMesh())

        #HIFAC = MainMesh.GetMesh().CutListOfGroups( [ ALLFAC ], [LOFAC] + [ MeshGroup for MeshGroup in Ext3DMeshGroups if not(MeshGroup.GetName()=='VOL') ], 'HIFAC' )

        # Scaling down the mesh to meter units
        if not(Scale==1.):
                MeshEditor = MainMesh.GetMeshEditor()
                MeshEditor.Scale( MainMesh.GetMesh(), SMESH.PointStruct( 0, 0, 0 ) ,[ Scale, Scale, Scale ], 0 )
     
               
def NormalizeVector (V,Norm):
        """
        This function returns a normalized vector (magnitude = Norm), parallel to the entered one
        """
        V = [float(Coor) for Coor in V]
        Norm = float(Norm)
        MagV = math.sqrt(V[0]*V[0]+V[1]*V[1]+V[2]*V[2])
        return [Coor*Norm/MagV for Coor in V]
  
