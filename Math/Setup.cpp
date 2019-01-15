// (C) 2018 University of Bristol. See License.txt


#include "Math/Setup.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"

#include "Tools/mkpath.h"

#include <fstream>

void write_online_setup(ofstream& outf, string dirname, const bigint& p, int lg2)
{
  if (p == 0)
    throw runtime_error("prime cannot be 0");

  stringstream ss;
  ss << dirname;
  cout << "Writing to file in " << ss.str() << endl;
  // create preprocessing dir. if necessary
  if (mkdir_p(ss.str().c_str()) == -1)
  {
    cerr << "mkdir_p(" << ss.str() << ") failed\n";
    throw file_error();
  }

  // Output the data
  ss << "/Params-Data";
  outf.open(ss.str().c_str());
  // Only need p and lg2 for online phase
  outf << p << endl;
  // Fix as a negative lg2 is a ``signal'' to choose slightly weaker
  // LWE parameters
  outf << abs(lg2) << endl;

  gfp::init_field(p, true);
  gf2n::init_field(lg2);
}

string get_prep_dir(int nparties, int lg2p, int gf2ndegree)
{
  if (gf2ndegree == 0)
    gf2ndegree = gf2n::default_length();
  stringstream ss;
  ss << PREP_DIR << nparties << "-" << lg2p << "-" << gf2ndegree << "/";
  return ss.str();
}
