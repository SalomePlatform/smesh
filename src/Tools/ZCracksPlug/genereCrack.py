import os, shutil
import sphere, ellipse, rectangle
import utilityFunctions as uF
from output import message

def main(data, outFile):
  activeCrack=data['crack']['actif']
  crack=data['crack'][activeCrack]

  res=False

  if activeCrack == 'Ellipse':
    res=generateEllipse(crack, outFile)

  elif activeCrack == 'Rectangle':
    res=generateRectangle(crack, outFile)

  elif activeCrack == 'Sphere':
    res=generateSphere(crack, outFile)

  elif activeCrack == 'Custom':
    res=generateCustom(crack, outFile)

  return(res)


def generateEllipse(crack, outFile):
  res=True
  test=uF.testStrictRange(crack['Rayon'])
  if not test:
    message('E','Bad Rayon',goOn=True)
    res=False
  demiGrandAxe=crack['Rayon'][0]

  if 'Rayon 2' not in crack.keys(): crack['Rayon 2']=[]
  if len(crack['Rayon 2'])==0:
    demiPetitAxe=demiGrandAxe
  else:
    test=uF.testStrictRange(crack['Rayon 2'])
    if not test:
      message('E','Bad Rayon 2',goOn=True)
      res=False
    demiPetitAxe=crack['Rayon 2'][0]

  test=uF.test3dVector(crack['Centre'])
  if not test:
    message('E','Invalid Centre',goOn=True)
    res=False
  centre=crack['Centre']

  test=uF.test3dVector(crack['Normale'])
  if not test:
    message('E','Invalid Normale',goOn=True)
    res=False
  normale=crack['Normale']

  if 'Direction' not in crack.keys(): crack['Direction']=[]
  if len(crack['Direction'])==0:
    if normale==[1.,0.,0.]:
      direction=[0.,1.,0.]
    else:
      direction=[1.,0.,0.]
  else:
    test=uF.test3dVector(crack['Direction'])
    if not test:
      message('E','Invalid Direction',goOn=True)
      res=False
    direction=crack['Direction']
    test=(direction!=normale)
    if not test:
      message('E','Normale and Direction are equals',goOn=True)
      res=False

  if 'Angle' not in crack.keys(): crack['Angle']=[]
  if len(crack['Angle'])==0:
    angle=0.0
  else:
    test=uF.testRange(crack['Angle'], inf=0.0, sup=360.)
    if not test:
      message('E','Angle not valid or out of range 0 to 360',goOn=True)
      res=False
    angle=crack['Angle'][0]

  if 'Rayon entaille' not in crack.keys(): crack['Rayon entaille']=[]
  if len(crack['Rayon entaille'])==0:
    rayon_entaille=0.0
  else:
    test=uF.testStrictRange(crack['Rayon entaille'], inf=0.0)
    if not test:
      message('E','rayon entaille not valid or negative',goOn=True)
      res=False
    rayon_entaille=crack['Rayon entaille'][0]

  if 'Extension' not in crack.keys(): crack['Extension']=[]
  if len(crack['Extension'])==0:
    extension=0.0
  else:
    test=uF.testStrictRange(crack['Extension'], inf=0.0)
    if not test:
      message('E','extension not valid or negative',goOn=True)
      res=False
    extension=crack['Extension'][0]

  if res:
    ellipse.generate(demiGrandAxe, centre, normale,
                      direction, demiPetitAxe, angle,
                      rayon_entaille, extension, outFile)
    return(True)
  else:
    return(False)

def generateRectangle(crack, outFile):
  res=True
  test=uF.testStrictRange(crack['Longueur'])
  if not test:
    message('E','Bad Longueur',goOn=True)
    res=False
  longueur=crack['Longueur'][0]

  if 'Largeur' not in crack.keys(): crack['Largeur']=[]
  if len(crack['Largeur'])==0:
    largeur=longueur
  else:
    test=uF.testStrictRange(crack['Largeur'])
    if not test:
      message('E','Bad Largeur',goOn=True)
      res=False
    largeur=crack['Largeur'][0]

  test=uF.test3dVector(crack['Centre'])
  if not test:
    message('E','Invalid Centre',goOn=True)
    res=False
  centre=crack['Centre']

  test=uF.test3dVector(crack['Normale'])
  if not test:
    message('E','Invalid Normale',goOn=True)
    res=False
  normale=crack['Normale']

  test=uF.test3dVector(crack['Direction'])
  if not test:
    message('E','Invalid Direction',goOn=True)
    res=False
  direction=crack['Direction']

  if 'Angle' not in crack.keys(): crack['Angle']=[]
  if len(crack['Angle'])==0:
    angle=0.0
  else:
    test=uF.testRange(crack['Angle'], inf=0.0, sup=360.)
    if not test:
      message('E','Angle not valid or out of range 0 to 360',goOn=True)
      res=False
    angle=crack['Angle'][0]

  if 'Rayon' not in crack.keys(): crack['Rayon']=[]
  if len(crack['Rayon'])==0:
    rayon=0.0
  else:
    test=uF.testRange(crack['Rayon'], inf=0.0)
    if not test:
      message('E','Rayon not valid',goOn=True)
      res=False
    rayon=crack['Rayon'][0]

  if 'Rayon entaille' not in crack.keys(): crack['Rayon entaille']=[]
  if len(crack['Rayon entaille'])==0:
    rayon_entaille=0.0
  else:
    test=uF.testStrictRange(crack['Rayon entaille'], inf=0.0)
    if not test:
      message('E','rayon entaille not valid or negative',goOn=True)
      res=False
    rayon_entaille=crack['Rayon entaille'][0]

  if res:
    rectangle.generate(longueur,largeur,centre,
                    normale,direction,angle,
                    rayon,rayon_entaille,outFile)
    return(True)
  else:
    return(False)

def generateSphere(crack, outFile):
  res=True
  test=uF.testStrictRange(crack['Rayon'])
  if not test:
    message('E','Bad Rayon',goOn=True)
    res=False
  rayon=crack['Rayon'][0]

  test=uF.test3dVector(crack['Centre'])
  if not test:
    message('E','Invalid Centre',goOn=True)
    res=False
  centre=crack['Centre']

  if res:
    sphere.generate(rayon,centre,outFile)
    return(True)
  else:
    return(False)

def generateCustom(crack, outFile):
  res=True
  test=os.path.isfile(crack['med file'])
  if not test:
    message('E','crack med file missing',goOn=True)
    res=False

  import salome
  salome.salome_init()
  theStudy = salome.myStudy
  import salome_notebook
  notebook = salome_notebook.NoteBook(theStudy)
  import  SMESH, SALOMEDS
  from salome.smesh import smeshBuilder

  smesh = smeshBuilder.New(theStudy)
  ([Maillage_1], status) = smesh.CreateMeshesFromMED(crack['med file'])
  isCrack=False
  for group in Maillage_1.GetGroups():
    if [group.GetType(), group.GetName()]==[SMESH.NODE, 'crack']: isCrack=True
  if isCrack:
    shutil.copy(crack['med file'],outFile)
  else:
    Group_1 = Maillage_1.CreateEmptyGroup( SMESH.NODE, 'crack' )
    nbAdd = Group_1.AddFrom( Maillage_1.GetMesh() )
    Maillage_1.ExportMED( outFile, 0, SMESH.MED_V2_2, 1, None ,1)
  return(True)



