/*******************************************************************************
 * Copyright (c) 2018, College of William & Mary
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
 * File: blaslapack.c
 *
 * Purpose - This file contains mostly C wrapper routines for
 *           calling various BLAS and LAPACK FORTRAN routines.
 *
 ******************************************************************************/


#include <stdlib.h>   /* free */
#include <string.h>   /* memmove */
#include <assert.h>
#include <math.h>
#include "template.h"
/* Keep automatically generated headers under this section  */
#ifndef CHECK_TEMPLATE
#include "blaslapack.h"
#include "auxiliary.h" // cyclic
#endif

#ifdef USE_HOST

#include "blaslapack_private.h"

static int free_fn_dummy (void *p, primme_context ctx) {
   (void)ctx;
   free(p);
   return 0;
}

/******************************************************************************
 * Function Num_malloc_iblasprimme - Allocate a vector of integers
 *
 * PARAMETERS
 * ---------------------------
 * n           The number of elements
 * v           returned pointer
 * 
 ******************************************************************************/

static int Num_malloc_iblasprimme(PRIMME_INT n, PRIMME_BLASINT **x, primme_context ctx) {
   (void)ctx;

   /* Quick exit */

   if (n <= 0) {
      *x = NULL;
      return 0;
   }

   /* Allocate memory */

   *x = (PRIMME_BLASINT *)malloc(sizeof(PRIMME_BLASINT) * n);
   if (*x == NULL) return PRIMME_MALLOC_FAILURE;

   /* Register the allocation */

   Mem_keep_frame(ctx);
   Mem_register_alloc(*x, free_fn_dummy, ctx);

   return 0;
}

/******************************************************************************
 * Function Num_free_iblasprimme - Free allocated a vector of integers
 *
 * PARAMETERS
 * ---------------------------
 * v           allocated pointer
 * 
 ******************************************************************************/

static int Num_free_iblasprimme(PRIMME_BLASINT *x, primme_context ctx) {
   /* Quick exit */

   if (!x) return 0;

   /* Deregister the allocation */

   Mem_deregister_alloc(x, ctx);

   /* Free pointer */

   free(x);

   return 0;
}



/*******************************************************************************
 * Subroutine Num_copy_Sprimme - y(0:n*incy-1:incy) = x(0:n*incx-1:incx)
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_copy_Sprimme(PRIMME_INT n, SCALAR *x, int incx, SCALAR *y, int incy,
                      primme_context ctx) {

   (void)ctx;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT lincx = incx;
   PRIMME_BLASINT lincy = incy;

   while (n > 0) {
      ln = (PRIMME_BLASINT)min(n, PRIMME_BLASINT_MAX - 1);
      XCOPY(&ln, x, &lincx, y, &lincy);
      n -= (PRIMME_INT)ln;
      x += ln;
      y += ln;
   }
   return 0;
}

/*******************************************************************************
 * Subroutine Num_gemm_Sprimme - C = op(A)*op(B), with C size m x n
 ******************************************************************************/

TEMPLATE_PLEASE
int  Num_gemm_Sprimme(const char *transa, const char *transb, int m, int n,
      int k, SCALAR alpha, SCALAR *a, int lda, SCALAR *b, int ldb, SCALAR beta,
      SCALAR *c, int ldc, primme_context ctx) {

   PRIMME_BLASINT lm = m;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT lk = k;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldb = ldb;
   PRIMME_BLASINT lldc = ldc;

   /* Zero dimension matrix may cause problems */
   if (m == 0 || n == 0) return 0;

   /* Quick exit */
   if (k == 0) {
      if (ABS(beta) == 0.0) {
         Num_zero_matrix_Sprimme(c, m, n, ldc, ctx);
      }
      else if (beta != (HSCALAR)1.0) {
         int i;
         for (i=0; i<n; i++) {
            Num_scal_Sprimme(m, beta, &c[ldc*i], 1, ctx);
         }
      }
      return 0;
   }
   if (n == 1) {
      PRIMME_INT mA; int nA;
      if (*transa == 'n' || *transa == 'N') mA = m, nA = k;
      else mA = k, nA = m;
      int incb = ((*transb == 'n' || *transb == 'N') ? 1 : ldb);
      return Num_gemv_Sprimme(
            transa, mA, nA, alpha, a, lda, b, incb, beta, c, 1, ctx);
   }

#ifdef NUM_CRAY
   _fcd transa_fcd, transb_fcd;

   transa_fcd = _cptofcd(transa, strlen(transa));
   transb_fcd = _cptofcd(transb, strlen(transb));
   XGEMM(transa_fcd, transb_fcd, &lm, &ln, &lk, &alpha, a, &llda, b, &lldb, &beta, 
         c, &lldc);
#else
   XGEMM(transa, transb, &lm, &ln, &lk, &alpha, a, &llda, b, &lldb, &beta, c, &lldc);
#endif

   return 0;

}

