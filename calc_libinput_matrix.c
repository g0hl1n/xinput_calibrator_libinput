// SPDX-License-Identifier: Unlicense

/*
 * libinput calibration calculation
 */

#include <stdio.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

/*
 * Compute the (Moore-Penrose) pseudo-inverse of a matrix.
 * 
 * PUBLIC DOMAIN by Charl Linssen <charl@itfromb.it>
 *
 * If the singular value decomposition (SVD) of A = UΣVᵀ then the
 * pseudoinverse A⁻¹ = VΣ⁻¹Uᵀ, where ᵀ indicates transpose and
 * Σ⁻¹ is obtained by taking the reciprocal of each nonzero element on
 * the diagonal, leaving zeros in place. Elements on the diagonal smaller
 * than ``rcond`` times the largest singular value are considered zero.
 *
 * @parameter A		Input matrix. **WARNING**: the input matrix ``A`` is
 * 			destroyed. However, it is still the responsibility of
 * 			the caller to free it.
 *
 * @parameter rcond	A real number specifying the singular value threshold
 * 			for inclusion. NumPy default for ``rcond`` is 1E-15.
 *
 * @returns A_pinv	Matrix containing the result. ``A_pinv`` is allocated
 * 			in this function and it is the responsibility of the
 * 			caller to free it.
 * 
 *
 */
gsl_matrix* moore_penrose_pinv(gsl_matrix *A, const double rcond) {

	gsl_matrix *V, *Sigma_pinv, *U, *A_pinv;
	gsl_matrix *_tmp_mat = NULL;
	gsl_vector *_tmp_vec;
	gsl_vector *u;
	double x, cutoff;
	size_t i, j;
	unsigned int n = A->size1;
	unsigned int m = A->size2;
	unsigned int was_swapped = 0;


	if (m > n) {
		/* libgsl SVD can only handle the case m <= n - transpose matrix */
		was_swapped = 1;
		_tmp_mat = gsl_matrix_alloc(m, n);
		gsl_matrix_transpose_memcpy(_tmp_mat, A);
		A = _tmp_mat;
		i = m;
		m = n;
		n = i;
	}

	/* do SVD */
	V = gsl_matrix_alloc(m, m);
	u = gsl_vector_alloc(m);
	_tmp_vec = gsl_vector_alloc(m);
	gsl_linalg_SV_decomp(A, V, u, _tmp_vec);
	gsl_vector_free(_tmp_vec);

	/* compute Σ⁻¹ */
	Sigma_pinv = gsl_matrix_alloc(m, n);
	gsl_matrix_set_zero(Sigma_pinv);
	cutoff = rcond * gsl_vector_max(u);

	for (i = 0; i < m; ++i) {
		if (gsl_vector_get(u, i) > cutoff) {
			x = 1. / gsl_vector_get(u, i);
		}
		else {
			x = 0.;
		}
		gsl_matrix_set(Sigma_pinv, i, i, x);
	}

	/* libgsl SVD yields "thin" SVD - pad to full matrix by adding zeros */
	U = gsl_matrix_alloc(n, n);
	gsl_matrix_set_zero(U);

	for (i = 0; i < n; ++i) {
		for (j = 0; j < m; ++j) {
			gsl_matrix_set(U, i, j, gsl_matrix_get(A, i, j));
		}
	}

	if (_tmp_mat != NULL) {
		gsl_matrix_free(_tmp_mat);
	}

	/* two dot products to obtain pseudoinverse */
	_tmp_mat = gsl_matrix_alloc(m, n);
	gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1., V, Sigma_pinv, 0., _tmp_mat);

	if (was_swapped) {
		A_pinv = gsl_matrix_alloc(n, m);
		gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1., U, _tmp_mat, 0., A_pinv);
	}
	else {
		A_pinv = gsl_matrix_alloc(m, n);
		gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1., _tmp_mat, U, 0., A_pinv);
	}

	gsl_matrix_free(_tmp_mat);
	gsl_matrix_free(U);
	gsl_matrix_free(Sigma_pinv);
	gsl_vector_free(u);
	gsl_matrix_free(V);

	return A_pinv;
}

