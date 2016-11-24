import sys, os, shutil, pickle, tempfile
import main, genereCrack, Zset
import utilityFunctions as uF

#commande="/bin/bash -c ""source $HOME/zebulon/Z8.6.6_NEW/do_config_bash"""
#os.system(commande)

def IHM():

  try:
    from PyQt5.QtWidgets import QApplication
  except:
    from PyQt4.QtGui import QApplication

  app = QApplication(sys.argv)
  myapp = main.ShipHolderApplication()
  myapp.show()
  sys.exit(app.exec_())


def SCRIPT(dataFile=None, data=None, dim=3, names=None):
  if dim!=3 and dim!=2:
    print 'ERROR'
    return(False)

  if dataFile==None and data==None:
    print 'One of dataFile or data is mandatory'
    return(False)

  if data==None: data=pickle.load(open(dataFile,'r'))

  print data

  tmpdir=tempfile.mkdtemp()
  uF.removeFromSessionPath('LD_LIBRARY_PATH', 'Meshgems-2111')

  if names==None: names={'saneGeoName':'salome_sane', 'crackGeoName':'salome_crack', 'crackedGeoName':'salome_cracked'}

  crackedMed=data['crackedName']
  crackMed=os.path.join(tmpdir,'crackMed.med')
  saneMed=data['saneName']

  saneGeo=os.path.join(tmpdir,names['saneGeoName']+'.geo')
  crackGeo=os.path.join(tmpdir,names['crackGeoName']+'.geo')
  crackedGeo=os.path.join(tmpdir,names['crackedGeoName']+'.geo')

  for f in [crackMed, crackedMed, saneGeo, crackGeo, crackedGeo]:
    if os.path.isfile(f): os.remove(f)

  print crackMed
  genereCrack.main(data, crackMed)
  goOn=os.path.isfile(crackMed)

  if goOn: Zset.medToGeo(crackMed, crackGeo, tmpdir)
  goOn=os.path.isfile(crackGeo)

  if dim==3:
    if goOn: Zset.medToGeo(saneMed,saneGeo, tmpdir)
  elif dim==2:
    if goOn: Zset.medToGeo(saneMed,saneGeo, tmpdir, opt=['  **to_3d'])
  goOn=os.path.isfile(saneGeo)

  if goOn: Zset.insertCrack(data, names, tmpdir)
  goOn=os.path.isfile(crackedGeo)

  if goOn: Zset.geoToMed(crackedMed, crackedGeo, tmpdir)
  goOn=os.path.isfile(crackedMed)

  if goOn: maxAR=uF.extendElsets(crackedMed)

  shutil.rmtree(tmpdir)

  return([os.path.isfile(crackedMed), maxAR])



