/*******************************************************************************
 * Copyright (c) 2017, College of William & Mary
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
 * File: auxiliary.c
 *
 * Purpose - Miscellanea functions to copy and permuting matrices.
 *
 ******************************************************************************/


#include <string.h>   /* memmove */
#include <assert.h>
#include <math.h>
#include "template.h"
#include "auxiliary.h"
#include "blaslapack.h"
#include "magma_wrapper.h"

#ifdef USE_HOST

#if !(defined (__APPLE__) && defined (__MACH__))
#  include <malloc.h> /* malloc */
#endif
#include <stdlib.h>   /* malloc, free */

/******************************************************************************
 * Function Num_malloc - Allocate a vector of scalars
 *
 * PARAMETERS
 * ---------------------------
 * n           The number of elements
 * v           returned pointer
 * 
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_malloc_Sprimme(PRIMME_INT n, SCALAR **x, primme_context ctx) {
   (void)ctx;

   if (n <= 0) {
      *x = NULL;
      return 0;
   }
   *x = (SCALAR*)malloc(sizeof(SCALAR)*n);
   return *x == NULL ? PRIMME_MALLOC_FAILURE : 0;
}

/******************************************************************************
 * Function Num_free - Free allocated a vector of scalars
 *
 * PARAMETERS
 * ---------------------------
 * v           allocated pointer
 * 
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_free_Sprimme(SCALAR *x, primme_context ctx) {
   (void)ctx;

   if (x) free(x);
   return 0;
}

#ifdef USE_DOUBLE

/******************************************************************************
 * Function Num_malloc - Allocate a vector of integers
 *
 * PARAMETERS
 * ---------------------------
 * n           The number of elements
 * v           returned pointer
 * 
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_malloc_iprimme(PRIMME_INT n, int **x, primme_context ctx) {
   (void)ctx;

   if (n <= 0) {
      *x = NULL;
      return 0;
   }
   *x = (int*)malloc(sizeof(int)*n);
   return *x == NULL;
}

/******************************************************************************
 * Function Num_free - Free allocated a vector of integers
 *
 * PARAMETERS
 * ---------------------------
 * v           allocated pointer
 * 
 ******************************************************************************/

TEMPLATE_PLEASE
int Num_free_iprimme(int *x, primme_context ctx) {
   (void)ctx;

   if (x) free(x);
   return 0;
}

#endif /* USE_DOUBLE */

/******************************************************************************
 * Function Num_copy_matrix - Copy the matrix x into y
 *
 * PARAMETERS
 * ---------------------------
 * x           The source matrix
 * m           The number of rows of x
 * n           The number of columns of x
 * ldx         The leading dimension of x
 * y           On output y = x
 * ldy         The leading dimension of y
 *
 * NOTE: x and y *can* overlap
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_copy_matrix_Sprimme(SCALAR *x, PRIMME_INT m, PRIMME_INT n,
      PRIMME_INT ldx, SCALAR *y, PRIMME_INT ldy,
      primme_context ctx) {
   (void)ctx;

   PRIMME_INT i, j;

   assert(m == 0 || n == 0 || (ldx >= m && ldy >= m));

   /* Do nothing if x and y are the same matrix */
   if (x == y && ldx == ldy)
      return;

   /* Copy a contiguous memory region */
   if (ldx == ldy && ldx == m) {
      memmove(y, x, sizeof(SCALAR) * m * n);
   }

   /* Copy the matrix some rows back or forward */
   else if (ldx == ldy && (y > x ? y - x : x - y) < ldx) {
      for (i = 0; i < n; i++)
         memmove(&y[i * ldy], &x[i * ldx], sizeof(SCALAR) * m);
   }

   /* Copy the matrix some columns forward */
   else if (ldx == ldy && y > x && y - x > ldx) {
      for (i = n - 1; i >= 0; i--)
         for (j = 0; j < m; j++)
            y[i * ldy + j] = x[i * ldx + j];
   }

   /* Copy the matrix some columns backward, and other cases */
   else {
      /* TODO: assert x and y don't overlap */
      for (i = 0; i < n; i++)
         for (j = 0; j < m; j++) {
            // TEMP!!!
            if(REAL_PART(x[i*ldx+j]) > 0)
            y[i * ldy + j] = x[i * ldx + j];
            else
            y[i * ldy + j] = x[i * ldx + j];
         }
   }
}