int main(int argc, char **argv) {
	const unsigned int col = 4;
	const unsigned int row = 3;
	const double rcond = 1E-15;

	const double oneEigth = 1.0/8.0;
	const double sevenEight = 7.0/8.0;

	if (argc < 11) {
		printf("usage: %s x-res y-res x0 y0 x1 y1 x2 y2 x3 y3 [swap-x-y]\n", argv[0]);
		return EXIT_FAILURE;
	}

        int xres = atoi(argv[1]);
        int yres = atoi(argv[2]);
        int x0in = atoi(argv[3]);
        int y0in = atoi(argv[4]);
        int x1in = atoi(argv[5]);
        int y1in = atoi(argv[6]);
        int x2in = atoi(argv[7]);
        int y2in = atoi(argv[8]);
        int x3in = atoi(argv[9]);
        int y3in = atoi(argv[10]);

	int swapxy = 0;
	if (argc > 11)
		swapxy = 1;

	double x0 = (double)x0in / (double)xres;
	double y0 = (double)y0in / (double)yres;
	double x1 = (double)x1in / (double)xres;
	double y1 = (double)y1in / (double)yres;
	double x2 = (double)x2in / (double)xres;
	double y2 = (double)y2in / (double)yres;
	double x3 = (double)x3in / (double)xres;
	double y3 = (double)y3in / (double)yres;


	gsl_matrix *A = gsl_matrix_alloc(row, col);
	gsl_matrix *C = gsl_matrix_alloc(row, col);
	gsl_matrix *R = gsl_matrix_alloc(3, 3);
	gsl_matrix *S;
	gsl_matrix *T;
	gsl_matrix *A_pinv;

	gsl_matrix_set(A, 0, 0, x0);
	gsl_matrix_set(A, 0, 1, x1);
	gsl_matrix_set(A, 0, 2, x2);
	gsl_matrix_set(A, 0, 3, x3);
	gsl_matrix_set(A, 1, 0, y0);
	gsl_matrix_set(A, 1, 1, y1);
	gsl_matrix_set(A, 1, 2, y2);
	gsl_matrix_set(A, 1, 3, y3);
	gsl_matrix_set(A, 2, 0, 1.0);
	gsl_matrix_set(A, 2, 1, 1.0);
	gsl_matrix_set(A, 2, 2, 1.0);
	gsl_matrix_set(A, 2, 3, 1.0);

	gsl_matrix_set(C, 0, 0, oneEigth);
	gsl_matrix_set(C, 0, 1, sevenEight);
	gsl_matrix_set(C, 0, 2, oneEigth);
	gsl_matrix_set(C, 0, 3, sevenEight);
	gsl_matrix_set(C, 1, 0, oneEigth);
	gsl_matrix_set(C, 1, 1, oneEigth);
	gsl_matrix_set(C, 1, 2, sevenEight);
	gsl_matrix_set(C, 1, 3, sevenEight);
	gsl_matrix_set(C, 2, 0, 1.0);
	gsl_matrix_set(C, 2, 1, 1.0);
	gsl_matrix_set(C, 2, 2, 1.0);
	gsl_matrix_set(C, 2, 3, 1.0);

	A_pinv = moore_penrose_pinv(A, rcond);
	gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, C, A_pinv, 0.0, R);

	if (swapxy) {
		S = gsl_matrix_alloc(3, 3);

		/* extra swapping x-y axes as requested */
		gsl_matrix_set(S, 0, 0,  0.0);
		gsl_matrix_set(S, 0, 1, -1.0);
		gsl_matrix_set(S, 0, 2,  1.0);
		gsl_matrix_set(S, 1, 0, -1.0);
		gsl_matrix_set(S, 1, 1,  0.0);
		gsl_matrix_set(S, 1, 2,  1.0);
		gsl_matrix_set(S, 2, 0,  0.0);
		gsl_matrix_set(S, 2, 1,  0.0);
		gsl_matrix_set(S, 2, 2,  1.0);

		T = R;
		R = gsl_matrix_alloc(3, 3);
		gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, T, S, 0.0, R);
	}

        printf("%f, %f, %f, %f, %f, %f, %f, %f, %f\n",
                gsl_matrix_get((const gsl_matrix *)R, 0, 0), gsl_matrix_get((const gsl_matrix *)R, 0, 1), gsl_matrix_get((const gsl_matrix *)R, 0, 2),
                gsl_matrix_get((const gsl_matrix *)R, 1, 0), gsl_matrix_get((const gsl_matrix *)R, 1, 1), gsl_matrix_get((const gsl_matrix *)R, 1, 2),
                gsl_matrix_get((const gsl_matrix *)R, 2, 0), gsl_matrix_get((const gsl_matrix *)R, 2, 1), gsl_matrix_get((const gsl_matrix *)R, 2, 2));

	gsl_matrix_free(A);
	gsl_matrix_free(C);
	gsl_matrix_free(R);
	gsl_matrix_free(A_pinv);

	if (swapxy) {
		gsl_matrix_free(S);
		gsl_matrix_free(T);
	}
	return EXIT_SUCCESS;
}
