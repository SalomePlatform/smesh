#  Copyright (C) 2003  CEA/DEN, EDF R&D
#
#
#
#  File   : batchmode_smesh.py
#  Author : Oksana TCHEBANOVA
#  Module : SMESH
#  $Header$

from batchmode_salome import *
import SMESH

#--------------------------------------------------------------------------
modulecatalog = naming_service.Resolve("/Kernel/ModulCatalog")

smesh = lcc.FindOrLoadComponent("FactoryServer", "SMESH")
myStudyBuilder = myStudy.NewBuilder()

if myStudyBuilder is None:
	raise RuntimeError, " Null myStudyBuilder"

father = myStudy.FindComponent("MESH")
if father is None:
        father = myStudyBuilder.NewComponent("MESH")
        A1 = myStudyBuilder.FindOrCreateAttribute(father, "AttributeName");
        FName = A1._narrow(SALOMEDS.AttributeName)
        #FName.SetValue("Mesh")	
	Comp = modulecatalog.GetComponent( "SMESH" )
	FName.SetValue( Comp._get_componentusername() )
      	A2 = myStudyBuilder.FindOrCreateAttribute(father, "AttributePixMap");
      	aPixmap = A2._narrow(SALOMEDS.AttributePixMap);
	aPixmap.SetPixMap( "ICON_OBJBROWSER_Mesh" );

myStudyBuilder.DefineComponentInstance(father,smesh)

mySComponentMesh = father._narrow(SALOMEDS.SComponent)

Tag_HypothesisRoot  = 1;
Tag_AlgorithmsRoot  = 2;
  
Tag_RefOnShape      = 1;
Tag_RefOnAppliedHypothesis = 2;
Tag_RefOnAppliedAlgorithms = 3;
  
Tag_SubMeshOnVertex = 4;
Tag_SubMeshOnEdge = 5;
Tag_SubMeshOnFace = 6;
Tag_SubMeshOnSolid = 7;
Tag_SubMeshOnCompound = 8;

Tag = {"HypothesisRoot":1,"AlgorithmsRoot":2,"RefOnShape":1,"RefOnAppliedHypothesis":2,"RefOnAppliedAlgorithms":3,"SubMeshOnVertex":4,"SubMeshOnEdge":5,"SubMeshOnFace":6,"SubMeshOnSolid":7,"SubMeshOnCompound":8}

# -- enumeration --
ShapeTypeCompSolid = 1
ShapeTypeSolid     = 2
ShapeTypeShell     = 3
ShapeTypeFace      = 4
ShapeTypeWire      = 5
ShapeTypeEdge      = 6
ShapeTypeVertex    = 7

# -- enumeration ShapeType as a dictionary --
ShapeType = {"CompSolid":1, "Solid":2, "Shell":3, "Face":4, "Wire":5, "Edge":6, "Vertex":7}

#------------------------------------------------------------
def Init():
  	 pass