/******************************************************************************
 * Function Num_copy_matrix_conj - Copy the matrix x' into y
 *
 * PARAMETERS
 * ---------------------------
 * x           The source matrix
 * m           The number of rows of x
 * n           The number of columns of x
 * ldx         The leading dimension of x
 * y           On output y = x
 * ldy         The leading dimension of y
 *
 * NOTE: x and y *can't* overlap
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_copy_matrix_conj_Sprimme(SCALAR *x, PRIMME_INT m, PRIMME_INT n,
      PRIMME_INT ldx, SCALAR *y, PRIMME_INT ldy, primme_context ctx) {
   (void)ctx;

   PRIMME_INT i, j;

   assert(m == 0 || n == 0 || (ldx >= m && ldy >= n));

   /* TODO: assert x and y don't overlap */
   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
            y[j * ldy + i] = CONJ(x[i * ldx + j]);
}


/******************************************************************************
 * Function Num_zero_matrix - Zero the matrix
 *
 * PARAMETERS
 * ---------------------------
 * x           The matrix
 * m           The number of rows of x
 * n           The number of columns of x
 * ldx         The leading dimension of x
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_zero_matrix_Sprimme(SCALAR *x, PRIMME_INT m, PRIMME_INT n,
      PRIMME_INT ldx, primme_context ctx) {
  (void)ctx;

   PRIMME_INT i,j;

   for (i=0; i<n; i++)
      for (j=0; j<m; j++)
         x[i*ldx+j] = 0.0;
} 

/******************************************************************************
 * Function Num_set_matrix - Set all elements in a matrix with a given value
 *
 * PARAMETERS
 * ---------------------------
 * x           The matrix
 * m           The number of rows of x
 * n           The number of columns of x
 * ldx         The leading dimension of x
 * value       The value to set
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_set_matrix_Sprimme(SCALAR *x, PRIMME_INT m, PRIMME_INT n,
      PRIMME_INT ldx, SCALAR value, primme_context ctx) {

  (void)ctx;
   PRIMME_INT i,j;

   for (i=0; i<n; i++)
      for (j=0; j<m; j++)
         x[i*ldx+j] = value;

} 

/******************************************************************************
 * Function Num_copy_trimatrix - Copy the upper/lower triangular part of the
 *    matrix x into y
 *
 * PARAMETERS
 * ---------------------------
 * x           The source matrix
 * m           The number of rows of x
 * n           The number of columns of x
 * ldx         The leading dimension of x
 * ul          if 0, copy the upper part; otherwise copy the lower part 
 * i0          The row index that diagonal starts from
 * y           On output y = x
 * ldy         The leading dimension of y
 * zero        If nonzero, zero the triangular part not copied
 *
 * NOTE: the distance between x and y can be less than ldx, or
 *       x and y *cannot* overlap at all
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_copy_trimatrix_Sprimme(SCALAR *x, int m, int n, int ldx, int ul,
      int i0, SCALAR *y, int ldy, int zero) {

   int i, j, jm;

   assert(m == 0 || n == 0 || (ldx >= m && ldy >= m));
   if (x == y) return;
   if (ul == 0) {
      /* Copy upper part */

      if (ldx == ldy && (y > x ? y-x : x-y) < ldx) {
         /* x and y overlap */
         for (i=0; i<n; i++) {
            memmove(&y[i*ldy], &x[i*ldx], sizeof(SCALAR)*min(i0+i+1, m));
            /* zero lower part*/
            if (zero) for (j=min(i0+i+1, m); j<m; j++) y[i*ldy+j] = 0.0;
         }
      }
      else {
         /* x and y don't overlap */
         for (i=0; i<n; i++) {
            for (j=0, jm=min(i0+i+1, m); j<jm; j++)
               y[i*ldy+j] = x[i*ldx+j];
            /* zero lower part*/
            if (zero) for (j=min(i0+i+1, m); j<m; j++) y[i*ldy+j] = 0.0;
         }
      }
   }
   else {
      /* Copy lower part */

      if (ldx == ldy && (y > x ? y-x : x-y) < ldx) {
         /* x and y overlap */
         for (i=0; i<n; i++) {
            memmove(&y[i*ldy+i0+i], &x[i*ldx+i0+i], sizeof(SCALAR)*(m-min(i0+i, m)));
            /* zero upper part*/
            if (zero) for (j=0, jm=min(i0+i, m); j<jm; j++) y[i*ldy+j] = 0.0;
         }
      }
      else {
         /* x and y don't overlap */
         for (i=0; i<n; i++) {
            for (j=i+i0; j<m; j++)
               y[i*ldy+j] = x[i*ldx+j];
            /* zero upper part*/
            if (zero) for (j=0, jm=min(i0+i, m); j<jm; j++) y[i*ldy+j] = 0.0;
         }
      }
   }
}


