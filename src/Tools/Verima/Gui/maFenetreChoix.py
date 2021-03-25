# Copyright (C) 2013-2021  EDF R&D
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

from qtsalome import *
from .desFenetreChoix import Ui_Choix
from .monEditor import TableEditor

# Import des panels

class MaFenetreChoix(Ui_Choix ,QDialog):
  """
  """
  def __init__(self,db):
      QDialog.__init__(self)
      self.setModal(True)
      self.setupUi(self)
      self.db=db
      self.connecterSignaux()

  def connecterSignaux(self):
      self.PBMailleur.clicked.connect(self.MailleurPush)
      self.PBMaillage.clicked.connect(self.MaillagePush)
      self.PBVersion.clicked.connect(self.VersionPush)
      self.PBMachine.clicked.connect(self.MachinePush)
      self.PBGroupe.clicked.connect(self.GroupePush)
      self.PBRatio.clicked.connect(self.RatioPush)
      self.PBGroupeRatio.clicked.connect(self.GroupeRatioPush)
      self.PBGroupeTaille.clicked.connect(self.GroupeTaillePush)
      self.PBPerf.clicked.connect(self.PerfPush)
      self.PBMaille.clicked.connect(self.MaillePush)
      self.PBTaille.clicked.connect(self.TaillePush)
      self.PBGroupesRef.clicked.connect(self.GroupesRefPush)

  def MailleurPush(self):
      editor=TableEditor(self.db.maTableMailleurs,self)
      editor.exec_()

  def MaillePush(self):
      editor=TableEditor(self.db.maTableMailles,self)
      editor.exec_()

  def TaillePush(self):
      editor=TableEditor(self.db.maTableTailles,self)
      editor.exec_()
      
  def RatioPush(self):
      editor=TableEditor(self.db.maTableRatios,self)
      editor.exec_()
      
  def GroupeRatioPush(self):
      editor=TableEditor(self.db.maTableGroupeRatios,self)
      editor.exec_()

  def GroupeTaillePush(self):
      editor=TableEditor(self.db.maTableGroupeTailles,self)
      editor.exec_()
      
      
  def PerfPush(self):
      editor=TableEditor(self.db.maTablePerfs,self)
      editor.exec_()
      
  def GroupePush(self):
      editor=TableEditor(self.db.maTableGroupes,self)
      editor.exec_()
      
  def GroupesRefPush(self):
      editor=TableEditor(self.db.maTableGroupesRef,self)
      editor.exec_()

  def MaillagePush(self):
      editor=TableEditor(self.db.maTableMaillages,self)
      editor.exec_()
      
  def VersionPush(self):
      editor=TableEditor(self.db.maTableVersions,self)
      editor.exec_()
      
  def MachinePush(self):
      editor=TableEditor(self.db.maTableMachines,self)
      editor.exec_()
      

  def on_buttonCancel_clicked(self):
      QDialog.reject(self)

  def on_buttonOk_clicked(self):
      QDialog.accept(self)

