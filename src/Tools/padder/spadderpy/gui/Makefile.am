include $(top_srcdir)/adm_local/unix/make_common_starter.am

#
# Files that compose the spadder graphical interface used by the
# plugin. They are installed in the spadder python package, under the
# sub-package plugins.
#
spadderpydir=$(smeshpypkgdir)/spadder/gui
dist_spadderpy_PYTHON = \
	__init__.py \
	plugindialog.py \
	inputdialog.py \
	inputdata.py

PYUIC_FILES = \
	plugindialog_ui.py \
	inputframe_ui.py

nodist_spadderpy_PYTHON = $(PYUIC_FILES)
CLEANFILES = $(PYUIC_FILES)

dist_spadderpy_DATA= \
	parameters.png \
	input.png \
	select.png \
	compute.png \
	refresh.png \
	publish.png \
	clear.png \
	addinput.png \
	deleteinput.png \
	concrete.png \
	steelbar.png

PYUIC = @PYUIC@
%_ui.py:%.ui
	$(PYUIC) -x $< -o $@

PYRCC = @PYRCC@
%_rc.py:%.qrc
	$(PYRCC) $< -o $@

EXTRA_DIST += $(PYUIC_FILES:%_ui.py=%.ui)