TEMPLATE_PLEASE
int Num_gemm_dhd_Sprimme(const char *transa, const char *transb, int m, int n,
      int k, SCALAR alpha, SCALAR *a, int lda, HSCALAR *b, int ldb, SCALAR beta,
      SCALAR *c, int ldc, primme_context ctx) {
   return Num_gemm_Sprimme(
         transa, transb, m, n, k, alpha, a, lda, b, ldb, beta, c, ldc, ctx);
}

TEMPLATE_PLEASE
int Num_gemm_ddh_Sprimme(const char *transa, const char *transb, int m, int n,
      int k, SCALAR alpha, SCALAR *a, int lda, HSCALAR *b, int ldb, SCALAR beta,
      SCALAR *c, int ldc, primme_context ctx) {
   return Num_gemm_Sprimme(
         transa, transb, m, n, k, alpha, a, lda, b, ldb, beta, c, ldc, ctx);
}

/*******************************************************************************
 * Subroutine Num_hemm_Sprimme - C = A*B or B*A where A is Hermitian,
 *    where C size m x n.
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_hemm_Sprimme(const char *side, const char *uplo, int m, int n,
      SCALAR alpha, SCALAR *a, int lda, SCALAR *b, int ldb, SCALAR beta, 
      SCALAR *c, int ldc) {

   PRIMME_BLASINT lm = m;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldb = ldb;
   PRIMME_BLASINT lldc = ldc;

   /* Zero dimension matrix may cause problems */
   if (m == 0 || n == 0) return 0;

#ifdef NUM_CRAY
   _fcd side_fcd, uplo_fcd;

   side_fcd = _cptofcd(side, strlen(side));
   uplo_fcd = _cptofcd(uplo, strlen(uplo));
   XHEMM(side_fcd, uplo_fcd, &lm, &ln, &alpha, a, &llda, b, &lldb, &beta, c, &lldc);
#else
   XHEMM(side, uplo, &lm, &ln, &alpha, a, &llda, b, &lldb, &beta, c, &lldc);
#endif 

   return 0;
}

/*******************************************************************************
 * Subroutine Num_trmm_Sprimme - B = A*B or B*A where A is triangular,
 *    with B size m x n.
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_trmm_Sprimme(const char *side, const char *uplo,
      const char *transa, const char *diag, int m, int n, SCALAR alpha,
      SCALAR *a, int lda, SCALAR *b, int ldb, primme_context ctx) {

   (void)ctx;
   PRIMME_BLASINT lm = m;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldb = ldb;

   /* Zero dimension matrix may cause problems */
   if (m == 0 || n == 0) return 0;

   XTRMM(side, uplo, transa, diag, &lm, &ln, &alpha, a, &llda, b, &lldb);

   return 0;
}

/*******************************************************************************
 * Subroutine Num_gemv_Sprimme - y = alpha*A*x + beta*y, with A size m x n
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_gemv_Sprimme(const char *transa, PRIMME_INT m, int n, SCALAR alpha,
      SCALAR *a, int lda, SCALAR *x, int incx, SCALAR beta, SCALAR *y, int incy,
      primme_context ctx) {

   PRIMME_BLASINT lm = m;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lincx = incx;
   PRIMME_BLASINT lincy = incy;

   /* Zero dimension matrix may cause problems */
   int tA = (*transa != 'n' && *transa != 'N' ? 1 : 0);
   PRIMME_INT mA = tA ? n : m, nA = tA ? m : n;
   if (mA == 0) return 0;

   /* Quick exit */
   if (nA == 0) {
      if (ABS(beta) == 0.0) {
         Num_zero_matrix_Sprimme(y, 1, mA, incy, ctx);
      }
      else {
      Num_scal_Sprimme(mA, beta, y, incy, ctx);
      }
      return 0;
   }
   if (mA == 1) {
      if (ABS(beta) != 0.0)
         y[0] *= beta;
      else
         y[0] = 0.0;
      if (tA) {
         y[0] += Num_dot_Sprimme(nA, a, 1, x, incx, ctx) * alpha;
      } else {
         y[0] += Num_dot_Sprimme(nA, a, lda, x, incx, ctx) * alpha;
      }
      return 0;
   }

   while(m > 0) {
      lm = (PRIMME_BLASINT)min(m, PRIMME_BLASINT_MAX-1);
      XGEMV(transa, &lm, &ln, &alpha, a, &llda, x, &lincx, &beta, y, &lincy);
      m -= (PRIMME_INT)lm;
      a += lm;
      if (!tA) {
         y += lm;
      }
      else {
         x += lm;
         beta = 1.0;
      }
   }

   return 0;
}

