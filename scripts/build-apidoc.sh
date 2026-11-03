#!/bin/sh

PACKAGE="wg-doc-api-$1.zip"
ROOT="$PWD"

cd wondergui/doc/api-reference
./make_docs.bat
rm -f "$ROOT/$PACKAGE"
zip -r "$ROOT/$PACKAGE" html

