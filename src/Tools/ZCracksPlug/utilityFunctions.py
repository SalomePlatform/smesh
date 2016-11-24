#import sys
#sys.path.append('/home/I60976/00_PROJETS/2015_INTEGRATION_ZCRACKS/zcracks_salome/zcracks')

import numpy, subprocess, sys
from os import remove, getpid, path, environ
from output import message

def calcCoordVectors(normalIN, directionIN):
  V3TEMP=numpy.cross(normalIN,directionIN)
  directionTEMP=numpy.cross(V3TEMP,normalIN)

  normalOUT=numpy.array(normalIN)/numpy.linalg.norm(normalIN)
  directionOUT=numpy.array(directionTEMP)/numpy.linalg.norm(directionTEMP)
  V3OUT=numpy.array(V3TEMP)/numpy.linalg.norm(V3TEMP)
  return(normalOUT, directionOUT, V3OUT)


def testStrictRange(x, inf=0.0, sup=False):
  test=False
  c1=(type(x)==list)
  if c1:
    c2=(len(x)==1)
    if c2:
      c3=(type(x[0])==type(inf))
      if c3:
        c4=(x[0]>inf)
        c5=True
        if sup!=False:
          c5=(x[0]<sup)
        if c4 and c5:
          test=True
  return(test)

def test3dVector(x):
  test=False
  c1=(type(x)==list)
  if c1:
    c2=(len(x)==3)
    if c2:
      c3=(type(x[0])==float)
      c4=(type(x[1])==float)
      c5=(type(x[2])==float)
      if c3 and c4 and c5:
        test=True
  return(test)

def testRange(x, inf=0.0, sup=False):
  test=False
  c1=(type(x)==list)
  if c1:
    c2=(len(x)==1)
    if c2:
      c3=(type(x[0])==type(inf))
      if c3:
        c4=(x[0]>=inf)
        c5=True
        if sup!=False:
          c5=(x[0]<=sup)
        if c4 and c5:
          test=True
  return(test)

def check(data):
  OK=True

  test=False
  c1=(data['crackedName']!='')
  if c1:
    test=True
  if not test:
    message('E','Invalid Cracked name',goOn=True)
    OK=False

  test=False
  c1=path.isfile(data['saneName'])
  if c1:
    c2=(data['saneName']!=data['crackedName'])
    if c2:
      test=True
    else:
      message('E','sane mesh and cracked mesh are identical',goOn=True)
      OK=False
  if not test:
    message('E','Bad sane mesh file',goOn=True)
    OK=False

  test=testStrictRange(data['minSize'])
  if not test:
    message('E','invalid min size',goOn=True)
    OK=False

  test=testStrictRange(data['maxSize'])
  if not test:
    message('E','invalid max size',goOn=True)
    OK=False

  if OK:
    test=(data['maxSize'][0]>=data['minSize'][0])
    if not test:
      message('E','min size greater than max size',goOn=True)
      OK=False

  test=testStrictRange(data['extractLength'])
  if not test:
    message('E','invalid extract length',goOn=True)
    OK=False

  test=testRange(data['gradation'], inf=1.0)
  if not test:
    message('E','invalid Gradation',goOn=True)
    OK=False

  test=testRange(data['layers'], inf=1)
  if not test:
    message('E','invalid layers',goOn=True)
    OK=False

  test=testRange(data['iterations'], inf=1)
  if not test:
    message('E','invalid iterations',goOn=True)
    OK=False
  return(OK)


def calcElemSize(A, R):
  x=R*(1.-numpy.cos(A/2.))
  h=R*numpy.sin(A/2.)
  return(x, h)

