/*
 * Copyright (c) 2018 XLAB d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "munit.h"

#include "cifer/sample/uniform.h"
#include "cifer/abe/policy.h"


MunitResult test_boolean_to_msp(const MunitParameter params[], void *data) {
    // define a boolean expression and make a corresponding msp structure
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
    cfe_msp msp;
    cfe_error check = cfe_boolean_to_msp(&msp, bool_exp, true);
    munit_assert(check == CFE_ERR_NONE);

    // create parameters to test msp
    cfe_mat sub_mat, sub_mat_transpose;
    cfe_mat_init(&sub_mat, 2, msp.mat.cols);
    cfe_mat_init(&sub_mat_transpose, msp.mat.cols, 2);

    // take a submatrix of the msp matrix on the second and fourth row
    // simulating that one has attribute 3 and 4.
    cfe_vec_copy(&(sub_mat.mat[0]), &(msp.mat.mat[1]));
    cfe_vec_copy(&(sub_mat.mat[1]), &(msp.mat.mat[3]));

    // test if the rows of sub_mat span tha vector [1, 1,..., 1]
    // using gaussian elimination
    cfe_mat_transpose(&sub_mat_transpose, &sub_mat);
    cfe_vec one_vec, x;
    mpz_t one, p;
    mpz_init_set_ui(one, 1);
    cfe_vec_init(&one_vec, sub_mat.cols);
    cfe_vec_set_const(&one_vec, one);

    mpz_init_set_ui(p, 17);
    check = cfe_gaussian_elimination(&x, &sub_mat_transpose, &one_vec, p);
    munit_assert(check == CFE_ERR_NONE);

    // define a faulty boolean expression and check for error
    char bool_exp_faulty[] = "(5 OR a3) AND ((2 OR 4) OR (1 AND 6))";
    check = cfe_boolean_to_msp(&msp, bool_exp_faulty, true);
    munit_assert(check == CFE_ERR_CORRUPTED_BOOL_EXPRESSION);

    // cleanup
    cfe_msp_free(&msp);
    cfe_mat_frees(&sub_mat, &sub_mat_transpose, NULL);
    cfe_vec_frees(&one_vec, &x, NULL);
    mpz_clears(one, p, NULL);

    return MUNIT_OK;
}

MunitResult test_gaussian_elimination(const MunitParameter params[], void *data) {
    // define parameters
    mpz_t p;
    mpz_init_set_ui(p, 17);
    cfe_mat mat;
    cfe_mat_init(&mat, 100, 50);
    cfe_vec x_test, x, vec;
    cfe_vec_init(&x_test, 50);
    cfe_vec_init(&vec, 100);

    // sample random mat, x_test and calculate vec = mat * x_test
    cfe_uniform_sample_vec(&x_test, p);
    cfe_uniform_sample_mat(&mat, p);
    cfe_mat_mul_vec(&vec, &mat, &x_test);
    cfe_vec_mod(&vec, &vec, p);

    // use gaussian elimination to get x solving the equation vec = mat * x
    cfe_error err = cfe_gaussian_elimination(&x, &mat, &vec, p);
    munit_assert(err == 0);

    // check if the result is correct, i.e. x_test = x
    for (size_t i = 0; i < x.size; i++) {
        munit_assert(mpz_cmp(x.vec[i], x_test.vec[i]) == 0);
    }

    // clear up
    mpz_clear(p);
    cfe_vec_frees(&x_test, &x, &vec, NULL);
    cfe_mat_free(&mat);
    return MUNIT_OK;
}

MunitResult test_str_to_int(const MunitParameter params[], void *data) {
    // test conversion from string to int
    char test_str[] = "15";
    int i = cfe_str_to_int(test_str);
    munit_assert(i == 15);

    // return an error (-1) if string is corrupted
    char test_faulty_str[] = "124h234";
    i = cfe_str_to_int(test_faulty_str);
    munit_assert(i == -1);

    return MUNIT_OK;
}

MunitTest policy_tests[] = {
        {(char *) "/test-boolean_to_msp",       test_boolean_to_msp,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-gaussian-elimination", test_gaussian_elimination, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-str-to-int",           test_str_to_int,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                                       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite policy_suite = {
        (char *) "/abe/policy", policy_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
