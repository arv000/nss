/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nss.h"
#include "pk11pub.h"
#include "sechash.h"
#include <memory>

#include "gtest/gtest.h"
#include "scoped_ptrs.h"

namespace nss_test {

// RSA-PSS test vectors, pss-vect.txt, Example 1: A 1024-bit RSA Key Pair
// <ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1-vec.zip>
const uint8_t kTestVector1Spki[] = {
  0x30, 0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
  0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81, 0x89, 0x02,
  0x81, 0x81, 0x00, 0xa5, 0x6e, 0x4a, 0x0e, 0x70, 0x10, 0x17, 0x58, 0x9a, 0x51,
  0x87, 0xdc, 0x7e, 0xa8, 0x41, 0xd1, 0x56, 0xf2, 0xec, 0x0e, 0x36, 0xad, 0x52,
  0xa4, 0x4d, 0xfe, 0xb1, 0xe6, 0x1f, 0x7a, 0xd9, 0x91, 0xd8, 0xc5, 0x10, 0x56,
  0xff, 0xed, 0xb1, 0x62, 0xb4, 0xc0, 0xf2, 0x83, 0xa1, 0x2a, 0x88, 0xa3, 0x94,
  0xdf, 0xf5, 0x26, 0xab, 0x72, 0x91, 0xcb, 0xb3, 0x07, 0xce, 0xab, 0xfc, 0xe0,
  0xb1, 0xdf, 0xd5, 0xcd, 0x95, 0x08, 0x09, 0x6d, 0x5b, 0x2b, 0x8b, 0x6d, 0xf5,
  0xd6, 0x71, 0xef, 0x63, 0x77, 0xc0, 0x92, 0x1c, 0xb2, 0x3c, 0x27, 0x0a, 0x70,
  0xe2, 0x59, 0x8e, 0x6f, 0xf8, 0x9d, 0x19, 0xf1, 0x05, 0xac, 0xc2, 0xd3, 0xf0,
  0xcb, 0x35, 0xf2, 0x92, 0x80, 0xe1, 0x38, 0x6b, 0x6f, 0x64, 0xc4, 0xef, 0x22,
  0xe1, 0xe1, 0xf2, 0x0d, 0x0c, 0xe8, 0xcf, 0xfb, 0x22, 0x49, 0xbd, 0x9a, 0x21,
  0x37, 0x02, 0x03, 0x01, 0x00, 0x01
};
// RSA-PSS test vectors, pss-vect.txt, Example 1.1
const uint8_t kTestVector1Data[] = {
  0xcd, 0xc8, 0x7d, 0xa2, 0x23, 0xd7, 0x86, 0xdf, 0x3b, 0x45, 0xe0, 0xbb, 0xbc,
  0x72, 0x13, 0x26, 0xd1, 0xee, 0x2a, 0xf8, 0x06, 0xcc, 0x31, 0x54, 0x75, 0xcc,
  0x6f, 0x0d, 0x9c, 0x66, 0xe1, 0xb6, 0x23, 0x71, 0xd4, 0x5c, 0xe2, 0x39, 0x2e,
  0x1a, 0xc9, 0x28, 0x44, 0xc3, 0x10, 0x10, 0x2f, 0x15, 0x6a, 0x0d, 0x8d, 0x52,
  0xc1, 0xf4, 0xc4, 0x0b, 0xa3, 0xaa, 0x65, 0x09, 0x57, 0x86, 0xcb, 0x76, 0x97,
  0x57, 0xa6, 0x56, 0x3b, 0xa9, 0x58, 0xfe, 0xd0, 0xbc, 0xc9, 0x84, 0xe8, 0xb5,
  0x17, 0xa3, 0xd5, 0xf5, 0x15, 0xb2, 0x3b, 0x8a, 0x41, 0xe7, 0x4a, 0xa8, 0x67,
  0x69, 0x3f, 0x90, 0xdf, 0xb0, 0x61, 0xa6, 0xe8, 0x6d, 0xfa, 0xae, 0xe6, 0x44,
  0x72, 0xc0, 0x0e, 0x5f, 0x20, 0x94, 0x57, 0x29, 0xcb, 0xeb, 0xe7, 0x7f, 0x06,
  0xce, 0x78, 0xe0, 0x8f, 0x40, 0x98, 0xfb, 0xa4, 0x1f, 0x9d, 0x61, 0x93, 0xc0,
  0x31, 0x7e, 0x8b, 0x60, 0xd4, 0xb6, 0x08, 0x4a, 0xcb, 0x42, 0xd2, 0x9e, 0x38,
  0x08, 0xa3, 0xbc, 0x37, 0x2d, 0x85, 0xe3, 0x31, 0x17, 0x0f, 0xcb, 0xf7, 0xcc,
  0x72, 0xd0, 0xb7, 0x1c, 0x29, 0x66, 0x48, 0xb3, 0xa4, 0xd1, 0x0f, 0x41, 0x62,
  0x95, 0xd0, 0x80, 0x7a, 0xa6, 0x25, 0xca, 0xb2, 0x74, 0x4f, 0xd9, 0xea, 0x8f,
  0xd2, 0x23, 0xc4, 0x25, 0x37, 0x02, 0x98, 0x28, 0xbd, 0x16, 0xbe, 0x02, 0x54,
  0x6f, 0x13, 0x0f, 0xd2, 0xe3, 0x3b, 0x93, 0x6d, 0x26, 0x76, 0xe0, 0x8a, 0xed,
  0x1b, 0x73, 0x31, 0x8b, 0x75, 0x0a, 0x01, 0x67, 0xd0
};
const uint8_t kTestVector1Sig[] = {
  0x90, 0x74, 0x30, 0x8f, 0xb5, 0x98, 0xe9, 0x70, 0x1b, 0x22, 0x94, 0x38, 0x8e,
  0x52, 0xf9, 0x71, 0xfa, 0xac, 0x2b, 0x60, 0xa5, 0x14, 0x5a, 0xf1, 0x85, 0xdf,
  0x52, 0x87, 0xb5, 0xed, 0x28, 0x87, 0xe5, 0x7c, 0xe7, 0xfd, 0x44, 0xdc, 0x86,
  0x34, 0xe4, 0x07, 0xc8, 0xe0, 0xe4, 0x36, 0x0b, 0xc2, 0x26, 0xf3, 0xec, 0x22,
  0x7f, 0x9d, 0x9e, 0x54, 0x63, 0x8e, 0x8d, 0x31, 0xf5, 0x05, 0x12, 0x15, 0xdf,
  0x6e, 0xbb, 0x9c, 0x2f, 0x95, 0x79, 0xaa, 0x77, 0x59, 0x8a, 0x38, 0xf9, 0x14,
  0xb5, 0xb9, 0xc1, 0xbd, 0x83, 0xc4, 0xe2, 0xf9, 0xf3, 0x82, 0xa0, 0xd0, 0xaa,
  0x35, 0x42, 0xff, 0xee, 0x65, 0x98, 0x4a, 0x60, 0x1b, 0xc6, 0x9e, 0xb2, 0x8d,
  0xeb, 0x27, 0xdc, 0xa1, 0x2c, 0x82, 0xc2, 0xd4, 0xc3, 0xf6, 0x6c, 0xd5, 0x00,
  0xf1, 0xff, 0x2b, 0x99, 0x4d, 0x8a, 0x4e, 0x30, 0xcb, 0xb3, 0x3c
};

// RSA-PSS test vectors, pss-vect.txt, Example 10: A 2048-bit RSA Key Pair
// <ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1-vec.zip>
const uint8_t kTestVector2Spki[] = {
  0x30, 0x82, 0x01, 0x21, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0e, 0x00, 0x30, 0x82,
  0x01, 0x09, 0x02, 0x82, 0x01, 0x00, 0xa5, 0xdd, 0x86, 0x7a, 0xc4, 0xcb, 0x02,
  0xf9, 0x0b, 0x94, 0x57, 0xd4, 0x8c, 0x14, 0xa7, 0x70, 0xef, 0x99, 0x1c, 0x56,
  0xc3, 0x9c, 0x0e, 0xc6, 0x5f, 0xd1, 0x1a, 0xfa, 0x89, 0x37, 0xce, 0xa5, 0x7b,
  0x9b, 0xe7, 0xac, 0x73, 0xb4, 0x5c, 0x00, 0x17, 0x61, 0x5b, 0x82, 0xd6, 0x22,
  0xe3, 0x18, 0x75, 0x3b, 0x60, 0x27, 0xc0, 0xfd, 0x15, 0x7b, 0xe1, 0x2f, 0x80,
  0x90, 0xfe, 0xe2, 0xa7, 0xad, 0xcd, 0x0e, 0xef, 0x75, 0x9f, 0x88, 0xba, 0x49,
  0x97, 0xc7, 0xa4, 0x2d, 0x58, 0xc9, 0xaa, 0x12, 0xcb, 0x99, 0xae, 0x00, 0x1f,
  0xe5, 0x21, 0xc1, 0x3b, 0xb5, 0x43, 0x14, 0x45, 0xa8, 0xd5, 0xae, 0x4f, 0x5e,
  0x4c, 0x7e, 0x94, 0x8a, 0xc2, 0x27, 0xd3, 0x60, 0x40, 0x71, 0xf2, 0x0e, 0x57,
  0x7e, 0x90, 0x5f, 0xbe, 0xb1, 0x5d, 0xfa, 0xf0, 0x6d, 0x1d, 0xe5, 0xae, 0x62,
  0x53, 0xd6, 0x3a, 0x6a, 0x21, 0x20, 0xb3, 0x1a, 0x5d, 0xa5, 0xda, 0xbc, 0x95,
  0x50, 0x60, 0x0e, 0x20, 0xf2, 0x7d, 0x37, 0x39, 0xe2, 0x62, 0x79, 0x25, 0xfe,
  0xa3, 0xcc, 0x50, 0x9f, 0x21, 0xdf, 0xf0, 0x4e, 0x6e, 0xea, 0x45, 0x49, 0xc5,
  0x40, 0xd6, 0x80, 0x9f, 0xf9, 0x30, 0x7e, 0xed, 0xe9, 0x1f, 0xff, 0x58, 0x73,
  0x3d, 0x83, 0x85, 0xa2, 0x37, 0xd6, 0xd3, 0x70, 0x5a, 0x33, 0xe3, 0x91, 0x90,
  0x09, 0x92, 0x07, 0x0d, 0xf7, 0xad, 0xf1, 0x35, 0x7c, 0xf7, 0xe3, 0x70, 0x0c,
  0xe3, 0x66, 0x7d, 0xe8, 0x3f, 0x17, 0xb8, 0xdf, 0x17, 0x78, 0xdb, 0x38, 0x1d,
  0xce, 0x09, 0xcb, 0x4a, 0xd0, 0x58, 0xa5, 0x11, 0x00, 0x1a, 0x73, 0x81, 0x98,
  0xee, 0x27, 0xcf, 0x55, 0xa1, 0x3b, 0x75, 0x45, 0x39, 0x90, 0x65, 0x82, 0xec,
  0x8b, 0x17, 0x4b, 0xd5, 0x8d, 0x5d, 0x1f, 0x3d, 0x76, 0x7c, 0x61, 0x37, 0x21,
  0xae, 0x05, 0x02, 0x03, 0x01, 0x00, 0x01
};
// RSA-PSS test vectors, pss-vect.txt, Example 10.1
const uint8_t kTestVector2Data[] = {
  0x88, 0x31, 0x77, 0xe5, 0x12, 0x6b, 0x9b, 0xe2, 0xd9, 0xa9, 0x68, 0x03, 0x27,
  0xd5, 0x37, 0x0c, 0x6f, 0x26, 0x86, 0x1f, 0x58, 0x20, 0xc4, 0x3d, 0xa6, 0x7a,
  0x3a, 0xd6, 0x09
};
const uint8_t kTestVector2Sig[] = {
  0x82, 0xc2, 0xb1, 0x60, 0x09, 0x3b, 0x8a, 0xa3, 0xc0, 0xf7, 0x52, 0x2b, 0x19,
  0xf8, 0x73, 0x54, 0x06, 0x6c, 0x77, 0x84, 0x7a, 0xbf, 0x2a, 0x9f, 0xce, 0x54,
  0x2d, 0x0e, 0x84, 0xe9, 0x20, 0xc5, 0xaf, 0xb4, 0x9f, 0xfd, 0xfd, 0xac, 0xe1,
  0x65, 0x60, 0xee, 0x94, 0xa1, 0x36, 0x96, 0x01, 0x14, 0x8e, 0xba, 0xd7, 0xa0,
  0xe1, 0x51, 0xcf, 0x16, 0x33, 0x17, 0x91, 0xa5, 0x72, 0x7d, 0x05, 0xf2, 0x1e,
  0x74, 0xe7, 0xeb, 0x81, 0x14, 0x40, 0x20, 0x69, 0x35, 0xd7, 0x44, 0x76, 0x5a,
  0x15, 0xe7, 0x9f, 0x01, 0x5c, 0xb6, 0x6c, 0x53, 0x2c, 0x87, 0xa6, 0xa0, 0x59,
  0x61, 0xc8, 0xbf, 0xad, 0x74, 0x1a, 0x9a, 0x66, 0x57, 0x02, 0x28, 0x94, 0x39,
  0x3e, 0x72, 0x23, 0x73, 0x97, 0x96, 0xc0, 0x2a, 0x77, 0x45, 0x5d, 0x0f, 0x55,
  0x5b, 0x0e, 0xc0, 0x1d, 0xdf, 0x25, 0x9b, 0x62, 0x07, 0xfd, 0x0f, 0xd5, 0x76,
  0x14, 0xce, 0xf1, 0xa5, 0x57, 0x3b, 0xaa, 0xff, 0x4e, 0xc0, 0x00, 0x69, 0x95,
  0x16, 0x59, 0xb8, 0x5f, 0x24, 0x30, 0x0a, 0x25, 0x16, 0x0c, 0xa8, 0x52, 0x2d,
  0xc6, 0xe6, 0x72, 0x7e, 0x57, 0xd0, 0x19, 0xd7, 0xe6, 0x36, 0x29, 0xb8, 0xfe,
  0x5e, 0x89, 0xe2, 0x5c, 0xc1, 0x5b, 0xeb, 0x3a, 0x64, 0x75, 0x77, 0x55, 0x92,
  0x99, 0x28, 0x0b, 0x9b, 0x28, 0xf7, 0x9b, 0x04, 0x09, 0x00, 0x0b, 0xe2, 0x5b,
  0xbd, 0x96, 0x40, 0x8b, 0xa3, 0xb4, 0x3c, 0xc4, 0x86, 0x18, 0x4d, 0xd1, 0xc8,
  0xe6, 0x25, 0x53, 0xfa, 0x1a, 0xf4, 0x04, 0x0f, 0x60, 0x66, 0x3d, 0xe7, 0xf5,
  0xe4, 0x9c, 0x04, 0x38, 0x8e, 0x25, 0x7f, 0x1c, 0xe8, 0x9c, 0x95, 0xda, 0xb4,
  0x8a, 0x31, 0x5d, 0x9b, 0x66, 0xb1, 0xb7, 0x62, 0x82, 0x33, 0x87, 0x6f, 0xf2,
  0x38, 0x52, 0x30, 0xd0, 0x70, 0xd0, 0x7e, 0x16, 0x66
};

static unsigned char* toUcharPtr(const uint8_t* v) {
  return const_cast<unsigned char*>(
    static_cast<const unsigned char*>(v));
}

class Pkcs11RsaPssTest : public ::testing::Test {
};

class Pkcs11RsaPssVectorTest : public Pkcs11RsaPssTest {
 public:
  void Verify(const uint8_t* spki, size_t spki_len, const uint8_t* data,
              size_t data_len, const uint8_t* sig, size_t sig_len) {
    // Verify data signed with PSS/SHA-1.
    SECOidTag hashOid = SEC_OID_SHA1;
    CK_MECHANISM_TYPE hashMech = CKM_SHA_1;
    CK_RSA_PKCS_MGF_TYPE mgf = CKG_MGF1_SHA1;

    // Set up PSS parameters.
    unsigned int hLen = HASH_ResultLenByOidTag(hashOid);
    CK_RSA_PKCS_PSS_PARAMS rsaPssParams = { hashMech, mgf, hLen };
    SECItem params = { siBuffer,
                       reinterpret_cast<unsigned char*>(&rsaPssParams),
                       sizeof(rsaPssParams) };

    // Import public key.
    SECItem spkiItem = { siBuffer, toUcharPtr(spki),
                         static_cast<unsigned int>(spki_len) };
    ScopedCERTSubjectPublicKeyInfo certSpki(
      SECKEY_DecodeDERSubjectPublicKeyInfo(&spkiItem));
    ScopedSECKEYPublicKey pubKey(SECKEY_ExtractPublicKey(certSpki.get()));

    // Hash the data.
    std::vector<uint8_t> hashBuf(hLen);
    SECItem hash = { siBuffer, &hashBuf[0],
                     static_cast<unsigned int>(hashBuf.size()) };
    SECStatus rv = PK11_HashBuf(hashOid, hash.data, toUcharPtr(data),
                                data_len);
    EXPECT_EQ(rv, SECSuccess);

    // Verify.
    CK_MECHANISM_TYPE mech = CKM_RSA_PKCS_PSS;
    SECItem sigItem = { siBuffer, toUcharPtr(sig),
                        static_cast<unsigned int>(sig_len) };
    rv = PK11_VerifyWithMechanism(pubKey.get(), mech, &params, &sigItem, &hash,
                                  nullptr);
    EXPECT_EQ(rv, SECSuccess);
  }
};

#define PSS_TEST_VECTOR_VERIFY(spki, data, sig) \
  Verify(spki, sizeof(spki), data, sizeof(data), sig, sizeof(sig));

TEST_F(Pkcs11RsaPssTest, GenerateAndSignAndVerify) {
  // Sign data with a 1024-bit RSA key, using PSS/SHA-256.
  SECOidTag hashOid = SEC_OID_SHA256;
  CK_MECHANISM_TYPE hashMech = CKM_SHA256;
  CK_RSA_PKCS_MGF_TYPE mgf = CKG_MGF1_SHA256;
  PK11RSAGenParams rsaGenParams = { 1024, 0x10001 };

  // Generate RSA key pair.
  ScopedPK11SlotInfo slot(PK11_GetInternalSlot());
  SECKEYPublicKey* pubKeyRaw = nullptr;
  ScopedSECKEYPrivateKey privKey(PK11_GenerateKeyPair(slot.get(),
                                                      CKM_RSA_PKCS_KEY_PAIR_GEN,
                                                      &rsaGenParams, &pubKeyRaw,
                                                      false, false, nullptr));
  ASSERT_TRUE(!!privKey && pubKeyRaw);
  ScopedSECKEYPublicKey pubKey(pubKeyRaw);

  // Generate random data to sign.
  uint8_t dataBuf[50];
  SECItem data = { siBuffer, dataBuf, sizeof(dataBuf) };
  unsigned int hLen = HASH_ResultLenByOidTag(hashOid);
  SECStatus rv = PK11_GenerateRandomOnSlot(slot.get(), data.data, data.len);
  EXPECT_EQ(rv, SECSuccess);

  // Allocate memory for the signature.
  std::vector<uint8_t> sigBuf(PK11_SignatureLen(privKey.get()));
  SECItem sig = { siBuffer, &sigBuf[0],
                  static_cast<unsigned int>(sigBuf.size()) };

  // Set up PSS parameters.
  CK_RSA_PKCS_PSS_PARAMS rsaPssParams = { hashMech, mgf, hLen };
  SECItem params = { siBuffer, reinterpret_cast<unsigned char*>(&rsaPssParams),
                     sizeof(rsaPssParams) };

  // Sign.
  CK_MECHANISM_TYPE mech = CKM_RSA_PKCS_PSS;
  rv = PK11_SignWithMechanism(privKey.get(), mech, &params, &sig, &data);
  EXPECT_EQ(rv, SECSuccess);

  // Verify.
  rv = PK11_VerifyWithMechanism(pubKey.get(), mech, &params, &sig, &data,
                                nullptr);
  EXPECT_EQ(rv, SECSuccess);

  // Verification with modified data must fail.
  data.data[0] ^= 0xff;
  rv = PK11_VerifyWithMechanism(pubKey.get(), mech, &params, &sig, &data,
                                nullptr);
  EXPECT_EQ(rv, SECFailure);

  // Verification with original data but the wrong signature must fail.
  data.data[0] ^= 0xff; // Revert previous changes.
  sig.data[0] ^= 0xff;
  rv = PK11_VerifyWithMechanism(pubKey.get(), mech, &params, &sig, &data,
                                nullptr);
  EXPECT_EQ(rv, SECFailure);
}

// RSA-PSS test vectors, pss-vect.txt, Example 1.1: A 1024-bit RSA Key Pair
// <ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1-vec.zip>
TEST_F(Pkcs11RsaPssVectorTest, VerifyKnownSignature1) {
  PSS_TEST_VECTOR_VERIFY(kTestVector1Spki, kTestVector1Data, kTestVector1Sig);
}

// RSA-PSS test vectors, pss-vect.txt, Example 10.1: A 2048-bit RSA Key Pair
// <ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1-vec.zip>
TEST_F(Pkcs11RsaPssVectorTest, VerifyKnownSignature2) {
  PSS_TEST_VECTOR_VERIFY(kTestVector2Spki, kTestVector2Data, kTestVector2Sig);
}

}  // namespace nss_test