#------------------------------------------------------------
def AddNewMesh(IOR):
	res,HypothesisRoot = mySComponentMesh.FindSubObject ( Tag_HypothesisRoot )
	if HypothesisRoot is None or res == 0:
		HypothesisRoot = myStudyBuilder.NewObjectToTag(mySComponentMesh, Tag_HypothesisRoot)
		anAttr = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributeName")
		aName = anAttr._narrow(SALOMEDS.AttributeName)
    		aName.SetValue("Hypothesis Definition")
    		anAttr = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributePixMap")
    		aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap)
    		aPixmap.SetPixMap( "mesh_tree_hypo.png" )
    		anAttr = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributeSelectable")
    		aSelAttr = anAttr._narrow(SALOMEDS.AttributeSelectable)
    		aSelAttr.SetSelectable(0);

	res, AlgorithmsRoot = mySComponentMesh.FindSubObject (Tag_AlgorithmsRoot)
	if AlgorithmsRoot is None  or res == 0:
    		AlgorithmsRoot = myStudyBuilder.NewObjectToTag (mySComponentMesh, Tag_AlgorithmsRoot)
		anAttr = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributeName")
    		aName = anAttr._narrow(SALOMEDS.AttributeName)
		aName.SetValue("Algorithms Definition");
    		anAttr = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributePixMap");
    		aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap);
    		aPixmap.SetPixMap( "mesh_tree_algo.png" );
    		anAttr = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributeSelectable");
    		aSelAttr = anAttr._narrow(SALOMEDS.AttributeSelectable);
    		aSelAttr.SetSelectable(0);

	HypothesisRoot = HypothesisRoot._narrow(SALOMEDS.SObject)
	newMesh = myStudyBuilder.NewObject(mySComponentMesh)
	newMesh = newMesh._narrow(SALOMEDS.SObject)
  	anAttr = myStudyBuilder.FindOrCreateAttribute(newMesh, "AttributePixMap")
  	aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap)
  	aPixmap.SetPixMap( "mesh_tree_mesh.png" )
  	anAttr = myStudyBuilder.FindOrCreateAttribute(newMesh, "AttributeIOR")
  	anIOR = anAttr._narrow(SALOMEDS.AttributeIOR)
  	anIOR.SetValue(IOR)
  	return newMesh.GetID()

#------------------------------------------------------------	
def AddNewHypothesis(IOR):
	res, HypothesisRoot = mySComponentMesh.FindSubObject (Tag_HypothesisRoot)
  	if HypothesisRoot is None or res == 0:
    		HypothesisRoot = myStudyBuilder.NewObjectToTag (mySComponentMesh, Tag_HypothesisRoot)
    		anAttr = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributeName");
    		aName = anAttr._narrow(SALOMEDS.AttributeName);
    		aName.SetValue("Hypothesis Definition");
    		anAttr = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributeSelectable");
    		aSelAttr = anAttr._narrow(SALOMEDS.AttributeSelectable);
    		aSelAttr.SetSelectable(0);
    		anAttr = myStudyBuilder.FindOrCreateAttribute(HypothesisRoot, "AttributePixMap");
    		aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap);
    		aPixmap.SetPixMap( "mesh_tree_hypo.png" );
 
  	# Add New Hypothesis
  	newHypo = myStudyBuilder.NewObject(HypothesisRoot)
	newHypo = newHypo._narrow(SALOMEDS.SObject)
  	anAttr  = myStudyBuilder.FindOrCreateAttribute(newHypo, "AttributePixMap")
  	aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap)
	H = orb.string_to_object(IOR)
  	H = H._narrow( SMESH.SMESH_Hypothesis );
 	aType = H.GetName();
  	aPixmap.SetPixMap( "mesh_tree_hypo.png_" + aType );
	anAttr = myStudyBuilder.FindOrCreateAttribute(newHypo, "AttributeIOR");
  	anIOR = anAttr._narrow(SALOMEDS.AttributeIOR);
  	anIOR.SetValue(IOR);
  	return newHypo.GetID();

#------------------------------------------------------------
def AddNewAlgorithms(IOR):
	res, AlgorithmsRoot = mySComponentMesh.FindSubObject (Tag_AlgorithmsRoot)
  	if  AlgorithmsRoot is None or res == 0:
    		AlgorithmsRoot = myStudyBuilde.NewObjectToTag (mySComponentMesh, Tag_AlgorithmsRoot)
    		anAttr = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributeName")
    		aName = anAttr._narrow(SALOMEDS.AttributeName);
    		aName.SetValue("Algorithms Definition");
    		anAttr = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributeSelectable")
    		aSelAttr = anAttr._narrow(SALOMEDS.AttributeSelectable);
    		aSelAttr.SetSelectable(0);
    		anAttr = myStudyBuilder.FindOrCreateAttribute(AlgorithmsRoot, "AttributePixMap");
    		aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap);
    		aPixmap.SetPixMap( "mesh_tree_algo.png" );

  # Add New Algorithms
  	newHypo = myStudyBuilder.NewObject(AlgorithmsRoot)
	newHypo = newHypo._narrow(SALOMEDS.SObject)
  	anAttr = myStudyBuilder.FindOrCreateAttribute(newHypo, "AttributePixMap");
  	aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap);
	H = orb.string_to_object(IOR)
  	H = H._narrow( SMESH.SMESH_Hypothesis);
  	aType = H.GetName();	#QString in fact
  	aPixmap.SetPixMap( "mesh_tree_algo.png_" + aType );
  	anAttr = myStudyBuilder.FindOrCreateAttribute(newHypo, "AttributeIOR");
  	anIOR = anAttr._narrow(SALOMEDS.AttributeIOR);
  	anIOR.SetValue(IOR);
 	return newHypo.GetID();


