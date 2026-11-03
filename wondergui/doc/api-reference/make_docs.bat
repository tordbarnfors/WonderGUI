
mkdir -p temp
rm -f temp/*
rm -rf html
find ../../source ../../../wax/source ../../../wax/extensions -type d \( -name md4c -o -name c-api \) -prune -o -type f \( -name '*.h' -o -name '*.cpp' \) ! -name 'wax_c_*' -exec cp {} temp \;
ruby ../../../scripts/predoxy.rb temp/*.h
doxygen
ruby ../../../scripts/postdoxy.rb html/class*.html