def meshCrack(geomObject, minSize, maxSize, chordal, dim):
  import salome

  salome.salome_init()
  theStudy = salome.myStudy

  import  SMESH, SALOMEDS
  from salome.smesh import smeshBuilder
  smesh = smeshBuilder.New(theStudy)
  Maillage = smesh.Mesh(geomObject)

  if dim==3:
    MG_CADSurf = Maillage.Triangle(algo=smeshBuilder.MG_CADSurf)
    MG_CADSurf_Parameters = MG_CADSurf.Parameters()
    MG_CADSurf_Parameters.SetPhysicalMesh( 0 )
    MG_CADSurf_Parameters.SetGeometricMesh( 1 )
    MG_CADSurf_Parameters.SetMinSize( minSize )
    MG_CADSurf_Parameters.SetMaxSize( maxSize )
    MG_CADSurf_Parameters.SetChordalError( chordal )

  elif dim==2:
    Regular_1D = Maillage.Segment()
    Adaptive = Regular_1D.Adaptive(minSize,maxSize,chordal)
    NETGEN_2D_ONLY = Maillage.Triangle(algo=smeshBuilder.NETGEN_2D)
  else:
    message('E',"error in mesh dimension",goOn=True)

  isDone = Maillage.Compute()

  #crack1 = Maillage.CreateEmptyGroup( SMESH.NODE, 'crack' )
  #nbAdd = crack1.AddFrom( Maillage.GetMesh() )
  #crack2 = Maillage.CreateEmptyGroup( SMESH.NODE, 'surface' )
  #nbAdd = crack2.AddFrom( Maillage.GetMesh() )

  return(Maillage)

def extendElsets(meshFile, outFile=None):

  if outFile==None: outFile=meshFile

  if not path.isfile(meshFile):
    message('E','Mesh med file is not valid')
    return('error')

  import SMESH, salome
  #salome.salome_init()
  theStudy = salome.myStudy
  from salome.smesh import smeshBuilder
  smesh = smeshBuilder.New(theStudy)

  ([mesh], status) = smesh.CreateMeshesFromMED(meshFile)

  mesh=cleanGroups(mesh)

  # Node color status
  nodeList=mesh.GetNodesId()
  volElemList=mesh.GetElementsByType(SMESH.VOLUME)
  surfElemList=mesh.GetElementsByType(SMESH.FACE)
  edgeElemList=mesh.GetElementsByType(SMESH.EDGE)
  colorList=[-1]*len(nodeList)

  case2D=True
  for group in mesh.GetGroups():
    if group.GetType()==SMESH.VOLUME and group.GetName()[:5]=='sides' : case2D=False

  sides=[]
  for group in mesh.GetGroups():
    if case2D:
      if group.GetType()==SMESH.FACE and group.GetName()[:5]=='sides':
        sides.append(group)
    else:
      if group.GetType()==SMESH.VOLUME and group.GetName()[:5]=='sides':
        sides.append(group)

  sortedSides=[None]*len(sides)
  for group in sides:
    N=group.GetName().replace('sides','').replace('_bset','').replace(' ','')
    N=int(N)
    sortedSides[N]=group

    elems=group.GetIDs()
    for elemId in elems:
      for elemNodeId in mesh.GetElemNodes(elemId) :
        colorList[elemNodeId-1]=N
  #print colorList

  crackOnly=True
  for iN in range(len(sides)/2):
    side0=sortedSides[2*iN]
    side1=sortedSides[2*iN+1]
    elemsOfside0=side0.GetIDs()
    elemsOfside1=side1.GetIDs()
    NodesOfside0=[]
    NodesOfside1=[]
    for elem in elemsOfside0: NodesOfside0+=mesh.GetElemNodes(elem)
    for elem in elemsOfside1: NodesOfside1+=mesh.GetElemNodes(elem)
    front=set(NodesOfside0).intersection(set(NodesOfside1))
    if len(front)==0: crackOnly=False

  if crackOnly:
    mesh.ExportMED(outFile, 0, SMESH.MED_V2_2, 1, None ,1)
    return('crack')

  # Propagates color using elem connectivity
  # Always propagates max color

  #elemToTreat=volElemList

  #while len(elemToTreat)>0 :
    #print len(elemToTreat)
    #for elemId in elemToTreat:
      #minColor=sys.maxint
      #maxColor=-sys.maxint
      #for elemNodeId in mesh.GetElemNodes(elemId) :
        #nodeColor=colorList[elemNodeId-1]
        #if nodeColor<minColor : minColor=nodeColor
        #if nodeColor>maxColor : maxColor=nodeColor
      #if minColor!=maxColor :
        #elemToTreat.remove(elemId)
        #for elemNodeId in mesh.GetElemNodes(elemId) :
          #colorList[elemNodeId-1]=maxColor

  ifChanged=True
  if case2D:
    elemList=[surfElemList,edgeElemList]
    grElemList=[[],[]]
  else:
    elemList=[volElemList,surfElemList,edgeElemList]
    grElemList=[[],[],[]]

  while ifChanged :
    ifChanged=False
    for elemId in elemList[0]:
      minColor=sys.maxint
      maxColor=-sys.maxint
      for elemNodeId in mesh.GetElemNodes(elemId) :
        nodeColor=colorList[elemNodeId-1]
        if nodeColor<minColor : minColor=nodeColor
        if nodeColor>maxColor : maxColor=nodeColor
      if minColor!=maxColor :
        ifChanged = True
        for elemNodeId in mesh.GetElemNodes(elemId) :
          colorList[elemNodeId-1]=maxColor

  for l in grElemList:
    for x in range(len(sides)):
      l.append([])

  for N, el in enumerate(elemList):
    for elemId in el:
      elemNodesId=mesh.GetElemNodes(elemId)
      elemColor=colorList[elemNodesId[0]-1]
      if elemColor>=0:
        grElemList[N][elemColor].append(elemId)

  #for elemId in surfElemList:
    #elemNodesId=mesh.GetElemNodes(elemId)
    #elemColor=colorList[elemNodesId[0]-1]
    #if elemColor>=0:
      #selem[elemColor].append(elemId)

  for n in range(len(sides)):
    if case2D:
      mesh.MakeGroupByIds('Extended_side%d' %n ,SMESH.FACE,grElemList[0][n])
      mesh.MakeGroupByIds('Extended_side%d' %n ,SMESH.EDGE,grElemList[1][n])
    else:
      mesh.MakeGroupByIds('Extended_side%d' %n ,SMESH.VOLUME,grElemList[0][n])
      mesh.MakeGroupByIds('Extended_side%d' %n ,SMESH.FACE,grElemList[1][n])
      mesh.MakeGroupByIds('Extended_side%d' %n ,SMESH.EDGE,grElemList[2][n])

  if outFile==None: outFile=meshFile
  mesh.ExportMED(outFile, 0, SMESH.MED_V2_2, 1, None ,1)
  return(True)


