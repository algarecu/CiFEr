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

#include "cifer/test.h"
#include "cifer/abe/fame.h"

MunitResult test_fame_end_to_end(const MunitParameter *params, void *data) {
    // create a new FAME struct
    cfe_fame fame;
    cfe_fame_init(&fame);

    // initialize and generate a public key and a secret key for the scheme
    cfe_fame_pub_key pk;
    cfe_fame_sec_key sk;
    cfe_fame_sec_key_init(&sk);
    cfe_fame_generate_master_keys(&pk, &sk, &fame);

    // create a message to be encrypted
    FP12_BN254 msg;
    FP12_BN254_one(&msg);

    // create a msp structure out of a boolean expression representing the
    // policy specifying which attributes are needed to decrypt the ciphertext
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
    cfe_msp msp;
    cfe_error err = cfe_boolean_to_msp(&msp, bool_exp, false);
    munit_assert(err == CFE_ERR_NONE);

    // initialize a ciphertext and encrypt the message based on the msp structure
    // describing the policy
    cfe_fame_cipher cipher;
    cfe_fame_cipher_init(&cipher, &msp);
    cfe_fame_encrypt(&cipher, &msg, &msp, &pk, &fame);

    // produce keys that are given to an entity with a set
    // of attributes in owned_attrib
    int owned_attrib[] = {1, 3, 6};
    cfe_fame_attrib_keys keys;
    cfe_fame_attrib_keys_init(&keys, 3); // the number of attributes needs to be specified
    cfe_fame_generate_attrib_keys(&keys, owned_attrib, 3, &sk, &fame);

    // decrypt the message with owned keys
    FP12_BN254 decryption;
    err = cfe_fame_decrypt(&decryption, &cipher, &keys, &fame);
    munit_assert(err == CFE_ERR_NONE);

    // check if the decryption equals the starting message
    munit_assert(FP12_BN254_equals(&msg, &decryption) == 1);

    //clear up
    cfe_fame_free(&fame);
    cfe_fame_sec_key_free(&sk);
    cfe_fame_cipher_free(&cipher);
    cfe_fame_attrib_keys_free(&keys);
    cfe_msp_free(&msp);

    return MUNIT_OK;
}

MunitTest fame_tests[] = {
        {(char *) "/end-to-end", test_fame_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite fame_suite = {
        (char *) "/abe/fame", fame_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
