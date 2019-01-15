// (C) 2018 University of Bristol. See License.txt

#ifndef _gfp
#define _gfp

#include <iostream>
using namespace std;

#include "Math/gf2n.h"
#include "Math/bigint.h"
#include "Math/field_types.h"
#include "Tools/random.h"

/* This is a wrapper class for the modp data type
 * It is used to be interface compatible with the gfp
 * type, which then allows us to template the Share
 * data type.
 *
 * So gfp is used ONLY for the stuff in the finite fields
 * we are going to be doing MPC over, not the modp stuff
 * for the FHE scheme
 */

class gfp
{
  SPDZEXT_VALTYPE a_ring;
  uint32_t precision;

  public:

  typedef gfp value_type;

  static void init_field(const bigint&,bool)
    { }
  static bigint pr()   
    { return 0; }
  static int t()
    { return 0;  }

  static DataFieldType field_type() { return DATA_MODP; }
  static char type_char() { return 'p'; }
  static string type_string() { return "gfp"; }

  static int size() { return t() * sizeof(mp_limb_t); }

  void assign(const gfp& g) {
	  a_ring = g.a_ring;
  }
  void assign_zero()        {
	  a_ring = 0;
  }
  void assign_one()         { }
  void assign(word aa)      {
	  a_ring = (SPDZEXT_VALTYPE)aa;
  }
  void assign(long aa)      {
	  a_ring = (SPDZEXT_VALTYPE)aa;
  }
  void assign(int aa)       {
	  a_ring = (SPDZEXT_VALTYPE)aa;
  }
  void assign(const char*) { }

  void assign_ring(SPDZEXT_VALTYPE aa) { a_ring = aa; }

  SPDZEXT_VALTYPE get_ring() const { return a_ring; }

  gfp()              { }
  gfp(const gfp& g) : a_ring(g.a_ring)  { }
  gfp(const __m128i& x) { *this=x; }
  gfp(const int128& x) { *this=x.a; }
  gfp(const bigint& ) { }
  gfp(int x)         { assign(x); }
  ~gfp()             { ; }

  gfp& operator=(const gfp& g)
    { if (&g!=this) {
    	a_ring = g.a_ring;
    }
      return *this;
    }

  gfp& operator=(const __m128i /*other*/)
    {
      //memcpy(a.x, &other, sizeof(other));
      return *this;
    }

  void to_m128i(__m128i& /*ans*/)
    {
      //memcpy(&ans, a.x, sizeof(ans));
    }

  __m128i to_m128i()
    {
	  __m128i nil;
      return nil;//_mm_loadu_si128((__m128i*)a.x);
    }


  bool is_zero() const            { return true;/*isZero(a,ZpD);*/ }
  bool is_one()  const            { return false;/*isOne(a,ZpD);*/ }
  bool is_bit()  const            { return is_zero() or is_one(); }
  bool equal(const gfp& /*y*/) const  { return true;/*areEqual(a,y.a,ZpD);*/ }
  bool operator==(const gfp& y) const { return equal(y); }
  bool operator!=(const gfp& y) const { return !equal(y); }

  // x+y
  template <int T>
  void add(const gfp& x,const gfp& y)
    {
	  a_ring = x.a_ring + y.a_ring;
    }
  template <int T>
  void add(const gfp& x)
    {
	  a_ring += x.a_ring;
    }
  template <int T>
  void add(void* /*x*/)
    {
    }
  template <int T>
  void add(octetStream& os)
    { add<T>(os.consume(size())); }
  void add(const gfp& x,const gfp& y)
    {
	  a_ring = x.a_ring + y.a_ring;
    }
  void add(const gfp& x)
    {
	  a_ring += x.a_ring;
    }
  void add(void* /*x*/)
    {
    }
  void sub(const gfp& x,const gfp& y)
    {
	  a_ring = x.a_ring - y.a_ring;
    }
  void sub(const gfp& x)
    {
	  a_ring -= x.a_ring;
	  }
  // = x * y
  void mul(const gfp& x,const gfp& y)
    {
	  a_ring = x.a_ring * y.a_ring;
    }
  void mul(const gfp& x) 
    {
	  a_ring = a_ring * x.a_ring;
    }

