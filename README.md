# libshe

[![Build Status](https://travis-ci.org/bogdan-kulynych/libshe.svg?branch=master)](https://travis-ci.org/bogdan-kulynych/libshe) [![Coverage Status](https://coveralls.io/repos/bogdan-kulynych/libshe/badge.svg?branch=master)](https://coveralls.io/r/bogdan-kulynych/libshe?branch=master)

Symmetric somewhat homomorphic encryption library.


## Introduction

Fully homomorphic encryption is a kind of encryption that allows to execute functions over the ciphertexts without decrypting them. This library implements a symmetric variant of originally asymmetric somewhat homomorphic encryption scheme over the integers by van Dijk et al. [(DGHV10)][DGHV10] using ciphertext compression techniques from [(CNT11)][CNT11]. The symmetricity of the scheme means that only the private key is used to encrypt and decrypt ciphertexts. A relatively small public element, however, is used in homomorphic operations, but it is not a real public key.

Such scheme is useful in secure function evaluation setting, where a client encrypts an input to an algorithm using their private key, sends it to a server which executes an algorithm homorphically, and sends the output back to the client. The client then obtains the output of the algorithm by decrypting server response using the private key.

See the following diagram for visual explanation.

- Let _f_ be an algorithm to be evaluated on a server.
- Let _a[1], a[2], ... a[n]_ be inputs of _f_ that client provides to the server.
- Let _b[1], b[2], ... b[n]_ be inputs of _f_ that server possesses.
- Let _p_ be the client's private key, and _x[0]_ be the corresponding public element

![SFE](misc/sfe.png)

See [technical report][Kul15] (draft) for details.


## Status

_Warning_. This is experimental software. **It is not to be used in mission-critical applications.**

### Roadmap

- [x] CI and coverage reports
- [x] Installation
- [ ] Timing and memory estimates
- [ ] Comparison with [HElib](https://github.com/shaih/HElib)
- [ ] Documentation
- [ ] Additional homomorphic operations


## Installation

### Requirements

- gcc >= 4.8
- [boost](http://www.boost.org/) >= 1.55
- [GMP](https://gmplib.org/) >= 6.0.0
- [lcov](http://ltp.sourceforge.net/coverage/lcov/readme.php) >= 1.11 (optional)

### Building

Build and install `libshe.so` library and headers:

```
make
sudo make install
```

Run tests:

```
make tests
```

_Note_. Running tests will compile sources with debug options. Do `make clean` before installing if tests were run previously.


## Usage

### Building

Use C++11 and link against _GMP_ and _Boost Serialization_ when building your program:

```
-std=c++11 -lgmp -lboost_serialization -lshe
```

Include libshe in your sources:

```cpp
#include <she.hpp>

using she::ParameterSet;
using she::PrivateKey;
// ...
```

### Example

The following example assumes a client and a server that are engaged in a two-party secure function evaluation protocol.

Client generates a parameter set:

```cpp
const ParameterSet params = ParameterSet::generate_parameter_set(62, 1, 42);
```

Given these parameters, the encryption scheme exhibits following properties:

   - Security level is medium **(62-bit)**
   - At least 1 multiplication can be evaluated on every bit in the ciphertext
   - The non-secure random number generator used in ciphertext compression is seeded with number 42

Client then constructs a private key object from generated parameters:

```cpp
const PrivateKey sk(params);
```

Encrypts the plaintext:

```cpp
const vector<bool> plaintext = {1, 0, 1, 0, 1, 0, 1, 0};
const auto compressed_ciphertext = sk.encrypt();
```

Serializes and sends compressed ciphertext to server.

Upon obtaining the compressed ciphertext, Server expands it to perform operations:

```cpp
const auto ciphertext = compressed_ciphertext.expand();
```

Executes the algorithm (here negation of an 8-bit input)

```cpp
const vector<bool> another_plaintext = {1, 1, 1, 1, 1, 1, 1, 1};
const auto response = ciphertext ^ another_plaintext;
```

Serializes the output and sends it back to the client.

Client decrypts the response and obtains the algorithm output in plaintext:

```cpp
const auto decrypted_response = sk.decrypt(response);
const vector<bool> expected_result = {0, 1, 0, 1, 0, 1, 0, 1};
assert(decrypted_response == expected_result);
```

Note that ciphertext can be compressed only during encryption on the client side, so cost for Server → Client communication is significantly higher than that of Client → Server communication.


## License

The code is released under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.html).

Copyright © 2015 Bogdan Kulynych. `hello [at] bogdankulynych.me`



[DGHV10]: http://eprint.iacr.org/2009/616.pdf
[CNT11]: http://eprint.iacr.org/2011/440.pdf
[Kul15]: http://bogdankulynych.me/papers/vdghv.pdf