/*******************************************************************************
 * Subroutine Num_gemv_ddh_Sprimme - y = alpha*A*x + beta*y, with A size m x n
 * NOTE: A and x are in device and y is in host
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_gemv_ddh_Sprimme(const char *transa, PRIMME_INT m, int n, SCALAR alpha,
      SCALAR *a, int lda, SCALAR *x, int incx, SCALAR beta, HSCALAR *y,
      int incy, primme_context ctx) {

   return Num_gemv_Sprimme(
         transa, m, n, alpha, a, lda, x, incx, beta, y, incy, ctx);
}

/*******************************************************************************
 * Subroutine Num_gemv_dhd_Sprimme - y = alpha*A*x + beta*y, with A size m x n
 * NOTE: A and y are in device and x is in host
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_gemv_dhd_Sprimme(const char *transa, PRIMME_INT m, int n, SCALAR alpha,
      SCALAR *a, int lda, HSCALAR *x, int incx, SCALAR beta, SCALAR *y,
      int incy, primme_context ctx) {

   return Num_gemv_Sprimme(
         transa, m, n, alpha, a, lda, x, incx, beta, y, incy, ctx);
}

/*******************************************************************************
 * Subroutine Num_hemv_Sprimme - y = alpha*A*x + beta*y where A is Hermitian
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_hemv_Sprimme(const char *uplo, int n, SCALAR alpha, 
   SCALAR *a, int lda, SCALAR *x, int incx, SCALAR beta, 
   SCALAR *y, int incy) {

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lincx = incx;
   PRIMME_BLASINT lincy = incy;

   /* Zero dimension matrix may cause problems */
   if (n == 0) return 0;

#ifdef NUM_CRAY
   _fcd uplo_fcd;

   uplo_fcd = _cptofcd(uplo, strlen(uplo));
   XHEMV(uplo_fcd, &ln, &alpha, a, &llda, x, &lincx, &beta, y, &lincy);
#else
   XHEMV(uplo, &ln, &alpha, a, &llda, x, &lincx, &beta, y, &lincy);
#endif

   return 0;
}

/*******************************************************************************
 * Subroutine Num_axpy_Sprimme - y += alpha*x
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_axpy_Sprimme(PRIMME_INT n, SCALAR alpha, SCALAR *x, int incx, 
   SCALAR *y, int incy, primme_context ctx) {

   (void)ctx;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT lincx = incx;
   PRIMME_BLASINT lincy = incy;

   while(n > 0) {
      ln = (PRIMME_BLASINT)min(n, PRIMME_BLASINT_MAX-1);
      XAXPY(&ln, &alpha, x, &lincx, y, &lincy);
      n -= (PRIMME_INT)ln;
      x += ln;
      y += ln;
   }

   return 0;
}

/*******************************************************************************
 * Subroutine Num_dot_Sprimme - y'*x
 ******************************************************************************/

TEMPLATE_PLEASE
SCALAR Num_dot_Sprimme(PRIMME_INT n, SCALAR *x, int incx, SCALAR *y, int incy,
                       primme_context ctx) {
   (void)ctx;
/* NOTE: vecLib doesn't follow BLAS reference for sdot */
#if defined(USE_COMPLEX) || (defined(USE_FLOAT) && (defined(__APPLE__) || defined(__MACH__)))
/* ---- Explicit implementation of the zdotc() --- */
   PRIMME_INT i;
   SCALAR zdotc = 0.0;
   if (n <= 0) return(zdotc);
   if (incx == 1 && incy == 1) {
      for (i=0;i<n;i++) { /* zdotc = zdotc + dconjg(x(i))* y(i) */
         zdotc += CONJ(x[i]) * y[i];
      }
   }
   else {
      for (i=0;i<n;i++) { /* zdotc = zdotc + dconjg(x(i))* y(i) */
         zdotc += CONJ(x[i*incx]) * y[i*incy];
      }
   }
   return zdotc;
/* -- end of explicit implementation of the zdotc() - */
#else
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT lincx = incx;
   PRIMME_BLASINT lincy = incy;
   SCALAR r = 0.0;

   while(n > 0) {
      ln = (PRIMME_BLASINT)min(n, PRIMME_BLASINT_MAX-1);
      r += XDOT(&ln, x, &lincx, y, &lincy);
      n -= (PRIMME_INT)ln;
      x += ln;
      y += ln;
   }

   return r;
#endif
}