#------------------------------------------------------------
def SetShape(ShapeEntry, MeshEntry):

	SO_MorSM = myStudy.FindObjectID( MeshEntry )
	SO_MorSM = SO_MorSM._narrow(SALOMEDS.SObject)
  	SO_GeomShape = myStudy.FindObjectID( ShapeEntry );
	SO_GeomShape = SO_GeomShape._narrow(SALOMEDS.SObject)

  	if SO_MorSM is not None and SO_GeomShape is not None :
    		SO = myStudyBuilder.NewObjectToTag (SO_MorSM, Tag_RefOnShape);
		SO = SO._narrow(SALOMEDS.SObject)
    		myStudyBuilder.Addreference (SO,SO_GeomShape);
  

#------------------------------------------------------------
def SetHypothesis(Mesh_Or_SubMesh_Entry, Hypothesis_Entry):
  SO_MorSM = myStudy.FindObjectID( Mesh_Or_SubMesh_Entry );
  SO_Hypothesis =  myStudy.FindObjectID( Hypothesis_Entry );

  if  SO_MorSM is not None and SO_Hypothesis is not None : 
    
        #Find or Create Applied Hypothesis root
    	res, AHR = SO_MorSM.FindSubObject (Tag_RefOnAppliedHypothesis)
    	if  AHR is None or res == 0: 
      		AHR = myStudyBuilder.NewObjectToTag (SO_MorSM, Tag_RefOnAppliedHypothesis);
      		anAttr = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributeName");
      		aName = anAttr._narrow(SALOMEDS.AttributeName);
      		aName.SetValue("Applied Hypothesis");
      		anAttr = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributeSelectable");
      		aSelAttr = anAttr._narrow(SALOMEDS.AttributeSelectable);
      		aSelAttr.SetSelectable(0);
      		anAttr = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributePixMap");
      		aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap);
      		aPixmap.SetPixMap( "mesh_tree_hypo.png" );
    		
    		SO = myStudyBuilder.NewObject(AHR);
		SO = SO._narrow(SALOMEDS.SObject)
    		myStudyBuilder.Addreference (SO,SO_Hypothesis);

#------------------------------------------------------------
def SetAlgorithms(Mesh_Or_SubMesh_Entry, Algorithms_Entry):
    SO_MorSM = myStudy.FindObjectID( Mesh_Or_SubMesh_Entry )
    SO_Algorithms = myStudy.FindObjectID( Algorithms_Entry )
    if  SO_MorSM != None and SO_Algorithms != None : 
    	#Find or Create Applied Algorithms root
    	res, AHR = SO_MorSM.FindSubObject (Tag_RefOnAppliedAlgorithms);
    	if AHR is None or res == 0: 
      		AHR = myStudyBuilder.NewObjectToTag (SO_MorSM, Tag_RefOnAppliedAlgorithms);
      		anAttr = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributeName");
      		aName = anAttr._narrow(SALOMEDS.AttributeName);
      		aName.SetValue("Applied Algorithm");
      		anAttr = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributeSelectable");
      		aSelAttr = anAttr._narrow(SALOMEDS.AttributeSelectable);
      		aSelAttr.SetSelectable(0);
      		anAttr = myStudyBuilder.FindOrCreateAttribute(AHR, "AttributePixMap");
      		aPixmap = anAttr._narrow(SALOMEDS.AttributePixMap);
      		aPixmap.SetPixMap( "mesh_tree_algo.png" );
    
    	SO = myStudyBuilder.NewObject(AHR);
    	myStudyBuilder.Addreference (SO,SO_Algorithms);
  

