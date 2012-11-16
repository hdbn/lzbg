////////////////////////////////////////////////////////////////////////////////
// bgCommon.cpp
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

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include "bgCommon.hpp"
#include "divsufsort.h"

#define PSV(i) pnsv[(i << 1)]
#define NSV(i) pnsv[(i << 1)+1]

#define LPS(i) lpspo[(i << 1)]
#define PREVOCC(i) lpspo[(i << 1)+1]

namespace LZBG {

  bool checkResult = false;

  ////////////////////////////////////////////////////////////
  // parse options and read/construct string & suffix array
  ////////////////////////////////////////////////////////////
  
  int * Init(int argc, char * argv[], std::string & s, unsigned int f, int * sa){
    int ch;
    std::string inFile, saFile;
    bool useSAcache = false;
    while ((ch = getopt(argc, argv, "f:xgh")) != -1) {
      switch (ch) {
      case 'f':
	inFile = optarg;
	break;
      case 'x':
	useSAcache = true;
	break;
      case 'g':
	checkResult = true;
	break;
      default:
	print_usage(argc, argv);
	exit(0);
      }
    }
    if(inFile.empty()){ print_usage(argc, argv); exit(0); }
    
    argc -= optind;
    argv += optind;
    ////////////////////////////////////////////////////////////
    
    stringFromFile(inFile, s);
    if(useSAcache){
      sa = saFromFile(s, inFile, sa, f);
    } else {
      sa = suffixArray(s, sa, f);
    }
    return(sa);
  }

  void print_usage(int argc, char * argv []){
    std::cout << "Usage  : " << argv[0] << " [options]" << std::endl
	      << "Options: " << std::endl
	      << "  -f iFile : file to process" << std::endl
	      << "  -x       : use iFile + '.sa' for suffix array cache"
	      << "  -g       : check if resulting factorization produces input string"
	      << std::endl;
    return;
  }