/*******************************************************************************
 * Subroutine Num_larnv_Sprimme - x(0:n*incy-1:incy) = rand(0:n-1)
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_larnv_Sprimme(int idist, PRIMME_INT *iseed, PRIMME_INT length,
      SCALAR *x, primme_context ctx) {

   (void)ctx;

#ifdef USE_COMPLEX
   /* Lapack's R core library doesn't have zlarnv. The functionality is */
   /* replaced by calling the REAL version doubling the length.         */

   assert(idist < 4); /* complex distributions are not supported */
   return Num_larnv_Rprimme(idist, iseed, length*2, (REAL*)x, ctx);
#else
   PRIMME_BLASINT lidist = idist;
   PRIMME_BLASINT llength;
   PRIMME_BLASINT temp[4];
   PRIMME_BLASINT *liseed;
   int i;

   if (sizeof(PRIMME_INT) == sizeof(PRIMME_BLASINT)) {
      liseed = (PRIMME_BLASINT*)iseed; /* cast avoid compiler warning */
   } else {
      liseed = temp;
      for(i=0; i<4; i++)
         liseed[i] = (PRIMME_BLASINT)iseed[i];
   }

   while(length > 0) {
      llength = (PRIMME_BLASINT)min(length, PRIMME_BLASINT_MAX-1);
      XLARNV(&lidist, liseed, &llength, x);
      length -= (PRIMME_INT)llength;
      x += llength;
   }

   if (sizeof(PRIMME_INT) != sizeof(PRIMME_BLASINT))
      for(i=0; i<4; i++)
         iseed[i] = (int)liseed[i];

   return 0;
#endif
}

/*******************************************************************************
 * Subroutine Num_scal_Sprimme - x(0:n*incx-1:incx) *= alpha
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_scal_Sprimme(
      PRIMME_INT n, SCALAR alpha, SCALAR *x, int incx, primme_context ctx) {

   (void)ctx;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT lincx = incx;

   while(n > 0) {
      ln = (PRIMME_BLASINT)min(n, PRIMME_BLASINT_MAX-1);
      XSCAL(&ln, &alpha, x, &lincx);
      n -= (PRIMME_INT)ln;
      x += ln;
   }

   return 0;
}

/*******************************************************************************
 * Subroutine Num_swap_Sprimme - swap x(0:n*incx-1:incx) and y(0:n*incy-1:incy)
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_swap_Sprimme(PRIMME_INT n, SCALAR *x, int incx, SCALAR *y, int incy,
      primme_context ctx) {

   (void)ctx;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT lincx = incx;
   PRIMME_BLASINT lincy = incy;

   while(n > 0) {
      ln = (PRIMME_BLASINT)min(n, PRIMME_BLASINT_MAX-1);
      XSWAP(&ln, x, &lincx, y, &lincy);
      n -= (PRIMME_INT)ln;
      x += ln;
      y += ln;
   }

   return 0;
}

/*******************************************************************************
 * Subroutines for dense eigenvalue decomposition
 * NOTE: xheevx is used instead of xheev because xheev is not in ESSL
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_heev_Sprimme(const char *jobz, const char *uplo, int n, SCALAR *a,
      int lda, REAL *w, primme_context ctx) {
#ifndef USE_XHEEV

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldwork = 0;
   PRIMME_BLASINT linfo = 0;
   SCALAR *z;
   REAL abstol=0.0;
#  ifdef USE_COMPLEX
   REAL *rwork;
#  endif
   PRIMME_BLASINT *iwork, *ifail;
   REAL   dummyr=0;
   PRIMME_BLASINT dummyi=0;

   /* Zero dimension matrix may cause problems */
   if (n == 0) return 0;

   /* Allocate arrays */

   CHKERR(Num_malloc_Sprimme(n*n, &z, ctx)); 
#  ifdef USE_COMPLEX
   CHKERR(Num_malloc_Rprimme(7*n, &rwork, ctx));
