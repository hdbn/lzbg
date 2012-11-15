#
# This file is a scons build script for lz factorization programs
# BGS, iBGS, BGL, iBGL, BGT, iBGT
# Copyright 2012 Hideo Bannai & Keisuke Goto
# 

import os, sys, glob

env = Environment(CC="gcc", CXX="g++", 
                  CCFLAGS="-Wall -fast -DNDEBUG -msse4.2",
                  LINKFLAGS="-fast -msse4.2"
                  )

sources_common = ['bgCommon.cpp', 'divsufsort.c']
objects_common = env.Object(sources_common)

def makeprog(onlyfor_sources, progname):
    objects_onlyfor = env.Object(onlyfor_sources)
    env.Program(progname, objects_common + objects_onlyfor)

progs = [
    ['bgtMain.cpp', 'lzBGT'],
    ['ibgtMain.cpp', 'lziBGT'],
    ['bglMain.cpp', 'lzBGL'],
    ['ibglMain.cpp', 'lziBGL'],
    ['bgsMain.cpp', 'lzBGS'],
    ['ibgsMain.cpp', 'lziBGS'],
    ['ogMain.cpp', 'lzOG'],
    ['iogMain.cpp', 'lziOG']

    ]

for fin, fout in progs:
    makeprog(fin, fout)
