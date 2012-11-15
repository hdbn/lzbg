////////////////////////////////////////////////////////////////////////////////
// bgtMain.cpp
//   lz factorization via PSV_text and NSV_text using peak elimination
//   uses 13N bytes space
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

void peakElim(int j, int i, int * psv, int * nsv, int bot){
  if(j < i){
    psv[i] = j;
    if(nsv[i] != bot){
      peakElim(j, nsv[i], psv, nsv, bot);
    }
  } else {
    nsv[j] = i;
    if(psv[j] != bot){
      peakElim(psv[j], i, psv, nsv, bot);
    }
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
  // std::vector<int> phi(n), psv(n);
  int * phi = new int[n], * psv = new int[n];
  int * nsv = sa; // reuse suffix array

  double t2 = gettime();
  phi[sa[0]] = sa[n-1];
  for(i = 1; i < n; i++) phi[sa[i]] = sa[i-1];
  std::cout << "Time for phi: " << gettime() - t1 << std::endl;
  t2 = gettime();
  for(i = 0; i < n; i++) psv[i] = nsv[i] = -1;
  for(i = 0; i < n; i++) peakElim(phi[i], i, psv, nsv, -1);
  std::cout << "Time for pnsv: " << gettime() - t2 << std::endl;

  ////////////////////////////////////////////////////////////
  // calculate LZ factorization from text order PSV, NSV
  ////////////////////////////////////////////////////////////
  std::vector<std::pair<int,int> > lz;
  t2 = gettime();
  lzFromTOPNSV(s, psv, nsv, lz);
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