#  endif
   CHKERR(Num_malloc_iblasprimme(5*n, &iwork, ctx));
   CHKERR(Num_malloc_iblasprimme(n, &ifail, ctx));

   /* Call to know the optimal workspace */

   lldwork = -1;
   SCALAR lwork0 = 0;
   XHEEVX(jobz, "A", uplo, &ln, a, &llda, &dummyr, &dummyr,
         &dummyi, &dummyi, &abstol, &dummyi, w, z, &ln, &lwork0, &lldwork,
#  ifdef USE_COMPLEX
         &dummyr,
#  endif
         iwork, ifail, &linfo);
   lldwork = REAL_PART(lwork0);

   if (linfo == 0) {
      SCALAR *work = NULL;
      CHKERR(Num_malloc_Sprimme(lldwork, &work, ctx));
      XHEEVX(jobz, "A", uplo, &ln, a, &llda, &dummyr, &dummyr,
            &dummyi, &dummyi, &abstol, &dummyi, w, z, &ln, work, &lldwork,
#  ifdef USE_COMPLEX
            rwork,
#  endif
            iwork, ifail, &linfo);
      CHKERR(Num_free_Sprimme(work, ctx));
   }

   /* Copy z to a */
   Num_copy_matrix_Sprimme(z, n, n, n, a, lda, ctx);

   CHKERR(Num_free_Sprimme(z, ctx)); 
#  ifdef USE_COMPLEX
   CHKERR(Num_free_Rprimme(rwork, ctx));
#  endif
   CHKERR(Num_free_iblasprimme(iwork, ctx));
   CHKERR(Num_free_iblasprimme(ifail, ctx));

   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xheev with info %d",
          (int)linfo);
   return 0;

#else /* USE_XHEEV */

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldwork = 0;
   PRIMME_BLASINT linfo = 0;
#  ifdef USE_COMPLEX
   REAL *rwork;
#  endif
   SCALAR dummys=0;
   REAL   dummyr=0;

   /* Zero dimension matrix may cause problems */
   if (n == 0) return;

#  ifdef USE_COMPLEX
   CHKERR(Num_malloc_Sprimme(3*n, &rwork));
#  endif

   /* Call to know the optimal workspace */

   lldwork = -1;
   SCALAR lwork0 = 0;
   XHEEV(jobz, uplo, &ln, a, &llda, w, &lwork0, &lldwork,
#     ifdef USE_COMPLEX
         rwork,
#     endif
         &linfo); 
   lldwork = REAL_PART(lwork0);

   if (linfo == 0) {
      SCALAR *work = NULL;
      CHKERR(Num_malloc_Sprimme(lldwork, &work, ctx));
      XHEEV(jobz, uplo, &ln, a, &llda, w, work, &lldwork,
#     ifdef USE_COMPLEX
            rwork,
#     endif
            &linfo); 
      CHKERR(Num_free_Sprimme(work, ctx));
   }

#  ifdef USE_COMPLEX
   CHKERR(Num_free_Sprimme(rwork));
#  endif
   
   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xheev with info %d",
          (int)linfo);
   return 0;

#endif
}


/*******************************************************************************
 * Subroutines for dense generalize eigenvalue decomposition
 * NOTE: xhegvx is used instead of xhegv because xhegv is not in ESSL
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_hegv_Sprimme(const char *jobz, const char *uplo, int n, SCALAR *a,
      int lda, SCALAR *b0, int ldb0, REAL *w, primme_context ctx) {

   /* Call heev if b is null */
   if (b0 == NULL) {
      return Num_heev_Sprimme(jobz, uplo, n, a, lda, w, ctx);
   }

#ifndef USE_XHEGV

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldwork = 0;
   PRIMME_BLASINT linfo = 0;
   PRIMME_BLASINT ONE = 1;
   SCALAR *z, *b;
   REAL abstol=0.0;
#  ifdef USE_COMPLEX
   REAL *rwork;
#  endif
   PRIMME_BLASINT *iwork, *ifail;
   REAL   dummyr=0;
   PRIMME_BLASINT dummyi=0;

   /* Zero dimension matrix may cause problems */
   if (n == 0) return 0;

   /* Allocate arrays */

   CHKERR(Num_malloc_Sprimme(n*n, &z, ctx)); 
   CHKERR(Num_malloc_Sprimme(n*n, &b, ctx)); 
#  ifdef USE_COMPLEX
   CHKERR(Num_malloc_Rprimme(7*n, &rwork, ctx));
