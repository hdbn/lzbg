////////////////////////////////////////////////////////////////////////////////
// bglMain.cpp
//   lz factorization via PSV_lex and NSV_lex using peak elimination
//   uses 17N bytes space
////////////////////////////////////////////////////////////////////////////////
// Copyright 2012 Hideo Bannai & Keisuke Goto
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
////////////////////////////////////////////////////////////////////////////////

#include "bgCommon.hpp"
#include <iostream>
#include <fstream>

using namespace LZBG;

// this is a tail recursive call
void peakElim(int p, int c, int *psv, int *nsv, int *sa){  
  if(p < 0 || sa[p] < sa[c]){ 
    psv[c] = p;
  } else { // sa[p] > sa[c] // p is peak
    nsv[p] = c;
    peakElim(psv[p], c, psv, nsv, sa);
  }
}

int main(int argc, char * argv[]){

  std::string s;

  // parse options and read/construct string & suffix array
  int * sa = Init(argc, argv, s);
  
  ////////////////////////////////////////////////////////////
  // calculate text order PSV, NSV using phi
  ////////////////////////////////////////////////////////////
  double t1 = gettime();
  int n = s.size(), i;
  int * rank = new int[n], * psv = new int[n], * nsv = new int[n];

  double t2 = gettime();
  for(i = 0; i < n; i++) rank[sa[i]] = i;
  std::cout << "Time for rank: " << gettime() - t1 << std::endl;
  t2 = gettime();
  for(int i = 0; i < n; i++) nsv[i] = -1;
  psv[0] = -1;
  for(int i = 1; i < n; i++) peakElim(i-1, i, psv, nsv, sa);

  std::cout << "Time for pnsv: " << gettime() - t2 << std::endl;

  ////////////////////////////////////////////////////////////
  // calculate LZ factorization from text order PSV, NSV
  ////////////////////////////////////////////////////////////
  std::vector<std::pair<int,int> > lz;
  t2 = gettime();
  lzFromLOPNSV(s, sa, rank, psv, nsv, lz);
  std::cout << "Time for lz: " << gettime() - t2 << std::endl;
  std::cout << "# of lz factors: " << lz.size() << std::endl;
  std::cout << "Total: " << gettime() - t1 << std::endl;
  if(checkResult){
    std::string t = lz2str(lz);
    if(s != t) std::cerr << "CHECK: ERROR: mismatch" << std::endl;
    else std::cerr << "CHECK: OK" << std::endl;
  }
  return 0;
}
