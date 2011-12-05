include $(top_srcdir)/adm_local/unix/make_common_starter.am

SUBDIRS = 

if SMESH_ENABLE_GUI
SUBDIRS += gui plugin
endif

DIST_SUBDIRS = gui plugin

spadderpydir=$(smeshpypkgdir)/spadder
spadderpy_PYTHON = \
	__init__.py \
	configreader.py

salomeplugins_DATA = \
	padder.cfg
