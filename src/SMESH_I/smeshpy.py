
import salome
import SMESH

from SALOME_utilities import *

#=============================================================================

class smeshpy:
    _geom = None
    _smesh = None
    _studyId = None

    #--------------------------------------------------------------------------

    def __init__(self):
        try:
            self._geom = salome.lcc.FindOrLoadComponent("FactoryServer","GEOM")
            self._smesh = salome.lcc.FindOrLoadComponent("FactoryServer","SMESH")
        except:
            MESSAGE( "exception in smeshpy:__init__" )
        self._studyId = salome.myStudyId

    #--------------------------------------------------------------------------

    def Init(self, shapeId):
        try:
            shape = salome.IDToObject(shapeId)
            aMesh = self._smesh.Init(self._geom, self._studyId, shape)
            return aMesh
        except:
            MESSAGE( "exception in smeshpy:Init" )
            return None

    #--------------------------------------------------------------------------

    def CreateHypothesis(self, name):
        try:
            hyp = self._smesh.CreateHypothesis(name,self._studyId)
            return hyp
        except:
            MESSAGE( "exception in smeshpy:CreateHypothesis" )
            return None    

    #--------------------------------------------------------------------------

    def Compute(self, mesh, shapeId):
        try:
            shape = salome.IDToObject(shapeId)
            ret=self._smesh.Compute(mesh, shape)
            return ret
        except:
            MESSAGE( "exception in smeshpy:Compute" )
            return 0    

#=============================================================================
##    #--------------------------------------------------------------------------

##def SmeshInit(shapeId):
##    import salome
##    import SMESH
##    geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
##    smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
##    shape = salome.IDToObject(shapeId)
##    studyId = salome.myStudyId
##    MESSAGE( str(studyId) )
##    aMesh = smesh.Init(geom, studyId, shape)
##    return aMesh

##    #--------------------------------------------------------------------------
