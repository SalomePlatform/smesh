// const strings
var gaProj = new Array();
var gsRoot = "";

function setRoot(sRoot)
{
	gsRoot = sRoot
}

function aPE(sProjPath, sRootPath)
{
	gaProj[gaProj.length] = new tocProjEntry(sProjPath, sRootPath);
}

function tocProjEntry(sProjPath, sRootPath) 
{
	this.sPPath = sProjPath;
	this.sRPath = sRootPath;
}


function window_OnLoad()
{
	if (parent && parent != this && parent.projReady) {
		parent.projReady(gsRoot, gaProj);
	}
}
window.onload = window_OnLoad;