def cleanGroups(mesh):
  import SMESH
  for group in mesh.GetGroups():
    if '_bset' in group.GetName():
      group.SetName(group.GetName().replace('_bset',''))

    if group.GetType()==SMESH.NODE:
      if group.GetName() in ['SURFACE','lip','SFRONT_NODES','FRONT']: mesh.RemoveGroup(group)

    #elif group.GetType()==SMESH.EDGE:

    elif group.GetType()==SMESH.FACE:
      if group.GetName() in ['SURFACE','Nlip']:
        mesh.RemoveGroup(group)

    elif group.GetType()==SMESH.VOLUME:
      if (group.GetName() in ['ELSET0','AUTO']) or (group.GetName()[:4] in ['SIDE']) :
        mesh.RemoveGroup(group)

  return(mesh)


def getMaxAspectRatio(tmpdir):
  logFile=path.join(tmpdir,'MESHING_OUTPUT')
  print logFile
  if not path.isfile(logFile): return(-1)

  import re
  f = open(logFile, "r")
  for line in f:
    if re.search("WORST ELEMENT QUALITY", line):  maxAR=line

  f.close()
  for r in [' ','WORSTELEMENTQUALITY','\n']: maxAR=maxAR.replace(r,'')
  return(float(maxAR))


#def extendElsets(meshFile):
  #if not path.isfile(meshFile):
    #message('E','Mesh med file is not valid')
    #return(-1)

  #import SMESH, salome
  ##salome.salome_init()
  #theStudy = salome.myStudy
  #from salome.smesh import smeshBuilder
  #smesh = smeshBuilder.New(theStudy)

  #([mesh], status) = smesh.CreateMeshesFromMED(meshFile)

  ## Node color status
  #nodeList=mesh.GetNodesId()
  #colorList=[0]*len(nodeList)

  ## Init using SIDE0 SIDE1
  #for group in mesh.GetGroups():
    #if group.GetType()==SMESH.FACE :
      #color=0
      #if group.GetName()[0:4]=='SIDE0' :
        #color=1
      #elif group.GetName()[0:4]=='SIDE1' :
        #color=2
      #else : continue
      ## Get faces
      #faces=group.GetIDs()
      ## Set faces nodes to given color
      #for face_id in faces :
        #for face_node_id in mesh.GetElemNodes(face_id) :
          #colorList[face_node_id-1]=color

  ## Propagates color using elem connectivity
  ## Always propagates max color
  #volElemList=mesh.GetElementsByType(SMESH.VOLUME)
  #ifChanged=True
  #while ifChanged :
    #ifChanged=False
    #minColor=100
    #maxColor=0
    #for elemId in volElemList:
      #for elemNodeId in mesh.GetElemNodes(elemId) :
        #nodeColor=colorList[elemNodeId-1]
        #if nodeColor<minColor : minColor=nodeColor
        #if nodeColor>maxColor : maxColor=nodeColor
      #if minColor!=maxColor :
        #ifChanged = True
        #for elemNodeId in mesh.GetElemNodes(elemId) :
          #colorList[elemNodeId-1]=maxColor

  #velem0 = []
  #velem1 = []
  #for elemId in volElemList:
    #elemNodesId=mesh.GetElemNodes(elemId)
    #elemColor=colorList[elemNodesId[0]-1]
    #if(elemColor==1) : velem0.append(elemId)
    #if(elemColor==2) : velem1.append(elemId)

  #mesh.MakeGroupByIds('SIDE_co',SMESH.VOLUME,velem0)
  #mesh.MakeGroupByIds('SIDE_ext',SMESH.VOLUME,velem1)

  #surfElemList=mesh.GetElementsByType(SMESH.FACE)
  #selem0 = []
  #selem1 = []
  #nbelem0=0
  #nbelem1=0

  #for elemId in surfElemList:
    #elemNodesId=mesh.GetElemNodes(elemId)
    #elemColor=colorList[elemNodesId[0]-1]
    #if(elemColor==1) : selem0.append(elemId)
    #if(elemColor==2) : selem1.append(elemId)

  #mesh.MakeGroupByIds('SIDE_co',SMESH.FACE,selem0)
  #mesh.MakeGroupByIds('SIDE_ext',SMESH.FACE,selem1)

  #maxAR=0.
  #for elem in volElemList:
    #maxAR=max(mesh.GetAspectRatio(elem),maxAR)
  #for elem in surfElemList:
    #maxAR=max(mesh.GetAspectRatio(elem),maxAR)

  #mesh.ExportMED(meshFile, 0, SMESH.MED_V2_2, 1, None ,1)
  #return(maxAR)


