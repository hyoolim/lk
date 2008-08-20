#include "random.h"
#include "ext.h"
#include <sys/time.h>
#include <time.h>
#define RANDOM (LK_RANDOM(self))

/* private func - see below for real impl */
static void init_genrand(lk_random_t *self, unsigned long s);
static unsigned long genrand_int32(lk_random_t *self);
/* static long genrand_int31(lk_random_t *self); */
static double genrand_real1(lk_random_t *self);
/* static double genrand_real2(lk_random_t *self);
static double genrand_real3(lk_random_t *self);
static double genrand_res53(lk_random_t *self); */

/* ext map */
static LK_OBJ_DEFALLOCFUNC(alloc__rd) {
    static int n = 0;
    struct timeval tv;
    int seed;
    gettimeofday(&tv, 0);
    seed = tv.tv_sec ^ tv.tv_usec ^ n ++;
    RANDOM->seed = lk_fi_new(LK_VM(self), seed);
    init_genrand(RANDOM, seed);
}
LK_LIB_DEFINECFUNC(init__rd_fi) {
    init_genrand(RANDOM, INT(RANDOM->seed = LK_FI(ARG(0))));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(fixed_integer__rd) {
    RETURN(lk_fi_new(VM, (int)genrand_int32(RANDOM))); }
LK_LIB_DEFINECFUNC(fixed_real__rd) {
    RETURN(lk_fr_new(VM, genrand_real1(RANDOM))); }
LK_EXT_DEFINIT(lk_random_extinit) {
    lk_object_t *obj = vm->t_obj, *fi = vm->t_fi;
    lk_object_t *rd = lk_object_allocwithsize(obj, sizeof(lk_random_t));
    LK_RANDOM(rd)->seed = lk_fi_new(vm, 0);
    init_genrand(LK_RANDOM(rd), 0);
    lk_object_setallocfunc(rd, alloc__rd);
    lk_lib_setGlobal("Random", rd);
    lk_lib_setCFunc(rd, "init!", init__rd_fi, fi, NULL);
    lk_lib_setCFunc(rd, "fixed_integer", fixed_integer__rd, NULL);
    lk_lib_setCFunc(rd, "fixed_real", fixed_real__rd, NULL);
    lk_lib_setCField(rd, "seed", fi, offsetof(lk_random_t, seed));
}

/* name conflict with ext rand gen */
#undef N

/* modified and cut so that mult rand gen are possible */
/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/* Period parameters */  
#define N LK_RANDOM_N
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

/* */
#define mt self->mt
#define mti self->mti

/* initializes mt[N] with a seed */
static void init_genrand(lk_random_t *self, unsigned long s) {
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* generates a random number on [0,0xffffffff]-interval */
static unsigned long genrand_int32(lk_random_t *self) {
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(self, 5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
  
    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
/* static long genrand_int31(lk_random_t *self) {
    return (long)(genrand_int32(self)>>1);
} */

/* generates a random number on [0,1]-real-interval */
static double genrand_real1(lk_random_t *self) {
    return genrand_int32(self)*(1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
/* static double genrand_real2(lk_random_t *self) {
    return genrand_int32(self)*(1.0/4294967296.0); 
    * divided by 2^32 *
} */

/* generates a random number on (0,1)-real-interval */
/* static double genrand_real3(lk_random_t *self) {
    return (((double)genrand_int32(self)) + 0.5)*(1.0/4294967296.0); 
    * divided by 2^32 *
} */

/* generates a random number on [0,1) with 53-bit resolution*/
/* static double genrand_res53(lk_random_t *self) { 
    unsigned long a=genrand_int32(self)>>5, b=genrand_int32(self)>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
}  */