/******************************************************************************
 * Function Num_copy_trimatrix_compact - Copy the upper triangular part of the matrix x
 *    into y contiguously, i.e., y has all columns of x row-stacked
 *
 * PARAMETERS
 * ---------------------------
 * x           The source upper triangular matrix
 * m           The number of rows of x
 * n           The number of columns of x
 * ldx         The leading dimension of x
 * i0          The row index that diagonal starts from
 * y           On output y = x and nonzero elements of y are contiguous
 * ly          Output the final length of y
 *
 * NOTE: x and y *cannot* overlap
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_copy_trimatrix_compact_Sprimme(SCALAR *x, PRIMME_INT m, int n,
      PRIMME_INT ldx, int i0, SCALAR *y, int *ly) {

   int i, j, k;

   assert(m == 0 || n == 0 || ldx >= m);

   for (i=0, k=0; i<n; i++)
      for (j=0; j<=i+i0; j++)
         y[k++] = x[i*ldx+j];
   if (ly) *ly = k;
}

/******************************************************************************
 * Function Num_copy_compact_trimatrix - Copy y into the upper triangular part of the
 *    matrix x
 *
 * PARAMETERS
 * ---------------------------
 * x           The source vector
 * m           The number of rows of y
 * n           The number of columns of y
 * i0          The row index that diagonal starts from
 * y           On output the upper triangular part of y has x
 * ldy         The leading dimension of y
 *
 * NOTE: x and y *cannot* overlap
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_copy_compact_trimatrix_Sprimme(SCALAR *x, PRIMME_INT m, int n, int i0,
      SCALAR *y, int ldy) {

   int i, j, k;

   assert(m == 0 || n == 0 || (ldy >= m && m >= n));

   for (i=n-1, k=(n+1)*n/2+i0*n-1; i>=0; i--)
      for (j=i+i0; j>=0; j--)
         y[i*ldy+j] = x[k--];
}

/*******************************************************************************
 * Subroutine compute_submatrix - This subroutine computes the nX x nX submatrix
 *    R = X'*H*X, where H stores the upper triangular part of a symmetric matrix.
 *    
 * Input parameters
 * ----------------
 * X        The coefficient vectors retained from the previous iteration
 *
 * nX       Number of columns of X
 *
 * H        Matrix
 *
 * nH       Dimension of H
 *
 * ldH      Leading dimension of H
 *
 * rwork    Work array.  Must be of size nH x nX
 *
 * lrwork   Length of the work array
 *
 * ldR      Leading dimension of R
 *
 * Output parameters
 * -----------------
 * R - nX x nX matrix computed 
 *
 ******************************************************************************/

