// (C) 2018 University of Bristol. See License.txt


#include "Math/gfp.h"

#include "Exceptions/Exceptions.h"

void gfp::almost_randomize(PRNG& /*G*/)
{
  //G.get_octets((octet*)a.x,t()*sizeof(mp_limb_t));
  //a.x[t()-1]&=ZpD.mask;
}

void gfp::AND(const gfp& x,const gfp& y)
{
	this->a_ring = x.a_ring & y.a_ring;
}

void gfp::OR(const gfp& x,const gfp& y)
{
	this->a_ring = x.a_ring | y.a_ring;
}

void gfp::XOR(const gfp& x,const gfp& y)
{
	this->a_ring = x.a_ring ^ y.a_ring;
}

void gfp::SHL(const gfp& x,int n)
{
	if (!x.is_zero()) {
		if (n != 0) {
			this->a_ring = x.a_ring << n;
		} else
			assign(x);
	} else {
		assign_zero();
	}
}


void gfp::SHR(const gfp& x,int n)
{
	if (!x.is_zero())
	{
		if (n != 0)
		{
			assign_ring(x.a_ring >> n);
		}
		else
			assign(x);
	}
	else
	{
		assign_zero();
	}
}
