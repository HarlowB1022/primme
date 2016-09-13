/*******************************************************************************
 * Copyright (c) 2016, College of William & Mary
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the College of William & Mary nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COLLEGE OF WILLIAM & MARY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * PRIMME: https://github.com/primme/primme
 * Contact: Andreas Stathopoulos, a n d r e a s _at_ c s . w m . e d u
 *******************************************************************************
 *   NOTE: THIS FILE IS AUTOMATICALLY GENERATED. PLEASE DON'T MODIFY
 ******************************************************************************/


#ifndef main_iter_H
#define main_iter_H
#if !defined(CHECK_TEMPLATE) && !defined(main_iter_Sprimme)
#  define main_iter_Sprimme CONCAT(main_iter_,SCALAR_SUF)
#endif
#if !defined(CHECK_TEMPLATE) && !defined(main_iter_Rprimme)
#  define main_iter_Rprimme CONCAT(main_iter_,REAL_SUF)
#endif
int main_iter_dprimme(double *evals, int *perm, double *evecs, int64_t ldevecs,
   double *resNorms, double machEps, int *intWork, void *realWork,
   primme_params *primme);
#if !defined(CHECK_TEMPLATE) && !defined(prepare_candidates_Sprimme)
#  define prepare_candidates_Sprimme CONCAT(prepare_candidates_,SCALAR_SUF)
#endif
#if !defined(CHECK_TEMPLATE) && !defined(prepare_candidates_Rprimme)
#  define prepare_candidates_Rprimme CONCAT(prepare_candidates_,REAL_SUF)
#endif
int prepare_candidates_dprimme(double *V, int64_t ldV, double *W,
      int64_t ldW, int64_t nLocal, double *H, int ldH, int basisSize,
      double *X, double *R, double *hVecs, int ldhVecs, double *hVals,
      double *hSVals, int *flags, int numEvals, double *blockNorms,
      int blockNormsSize, int maxBlockSize, double *evecs, int numLocked,
      int64_t ldevecs, double *evals, double *resNorms, int targetShiftIndex,
      double machEps, int *iev, int *blockSize, int *recentlyConverged,
      int *numArbitraryVecs, double *smallestResNorm, double *hVecsRot,
      int ldhVecsRot, int *reset, double *rwork, size_t *rworkSize, int *iwork,
      int iworkSize, primme_params *primme);
int main_iter_zprimme(double *evals, int *perm, PRIMME_COMPLEX_DOUBLE *evecs, int64_t ldevecs,
   double *resNorms, double machEps, int *intWork, void *realWork,
   primme_params *primme);
int prepare_candidates_zprimme(PRIMME_COMPLEX_DOUBLE *V, int64_t ldV, PRIMME_COMPLEX_DOUBLE *W,
      int64_t ldW, int64_t nLocal, PRIMME_COMPLEX_DOUBLE *H, int ldH, int basisSize,
      PRIMME_COMPLEX_DOUBLE *X, PRIMME_COMPLEX_DOUBLE *R, PRIMME_COMPLEX_DOUBLE *hVecs, int ldhVecs, double *hVals,
      double *hSVals, int *flags, int numEvals, double *blockNorms,
      int blockNormsSize, int maxBlockSize, PRIMME_COMPLEX_DOUBLE *evecs, int numLocked,
      int64_t ldevecs, double *evals, double *resNorms, int targetShiftIndex,
      double machEps, int *iev, int *blockSize, int *recentlyConverged,
      int *numArbitraryVecs, double *smallestResNorm, PRIMME_COMPLEX_DOUBLE *hVecsRot,
      int ldhVecsRot, int *reset, PRIMME_COMPLEX_DOUBLE *rwork, size_t *rworkSize, int *iwork,
      int iworkSize, primme_params *primme);
int main_iter_sprimme(float *evals, int *perm, float *evecs, int64_t ldevecs,
   float *resNorms, double machEps, int *intWork, void *realWork,
   primme_params *primme);
int prepare_candidates_sprimme(float *V, int64_t ldV, float *W,
      int64_t ldW, int64_t nLocal, float *H, int ldH, int basisSize,
      float *X, float *R, float *hVecs, int ldhVecs, float *hVals,
      float *hSVals, int *flags, int numEvals, float *blockNorms,
      int blockNormsSize, int maxBlockSize, float *evecs, int numLocked,
      int64_t ldevecs, float *evals, float *resNorms, int targetShiftIndex,
      double machEps, int *iev, int *blockSize, int *recentlyConverged,
      int *numArbitraryVecs, double *smallestResNorm, float *hVecsRot,
      int ldhVecsRot, int *reset, float *rwork, size_t *rworkSize, int *iwork,
      int iworkSize, primme_params *primme);
int main_iter_cprimme(float *evals, int *perm, PRIMME_COMPLEX_FLOAT *evecs, int64_t ldevecs,
   float *resNorms, double machEps, int *intWork, void *realWork,
   primme_params *primme);
int prepare_candidates_cprimme(PRIMME_COMPLEX_FLOAT *V, int64_t ldV, PRIMME_COMPLEX_FLOAT *W,
      int64_t ldW, int64_t nLocal, PRIMME_COMPLEX_FLOAT *H, int ldH, int basisSize,
      PRIMME_COMPLEX_FLOAT *X, PRIMME_COMPLEX_FLOAT *R, PRIMME_COMPLEX_FLOAT *hVecs, int ldhVecs, float *hVals,
      float *hSVals, int *flags, int numEvals, float *blockNorms,
      int blockNormsSize, int maxBlockSize, PRIMME_COMPLEX_FLOAT *evecs, int numLocked,
      int64_t ldevecs, float *evals, float *resNorms, int targetShiftIndex,
      double machEps, int *iev, int *blockSize, int *recentlyConverged,
      int *numArbitraryVecs, double *smallestResNorm, PRIMME_COMPLEX_FLOAT *hVecsRot,
      int ldhVecsRot, int *reset, PRIMME_COMPLEX_FLOAT *rwork, size_t *rworkSize, int *iwork,
      int iworkSize, primme_params *primme);
#endif
