#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#  File   : batchmode_smesh.py
#  Author : Oksana TCHEBANOVA
#  Module : SMESH
#  $Header$
#
from batchmode_salome import *
from batchmode_geompy import ShapeType
import SMESH

#--------------------------------------------------------------------------
modulecatalog = naming_service.Resolve("/Kernel/ModulCatalog")

smesh = lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(myStudy)
myStudyBuilder = myStudy.NewBuilder()

if myStudyBuilder is None:
	raise RuntimeError, " Null myStudyBuilder"

father = myStudy.FindComponent("SMESH")
if father is None:
        father = myStudyBuilder.NewComponent("SMESH")
        FName = myStudyBuilder.FindOrCreateAttribute(father, "AttributeName")
	Comp = modulecatalog.GetComponent("SMESH")
	FName.SetValue(Comp._get_componentusername())
      	aPixmap = myStudyBuilder.FindOrCreateAttribute(father, "AttributePixMap")
	aPixmap.SetPixMap("ICON_OBJBROWSER_Mesh")

myStudyBuilder.DefineComponentInstance(father,smesh)

mySComponentMesh = father._narrow(SALOMEDS.SComponent)

Tag_HypothesisRoot  = 1
Tag_AlgorithmsRoot  = 2
  
Tag_RefOnShape      = 1
Tag_RefOnAppliedHypothesis = 2
Tag_RefOnAppliedAlgorithms = 3
  
Tag_SubMeshOnVertex = 4
Tag_SubMeshOnEdge = 5
Tag_SubMeshOnFace = 6
Tag_SubMeshOnSolid = 7
Tag_SubMeshOnCompound = 8

Tag = {"HypothesisRoot":1,"AlgorithmsRoot":2,"RefOnShape":1,"RefOnAppliedHypothesis":2,"RefOnAppliedAlgorithms":3,"SubMeshOnVertex":4,"SubMeshOnEdge":5,"SubMeshOnFace":6,"SubMeshOnSolid":7,"SubMeshOnCompound":8}

#------------------------------------------------------------
def Init():
  	 pass
#------------------------------------------------------------
def AddNewMesh(IOR):
	# VSR: added temporarily - objects are published automatically by the engine
	aSO = myStudy.FindObjectIOR( IOR )
	if aSO is not None:
		return aSO.GetID()
	# VSR ######################################################################
	
	res,HypothesisRoot = mySComponentMesh.FindSubObject ( Tag_HypothesisRoot )
	if HypothesisRoot is None or res == 0:
		HypothesisRoot = myStudyBuilder.NewObjectToTag(mySComponentMesh, Tag_HypothesisRoot)
		aName = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributeName")
    		aName.SetValue("Hypotheses")
    		aPixmap = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributePixMap")
    		aPixmap.SetPixMap( "mesh_tree_hypo.png" )
    		aSelAttr = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributeSelectable")
    		aSelAttr.SetSelectable(0)

	res, AlgorithmsRoot = mySComponentMesh.FindSubObject (Tag_AlgorithmsRoot)
	if AlgorithmsRoot is None  or res == 0:
    		AlgorithmsRoot = myStudyBuilder.NewObjectToTag (mySComponentMesh, Tag_AlgorithmsRoot)
		aName = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributeName")
		aName.SetValue("Algorithms")
    		aPixmap = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributePixMap")
    		aPixmap.SetPixMap( "mesh_tree_algo.png" )
    		aSelAttr = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributeSelectable")
    		aSelAttr.SetSelectable(0)

	HypothesisRoot = HypothesisRoot._narrow(SALOMEDS.SObject)
	newMesh = myStudyBuilder.NewObject(mySComponentMesh)
  	aPixmap = myStudyBuilder.FindOrCreateAttribute(newMesh, "AttributePixMap")
  	aPixmap.SetPixMap( "mesh_tree_mesh.png" )
  	anIOR = myStudyBuilder.FindOrCreateAttribute(newMesh, "AttributeIOR")
  	anIOR.SetValue(IOR)
  	return newMesh.GetID()

#------------------------------------------------------------	
def AddNewHypothesis(IOR):
	# VSR: added temporarily - objects are published automatically by the engine
	aSO = myStudy.FindObjectIOR( IOR )
	if aSO is not None:
		return aSO.GetID()
	# VSR ######################################################################

	res, HypothesisRoot = mySComponentMesh.FindSubObject (Tag_HypothesisRoot)
  	if HypothesisRoot is None or res == 0:
    		HypothesisRoot = myStudyBuilder.NewObjectToTag (mySComponentMesh, Tag_HypothesisRoot)
    		aName = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributeName")
    		aName.SetValue("Hypotheses")
    		aSelAttr = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributeSelectable")
    		aSelAttr.SetSelectable(0)
    		aPixmap = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributePixMap")
    		aPixmap.SetPixMap( "mesh_tree_hypo.png" )
 
  	# Add New Hypothesis
  	newHypo = myStudyBuilder.NewObject(HypothesisRoot)
  	aPixmap = myStudyBuilder.FindOrCreateAttribute(newHypo, "AttributePixMap")
	H = orb.string_to_object(IOR)
 	aType = H.GetName()
  	aPixmap.SetPixMap( "mesh_tree_hypo.png_" + aType )
	anIOR = myStudyBuilder.FindOrCreateAttribute(newHypo, "AttributeIOR")
  	anIOR.SetValue(IOR)
  	return newHypo.GetID()

