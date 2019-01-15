// (C) 2018 University of Bristol. See License.txt

/*
 * Setup.h
 *
 */

#ifndef MATH_SETUP_H_
#define MATH_SETUP_H_

#include "Math/bigint.h"

#include <iostream>
using namespace std;

#ifndef PREP_DIR
#define PREP_DIR "Player-Data/"
#endif

/*
 * Routines to create and read setup files for the finite fields
 */

// Create setup file for gfp and gf2n
void write_online_setup(ofstream& outf, string dirname, const bigint& p, int lg2);

// get main directory for prep. data
string get_prep_dir(int nparties, int lg2p, int gf2ndegree);


#endif /* MATH_SETUP_H_ */
