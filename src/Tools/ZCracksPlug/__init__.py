# Copyright (C) 2016-2021  EDF R&D
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

import sys, os, shutil, pickle, tempfile
from Zcracks import main, genereCrack, Zset
from Zcracks import utilityFunctions as uF

os.environ['QT_QPA_PLATFORM_PLUGIN_PATH']=os.path.join(os.environ['QTDIR'],'plugins','platforms')

#commande="/bin/bash -c ""source $HOME/zebulon/Z8.6.6_NEW/do_config_bash"""
#os.system(commande)

def IHM():

  from PyQt5.QtWidgets import QApplication

  app = QApplication(sys.argv)
  myapp = main.ShipHolderApplication()
  myapp.show()
  sys.exit(app.exec_())


def SCRIPT(dataFile=None, data=None, dim=3, names=None):
  if dim!=3 and dim!=2:
    print('ERROR')
    return(False)

  if dataFile==None and data==None:
    print('One of dataFile or data is mandatory')
    return(False)

  if data==None: data=pickle.load(open(dataFile,'r'))

  print(data)

  tmpdir=tempfile.mkdtemp(prefix='tmpZcracks')

  if names==None: names={'saneGeoName':'salome_sane', 'crackGeoName':'salome_crack', 'crackedGeoName':'salome_cracked'}

  crackedMed=data['crackedName']
  crackMed=os.path.join(tmpdir,'crackMed.med')
  saneMed=data['saneName']

  saneGeo=os.path.join(tmpdir,names['saneGeoName']+'.geo')
  crackGeo=os.path.join(tmpdir,names['crackGeoName']+'.geo')
  crackedGeo=os.path.join(tmpdir,names['crackedGeoName']+'.geo')

  for f in [crackMed, crackedMed, saneGeo, crackGeo, crackedGeo]:
    if os.path.isfile(f): os.remove(f)

  print(crackMed)
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



