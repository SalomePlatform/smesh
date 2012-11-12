# -*- coding: utf-8 -*-
# Copyright (C) 2007-2012   EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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
# Modules Python
# Modules Eficas

import os
from YamsPlugDialog import Ui_YamsPlugDialog
from monViewText import MonViewText
from PyQt4.QtGui import *
from PyQt4.QtCore import *


class MonYamsPlugDialog(Ui_YamsPlugDialog,QWidget):
  """
  """
  def __init__(self):
        QWidget.__init__(self)
        self.setupUi(self)
        self.connecterSignaux()
        self.fichierIn=""
        self.fichierOut=""
        self.MeshIn="" 
        self.num=1

#	Ces parametres ne sont pas remis à rien par le clean
        self.paramsFile= os.path.abspath(os.path.join(os.environ['HOME'],'.yams.dat'))
        self.LE_ParamsFile.setText(self.paramsFile)
        self.LE_MeshFile.setText("")
        self.LE_MeshSmesh.setText("")

  def connecterSignaux(self) :
        self.connect(self.PB_Cancel,SIGNAL("clicked()"),self.PBCancelPressed)
        self.connect(self.PB_Default,SIGNAL("clicked()"),self.clean)
        self.connect(self.PB_Help,SIGNAL("clicked()"),self.PBHelpPressed)
        self.connect(self.PB_Load,SIGNAL("clicked()"),self.PBLoadPressed)
        self.connect(self.PB_OK,SIGNAL("clicked()"),self.PBOKPressed)
        self.connect(self.PB_Save,SIGNAL("clicked()"),self.PBSavePressed)
        self.connect(self.PB_MeshFile,SIGNAL("clicked()"),self.PBMeshFilePressed)
        self.connect(self.PB_MeshSmesh,SIGNAL("clicked()"),self.PBMeshSmeshPressed)
        self.connect(self.PB_ParamsFileExplorer,SIGNAL("clicked()"),self.setParamsFileName)
        self.connect(self.LE_MeshFile,SIGNAL("returnPressed()"),self.meshFileNameChanged)
        self.connect(self.LE_ParamsFile,SIGNAL("returnPressed()"),self.paramsFileNameChanged)


  def PBHelpPressed(self):
        try :
          maDoc=os.environ['DISTENE_YAMS_DOC_PDF']
          commande='kpdf '+maDoc
          os.system (commande)
        except:
          QMessageBox.warning( self, "Help unavailable", str(maDoc) + " not found")


  def PBOKPressed(self):
        if not(self.PrepareLigneCommande()) : return
        self.PBSavePressed(NomHypo=True)
        maFenetre=MonViewText(self,self.commande)
        if os.path.isfile(self.fichierOut) :self.enregistreResultat()

  def enregistreResultat(self):
        import smesh
        import SMESH
        import salome
        from salome.kernel import studyedit

        maStudy=studyedit.getActiveStudy()
        smesh.SetCurrentStudy(maStudy)
        (outputMesh, status) = smesh.CreateMeshesFromGMF(self.fichierOut)
        meshname = 'yams'+str(self.num)
        smesh.SetName(outputMesh.GetMesh(), meshname)
        outputMesh.Compute()


        self.editor = studyedit.getStudyEditor()    # 
        moduleEntry=self.editor.findOrCreateComponent("SMESH","SMESH")
        HypReMeshEntry = self.editor.findOrCreateItem( moduleEntry, name = 'HypoForRemesh',
                                           comment = 'HypoForRemshing')
        monStudyBuilder=maStudy.NewBuilder();
        monStudyBuilder.NewCommand();
        newStudyIter=monStudyBuilder.NewObject(HypReMeshEntry)
        aNameAttrib=monStudyBuilder.FindOrCreateAttribute(newStudyIter,"AttributeName")
        hypoName = 'monHypo_Yams_'+str(self.num)
        aNameAttrib.SetValue(hypoName)
        aCommentAttrib=monStudyBuilder.FindOrCreateAttribute(newStudyIter,"AttributeComment")
        aCommentAttrib.SetValue(str(self.commande))
        
        SOMesh=maStudy.FindObjectByName(meshname ,"SMESH")[0]
        newLink=monStudyBuilder.NewObject(SOMesh)
        monStudyBuilder.Addreference(newLink, newStudyIter);
        if salome.sg.hasDesktop(): salome.sg.updateObjBrowser(0)
        self.num+=1
        return True

  def PBSavePressed(self,NomHypo=False):
        if NomHypo : text = '# Params for Hypothese : monHypo_Yams_'+str(self.num - 1)+"\n"
        else :       text = '# Save intermediate params \n' 
        text += "# Params for mesh : " +  self.LE_MeshSmesh.text() +'\n'
        for RB in self.GBOptim.findChildren(QRadioButton,):
            if RB.isChecked()==True:
               text+="Optimisation ='"+RB.text()+"'\n"
               break
        for RB in self.GBUnit.findChildren(QRadioButton,):
            if RB.isChecked()==True:
               text+="Units ='"+RB.text()+"'\n"
        text+='Chordal_Tolerance_Deviation='+str(self.SP_Tolerance.value())+'\n'

        text+='Ridge_Detection=' + str(self.CB_Ridge.isChecked())+'\n'
        text+='Split_Edge='      + str(self.CB_SplitEdge.isChecked())+'\n'
        text+='Point_Smoothing=' + str(self.CB_Point.isChecked())+'\n'
        text+='Geometrical_Approximation='+ str(self.SP_Geomapp.value())  +'\n'
        text+='Ridge_Angle='              + str(self.SP_Ridge.value())    +'\n'
        text+='Maximum_Size='             + str(self.SP_MaxSize.value())  +'\n'
        text+='Minimum_Size='             + str(self.SP_MaxSize.value())  +'\n'
        text+='Mesh_Gradation='           + str(self.SP_Gradation.value())+'\n'

        text+='Verbosity='                + str(self.SP_Verbosity.value())+'\n'
        text+='Memory='                   + str(self.SP_Memory.value())+'\n'
        text+='\n\n'

        try :
           f=open(self.paramsFile,'a')
        except :
           QMessageBox.warning( self, "File", "Unable to open "+self.paramsFile)
           return
        try :
           f.write(text)
        except :
           QMessageBox.warning( self, "File", "Unable to write "+self.paramsFile)
           return
        f.close()

  def PBLoadPressed(self):
        try :
           f=open(self.paramsFile,'r')
        except :
           QMessageBox.warning( self, "File", "Unable to open "+self.paramsFile)
           return
        try :
           text=f.read()
        except :
           QMessageBox.warning( self, "File", "Unable to read "+self.paramsFile)
           return
        f.close()
        d={}
        exec text in d
        for RB in self.GBOptim.findChildren(QRadioButton,):
            if d['Optimisation']== RB.text():
               RB.setChecked(True)
               break
        for RB in self.GBUnit.findChildren(QRadioButton,):
            if d['Units']== RB.text():
               RB.setChecked(True)
               break
        self.SP_Tolerance.setValue(d['Chordal_Tolerance_Deviation'])

        self.CB_Ridge.setChecked(d['Ridge_Detection'])
        self.CB_Point.setChecked(d['Point_Smoothing'])
        self.CB_SplitEdge.setChecked(d['Split_Edge'])
        self.SP_Geomapp.setValue(d['Geometrical_Approximation'])
        self.SP_Ridge.setValue(d['Ridge_Angle'])
        self.SP_MaxSize.setValue(d['Maximum_Size'])
        self.SP_MinSize.setValue(d['Minimum_Size'])
        self.SP_Gradation.setValue(d['Mesh_Gradation'])

        self.SP_Verbosity.setValue(d['Verbosity'])
        self.SP_Memory.setValue(d['Memory'])


  def PBCancelPressed(self):
        self.close()

  def PBMeshFilePressed(self):
       fd = QFileDialog(self, "select an existing Mesh file", self.LE_MeshFile.text(), "Mesh-Files (*.mesh);;All Files (*)")
       if fd.exec_():
          infile = fd.selectedFiles()[0]
          self.LE_MeshFile.setText(infile)
          self.fichierIn=infile.toLatin1()

  def setParamsFileName(self):
       fd = QFileDialog(self, "select a file", self.LE_ParamsFile.text(), "dat Files (*.dat);;All Files (*)")
       if fd.exec_():
          infile = fd.selectedFiles()[0]
          self.LE_ParamsFile.setText(infile)
          self.paramsFile=infile.toLatin1()


  def meshFileNameChanged(self):
      self.fichierIn=self.LE_MeshFile.text()
      if os.path.exists(self.fichierIn): return
      QMessageBox.warning( self, "Unknown File", "File doesn't exist")

  def paramsFileNameChanged(self):
      self.paramsFile=self.LE_ParamsFile.text()

  def PBMeshSmeshPressed(self):
      import salome
      import smesh
      from salome.kernel import studyedit
      from salome.smesh.smeshstudytools import SMeshStudyTools
      from salome.gui import helper as guihelper
      from omniORB import CORBA

      mySObject, myEntry = guihelper.getSObjectSelected()
      if CORBA.is_nil(mySObject) or mySObject==None:
         QMessageBox.critical(self, "Mesh", "select an input mesh")
         return
      self.smeshStudyTool = SMeshStudyTools()
      self.__selectedMesh = self.smeshStudyTool.getMeshObjectFromSObject(mySObject)
      if CORBA.is_nil(self.__selectedMesh):
         QMessageBox.critical(self, "Mesh", "select an input mesh")
         return
      myName = mySObject.GetName()
      self.MeshIn=myName
      self.LE_MeshSmesh.setText(myName)

  def prepareFichier(self):
      self.fichierIn="/tmp/PourYam_"+str(self.num)+".mesh"
      import SMESH
      self.__selectedMesh.ExportGMF(self.__selectedMesh,self.fichierIn)

  def PrepareLigneCommande(self):
      self.commande="yams "
      verbosity=str(self.SP_Verbosity.value())
      self.commande+="-v "+verbosity
      for obj in self.mesRB.children():
          try :
           if obj.isChecked():
              self.style=obj.objectName().remove(0,3)
              self.style.replace("_","-")
              break
          except :
              pass
      self.commande+=" -O "+self.style.toLatin1()
      if self.fichierIn=="" and self.MeshIn=="" :
         QMessageBox.critical(self, "Mesh", "select an input mesh")
         return False
      if self.MeshIn!="" : self.prepareFichier()
      if not (os.path.isfile(self.fichierIn)):
         QMessageBox.critical(self, "File", "unable to read GMF Mesh in "+str(self.fichierIn))
         return False

      deb=os.path.splitext(self.fichierIn)
      self.fichierOut=deb[0]+'.d.meshb'

      if self.RB_Absolute.isChecked()==True :
         self.commande+=' -Dabsolute'
      else :
         self.commande+=' -Drelative'
      self.commande+=',tolerance=%f'%self.SP_Tolerance.value()
      if self.CB_Ridge.isChecked()==False : self.commande+=',nr'
      if self.CB_Point.isChecked()==False : self.commande+=',ns'
      if self.SP_Geomapp.value()!=0.04 : self.commande+=',geomapp=%f'%self.SP_Geomapp.value()
      if self.SP_Ridge.value()!=45.0 : self.commande+=',ridge=%f'%self.SP_Ridge.value()
      if self.SP_MaxSize.value()!=100 : self.commande+=',maxsize=%f'%self.SP_MaxSize.value()
      if self.SP_MinSize.value()!=5 : self.commande+=',minsize=%f'%self.SP_MinSize.value()
      if self.SP_Gradation.value()!=1.3 : self.commande+=',gradation=%f'%self.SP_MaxSize.value()
      if self.CB_SplitEdge.isChecked()==True : self.commande+=',splitedge=1'

      if self.SP_Verbosity.value()!=3 : self.commande+=' -v %d'%self.SP_Verbosity.value()
      if self.SP_Memory.value()!=0 : self.commande+=' -m %d'%self.SP_Memory.value()

      self.commande+=" "+self.fichierIn
      return True

  def clean(self):
        self.RB_0.setChecked(True)
        self.RB_G.setChecked(False)
        self.RB_U.setChecked(False)
        self.RB_S.setChecked(False)
        self.RB_2.setChecked(False)
        self.RB_1.setChecked(False)
        self.RB_Absolute.setChecked(False)
        self.RB_Relative.setChecked(True)
        self.SP_Tolerance.setProperty("value", 0.1)
        self.SP_Geomapp.setProperty("value", 0.04)
        self.SP_Ridge.setProperty("value", 45.0)
        self.SP_Gradation.setProperty("value", 1.3)
        self.CB_Ridge.setChecked(True)
        self.CB_Point.setChecked(True)
        self.CB_SplitEdge.setChecked(False)
        self.SP_MaxSize.setProperty("value", -2.0)
        self.SP_MinSize.setProperty("value", -2.0)
        self.SP_Verbosity.setProperty("value", 3)
        self.SP_Memory.setProperty("value", 0)


__dialog=None
def getDialog():
    """
    This function returns a singleton instance of the plugin dialog.
    c est obligatoire pour faire un show sans parent...
    """
    global __dialog
    if __dialog is None:
        __dialog = MonYamsPlugDialog()
    #else :
    #   __dialog.clean()
    return __dialog