def removeFromSessionPath(envVar, patern):
  if type(patern) is not list: patern=[patern]
  if type(envVar) is not list: envVar=[envVar]

  for env in envVar:
    path=environ[env]
    listPath=path.split(':')
    for p in listPath:
      for pat in patern:
        if pat in p:
          path=path.replace(p,'')
          path.replace('::',':')
    environ[env]=path


#def isPlane(geomObject, eps=1.e-9):
  #import salome
  #salome.salome_init()
  #theStudy = salome.myStudy

  #import salome_notebook
  #notebook = salome_notebook.NoteBook(theStudy)

  #import GEOM
  #from salome.geom import geomBuilder
  #geompy = geomBuilder.New(theStudy)

  #Vs=geompy.SubShapeAll(geomObject, geompy.ShapeType["VERTEX"])
  #if len(Vs)<=3:
    #return(True)
  #elif len(Vs)>3:
    #P0=numpy.array(geompy.GetPosition(Vs[0])[:3])
    #P1=numpy.array(geompy.GetPosition(Vs[1])[:3])
    #P2=numpy.array(geompy.GetPosition(Vs[2])[:3])
    #V01=P1-P0
    #V02=P2-P0
    #V12=P2-P1
    #norm01=numpy.linalg.norm(V01)
    #norm02=numpy.linalg.norm(V02)
    #norm12=numpy.linalg.norm(V12)
    #if (norm01<eps) or (norm02<eps) or (norm12<eps):
      #print 'error'
      #return(False)
    #else:
      #N=numpy.cross(V01,V02)
      #N=N/numpy.linalg.norm(N)
      #maxDist=0.
      #for P in Vs[3:]:
        #Pi=numpy.array(geompy.GetPosition(P)[:3])
        #V=Pi-P0
        #d=numpy.dot(V,N)
        #maxDist=numpy.max([maxDist,numpy.abs(d)])
  #else:
    #print 'error'
    #return(False)

  #return(maxDist<eps)