  ////////////////////////////////////////////////////////////
  //read input string from file
  ////////////////////////////////////////////////////////////
  void stringFromFile(const std::string & fileName, std::string & s){
    struct stat st;
    size_t fileSize;
    if(stat(fileName.c_str(), &st)){
      std::cerr << "failed to stat file: " << fileName << std::endl;
      return;
    }
    std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary);
    if(!ifs){
      std::cerr << "failed to read file: " << fileName << std::endl;
      return;
    }
    fileSize = st.st_size;
    if(fileSize != static_cast<size_t>(static_cast<int>(fileSize))){
      std::cerr << "ERROR: The file size is too big to fit in int. Cannot process." << std::endl;
      return;
    }
    s.resize(fileSize);
    ifs.read(reinterpret_cast<char*>(&s[0]), fileSize);
  }


  int * suffixArray(const std::string & s, int * sa, unsigned int f){
    if(sa == 0){
      size_t sasize = (f & DOUBLE_SA) ? s.size() * 2 : s.size();
      sa = new int[sasize];
    }
    std::cerr << "Building suffix array..." << std::flush;
    divsufsort(reinterpret_cast<const unsigned char *>(s.c_str()), sa, s.size());
    std::cerr << "done" << std::endl;
    return (sa);
  }

  int * saFromFile(const std::string & s, const std::string & fname, int * sa, unsigned int f){
    ////////////////////////////////////////////////////////////
    // check if suffix array file if it exists
    ////////////////////////////////////////////////////////////
    bool remake = false;
    struct stat st1, st2;
    std::string safname = fname + ".sa";
    if(stat(fname.c_str(), &st1)) remake = true;
    if(stat(safname.c_str(), &st2)) remake = true;
    if(st1.st_mtime >= st2.st_mtime){
      remake = true;
    }
    std::ifstream sfs(safname.c_str(), std::ios::in | std::ios::binary);
    if(!sfs){
      remake = true;
    } else if(sfs){
      const size_t fileSize = st2.st_size;
      if (fileSize != sizeof(int) * s.size()){
	remake = true;
      } else {
	std::cerr << "reading suffix array from: " << safname << std::flush;
	if(sa == 0){
	  size_t sasize = (f & DOUBLE_SA) ? (fileSize / sizeof(int)) * 2 : fileSize / sizeof(int);
	  sa = new int[sasize];
	}
	sfs.read(reinterpret_cast<char*>(sa), fileSize);
	std::cerr << " ...done" << std::endl;
      }
    }
    ////////////////////////////////////////////////////////////
    // construct suffix array and cache it to file
    ////////////////////////////////////////////////////////////
    if(remake){
      std::cerr << "suffix array file: " << safname << " not found, invalid or out of date." << std::endl;
      sa = suffixArray(s, sa, f);
      std::cerr << "Saving suffix array to file..." << std::flush;
      std::ofstream ofs(safname.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
      ofs.write(reinterpret_cast<const char*>(sa), sizeof(int) * s.size());
      std::cerr << "done" << std::endl;
    }
    return(sa);
  }

  inline int naiveLCP(const char * const x, int i, int j, int n){
    int l = 0;
    while(j < n && x[i++] == x[j++]){ l++; }
    return l;
  }
  
  void lzFromTOPNSV(const std::string & s, 
		    const int * psv,
		    const int * nsv,
		    std::vector<std::pair<int,int> > & lz){
    
    ////////////////////////////////////////////////////////////
    // main LZ factorization
    ////////////////////////////////////////////////////////////
    size_t p = 1;
    lz.clear();
    lz.push_back(std::make_pair(0, s[0]));
    while(p < s.size()){
      int prevPos = psv[p];
      int lpf = (psv[p] < 0) ? 0 : naiveLCP(s.c_str(), psv[p], p, s.size());
      int nlen = (nsv[p] < 0) ? 0 : naiveLCP(s.c_str(), nsv[p], p, s.size());
      if(nlen > lpf){ lpf = nlen; prevPos = nsv[p]; }
      if(lpf > 0){
	lz.push_back(std::make_pair(lpf,prevPos));
	p += lpf;
      } else {
	lz.push_back(std::make_pair(0, s[p]));
	p++;
      }
    }
    return;
    ////////////////////////////////////////////////////////////
  }

  void lzFromTOPNSV(const std::string & s, 
		    const int * pnsv,
		    std::vector<std::pair<int,int> > & lz){
    
    ////////////////////////////////////////////////////////////
    // main LZ factorization
    ////////////////////////////////////////////////////////////
    size_t p = 1;
    lz.clear();
    lz.push_back(std::make_pair(0, s[0]));
    while(p < s.size()){
      int prevPos = PSV(p);
      int lpf = (PSV(p) < 0) ? 0 : naiveLCP(s.c_str(), PSV(p), p, s.size());
      int nlen = (NSV(p) < 0) ? 0 : naiveLCP(s.c_str(), NSV(p), p, s.size());
      if(nlen > lpf){ lpf = nlen; prevPos = NSV(p); }
      if(lpf > 0){
	lz.push_back(std::make_pair(lpf,prevPos));
	p += lpf;
      } else {
	lz.push_back(std::make_pair(0, s[p]));
	p++;
      }
    }
    return;
    ////////////////////////////////////////////////////////////
  }

  void lzFromLOPNSV(const std::string & s, 
		    const int * sa,
		    const int * rank,
		    const int * psv,
		    const int * nsv,
		    std::vector<std::pair<int,int> > & lz){
    
    ////////////////////////////////////////////////////////////
    // main LZ factorization
    ////////////////////////////////////////////////////////////
    size_t p = 1;
    lz.clear();
    lz.push_back(std::make_pair(0, s[0]));
    while(p < s.size()){
      int i = rank[p];
      int prevPos = sa[psv[i]];
      int lpf = (psv[i] < 0) ? 0 : naiveLCP(s.c_str(), sa[psv[i]], p, s.size());
      int nlen = (nsv[i] < 0) ? 0 : naiveLCP(s.c_str(), sa[nsv[i]], p, s.size());
      if(nlen > lpf){ lpf = nlen; prevPos = sa[nsv[i]]; }
      if(lpf > 0){
	lz.push_back(std::make_pair(lpf,prevPos));
	p += lpf;
      } else {
	lz.push_back(std::make_pair(0, s[p]));
	p++;
      }
    }
    return;
    ////////////////////////////////////////////////////////////
  }

  void lzFromLOPNSVtp(const std::string & s, 
		      const int * sa,
		      const int * rank,
		      const int * psv,
		      const int * nsv,
		      std::vector<std::pair<int,int> > & lz){
    
    ////////////////////////////////////////////////////////////
    // main LZ factorization
    ////////////////////////////////////////////////////////////
    size_t p = 1;
    lz.clear();
    lz.push_back(std::make_pair(0, s[0]));
    while(p < s.size()){
      int i = rank[p];
      int prevPos = psv[i];
      int lpf = (psv[i] < 0) ? 0 : naiveLCP(s.c_str(), psv[i], p, s.size());
      int nlen = (nsv[i] < 0) ? 0 : naiveLCP(s.c_str(), nsv[i], p, s.size());
      if(nlen > lpf){ lpf = nlen; prevPos = nsv[i]; }
      if(lpf > 0){
	lz.push_back(std::make_pair(lpf,prevPos));
	p += lpf;
      } else {
	lz.push_back(std::make_pair(0, s[p]));
	p++;
      }
    }
    return;
    ////////////////////////////////////////////////////////////
  }

  void lzFromLOPNSV(const std::string & s, 
		    const int * sa,
		    const int * rank,
		    const int * pnsv,
		    std::vector<std::pair<int,int> > & lz){
    
    ////////////////////////////////////////////////////////////
    // main LZ factorization
    ////////////////////////////////////////////////////////////
    size_t p = 1;
    lz.clear();
    lz.push_back(std::make_pair(0, s[0]));
    while(p < s.size()){
      int i = rank[p];
      int prevPos = sa[PSV(i)];
      int lpf = (PSV(i) < 0) ? 0 : naiveLCP(s.c_str(), sa[PSV(i)], p, s.size());
      int nlen = (NSV(i) < 0) ? 0 : naiveLCP(s.c_str(), sa[NSV(i)], p, s.size());
      if(nlen > lpf){ lpf = nlen; prevPos = sa[NSV(i)]; }
      if(lpf > 0){
	lz.push_back(std::make_pair(lpf,prevPos));
	p += lpf;
      } else {
	lz.push_back(std::make_pair(0, s[p]));
	p++;
      }
    }
    return;
    ////////////////////////////////////////////////////////////
  }

  void lzFromLOPNSVtp(const std::string & s, 
		      const int * sa,
		      const int * rank,
		      const int * pnsv,
		      std::vector<std::pair<int,int> > & lz){
    
    ////////////////////////////////////////////////////////////
    // main LZ factorization
    ////////////////////////////////////////////////////////////
    size_t p = 1;
    lz.clear();
    lz.push_back(std::make_pair(0, s[0]));
    while(p < s.size()){
      int i = rank[p];
      int prevPos = PSV(i);
      int lpf = (PSV(i) < 0) ? 0 : naiveLCP(s.c_str(), PSV(i), p, s.size());
      int nlen = (NSV(i) < 0) ? 0 : naiveLCP(s.c_str(), NSV(i), p, s.size());
      if(nlen > lpf){ lpf = nlen; prevPos = NSV(i); }
      if(lpf > 0){
	lz.push_back(std::make_pair(lpf,prevPos));
	p += lpf;
      } else {
	lz.push_back(std::make_pair(0, s[p]));
	p++;
      }
    }
    return;
    ////////////////////////////////////////////////////////////
  }

  void lzFromTOLPFPO(const std::string & s,
		     const int * lps,
		     const int * prevOcc,
		     std::vector<std::pair<int,int> > & lz){
    size_t p = 1;
    lz.clear();
    lz.push_back(std::make_pair(0, s[0]));
    while(p < s.size()){
      if( lps[p] < 1 ){
	lz.push_back(std::make_pair(0, s[p]));
	p++;
      } else {
	lz.push_back(std::make_pair(lps[p], prevOcc[p]));
	p += lps[p];
      }
    }
  }

  void lzFromTOLPFPO(const std::string & s,
		     const int * lpspo,
		     std::vector<std::pair<int,int> > & lz){
    size_t p = 1;
    lz.clear();
    lz.push_back(std::make_pair(0, s[0]));
    while(p < s.size()){
      if( LPS(p) < 1 ){
	lz.push_back(std::make_pair(0, s[p]));
	p++;
      } else {
	lz.push_back(std::make_pair(LPS(p), PREVOCC(p)));
	p += LPS(p);
      }
    }
  }

  std::string lz2str(const std::vector<std::pair<int,int> > & lz){
    std::string s;
    size_t i;
    int j;
    for(i = 0; i < lz.size(); i++){
      if(lz[i].first == 0){
	s.push_back(static_cast<char>(lz[i].second));
      } else {
	for(j = 0; j < lz[i].first; j++){
	  s.push_back(s[lz[i].second+j]);
	}
      }
    }
    return(s);
  }

  double gettime(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((double)tv.tv_sec)
	    + ((double)tv.tv_usec)*1e-6);
  }
}
