#include <tommath.h>
#ifdef BN_MP_DR_REDUCE_C

int
mp_dr_reduce (mp_int * x, mp_int * n, mp_digit k)
{
  int      err, i, m;
  mp_word  r;
  mp_digit mu, *tmpx1, *tmpx2;

  /* m = digits in modulus */
  m = n->used;

  /* ensure that "x" has at least 2m digits */
  if (x->alloc < m + m) {
    if ((err = mp_grow (x, m + m)) != MP_OKAY) {
      return err;
    }
  }

top:
  /* aliases for digits */
  /* alias for lower half of x */
  tmpx1 = x->dp;

  /* alias for upper half of x, or x/B**m */
  tmpx2 = x->dp + m;

  /* set carry to zero */
  mu = 0;

  /* compute (x mod B**m) + k * [x/B**m] inline and inplace */
  for (i = 0; i < m; i++) {
      r         = ((mp_word)*tmpx2++) * ((mp_word)k) + *tmpx1 + mu;
      *tmpx1++  = (mp_digit)(r & MP_MASK);
      mu        = (mp_digit)(r >> ((mp_word)DIGIT_BIT));
  }

  /* set final carry */
  *tmpx1++ = mu;

  /* zero words above m */
  for (i = m + 1; i < x->used; i++) {
      *tmpx1++ = 0;
  }

  /* clamp, sub and return */
  mp_clamp (x);

  /* if x >= n then subtract and reduce again
   * Each successive "recursion" makes the input smaller and smaller.
   */
  if (mp_cmp_mag (x, n) != MP_LT) {
    s_mp_sub(x, n, x);
    goto top;
  }
  return MP_OKAY;
}
#endif

/* $Source: /cvs/libtom/libtommath/bn_mp_dr_reduce.c,v $ */
/* $Revision: 1.3 $ */
/* $Date: 2006/03/31 14:18:44 $ */
