
mkdir temp
rm -f temp/*
rm -rf html
cp ../../src/baselibrary/base/* ../../src/fontengines/**/* ../../src/baselibrary/components/* ../../src/gfxdevices/**/* ../../src/baselibrary/sizebrokers/* ../../src/baselibrary/skins/* ../../src/baselibrary/skins/**/* ../../src/baselibrary/stream/* ../../src/baselibrary/textlayouts/* ../../src/baselibrary/numberlayouts/* ../../src/baselibrary/widgets/* ../../src/baselibrary/widgets/**/* temp
ruby ../../scripts/predoxy.rb temp/*.h
doxygen
ruby ../../scripts/postdoxy.rb html/class*.html