#  endif
   CHKERR(Num_malloc_iblasprimme(5*n, &iwork, ctx));
   CHKERR(Num_malloc_iblasprimme(n, &ifail, ctx));

   Num_copy_trimatrix_Sprimme(b0, n, n, ldb0,
         *uplo == 'U' || *uplo == 'u' ? 0 : 1, 0, b, n,
         0 /*not to zero rest of the matrix */);

   /* Call to know the optimal workspace */

   lldwork = -1;
   SCALAR lwork0 = 0;
   XHEGVX(&ONE, jobz, "A", uplo, &ln, a, &llda, b, &ln, &dummyr, &dummyr,
         &dummyi, &dummyi, &abstol, &dummyi, w, z, &ln, &lwork0, &lldwork,
#  ifdef USE_COMPLEX
         rwork,
#  endif
         iwork, ifail, &linfo);
   lldwork = REAL_PART(lwork0);

   if (linfo == 0) {
      SCALAR *work = NULL;
      CHKERR(Num_malloc_Sprimme(lldwork, &work, ctx));
      XHEGVX(&ONE, jobz, "A", uplo, &ln, a, &llda, b, &ln, &dummyr, &dummyr,
            &dummyi, &dummyi, &abstol, &dummyi, w, z, &ln, work, &lldwork,
#  ifdef USE_COMPLEX
            rwork,
#  endif
            iwork, ifail, &linfo);
      CHKERR(Num_free_Sprimme(work, ctx));
   }

   /* Copy z to a */
   Num_copy_matrix_Sprimme(z, n, n, n, a, lda, ctx);

   CHKERR(Num_free_Sprimme(z, ctx)); 
   CHKERR(Num_free_Sprimme(b, ctx)); 
#  ifdef USE_COMPLEX
   CHKERR(Num_free_Rprimme(rwork, ctx));
#  endif
   CHKERR(Num_free_iblasprimme(iwork, ctx));
   CHKERR(Num_free_iblasprimme(ifail, ctx));

   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xhegvx with info %d",
          (int)linfo);
   return 0;


#else /* USE_XHEGV */

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldwork = 0;
   PRIMME_BLASINT linfo = 0;
   PRIMME_BLASINT ONE = 1;
   SCALAR *b;
#  ifdef USE_COMPLEX
   REAL *rwork;
#  endif
   SCALAR dummys=0;
   REAL   dummyr=0;

   /* Zero dimension matrix may cause problems */
   if (n == 0) return;

   CHKERR(Num_malloc_Sprimme(n*n, &b, ctx)); 
#  ifdef USE_COMPLEX
   CHKERR(Num_malloc_Sprimme(3*n, &rwork, ctx));
#  endif

   Num_copy_matrix_Sprimme(b0, n, n, ldb0, b, n);

   /* Call to know the optimal workspace */

   lldwork = -1;
   SCALAR lwork0 = 0;
   XHEGV(&ONE, jobz, uplo, &ln, a, &llda, b, &ln, w, &lwork0, &lldwork,
#  ifdef USE_COMPLEX
         rwork,
#  endif
         &linfo);
   lldwork = REAL_PART(lwork0);

   if (linfo == 0) {
      SCALAR *work = NULL;
      CHKERR(Num_malloc_Sprimme(lldwork, &work, ctx));
      XHEGV(&ONE, jobz, uplo, &ln, a, &llda, b, &ln, w, work, &lldwork,
#  ifdef USE_COMPLEX
            rwork,
#  endif
            &linfo);
      CHKERR(Num_free_Sprimme(work, ctx));
   }

   CHKERR(Num_free_Sprimme(b, ctx)); 
#  ifdef USE_COMPLEX
   CHKERR(Num_free_Sprimme(rwork, ctx));
#  endif
 
#endif
}

/*******************************************************************************
 * Subroutines for dense singular value decomposition
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_gesvd_Sprimme(const char *jobu, const char *jobvt, int m, int n,
      SCALAR *a, int lda, REAL *s, SCALAR *u, int ldu, SCALAR *vt, int ldvt,
      primme_context ctx){

   PRIMME_BLASINT lm = m;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldu = ldu;
   PRIMME_BLASINT lldvt = ldvt;
   PRIMME_BLASINT lldwork = 0;
   PRIMME_BLASINT linfo = 0;
#ifdef USE_COMPLEX
   REAL dummyr = 0.0;
#endif

   /* Zero dimension matrix may cause problems */
   if (m == 0 || n == 0) return 0;

   /* Call to know the optimal workspace */

   lldwork = -1;
   SCALAR lwork0 = 0;
   XGESVD(jobu, jobvt, &lm, &ln, a, &llda, s, u, &lldu, vt, &lldvt, &lwork0,
          &lldwork,
#ifdef USE_COMPLEX
          &dummyr,
#endif
         &linfo);
   lldwork = REAL_PART(lwork0);

   if (linfo == 0) {
      SCALAR *work = NULL;
      CHKERR(Num_malloc_Sprimme(lldwork, &work, ctx));
#  ifdef USE_COMPLEX
      REAL *rwork;
      CHKERR(Num_malloc_Rprimme(5*n, &rwork, ctx));
#  endif
      XGESVD(jobu, jobvt, &lm, &ln, a, &llda, s, u, &lldu, vt, &lldvt, work,
            &lldwork,
#ifdef USE_COMPLEX
            rwork,
#endif
            &linfo);
      CHKERR(Num_free_Sprimme(work, ctx));
#  ifdef USE_COMPLEX
      CHKERR(Num_free_Rprimme(rwork, ctx));
#  endif
   }

   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xgesvd with info %d",
          (int)linfo);
   return 0;
}

