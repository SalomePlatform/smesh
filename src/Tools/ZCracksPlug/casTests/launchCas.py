
from Zcracks import genereCrack, Zset
from Zcracks import utilityFunctions as uF
import genereCube

from math import sqrt


if False:
  import Zcracks
  from Zcracks import casTests
  from Zcracks.casTests import launchCas
  launchCas.LAUNCH(['10'])

import os, shutil
import tempfile
import string

#tmpdir = "/local00/home/B27118/projets/Zcracks/Zcracks/casTests/tmpdir"
#if not os.path.isdir(tmpdir): os.mkdir(tmpdir)
tmpdir=tempfile.mktemp(prefix='tmpZcracks')
print "tmpdir=", tmpdir

meshgemsdir=os.environ('MESHGEMSHOME')
if len(meshgemsdir) > 0:
  meshgems=string.split(meshgemsdir,os.sep)[-1]
  uF.removeFromSessionPath('LD_LIBRARY_PATH', meshgems)

def LAUNCH(listCas=[]):
  if type(listCas)!=list: listCas=[listCas]

  N=20
  L=1.
  te=L/N
  offset=te/2.
  genereCube.cube3D(L, N, os.path.join(tmpdir,'cube3D.med'))
  genereCube.cube2D(L, N, os.path.join(tmpdir,'cube2D.med'))
  crack={}
  synthese={}

  # -------- #
  #  CAS 2D  #
  # -------- #
  data={'minSize':[te/10.], 'maxSize':[te], 'extractLength':[2.*te], 'is2D':True, 'crack':crack,
        'grEdge':['EDGEXP EDGEXM EDGEYP EDGEYM']}

  cas='1'
  crack['actif']='Ellipse'
  crack[crack['actif']]={'Rayon':[L/2.], 'Centre':[L/2., offset, 0.], 'Normale':[0., 1., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)

  cas='2'
  crack['actif']='Ellipse'
  crack[crack['actif']]={'Rayon':[L], 'Centre':[L/2., 0., 0.], 'Normale':[1., 1., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)

  cas='3'
  crack['actif']='Ellipse'
  crack[crack['actif']]={'Rayon':[L*sqrt(2.)], 'Centre':[-L/2., L/2., 0.], 'Normale':[1., 1., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)

  cas='4'
  crack['actif']='Ellipse'
  crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[0., offset, 0.], 'Normale':[0., 1., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)

  data['grFace']=['FACEXM FACEXP']
  cas='5'
  crack['actif']='Ellipse'; crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[0., offset, 0.], 'Normale':[0., 1., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)

  cas='6'
  crack['actif']='Ellipse'
  crack[crack['actif']]={'Rayon':[L/8.], 'Centre':[-L/16., offset, 0.], 'Normale':[0., 1., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)


  data['grFace']=['']
  cas='7'
  crack['actif']='Sphere'
  crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[0., 0., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)

  cas='8'
  crack['actif']='Sphere'
  crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[L/2., 0., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)

  data['grFace']=['FACEXM FACEXP']
  cas='9'
  crack['actif']='Sphere'
  crack['Sphere']={'Rayon':[L/4.], 'Centre':[0., 0., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute2D(data, cas)

  # -------- #
  #  CAS 3D  #
  # -------- #
  data['grEdge']=['']
  data['grFace']=['FACEXP FACEXM FACEYP FACEYM FACEZP FACEZM']
  data['is2D']=False

  cas='10'
  crack['actif']='Rectangle'
  crack[crack['actif']]={'Centre':[L/2., offset, 0.], 'Normale':[0., 1., 0.], 'Direction':[1., 0., 0.], 'Longueur':[L/2.], 'Largeur':[L]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  cas='11'
  crack['actif']='Sphere'
  crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[0., 0., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  cas='12'
  crack['actif']='Ellipse'
  crack[crack['actif']]={'Centre':[L/4., L/4., L/4.], 'Normale':[1., 1., 1.], 'Rayon':[L]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  cas='13'
  crack['actif']='Sphere'
  crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[L/2., 0., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  cas='14'
  crack['actif']='Ellipse'
  crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[L/2., offset, 0.], 'Normale':[0., 1., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  cas='15'
  crack['actif']='Rectangle'
  crack[crack['actif']]={'Centre':[L/2., 0., 0.], 'Normale':[0., 1., 0.], 'Direction':[-1., 0., 0.],
                         'Longueur':[L/2.], 'Largeur':[L], 'Rayon entaille':[te*1.5]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  cas='16'
  crack['actif']='Rectangle'
  crack[crack['actif']]={'Centre':[L/2., offset, 0.], 'Normale':[0., 1., 0.], 'Direction':[1., 0., 0.],
                         'Longueur':[L/2.], 'Largeur':[L/4.], 'Rayon':[2.*te]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  cas='17'
  crack['actif']='Rectangle'
  crack[crack['actif']]={'Centre':[0., offset, 0.], 'Normale':[0., 1., 0.], 'Direction':[1., 0., 0.],
                         'Longueur':[L/4.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  cas='18'
  crack['actif']='Rectangle'
  crack[crack['actif']]={'Centre':[0., 0., 0.], 'Normale':[1., 1., 0.], 'Direction':[1., -1., 0.],
                         'Longueur':[L], 'Angle':[180.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)


  data['grVol']=['VOLYP VOLYM']

  cas='19'
  crack['actif']='Rectangle'
  crack[crack['actif']]={'Centre':[0., 0., 0.], 'Normale':[0., 1., 0.], 'Direction':[1., 0., 0.],
                         'Longueur':[L], 'Angle':[180.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  data['grVol']=['VOLXP VOLXM']

  cas='20'
  crack['actif']='Sphere'
  crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[0., 0., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  data['grVol']=['VOLYP VOLYM']

  cas='21'
  crack['actif']='Ellipse'
  crack[crack['actif']]={'Rayon':[L/4.], 'Centre':[L/2., offset, 0.], 'Normale':[0., 1., 0.]}
  if cas in listCas or listCas==[]:
    synthese[cas]=execute3D(data, cas)

  OK=[]
  NOOK=[]
  for s in synthese.keys():
    if synthese[s]:
      OK.append(s)
    else:
      NOOK.append(s)

  print 'OK:'
  print OK
  print ' '
  print 'NOOK:'
  print NOOK
  print ' '

  return(synthese)



def execute3D(data, cas):
  names={'saneGeoName':'cube3D', 'crackGeoName':'crack'+cas, 'crackedGeoName':'cracked'+cas}

  crackMed=os.path.join(tmpdir,'crack'+cas+'.med')
  crackedMed=os.path.join(tmpdir,'cracked'+cas+'.med')

  saneGeo=os.path.join(tmpdir,names['saneGeoName'],'.geo')
  crackGeo=os.path.join(tmpdir,names['crackGeoName'],'.geo')
  crackedGeo=os.path.join(tmpdir,names['crackedGeoName'],'.geo')

  for f in [crackMed, crackedMed, saneGeo, crackGeo, crackedGeo]:
    if os.path.isfile(f): os.remove(f)

  genereCrack.main(data, crackMed)

  Zset.medToGeo(os.path.join(tmpdir,names['saneGeoName']),names['saneGeoName'], tmpdir)
  Zset.medToGeo(crackMed, names['crackGeoName'], tmpdir)
  Zset.insertCrack(data, names, tmpdir)
  shutil.copy(os.path.join(tmpdir,'_mesh_out_to_ghs3d.mesh'),os.path.join(tmpdir,'mesh'+cas+'.mesh'))
  shutil.copy(os.path.join(tmpdir,'insert.z7p'),os.path.join(tmpdir,'insert'+cas+'.z7p'))

  Zset.geoToMed(crackedMed, names['crackedGeoName'], tmpdir)

  if os.path.isfile(crackedMed):
    uF.extendElsets(crackedMed)
    maxAR=uF.getMaxAspectRatio(tmpdir)
    return(maxAR<30. and maxAR>=1.)
  else:
    return(False)



def execute2D(data, cas):
  names={'saneGeoName':'cube2D', 'crackGeoName':'crack'+cas, 'crackedGeoName':'cracked'+cas}

  crackMed=os.path.join(tmpdir,'crack'+cas+'.med')
  crackedMed=os.path.join(tmpdir,'cracked'+cas+'.med')

  saneGeo=os.path.join(tmpdir,names['saneGeoName'],'.geo')
  crackGeo=os.path.join(tmpdir,names['crackGeoName'],'.geo')
  crackedGeo=os.path.join(tmpdir,names['crackedGeoName'],'.geo')

  for f in [crackMed, crackedMed, saneGeo, crackGeo, crackedGeo]:
    if os.path.isfile(f): os.remove(f)

  genereCrack.main(data, crackMed)

  Zset.medToGeo(os.path.join(tmpdir,names['saneGeoName']),names['saneGeoName'], tmpdir, opt=['  **to_3d'])
  Zset.medToGeo(crackMed, names['crackGeoName'], tmpdir)
  Zset.insertCrack(data, names, tmpdir)
  shutil.copy(os.path.join(tmpdir,'_mesh_out_.mesh'),os.path.join(tmpdir,'mesh'+cas+'.mesh'))
  shutil.copy(os.path.join(tmpdir,'insert.z7p'),os.path.join(tmpdir,'insert'+cas+'.z7p'))
  Zset.geoToMed(crackedMed, names['crackedGeoName'], tmpdir)

  if os.path.isfile(crackedMed):
    uF.extendElsets(crackedMed)
    maxAR=uF.getMaxAspectRatio(tmpdir)
    return(maxAR<30. and maxAR>=1.)
  else:
    return(False)