#------------------------------------------------------------
def AddNewAlgorithms(IOR):
	# VSR: added temporarily - objects are published automatically by the engine
	aSO = myStudy.FindObjectIOR( IOR )
	if aSO is not None:
		return aSO.GetID()
	# VSR ######################################################################

	res, AlgorithmsRoot = mySComponentMesh.FindSubObject (Tag_AlgorithmsRoot)
  	if  AlgorithmsRoot is None or res == 0:
    		AlgorithmsRoot = myStudyBuilde.NewObjectToTag (mySComponentMesh, Tag_AlgorithmsRoot)
    		aName = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributeName")
    		aName.SetValue("Algorithms")
    		aSelAttr = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributeSelectable")
    		aSelAttr.SetSelectable(0)
    		aPixmap = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributePixMap")
    		aPixmap.SetPixMap( "mesh_tree_algo.png" )

  # Add New Algorithms
  	newHypo = myStudyBuilder.NewObject(AlgorithmsRoot)
  	aPixmap = myStudyBuilder.FindOrCreateAttribute(newHypo, "AttributePixMap")
  	aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap)
	H = orb.string_to_object(IOR)
  	aType = H.GetName();	#QString in fact
  	aPixmap.SetPixMap( "mesh_tree_algo.png_" + aType )
  	anIOR = myStudyBuilder.FindOrCreateAttribute(newHypo, "AttributeIOR")
  	anIOR.SetValue(IOR)
 	return newHypo.GetID()


#------------------------------------------------------------
def SetShape(ShapeEntry, MeshEntry):
	SO_MorSM = myStudy.FindObjectID( MeshEntry )
  	SO_GeomShape = myStudy.FindObjectID( ShapeEntry )

  	if SO_MorSM is not None and SO_GeomShape is not None :
		# VSR: added temporarily - shape reference is published automatically by the engine
		res, Ref = SO_MorSM.FindSubObject( Tag_RefOnShape )
		if res == 1 :
			return
		# VSR ######################################################################
	
    		SO = myStudyBuilder.NewObjectToTag (SO_MorSM, Tag_RefOnShape)
    		myStudyBuilder.Addreference (SO,SO_GeomShape)


#------------------------------------------------------------
def SetHypothesis(Mesh_Or_SubMesh_Entry, Hypothesis_Entry):
  SO_MorSM = myStudy.FindObjectID( Mesh_Or_SubMesh_Entry )
  SO_Hypothesis =  myStudy.FindObjectID( Hypothesis_Entry )

  if  SO_MorSM is not None and SO_Hypothesis is not None : 
    
        #Find or Create Applied Hypothesis root
    	res, AHR = SO_MorSM.FindSubObject (Tag_RefOnAppliedHypothesis)
    	if  AHR is None or res == 0: 
      		AHR = myStudyBuilder.NewObjectToTag (SO_MorSM, Tag_RefOnAppliedHypothesis)
      		aName = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributeName")
		
		# The same name as in SMESH_Mesh_i::AddHypothesis() ##################
      		aName.SetValue("Applied hypotheses")
		
      		aSelAttr = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributeSelectable")
      		aSelAttr.SetSelectable(0)
      		aPixmap = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributePixMap")
      		aPixmap.SetPixMap( "mesh_tree_hypo.png" )
		
	# VSR: added temporarily - reference to applied hypothesis is published automatically by the engine
	else :
		it = myStudy.NewChildIterator(AHR)
		while it.More() :
			res, Ref = it.Value().ReferencedObject()
			if res and Ref is not None and Ref.GetID() == Hypothesis_Entry :
				return
			it.Next()
	# VSR ######################################################################
	
	SO = myStudyBuilder.NewObject(AHR)
    	myStudyBuilder.Addreference (SO,SO_Hypothesis)

