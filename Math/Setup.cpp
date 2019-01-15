// (C) 2018 University of Bristol. See License.txt


#include "Math/Setup.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"

#include "Tools/mkpath.h"

#include <fstream>

string get_prep_dir(int nparties, int lg2p, int gf2ndegree)
{
  if (gf2ndegree == 0)
    gf2ndegree = gf2n::default_length();
  stringstream ss;
  ss << PREP_DIR << nparties << "-" << lg2p << "-" << gf2ndegree << "/";
  return ss.str();
}
