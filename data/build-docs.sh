#!/bin/bash

set -e

mkdir doc-temp || true
cd doc-temp

gdbus-codegen --generate-docbook=docbook ../registry.xml ../client.xml ../store.xml ../backend.xml

find . -name "docbook*.xml" -exec docbook2x-texi {} \;

find . -name "com.canonical.*.texi" -exec texi2pdf --quiet {} \;

pdfunite com.canonical.*.pdf ../dbus-interface.pdf