#------------------------------------------------------------
def SetAlgorithms(Mesh_Or_SubMesh_Entry, Algorithms_Entry):
    SO_MorSM = myStudy.FindObjectID( Mesh_Or_SubMesh_Entry )
    SO_Algorithms = myStudy.FindObjectID( Algorithms_Entry )
    if  SO_MorSM != None and SO_Algorithms != None : 
    	#Find or Create Applied Algorithms root
    	res, AHR = SO_MorSM.FindSubObject (Tag_RefOnAppliedAlgorithms)
    	if AHR is None or res == 0: 
      		AHR = myStudyBuilder.NewObjectToTag (SO_MorSM, Tag_RefOnAppliedAlgorithms)
      		aName = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributeName")

		# The same name as in SMESH_Mesh_i::AddHypothesis() ##################
      		aName.SetValue("Applied algorithms")
		
      		aSelAttr = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributeSelectable")
      		aSelAttr.SetSelectable(0)
      		aPixmap = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributePixMap")
      		aPixmap.SetPixMap( "mesh_tree_algo.png" )
			
	# VSR: added temporarily - reference to applied hypothesis is published automatically by the engine
	else :
		it = myStudy.NewChildIterator(AHR)
		while it.More() :
			res, Ref = it.Value().ReferencedObject()
			if res and Ref is not None and Ref.GetID() == Algorithms_Entry :
				return
			it.Next()
	# VSR ######################################################################
	
    	SO = myStudyBuilder.NewObject(AHR)
    	myStudyBuilder.Addreference (SO,SO_Algorithms)
  

#------------------------------------------------------------
def UnSetHypothesis( Applied_Hypothesis_Entry ):
  	SO_Applied_Hypothesis = myStudy.FindObjectID( Applied_Hypothesis_Entry )
  	if SO_Applied_Hypothesis : 
    		myStudyBuilder.RemoveObject(SO_Applied_Hypothesis)
	

#------------------------------------------------------------
def AddSubMesh ( SO_Mesh_Entry, SM_IOR, ST):
	# VSR: added temporarily - objects are published automatically by the engine
	aSO = myStudy.FindObjectIOR( SM_IOR )
	if aSO is not None:
		return aSO.GetID()
	# VSR ######################################################################
	
	SO_Mesh = myStudy.FindObjectID( SO_Mesh_Entry )
  	if ( SO_Mesh ) : 
    
    		if  ST == ShapeType["COMPSOLID"] : 
			Tag_Shape = Tag_SubMeshOnSolid
			Name = "SubMeshes on Solid"
    		elif ST == ShapeType["FACE"] :
			Tag_Shape = Tag_SubMeshOnFace
			Name = "SubMeshes on Face"
    		elif ST == ShapeType["EDGE"] :
			Tag_Shape = Tag_SubMeshOnEdge
			Name = "SubMeshes on Edge"
    		elif ST == ShapeType["VERTEX"] :
			Tag_Shape = Tag_SubMeshOnVertex
			Name = "SubMeshes on Vertex"
    		else :
      			Tag_Shape = Tag_SubMeshOnCompound
			Name = "SubMeshes on Compound"
    		
		res, SubmeshesRoot = SO_Mesh.FindSubObject (Tag_Shape)
    		if SubmeshesRoot is None or res == 0:
      			SubmeshesRoot = myStudyBuilder.NewObjectToTag (SO_Mesh, Tag_Shape)
      			aName = myStudyBuilder.FindOrCreateAttribute(SubmeshesRoot, "AttributeName")
      			aName.SetValue(Name)
      			aSelAttr = myStudyBuilder.FindOrCreateAttribute(SubmeshesRoot, "AttributeSelectable")
      			aSelAttr.SetSelectable(0)
    		
    		SO = myStudyBuilder.NewObject (SubmeshesRoot)
    		anIOR = myStudyBuilder.FindOrCreateAttribute(SO, "AttributeIOR")
    		anIOR.SetValue(SM_IOR)
    		return  SO.GetID()

  	return None

#------------------------------------------------------------
def AddSubMeshOnShape (Mesh_Entry, GeomShape_Entry, SM_IOR, ST) :
	# VSR: added temporarily - objects are published automatically by the engine
	aSO = myStudy.FindObjectIOR( SM_IOR )
	if aSO is not None:
		return aSO.GetID()
	# VSR ######################################################################
	SO_GeomShape = myStudy.FindObjectID( GeomShape_Entry )
	if  SO_GeomShape != None : 
		SM_Entry = AddSubMesh (Mesh_Entry,SM_IOR,ST)
		SO_SM = myStudy.FindObjectID( SM_Entry )

		if  SO_SM != None :
			SetShape (GeomShape_Entry, SM_Entry)
			return SM_Entry

	return None


#------------------------------------------------------------
def SetName(Entry, Name):
	SO = myStudy.FindObjectID( Entry )
  	if SO != None : 
   		aName = myStudyBuilder.FindOrCreateAttribute(SO, "AttributeName")
    		aName.SetValue(Name)
