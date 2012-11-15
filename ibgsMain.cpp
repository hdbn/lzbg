////////////////////////////////////////////////////////////////////////////////
// ibgsMain.cpp
//   lz factorization via PSV_lex and NSV_lex using stack
//   uses 17N bytes + stack space
//   PSV/NSV values are interleaved in a single array.
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
#include <stack>

#define PSV(i) pnsv[(i << 1)]
#define NSV(i) pnsv[(i << 1)+1]

using namespace LZBG;

void calcPNSV(const int * sa, int n, int * pnsv){
  int i;
  std::stack<int,std::vector<int> > S;

  for(i = 0; i < n; i++){
    const int x = sa[i];
    while(!S.empty() && sa[S.top()] > x){       // pop while new element is smaller
      NSV(S.top()) = x; S.pop();
    }
    PSV(i) = S.empty() ? -1 : sa[S.top()];
    S.push(i);
  }
  while(!S.empty()){
    NSV(S.top()) = -1; S.pop();
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
  int * rank = new int[n], * pnsv = new int[2*n];

  double t2 = gettime();
  for(i = 0; i < n; i++) rank[sa[i]] = i;
  std::cout << "Time for rank: " << gettime() - t1 << std::endl;
  t2 = gettime();
  calcPNSV(sa, n, pnsv);
  std::cout << "Time for pnsv: " << gettime() - t2 << std::endl;

  ////////////////////////////////////////////////////////////
  // calculate LZ factorization from text order PSV, NSV
  ////////////////////////////////////////////////////////////
  std::vector<std::pair<int,int> > lz;
  t2 = gettime();
  lzFromLOPNSVtp(s, sa, rank, pnsv, lz);
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
