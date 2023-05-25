/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 08/03/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#include "Centaur.hpp"
#include "Protocol.hpp"

#include <cryptopp/base64.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>

#include <fmt/core.h>
#include <fstream>

cen::protocol::Encryption::Encryption()  = default;
cen::protocol::Encryption::~Encryption() = default;

std::string cen::protocol::Encryption::EncryptAES(const std::string &plaintext, const std::string &key, const std::string &iv)
{
    using namespace CryptoPP;

    std::string ciphertext;
    std::string output;

    try
    {
        AutoSeededRandomPool prng;

        CBC_Mode<AES>::Encryption encryption;
        encryption.SetKeyWithIV(reinterpret_cast<const byte *>(key.data()), key.size(), reinterpret_cast<const byte *>(iv.data()), iv.size());

        StringSource(plaintext, true,
            new StreamTransformationFilter(encryption,
                new StringSink(ciphertext)));

        // Encode ciphertext as Base64
        StringSource(ciphertext, true,
            new Base64Encoder(new StringSink(output)));

        return output;
    } catch (const Exception &e)
    {
        throw std::runtime_error(e.what());
    }
}

std::string cen::protocol::Encryption::DecryptAES(const std::string &ciphertext, const std::string &key, const std::string &iv)
{
    using namespace CryptoPP;
    std::string decryptedText;

    try
    {
        std::string decodedCiphertext;

        // Decode ciphertext from Base64
        StringSource(ciphertext, true,
            new Base64Decoder(new StringSink(decodedCiphertext)));

        CBC_Mode<AES>::Decryption decryption;
        decryption.SetKeyWithIV(reinterpret_cast<const byte *>(key.data()), key.size(), reinterpret_cast<const byte *>(iv.data()), iv.size());

        StringSource(decodedCiphertext, true,
            new StreamTransformationFilter(decryption,
                new StringSink(decryptedText)));
    } catch (const Exception &e)
    {
        throw std::runtime_error(e.what());
    }

    return decryptedText;
}
