@ECHO OFF

clang-cl test.cpp /nologo /Zi /fsanitize=address /EHsc /O2