TEMPLATE_PLEASE
int compute_submatrix_Sprimme(SCALAR *X, int nX, int ldX, SCALAR *H, int nH,
                              int ldH, SCALAR *R, int ldR, primme_context ctx) {

  if (nH == 0 || nX == 0)
    return 0;

   SCALAR *rwork;
   CHKERR(Num_malloc_Sprimme((size_t)nH * (size_t)nX, &rwork, ctx));
   Num_hemm_Sprimme("L", "U", nH, nX, 1.0, H, ldH, X, ldX, 0.0, rwork, nH);
   Num_gemm_Sprimme("C", "N", nX, nX, nH, 1.0, X, ldX, rwork, nH, 0.0, R, ldR,
                    ctx);
   CHKERR(Num_free_Sprimme(rwork, ctx));

  return 0;
}

#endif /* USE_HOST */

/******************************************************************************
 * Function Num_copy_matrix_columns - Copy the matrix x(:,xin) into y(:,yin)
 *
 * PARAMETERS
 * ---------------------------
 * x           The source matrix
 * m           The number of rows of x
 * xin         The column indices to copy
 * n           The number of columns of x
 * ldx         The leading dimension of x
 * y           On output y(:,yin) = x(:,xin)
 * yin         The column indices of y to be modified
 * ldy         The leading dimension of y
 *
 * NOTE: x(:,xin) and y(:,yin) *cannot* overlap
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_copy_matrix_columns_Sprimme(SCALAR *x, PRIMME_INT m, int *xin, int n,
                                     PRIMME_INT ldx, SCALAR *y, int *yin,
                                     PRIMME_INT ldy, primme_context ctx) {

  int i;

  /* TODO: assert x and y don't overlap */
  for (i = 0; i < n; i++) {
    Num_copy_Sprimme(m, &x[(xin ? xin[i] : i) * ldx], 1,
                     &y[(yin ? yin[i] : i) * ldy], 1, ctx);
  }
}

/******************************************************************************
 * Function Num_copy_matrix_rows - Copy the matrix x(xin,:) into y(yin,:)
 *
 * PARAMETERS
 * ---------------------------
 * x           The source matrix
 * xim         The row indices to copy
 * m           The number of rows of x
 * n           The number of columns of x
 * ldx         The leading dimension of x
 * y           On output y(yin,:) = x(xin,:)
 * yim         The row indices of y to be modified
 * ldy         The leading dimension of y
 *
 * NOTE: x(xin,:) and y(yin,:) *cannot* overlap
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_copy_matrix_rows_Sprimme(SCALAR *x, int *xim, int m, int n,
                                     PRIMME_INT ldx, SCALAR *y, int *yim,
                                     PRIMME_INT ldy, primme_context ctx) {

  int i;

  /* TODO: assert x and y don't overlap */
  for (i = 0; i < m; i++) {
    Num_copy_Sprimme(n, &x[xim ? xim[i] : i], ldx,
                     &y[yim ? yim[i] : i], ldy, ctx);
  }
}

/******************************************************************************
 * Subroutine permute_vecs - This routine permutes a set of vectors according
 *            to a permutation array perm.
 *
 * INPUT ARRAYS AND PARAMETERS
 * ---------------------------
 * m, n, ld    The number of rows and columns and the leading dimension of
 *vecs perm        The permutation of the columns rwork       Temporary space
 *of size the number of rows iwork       Temporary space of size the number
 *of columns
 *
 * INPUT/OUTPUT ARRAYS
 * -------------------
 * vecs        The matrix whose columns will be reordered
 *
 ******************************************************************************/

