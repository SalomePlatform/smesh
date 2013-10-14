PYUIC = pyuic4
.PHONY : all
.SUFFIXES : .ui .py


PY_FILES = Gui/desFenetreChoix.py Gui/myMain.py Gui/nomBase.py

%.py:%.ui
	${PYUIC} -x -o $@ $<

all : $(PY_FILES)
clean :
	-rm -rf $(PY_FILES) *.pyc 
cleandb:
	-rm -rf myMesh.db
veryclean :
	-rm -rf $(PY_FILES) *.pyc  myMesh.db

