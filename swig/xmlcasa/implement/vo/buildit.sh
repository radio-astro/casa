#!/bin/bash
swig -Wall -c++ -python vo.i
clang++ -c vo_cmpt.cc vo_wrap.cxx -I/opt/casa/active/darwinllvm/include/casacore -I/opt/casa/active/code/include -I/opt/casa/darwin11/Library/Frameworks/Python.framework/Headers -I. -I/Users/wyoung/VAO/vo-cli/include
clang++ -shared *.o -o _vo.so -L/Users/wyoung/VAO/vo-cli/lib -lvoclient -F/opt/casa/darwin11/Library/Frameworks/ -framework Python