TEMPLATE_PLEASE
int permute_vecs_Sprimme(SCALAR *vecs, PRIMME_INT m, int n, PRIMME_INT ld,
                         int *perm_, primme_context ctx) {

   int currentIndex;     /* Index of vector in sorted order                   */
   int sourceIndex;      /* Position of out-of-order vector in original order */
   int destinationIndex; /* Position of out-of-order vector in sorted order */
   int tempIndex;        /* Used to swap                                      */
   int *perm;            /* A copy of perm_                                   */
   SCALAR *rwork;        /* vector buffer */

   CHKERR(Num_malloc_iprimme(n, &perm, ctx));
   CHKERR(Num_malloc_Sprimme(m, &rwork, ctx));

   /* Check perm_ is a permutation */

#ifndef NDEBUG
   for (tempIndex = 0; tempIndex < n; tempIndex++)
      perm[tempIndex] = 0;
   for (tempIndex = 0; tempIndex < n; tempIndex++) {
      assert(0 <= perm_[tempIndex] && perm_[tempIndex] < n);
      perm[perm_[tempIndex]] = 1;
   }
   for (tempIndex = 0; tempIndex < n; tempIndex++)
      assert(perm[tempIndex] == 1);
#endif

   /* Copy of perm_ into perm, to avoid to modify the input permutation */

   for (tempIndex = 0; tempIndex < n; tempIndex++)
      perm[tempIndex] = perm_[tempIndex];

   /* Continue until all vectors are in the sorted order */

   currentIndex = 0;
   while (1) {

      /* Find a vector that does not belong in its original position */
      while ((currentIndex < n) && (perm[currentIndex] == currentIndex)) {
         currentIndex++;
      }

      /* Return if they are in the sorted order */
      if (currentIndex >= n) {
         break;
      }

      /* Copy the vector to a buffer for swapping */
      Num_copy_Sprimme(m, &vecs[currentIndex * ld], 1, rwork, 1, ctx);

      destinationIndex = currentIndex;
      /* Copy vector perm[destinationIndex] into position destinationIndex */

      while (perm[destinationIndex] != currentIndex) {

         sourceIndex = perm[destinationIndex];
         Num_copy_Sprimme(m, &vecs[sourceIndex * ld], 1,
                          &vecs[destinationIndex * ld], 1, ctx);
         tempIndex = perm[destinationIndex];
         perm[destinationIndex] = destinationIndex;
         destinationIndex = tempIndex;
      }

      /* Copy the vector from the buffer to where it belongs */
      Num_copy_Sprimme(m, rwork, 1, &vecs[destinationIndex * ld], 1, ctx);
      perm[destinationIndex] = destinationIndex;

      currentIndex++;
   }

   /* Check permutation */
   for (currentIndex = 0; currentIndex < n; currentIndex++)
      assert(perm[currentIndex] == currentIndex);

   CHKERR(Num_free_iprimme(perm, ctx));
   CHKERR(Num_free_Sprimme(rwork, ctx));

   return 0;
}

#ifdef USE_DOUBLE
TEMPLATE_PLEASE
int permute_vecs_iprimme(int *vecs, int n, int *perm_, primme_context ctx) {

   int currentIndex;     /* Index of vector in sorted order                   */
   int sourceIndex;      /* Position of out-of-order vector in original order */
   int destinationIndex; /* Position of out-of-order vector in sorted order   */
   int tempIndex;        /* Used to swap                                      */
   int *perm;            /* A copy of perm_                                   */
   int aux;

   /* Check that perm_ and iwork do not overlap */

   CHKERR(Num_malloc_iprimme(n, &perm, ctx));

   /* Check perm_ is a permutation */

#ifndef NDEBUG
   for (tempIndex=0; tempIndex<n; tempIndex++) perm[tempIndex] = 0;
   for (tempIndex=0; tempIndex<n; tempIndex++) {
      assert(0 <= perm_[tempIndex] && perm_[tempIndex] < n);
      perm[perm_[tempIndex]] = 1;
   }
   for (tempIndex=0; tempIndex<n; tempIndex++) assert(perm[tempIndex] == 1);
#endif

   /* Copy of perm_ into perm, to avoid to modify the input permutation */

   for (tempIndex=0; tempIndex<n; tempIndex++)
      perm[tempIndex] = perm_[tempIndex];

   /* Continue until all vectors are in the sorted order */

   currentIndex = 0;
   while (1) {

      /* Find a vector that does not belong in its original position */
      while ((currentIndex < n) && (perm[currentIndex] == currentIndex)) {
         currentIndex++;
      }

      /* Return if they are in the sorted order */
      if (currentIndex >= n) {
         break;
      }

      /* Copy the vector to a buffer for swapping */
      aux = vecs[currentIndex];

      destinationIndex = currentIndex;
      /* Copy vector perm[destinationIndex] into position destinationIndex */

      while (perm[destinationIndex] != currentIndex) {

         sourceIndex = perm[destinationIndex];
         vecs[destinationIndex] = vecs[sourceIndex];
         tempIndex = perm[destinationIndex];
         perm[destinationIndex] = destinationIndex;
         destinationIndex = tempIndex;
      }

      /* Copy the vector from the buffer to where it belongs */
      vecs[destinationIndex] = aux;
      perm[destinationIndex] = destinationIndex;

      currentIndex++;
   }

   /* Check permutation */
   for (currentIndex=0; currentIndex < n; currentIndex++)
      assert(perm[currentIndex] == currentIndex);

   CHKERR(Num_free_iprimme(perm, ctx));

   return 0;
}
#endif


