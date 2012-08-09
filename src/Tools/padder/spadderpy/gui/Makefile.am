# Copyright (C) 2011-2012  EDF R&D
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

%_ui.py:%.ui
	$(PYUIC) -x $< -o $@

%_rc.py:%.qrc
	$(PYRCC) $< -o $@

EXTRA_DIST += $(PYUIC_FILES:%_ui.py=%.ui)
