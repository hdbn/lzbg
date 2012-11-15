////////////////////////////////////////////////////////////////////////////////
// bgCommon.hpp
//   common routines for lz factorization BGS, iBGS, BGL, iBGL, BGT, iBGT
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

#ifndef __LZBG_COMMON_HPP__
#define __LZBG_COMMON_HPP__

#include <string>
#include <vector>

namespace LZBG {

  extern bool checkResult;
  
  enum FLAGS {
    DOUBLE_SA = 1 // allocate double required memory for suffix array
  };
  
  // print usage information
  void print_usage(int argc, char * argv []);

  // parse options and read/construct string & suffix array
  // allocates memory if sa == 0. return memory for sa.
  int * Init(int argc, char * argv[], std::string & s, 
	     unsigned int f = 0, int *sa = 0);

  // read file fileName into string s.
  void stringFromFile(const std::string & fileName, std::string & s);

  // calculate suffix array sa from s. allocates memory if sa == 0
  // return *sa
  int * suffixArray(const std::string & s, int * sa, unsigned int f);

  // read suffix array of string s from fname + '.sa' into sa.
  // if safname does not exist or seems invalid, create suffix array and save it
  // to fname + '.sa'
  // allocates memory if *sa == 0
  // return *sa
  int * saFromFile(const std::string & s, const std::string & fname, 
		   int * sa, unsigned int f);


  ////////////////////////////////////////////////////////////////////////////////
  // computing lz from PSV, NSV in text order
  ////////////////////////////////////////////////////////////////////////////////
  void lzFromTOPNSV(const std::string & s, 
		    const int * psv,
		    const int * nsv,
		    std::vector<std::pair<int,int> > & lz);

  // interleaving version
  void lzFromTOPNSV(const std::string & s, 
		    const int * pnsv,
		    std::vector<std::pair<int,int> > & lz);

  ////////////////////////////////////////////////////////////////////////////////
  // computing lz from PSV, NSV in lex order
  ////////////////////////////////////////////////////////////////////////////////
  void lzFromLOPNSV(const std::string & s,
		    const int * sa,
		    const int * rank,
		    const int * psv,
		    const int * nsv,
		    std::vector<std::pair<int,int> > & lz);

  // values of psv and nsv as text positions
  void lzFromLOPNSVtp(const std::string & s,
		      const int * sa,
		      const int * rank,
		      const int * psv,
		      const int * nsv,
		      std::vector<std::pair<int,int> > & lz);

  // interleaving version of lzFromLOPNSV
  void lzFromLOPNSV(const std::string & s,
		    const int * sa,
		    const int * rank,
		    const int * pnsv,
		    std::vector<std::pair<int,int> > & lz);

  // interleaving version of lzFromLOPNSVtp
  void lzFromLOPNSVtp(const std::string & s,
		      const int * sa,
		      const int * rank,
		      const int * pnsv,
		      std::vector<std::pair<int,int> > & lz);

  ////////////////////////////////////////////////////////////////////////////////
  // computing lz from LPF and PrevOcc in text order
  ////////////////////////////////////////////////////////////////////////////////
  void lzFromTOLPFPO(const std::string & s,
		     const int * lps,
		     const int * prevOcc,
		      std::vector<std::pair<int,int> > & lz);

  // interleaving version
  void lzFromTOLPFPO(const std::string & s,
		     const int * lpspo,
		     std::vector<std::pair<int,int> > & lz);

  // recover string from lz factorization
  std::string lz2str(const std::vector<std::pair<int,int> > & lz);
  double gettime();
};
#endif//__LZBG_COMMON_HPP__