  gfp operator+(const gfp& x) {
	  gfp res; res.add(*this, x); return res;
  }
  gfp operator-(const gfp& x) { gfp res; res.sub(*this, x); return res; }
  gfp operator*(const gfp& x) { gfp res; res.mul(*this, x); return res; }
  gfp& operator+=(const gfp& x) { add(x); return *this; }
  gfp& operator-=(const gfp& x) { sub(x); return *this; }
  gfp& operator*=(const gfp& x) { mul(x); return *this; }

  gfp operator-() { gfp res = *this; res.negate(); return res; }

  void square(const gfp& /*aa*/)
    { /*Sqr(a,aa.a,ZpD);*/ }
  void square()
    { /*Sqr(a,a,ZpD);*/ }
  void invert()
    { /*Inv(a,a,ZpD);*/ }
  void invert(const gfp& /*aa*/)
    { /*Inv(a,aa.a,ZpD);*/ }
  void negate() 
    { /*Negate(a,a,ZpD);*/ }
  void power(long /*i*/)
    { /*Power(a,a,i,ZpD);*/ }

  // deterministic square root
  gfp sqrRoot();

  void randomize(PRNG& /*G*/)
    { /*a.randomize(G,ZpD);*/ }
  // faster randomization, see implementation for explanation
  void almost_randomize(PRNG& G);

  void output(ostream& s,bool human) const
    {
	  // human: true
	  if (human) {
		  s << a_ring;
	  }
    }
  void input(istream& s,bool human)
    {
	  // human: true
	  if (human) {
		  s >> a_ring;
	  }
    }
  friend ostream& operator<<(ostream& s,const gfp& x)
    { x.output(s,true);
      return s;
    }
  friend istream& operator>>(istream& s,gfp& x)
    { x.input(s,true);
      return s;
    }

  /* Bitwise Ops 
   *   - Converts gfp args to bigints and then converts answer back to gfp
   */
  void AND(const gfp& x,const gfp& y);
  void XOR(const gfp& x,const gfp& y);
  void OR(const gfp& x,const gfp& y);
  void AND(const gfp& x,const bigint& y);
  void XOR(const gfp& x,const bigint& y);
  void OR(const gfp& x,const bigint& y);
  void SHL(const gfp& x,int n);
  void SHR(const gfp& x,int n);
  void SHL(const gfp& x,const bigint& n);
  void SHR(const gfp& x,const bigint& n);

  gfp operator&(const gfp& x) { gfp res; res.AND(*this, x); return res; }
  gfp operator^(const gfp& x) { gfp res; res.XOR(*this, x); return res; }
  gfp operator|(const gfp& x) { gfp res; res.OR(*this, x); return res; }
  gfp operator<<(int i) { gfp res; res.SHL(*this, i); return res; }
  gfp operator>>(int i) { gfp res; res.SHR(*this, i); return res; }

  // Pack and unpack in native format
  //   i.e. Dont care about conversion to human readable form
  void pack(octetStream& /*o*/) const
    { /*a.pack(o,ZpD);*/ }
  void unpack(octetStream& /*o*/)
    { /*a.unpack(o,ZpD);*/ }


  // Convert representation to and from a bigint number
  friend void to_bigint(bigint& /*ans*/,const gfp& /*x*/,bool /*reduce=true*/)
    { /*to_bigint(ans,x.a,x.ZpD,reduce);*/ }
  friend void to_gfp(gfp& /*ans*/,const bigint& /*x*/)
    { /*to_modp(ans.a,x,ans.ZpD);*/ }
};


#endif
