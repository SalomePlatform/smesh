from desFenetreChoix import Ui_Choix
from PyQt4  import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from monEditor import TableEditor

# Import des panels

class MaFenetreChoix(Ui_Choix ,QtGui.QDialog):
  """
  """
  def __init__(self,db):
      QtGui.QDialog.__init__(self)
      self.setModal(True)
      self.setupUi(self)
      self.db=db
      self.connecterSignaux()

  def connecterSignaux(self):
      self.connect(self.PBMailleur,SIGNAL("clicked()"),self.MailleurPush)
      self.connect(self.PBMaillage,SIGNAL("clicked()"),self.MaillagePush)
      self.connect(self.PBVersion,SIGNAL("clicked()"),self.VersionPush)
      self.connect(self.PBMachine,SIGNAL("clicked()"),self.MachinePush)
      self.connect(self.PBGroupe,SIGNAL("clicked()"),self.GroupePush)
      self.connect(self.PBRatio,SIGNAL("clicked()"),self.RatioPush)
      self.connect(self.PBPerf,SIGNAL("clicked()"),self.PerfPush)
      self.connect(self.PBMaille,SIGNAL("clicked()"),self.MaillePush)
      self.connect(self.PBTaille,SIGNAL("clicked()"),self.TaillePush)
      self.connect(self.PBGroupesRef,SIGNAL("clicked()"),self.GroupesRefPush)

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

