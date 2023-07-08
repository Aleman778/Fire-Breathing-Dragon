@echo off

IF NOT EXIST run_tree mkdir run_tree
pushd run_tree

rem Temporary should not use absolute path
C:\Dev\sqrrl\build\sqrrl.exe ../code/game.cpp

popd