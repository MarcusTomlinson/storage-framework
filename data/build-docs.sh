#!/bin/bash

#
# Copyright (C) 2016 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authors: James Henstridge <james.henstridge@canonical.com>
#

set -e

mkdir doc-temp || true
cd doc-temp

gdbus-codegen --generate-docbook=docbook ../registry.xml ../provider.xml

find . -name "docbook*.xml" -exec docbook2x-texi {} \;

find . -name "com.canonical.*.texi" -exec texi2pdf --quiet {} \;

pdfunite com.canonical.*.pdf ../dbus-interface.pdf
