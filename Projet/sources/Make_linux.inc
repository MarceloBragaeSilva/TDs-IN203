CC = gcc
CXX = g++-10
LIB =
CXXFLAGS=`mpic++ --showme:compile`
LIBS+=`mpic++ --showme:link`