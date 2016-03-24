# -*- coding: iso-8859-1 -*-
# Copyright (C) 2011-2016  EDF R&D
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
# Author : Guillaume Boulant (EDF)
#

import os

import salome
from salome.kernel import studyedit
from salome.gui.genericdialog import GenericDialog
from salome.gui import helper as guihelper
from salome.smesh.smeshstudytools import SMeshStudyTools

from omniORB import CORBA

from qtsalome import QIcon, QStandardItemModel, QStandardItem, QMessageBox, pyqtSignal

from inputframe_ui import Ui_InputFrame
from inputdata import InputData

DEBUG_MODE=True
GROUPNAME_MAXLENGTH=8

class InputDialog(GenericDialog):

    TBL_HEADER_LABEL=["Input Mesh", "Output group name"]
    
    inputValidated = pyqtSignal()

    def __init__(self, parent=None, name="InputDialog", modal=0):
        """
        This initializes a dialog windows to define the input data of
        the plugin function. The input data consist in a list of
        meshes characterizes each by a name, a pointer to the smesh
        servant object, a type and a group name (see data model in the
        inputdata.py).
        """
        GenericDialog.__init__(self, parent, name, modal)
        # Set up the user interface from Designer.
        self.__ui = Ui_InputFrame()
        # BE CAREFULL HERE, the ui form is NOT drawn in the global
        # dialog (already containing some generic widgets) but in the
        # center panel created in the GenericDialog as a void
        # container for the form. The InputFrame form is supposed
        # here to create only the widgets to be placed in the center
        # panel. Then, the setupUi function of this form draws itself
        # in the specified panel, i.e. the panel returned by
        # self.getPanel().
        self.__ui.setupUi(self.getPanel())

        self.setWindowTitle("Specification of input files")

        # The icon are supposed to be located in the plugin folder,
        # i.e. in the same folder than this python module file
        iconfolder=os.path.dirname(os.path.abspath(__file__))
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"select.png"))
        self.__ui.btnSmeshObject.setIcon(icon)
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"addinput.png"))
        self.__ui.btnAddInput.setIcon(icon)
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"deleteinput.png"))
        self.__ui.btnDeleteInput.setIcon(icon)

        # We specify here the items in the combo box (even if already
        # defined in the designer) so that we can be sure of the item
        # indexation.
        self.MESHTYPE_ICONS = {}
        meshTypeIndex = InputData.MESHTYPES.CONCRETE
        self.__ui.cmbMeshType.setItemText(meshTypeIndex, "Béton")
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"concrete.png"))
        self.__ui.cmbMeshType.setItemIcon(meshTypeIndex, icon)
        self.MESHTYPE_ICONS[meshTypeIndex] = icon

        meshTypeIndex = InputData.MESHTYPES.STEELBAR
        self.__ui.cmbMeshType.setItemText(meshTypeIndex, "Acier")
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"steelbar.png"))
        self.__ui.cmbMeshType.setItemIcon(meshTypeIndex, icon)
        self.MESHTYPE_ICONS[meshTypeIndex] = icon
        
        # The click on btnSmeshObject (signal clicked() emitted by the
        # button btnSmeshObject) is connected to the slot
        # onSelectSmeshObject, etc ...
        self.__ui.btnSmeshObject.clicked.connect( self.onSelectSmeshObject )
        self.__ui.btnAddInput.clicked.connect( self.onAddInput )
        self.__ui.btnDeleteInput.clicked.connect(  self.onDeleteInput )

        # Set up the model of the Qt table list
        self.__inputModel = QStandardItemModel(0,2)
        self.__inputModel.setHorizontalHeaderLabels(InputDialog.TBL_HEADER_LABEL)
        self.__ui.tblListInput.setModel(self.__inputModel)
        self.__ui.tblListInput.verticalHeader().hide()
        self.__ui.tblListInput.horizontalHeader().setStretchLastSection(True)
        # Note that the type is not display explicitly in the Qt table
        # because it is specified using an icon on the text of the
        # name item. 

        # Note that PADDER does not support group name longer than 8
        # characters. We apply then this limit in the gui field.
        self.__ui.txtGroupName.setMaxLength(GROUPNAME_MAXLENGTH)

        self.clear()

        self.smeshStudyTool = SMeshStudyTools()

    def clear(self):
        """
        This function clears the data gui area and associated values.
        """
        self.__ui.txtSmeshObject.setText("")
        self.__ui.txtGroupName.setText("")
        self.__inputModel.clear()
        self.__inputModel.setHorizontalHeaderLabels(InputDialog.TBL_HEADER_LABEL)
        if not DEBUG_MODE:
            self.__ui.txtSmeshObject.setEnabled(False)
            self.__ui.btnAddInput.setEnabled(False)
        self.__selectedMesh = None
        self.__dictInputData = {}
        self.__nbConcreteMesh = 0
        self.__nbSteelbarMesh = 0

    def accept(self):
        """
        This function is the slot connected to the button OK
        """
        # The dialog is raised in a non modal mode to get
        # interactivity with the parents windows. Then we have to emit
        # a signal to warn the parent observer that the dialog has
        # been validated so that it can process the event
        GenericDialog.accept(self)
        if self.wasOk():
            self.inputValidated.emit()

    def onSelectSmeshObject(self):
        '''
        This function is the slot connected on the mesh selection
        button. It memorizes the selected mesh and put its name in the
        text field of the dialog box.
        '''
        mySObject, myEntry = guihelper.getSObjectSelected()
        if CORBA.is_nil(mySObject):
            self.__ui.txtSmeshObject.setText("You must choose a mesh")
            self.__ui.txtGroupName.setText("")
            self.__ui.txtSmeshObject.setEnabled(False)
            self.__ui.btnAddInput.setEnabled(False)
            self.__selectedMesh = None
            return

        self.smeshStudyTool.updateStudy(studyedit.getActiveStudyId())
        self.__selectedMesh = self.smeshStudyTool.getMeshObjectFromSObject(mySObject)
        if CORBA.is_nil(self.__selectedMesh):
            self.__ui.txtSmeshObject.setText("The selected object is not a mesh")
            self.__ui.txtGroupName.setText("")
            self.__ui.txtSmeshObject.setEnabled(False)
            self.__ui.btnAddInput.setEnabled(False)
            self.__selectedMesh = None
            return
        myName = mySObject.GetName()
        self.__ui.txtSmeshObject.setText(myName)
        self.__ui.txtSmeshObject.setEnabled(True)
        self.__ui.btnAddInput.setEnabled(True)

        # We can suggest a default group name from the mesh name
        self.__ui.txtGroupName.setText(myName)

    def onAddInput(self):
        """
        This function is the slot connected to the Add button. It
        creates a new entry in the list of input data, or updates this
        entry if it already exists.
        """
        meshName   = str(self.__ui.txtSmeshObject.text()).strip()
        meshObject = self.__selectedMesh
        meshType   = self.__ui.cmbMeshType.currentIndex()
        groupName  = str(self.__ui.txtGroupName.text()).strip()

        self.__addInputInGui(meshName, meshObject, meshType, groupName)
        self.__addInputInMap(meshName, meshObject, meshType, groupName)

    def __addInputInGui(self, meshName, meshObject, meshType, groupName):
        """
        This function adds an entry with the specified data int the
        GUI table (for data visualization purpose).
        """
        # The mesh name is used as the key index in the model. We have
        # to check first if this item already exists in the list.
        tblItems = self.__inputModel.findItems(meshName)
        row = self.__inputModel.rowCount()
        if not tblItems:
            tblItems = []
            tblItems.append(QStandardItem()) # input mesh name
            tblItems.append(QStandardItem()) # output group name
        else:
            row = tblItems[0].index().row()
            tblItems.append(self.__inputModel.item(row,1))

        tblItems[0].setText(meshName)
        tblItems[0].setIcon(self.MESHTYPE_ICONS[meshType])
        tblItems[1].setText(groupName)
        self.__inputModel.setItem(row,0,tblItems[0])
        self.__inputModel.setItem(row,1,tblItems[1])
        self.__ui.tblListInput.setCurrentIndex(tblItems[0].index())

    def __addInputInMap(self, meshName, meshObject, meshType, groupName):
        """
        This function adds an entry with the specified data in the
        internal map (for data management purpose).
        """
        # if the entry already exists, we remove it to replace by a
        # new one
        if self.__dictInputData.has_key(meshName):
            self.__delInputFromMap(meshName)
        
        inputData = InputData()
        inputData.meshName   = meshName
        inputData.meshObject = meshObject
        inputData.meshType   = meshType
        inputData.groupName  = groupName
        # The key of the map is the mesh name
        self.__dictInputData[meshName] = inputData
        if inputData.meshType == InputData.MESHTYPES.CONCRETE:
            self.__nbConcreteMesh += 1
        else:
            self.__nbSteelbarMesh += 1

        print inputData
        print "meshType = ",inputData.meshType
        print "nb concrete mesh ",self.__nbConcreteMesh
        print "nb steelbar mesh ",self.__nbSteelbarMesh
            

    def onDeleteInput(self):
        """
        This function is the slot connected to the Delete button. It
        remove from the data list the entry selected in the Qt table.
        """
        selectedIdx = self.__ui.tblListInput.selectedIndexes()
        if selectedIdx:
            row  = selectedIdx[0].row()
            tblItem  = self.__inputModel.item(row,0)
            meshName = str(tblItem.text())
            self.__inputModel.takeRow(row)
            # Don't forget to remove this entry from the mesh object
            # internal dictionnary
            self.__delInputFromMap(meshName)

    def __delInputFromMap(self, meshName):
        """
        This function removes the specified entry from the internal
        map (for data management purpose) 
        """
        inputData = self.__dictInputData.pop(meshName)
        if inputData.meshType == InputData.MESHTYPES.CONCRETE:
            self.__nbConcreteMesh -= 1
        else:
            self.__nbSteelbarMesh -= 1

        print inputData
        print "nb concrete mesh ",self.__nbConcreteMesh
        print "nb steelbar mesh ",self.__nbSteelbarMesh


    def setData(self, listInputData=[]):
        """
        This function fills the dialog widgets with values provided by
        the specified data list.
        """
        self.clear()
        for inputData in listInputData:

            meshName   = inputData.meshName
            meshObject = inputData.meshObject
            meshType   = inputData.meshType
            groupName  = inputData.groupName
            
            self.__addInputInGui(meshName, meshObject, meshType, groupName)
            self.__addInputInMap(meshName, meshObject, meshType, groupName)

            if not DEBUG_MODE:
                self.onSelectSmeshObject()

    def getData(self):
        """
        This function returns a list of InputData that corresponds to
        the data in the dialog widgets of the current dialog.
        """
        # Note that the values() function returns a copy of the list
        # of values.
        return self.__dictInputData.values()
        
    def checkData(self):
        """
        This function checks if the data are valid, from the dialog
        window point of view.
        """
        if self.__nbConcreteMesh == 0 and self.__nbSteelbarMesh == 0:
            self.checkDataMessage = "You must define at least one mesh (CONCRETE or STEELBAR)"
            return False        
        if self.__nbConcreteMesh > 1:
            self.checkDataMessage  = "You define multiple CONCRETE meshes."
            self.checkDataMessage += "You should verify first that your version of PADDER support this configuration."
            # just warn the user, but don't block
            QMessageBox.information(self, "Info", self.checkDataMessage)
            return True

        return True


# ==============================================================================
# Basic use case
# ==============================================================================
#
def TEST_InputDialog():
    import sys
    from qtsalome import QApplication
    app = QApplication(sys.argv)
    app.lastWindowClosed.connect( app.quit )

    dlg=InputDialog()
    dlg.displayAndWait()
    if dlg.wasOk():
        print "OK has been pressed"

def TEST_InputDialog_setData():
    import sys
    from qtsalome import QApplication
    app = QApplication(sys.argv)
    app.lastWindowClosed.connect( app.quit )

    dlg=InputDialog()

    from inputdata import InputData
    inputData = InputData()
    inputData.meshName   = "myMesh"
    inputData.meshObject = None
    inputData.meshType   = InputData.MESHTYPES.CONCRETE
    inputData.groupName  = "myGroup"
    listInputData = []
    listInputData.append(inputData)
    
    dlg.setData2(listInputData)
    
    dlg.displayAndWait()
    if dlg.wasOk():
        print "OK has been pressed"
        outputListInputData = dlg.getData2()
        print outputListInputData


if __name__ == "__main__":
    #TEST_InputDialog()
    TEST_InputDialog_setData()

