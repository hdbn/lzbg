////////////////////////////////////////////////////////////////////////////////
// ogMain.cpp
//   lz factorization via PSV_text and NSV_text using peak elimination
//   uses 13N bytes space
//
//   based on the pseudo-code/algorithm in the paper:
//     E. Ohlebusch & S. Gog, "Lempel-Ziv Factorization Revisited",
//     In Proc. CPM 2011, LNCS 6661:15-26, 2011.
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

void sop(int i, int l, int j, int *lps, int *prevOcc){
  if(lps[i] == -1){
    lps[i] = l;
    prevOcc[i] = j;
  } else {
    if(lps[i] < l){
      if(prevOcc[i] > j)
	sop(prevOcc[i], lps[i], j, lps, prevOcc);
      else
	sop(j, lps[i], prevOcc[i], lps, prevOcc);
      lps[i] = l;
      prevOcc[i] = j;
    } else {
      if(prevOcc[i] > j)
	sop(prevOcc[i], l, j, lps, prevOcc);
      else
	sop(j, l, prevOcc[i], lps, prevOcc);
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
 
  int * phi = new int[n], * prevOcc = new int[n];
  int * lps = sa; // reuse suffix array

  double t2 = gettime();
  phi[sa[0]] = -1;
  for(i = 1; i < n; i++) phi[sa[i]] = sa[i-1];
  std::cout << "Time for phi: " << gettime() - t1 << std::endl;
  t2 = gettime();
  for(i = 0; i < n; i++) lps[i] = -1;

  const char * x = s.c_str();
  int l = 0;
  for(i = 0; i < n; i++){
    int j = phi[i];
    if(j >= 0){
      int p1 = i + l, p2 = j + l;
      while((p1 < n) && (p2 < n) && (x[p1++] == x[p2++])) l++;
    }
    if(i > j)
      sop(i, l, j, lps, prevOcc);
    else
      sop(j, l, i, lps, prevOcc);
    l = (l > 0) ? l - 1 : 0;
  }
  lps[0] = 0;
  std::cout << "Time for lps/prevOcc: " << gettime() - t2 << std::endl;

  ////////////////////////////////////////////////////////////
  // calculate LZ factorization from text order PSV, NSV
  ////////////////////////////////////////////////////////////
  std::vector<std::pair<int,int> > lz;
  t2 = gettime();
  lzFromTOLPFPO(s, lps, prevOcc, lz);
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
