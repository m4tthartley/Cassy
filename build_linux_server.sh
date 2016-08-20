
clear

pushd build

clang -g ../server.cc -o server -Wno-null-dereference

popd