/*******************************************************************************
 * Subroutine Num_hetrf_Sprimme - LL^H factorization with pivoting
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_hetrf_Sprimme(const char *uplo, int n, SCALAR *a, int lda, int *ipivot,
   primme_context ctx) {
#ifndef USE_ZGESV

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT *lipivot;
   PRIMME_BLASINT lldwork = 0;
   PRIMME_BLASINT linfo = 0; 

   /* Zero dimension matrix may cause problems */

   if (n == 0) return 0;

   if (sizeof(int) != sizeof(PRIMME_BLASINT)) {
      CHKERR(Num_malloc_iblasprimme(n, &lipivot, ctx));
   } else {
      lipivot = (PRIMME_BLASINT *)ipivot; /* cast avoid compiler warning */
   }

   /* Call to know the optimal workspace */
   
   lldwork = -1;
   SCALAR lwork0 = 0;
   XHETRF(uplo, &ln, a, &llda, lipivot, &lwork0, &lldwork, &linfo);
   lldwork = REAL_PART(lwork0);

   if (linfo == 0) {
      SCALAR *work;
      CHKERR(Num_malloc_Sprimme(lldwork, &work, ctx));
      XHETRF(uplo, &ln, a, &llda, lipivot, work, &lldwork, &linfo);
      CHKERR(Num_free_Sprimme(work, ctx));
   }

   if (sizeof(int) != sizeof(PRIMME_BLASINT)) {
      int i;
      for(i=0; i<n; i++)
         ipivot[i] = (int)lipivot[i];
      CHKERR(Num_free_iblasprimme(lipivot, ctx));
   }

   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xhetrf with info %d",
          (int)linfo);
   return 0;

#else /* USE_ZGESV */

   /* Lapack's R core library doesn't have zhetrf. The functionality is       */
   /* implemented by replacing the input matrix with a full general matrix.   */
   /* And Num_zhetrs_Sprimme will solve the general linear system.            */

   /* Copy the given upper/lower triangular part into the lower/upper part    */

   int i, j;
   if (*uplo == 'L' || *uplo == 'l') {
      for (i=0; i<n; i++) {
         for (j=i+1; j<n; j++) {
            a[lda*j+i] = CONJ(a[lda*i+j]);
         }
      }
   }
   else {
      for (i=1; i<n; i++) {
         for (j=0; j<i; j++) {
            a[lda*j+i] = CONJ(a[lda*i+j]);
         }
      }
   }

   return 0;

#endif
}

/*******************************************************************************
 * Subroutine Num_hetrs_Sprimme - b = A\b where A may store a LL^H factorization
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_hetrs_Sprimme(const char *uplo, int n, int nrhs, SCALAR *a, int lda,
      int *ipivot, SCALAR *b, int ldb, primme_context ctx) {

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT lnrhs = nrhs;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT *lipivot;
   PRIMME_BLASINT lldb = ldb;
   PRIMME_BLASINT linfo = 0; 

   /* Zero dimension matrix may cause problems */

   if (n == 0 || nrhs == 0) return 0;

   if (sizeof(int) != sizeof(PRIMME_BLASINT)) {
      CHKERR(Num_malloc_iblasprimme(n, &lipivot, ctx));
      int i;
      for(i=0; i<n; i++) {
         lipivot[i] = (PRIMME_BLASINT)ipivot[i];
      }
   } else {
      lipivot = (PRIMME_BLASINT *)ipivot; /* cast avoid compiler warning */
   }

#ifndef USE_ZGESV
   XHETRS(uplo, &ln, &lnrhs, a, &llda, lipivot, b, &lldb, &linfo);
   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xhetrs with info %d",
          (int)linfo);
#else /* USE_ZGESV */
   XGESV(&ln, &lnrhs, a, &llda, lipivot, b, &lldb, &linfo);
   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xgesv with info %d",
          (int)linfo);
#endif

   if (sizeof(int) != sizeof(PRIMME_BLASINT)) {
      CHKERR(Num_free_iblasprimme(lipivot, ctx));
   }

   return 0;
}

