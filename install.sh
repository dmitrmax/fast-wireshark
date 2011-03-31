
sed -i 's/subdirs (util)/# subdirs (util)/g' CMakeLists.txt
cmake .
make clean install