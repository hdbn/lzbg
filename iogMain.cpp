////////////////////////////////////////////////////////////////////////////////
// iogMain.cpp
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

#define LPS(i) lpspo[(i << 1)]
#define PREVOCC(i) lpspo[(i << 1)+1]

void sop(int i, int l, int j, int *lpspo){
  if(LPS(i) == -1){
    LPS(i) = l;
    PREVOCC(i) = j;
  } else {
    if(LPS(i) < l){
      if(PREVOCC(i) > j)
	sop(PREVOCC(i), LPS(i), j, lpspo);
      else
	sop(j, LPS(i), PREVOCC(i), lpspo);
      LPS(i) = l;
      PREVOCC(i) = j;
    } else {
      if(PREVOCC(i) > j)
	sop(PREVOCC(i), l, j, lpspo);
      else
	sop(j, l, PREVOCC(i), lpspo);
    }
  }
}

int main(int argc, char * argv[]){

  std::string s;

  // parse options and read/construct string & suffix array
  int * sa = Init(argc, argv, s, DOUBLE_SA);
  
  ////////////////////////////////////////////////////////////
  // calculate text order PSV, NSV using phi
  ////////////////////////////////////////////////////////////
  double t1 = gettime();
  int n = s.size(), i;
 
  int * phi = new int[n];
  int * lpspo = sa; // reuse suffix array

  double t2 = gettime();
  phi[sa[0]] = -1;
  for(i = 1; i < n; i++) phi[sa[i]] = sa[i-1];
  std::cout << "Time for phi: " << gettime() - t1 << std::endl;
  t2 = gettime();
  for(i = 0; i < n; i++) LPS(i) = -1;

  const char * x = s.c_str();
  int l = 0;
  for(i = 0; i < n; i++){
    int j = phi[i];
    if(j >= 0){
      int p1 = i + l, p2 = j + l;
      while((p1 < n) && (p2 < n) && (x[p1++] == x[p2++])) l++;
    }
    if(i > j)
      sop(i, l, j, lpspo);
    else
      sop(j, l, i, lpspo);
    l = (l > 0) ? l - 1 : 0;
  }
  LPS(0) = 0;
  std::cout << "Time for lps/prevOcc: " << gettime() - t2 << std::endl;

  ////////////////////////////////////////////////////////////
  // calculate LZ factorization from text order PSV, NSV
  ////////////////////////////////////////////////////////////
  std::vector<std::pair<int,int> > lz;
  t2 = gettime();
  lzFromTOLPFPO(s, lpspo, lz);
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
