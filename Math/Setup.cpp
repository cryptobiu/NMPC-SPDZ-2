// (C) 2018 University of Bristol. See License.txt


#include "Math/Setup.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"

#include "Tools/mkpath.h"

#include <fstream>

void generate_prime(bigint& p, int lgp, int m)
{
  // Here we choose a prime which is the order of a BN curve
  //    - Reason is that there are some applications where this
  //      would be a good idea. So I have hard coded it in here
  //    - This is pointless/impossible for lgp=32, 64 so for 
  //      these do something naive
  //    - Have not tested 256 and 512
  bigint u;
  int ex;
  if (lgp!=32 && lgp!=64)
    { u=1; u=u<<(lgp-1); u=sqrt(sqrt(u/36))/m;
      u=u*m;
      bigint q;
      //   cout << ex << " " << u << " " << numBits(u) << endl;
      p=(((36*u+36)*u+18)*u+6)*u+1;   // The group order of a BN curve
      q=(((36*u+36)*u+24)*u+6)*u+1;   // The base field size of a BN curve
      while (!probPrime(p) || !probPrime(q) || numBits(p)<lgp) 
        { u=u+m;
          p=(((36*u+36)*u+18)*u+6)*u+1;
          q=(((36*u+36)*u+24)*u+6)*u+1;
        }
    }
  else
    { ex=lgp-numBits(m);
      u=1; u=(u<<ex)*m;  p=u+1;
      while (!probPrime(p) || numBits(p)<lgp)
        { u=u+m;  p=u+1; }
    }
  cout << "\t p = " << p << "  u = " << u << "  :   ";
  cout << lgp << " <= " << numBits(p) << endl;
}

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