/******************************************************************************
 * Subroutine Num_compact_vecs - copy certain columns of matrix into another
 *       matrix, i.e., work = vecs(perm). If avoidCopy and perm indices are
 *       consecutive the routine returns a reference in vecs and doesn't copy.
 *            
 *
 * PARAMETERS
 * ---------------------------
 * 
 * vecs        The matrix
 * m           The number of rows of vecs
 * n           The number of columns of to copy
 * ld          The leading dimension of vecs
 * perm        The indices of columns to copy
 * work        The columns are copied to this matrix
 * ldwork      The leading dimension of work
 * avoidCopy   If nonzero, the copy is avoid
 *
 * return      Reference of a matrix with the columns ordered as perm
 *
 ******************************************************************************/

TEMPLATE_PLEASE
SCALAR* Num_compact_vecs_Sprimme(SCALAR *vecs, PRIMME_INT m, int n, 
      PRIMME_INT ld, int *perm, SCALAR *work, PRIMME_INT ldwork,
      int avoidCopy, primme_context ctx) {

   int i;

   if (avoidCopy) {
      for (i=0; i<n-1 && perm[i]+1 == perm[i+1]; i++);
      if (i >= n-1) return &vecs[ld*perm[0]];
   }

   for (i=0; i < n; i++) {
     Num_copy_matrix_Sprimme(&vecs[perm[i] * ld], m, 1, ld, &work[i * ldwork],
                             ld, ctx);
   }
   return work;
}

/******************************************************************************
 * Function Num_scale_matrix - Scale the column of the matrix x into y
 *
 * PARAMETERS
 * ---------------------------
 * x           The source matrix
 * m           The number of rows of x
 * n           The number of columns of x
 * ldx         The leading dimension of x
 * s           The scales
 * y           On output y = x
 * ldy         The leading dimension of y
 *
 * NOTE: x and y *can* overlap
 *
 ******************************************************************************/

TEMPLATE_PLEASE
void Num_scale_matrix_Sprimme(SCALAR *x, PRIMME_INT m, PRIMME_INT n,
      PRIMME_INT ldx, HREAL *s, SCALAR *y, PRIMME_INT ldy, primme_context ctx) {

   PRIMME_INT i;

   assert(m == 0 || n == 0 || (ldx >= m && ldy >= m));

   /* Copy the matrix some columns backward, and other cases */
   /* TODO: assert x and y don't overlap */
   for (i=0; i<n; i++) {
      Num_copy_Sprimme(m, &x[ldx*i], 1, &y[ldy*i], 1, ctx);
      Num_scal_Sprimme(m, s[i], &y[ldy*i], 1, ctx);
   }
}
