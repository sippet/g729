/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*------------------------------------------------------------------------*
 * Function Post_Process()                                                *
 *                                                                        *
 * Post-processing of output speech.                                      *
 *   - 2nd order high pass filter with cut off frequency at 100 Hz.       *
 *   - Multiplication by two of output speech with saturation.            *
 *-----------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"

#include "ld8a.h"
#include "tab_ld8a.h"

/*------------------------------------------------------------------------*
 * 2nd order high pass filter with cut off frequency at 100 Hz.           *
 * Designed with SPPACK efi command -40 dB att, 0.25 ri.                  *
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i]   + b[1]*x[i-1]   + b[2]*x[i-2]                      *
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];                     *
 *                                                                        *
 *     b[3] = {0.93980581E+00, -0.18795834E+01, 0.93980581E+00};          *
 *     a[3] = {0.10000000E+01, 0.19330735E+01, -0.93589199E+00};          *
 *-----------------------------------------------------------------------*/


/* Initialization of static values */

void Init_Post_Process(Post_Process_state *st)
{
  st->y2_hi = 0;
  st->y2_lo = 0;
  st->y1_hi = 0;
  st->y1_lo = 0;
  st->x0   = 0;
  st->x1   = 0;
}


void Post_Process(
  Post_Process_state *st,
  int16_t signal[],    /* input/output signal */
  int16_t lg)          /* length of signal    */
{
  int16_t i, x2;
  int32_t L_tmp;

  for(i=0; i<lg; i++)
  {
     x2 = st->x1;
     st->x1 = st->x0;
     st->x0 = signal[i];

     /*  y[i] = b[0]*x[i]   + b[1]*x[i-1]   + b[2]*x[i-2]    */
     /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */

     L_tmp     = Mpy_32_16(st->y1_hi, st->y1_lo, a100[1]);
     L_tmp     = L_add(L_tmp, Mpy_32_16(st->y2_hi, st->y2_lo, a100[2]));
     L_tmp     = L_mac(L_tmp, st->x0, b100[0]);
     L_tmp     = L_mac(L_tmp, st->x1, b100[1]);
     L_tmp     = L_mac(L_tmp, x2, b100[2]);
     L_tmp     = L_shl(L_tmp, 2);      /* Q29 --> Q31 (Q13 --> Q15) */

     /* Multiplication by two of output speech with saturation. */
     signal[i] = L_round(L_shl(L_tmp, 1));

     st->y2_hi = st->y1_hi;
     st->y2_lo = st->y1_lo;
     L_Extract(L_tmp, &st->y1_hi, &st->y1_lo);
  }
  return;
}