#------------------------------------------------------------
def UnSetHypothesis( Applied_Hypothesis_Entry ):
  	SO_Applied_Hypothesis = myStudy.FindObjectID( Applied_Hypothesis_Entry );
  	if SO_Applied_Hypothesis : 
    		myStudyBuilder.RemoveObject(SO_Applied_Hypothesis);
	

#------------------------------------------------------------
def AddSubMesh ( SO_Mesh_Entry, SM_IOR, ST):
	SO_Mesh = myStudy.FindObjectID( SO_Mesh_Entry )
  	if ( SO_Mesh ) : 
    
    		if  ST == ShapeTypeCompSolid : 
			Tag_Shape = Tag_SubMeshOnSolid;    
			Name = "SubMeshes On Solid";
    		elif ST == ShapeTypeFace :
			Tag_Shape = Tag_SubMeshOnFace;     
			Name = "SubMeshes On Face";
    		elif ST == ShapeTypeEdge :
			Tag_Shape = Tag_SubMeshOnEdge;     
			Name = "SubMeshes On Edge";
    		elif ST == ShapeTypeVertex :
			Tag_Shape = Tag_SubMeshOnVertex;   
			Name = "SubMeshes On Vertex";
    		else :
      			Tag_Shape = Tag_SubMeshOnCompound; 
			Name = "SubMeshes On Compound";
    		
		res, SubmeshesRoot = SO_Mesh.FindSubObject (Tag_Shape)
    		if SubmeshesRoot is None or res == 0:
      			SubmeshesRoot = myStudyBuilder.NewObjectToTag (SO_Mesh, Tag_Shape);
      			anAttr = myStudyBuilder.FindOrCreateAttribute(SubmeshesRoot, "AttributeName");
			
      			aName = anAttr._narrow(SALOMEDS.AttributeName);
      			aName.SetValue(Name);
      			anAttr = myStudyBuilder.FindOrCreateAttribute(SubmeshesRoot, "AttributeSelectable");
      			aSelAttr = anAttr._narrow(SALOMEDS.AttributeSelectable);
      			aSelAttr.SetSelectable(0);
    		
    		SO = myStudyBuilder.NewObject (SubmeshesRoot); 
		SO = SO._narrow(SALOMEDS.SObject)
    		anAttr = myStudyBuilder.FindOrCreateAttribute(SO, "AttributeIOR");
    		anIOR = anAttr._narrow(SALOMEDS.AttributeIOR);
    		anIOR.SetValue(SM_IOR);
    		return  SO.GetID();
 	 
  	return None;

#------------------------------------------------------------
def AddSubMeshOnShape (Mesh_Entry, GeomShape_Entry, SM_IOR, ST) :
  SO_GeomShape = myStudy.FindObjectID( GeomShape_Entry );
  if  SO_GeomShape != None : 
    	SM_Entry = AddSubMesh (Mesh_Entry,SM_IOR,ST);
    	SO_SM = myStudy.FindObjectID( SM_Entry );

    	if  SO_SM != None :
      		SetShape (GeomShape_Entry, SM_Entry);
      		return SO_SM.GetID();
    
  return None;


#------------------------------------------------------------
def SetName(Entry, Name):
	SO = myStudy.FindObjectID( Entry );
  	if SO != None : 
   		anAttr = myStudyBuilder.FindOrCreateAttribute(SO, "AttributeName");
    		aName = anAttr._narrow(SALOMEDS.AttributeName);
    		aName.SetValue(Name);
  