/*******************************************************************************
 * Subroutine Num_potrf_Sprimme - Cholesky factorization
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_potrf_Sprimme(const char *uplo, int n, SCALAR *a, int lda, int *info,
      primme_context ctx) {

   (void)ctx;

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT linfo = 0; 

   /* Zero dimension matrix may cause problems */
   if (n == 0) {
      if (info) *info = 0;
      return 0;
   }

   XPOTRF(uplo, &ln, a, &llda, &linfo);
   CHKERRM(info == NULL && linfo != 0, PRIMME_LAPACK_FAILURE,
         "Error in xpotrf with info %d\n", (int)linfo);
   if (info) *info = (int)linfo;

   return 0;
}


/*******************************************************************************
 * Subroutine Num_trsm_Sprimme - b = op(A)\b, where A is triangular
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_trsm_Sprimme(const char *side, const char *uplo, const char *transa,
      const char *diag, int m, int n, SCALAR alpha, SCALAR *a, int lda,
      SCALAR *b, int ldb) {

   PRIMME_BLASINT lm = m;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldb = ldb;

   /* Zero dimension matrix may cause problems */
   if (m == 0 || n == 0) return 0;

#ifdef NUM_CRAY
   _fcd side_fcd, uplo_fcd, transa_fcd, diag_fcd;

   side_fcd = _cptofcd(side, strlen(side));
   uplo_fcd = _cptofcd(uplo, strlen(uplo));
   transa_fcd = _cptofcd(transa, strlen(transa));
   diag_fcd = _cptofcd(diag, strlen(diag));
   XTRSM(side_fcd, uplo_fcd, transa_fcd, diag_fcd, &lm, &ln, &alpha, a, &llda, b, &lldb);
#else
   XTRSM(side, uplo, transa, diag, &lm, &ln, &alpha, a, &llda, b, &lldb);
#endif

   return 0;
}

/*******************************************************************************
 * Subroutine Num_getrf_Sprimme - Factorize A=LU
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_getrf_Sprimme(
      int m, int n, SCALAR *a, int lda, int *ipivot, primme_context ctx) {
   (void)ctx;

   PRIMME_BLASINT lm = m;
   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT *lipivot = NULL;
   PRIMME_BLASINT linfo = 0;

   /* Zero dimension matrix may cause problems */
   if (m == 0 || n == 0) return 0;

   if (sizeof(int) != sizeof(PRIMME_BLASINT)) {
      CHKERR(MALLOC_PRIMME(n, &lipivot));
   } else {
      lipivot = (PRIMME_BLASINT *)ipivot; /* cast avoid compiler warning */
   }

   XGETRF(&lm, &ln, a, &llda, lipivot, &linfo);

   if (sizeof(int) != sizeof(PRIMME_BLASINT)) {
      int i;
      if (ipivot) for(i=0; i<n; i++)
         ipivot[i] = (int)lipivot[i];
      free(lipivot);
   }

   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xgesv with info %d",
         (int)linfo);

   return 0;
}

/*******************************************************************************
 * Subroutine Num_getrs_Sprimme - Computes A\X where A=LU computed with getrf
 ******************************************************************************/
 
TEMPLATE_PLEASE
int Num_getrs_Sprimme(const char *trans, int n, int nrhs, SCALAR *a, int lda,
      int *ipivot, SCALAR *b, int ldb, primme_context ctx) {
   (void)ctx;

   PRIMME_BLASINT ln = n;
   PRIMME_BLASINT lnrhs = nrhs;
   PRIMME_BLASINT llda = lda;
   PRIMME_BLASINT lldb = ldb;
   PRIMME_BLASINT *lipivot = NULL;
   PRIMME_BLASINT linfo = 0;

   /* Zero dimension matrix may cause problems */
   if (n == 0 || nrhs == 0) return 0;

   if (sizeof(int) != sizeof(PRIMME_BLASINT)) {
      CHKERR(MALLOC_PRIMME(n, &lipivot));
      int i;
      for(i=0; i<n; i++) {
         lipivot[i] = (PRIMME_BLASINT)ipivot[i];
      }
   } else {
      lipivot = (PRIMME_BLASINT *)ipivot; /* cast avoid compiler warning */
   }

   XGETRS(trans, &ln, &lnrhs, a, &llda, lipivot, b, &lldb, &linfo);

   if (sizeof(int) != sizeof(PRIMME_BLASINT)) {
      free(lipivot);
   }

   CHKERRM(linfo != 0, PRIMME_LAPACK_FAILURE, "Error in xgetrs with info %d",
         (int)linfo);

   return 0;
}

#endif /* USE_HOST */
