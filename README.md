she
---

Symmetric Somewhat Homomorphic Encryption.


## Introduction

This library implements a symmetric variant of originally asymmetric homomorphic encryption scheme over the integers by van Dijk et al. [(DGHV10)][DGHV10] using ciphertext compression techniques from [(CNT11)][CNT11]. The symmetricity of the scheme means that only the private key is used to encrypt and decrypt ciphertexts. Such scheme is useful in secure (remote) function execution context, where a client encrypts an input to an algorithm using their private key, sends it to a server which executes an algorithm homorphically using the client's input without deciphering it, sends the output to the client. The client obtains the output of the algorithm by decrypting server response using the private key.

A relatively small public element, however, is used in homomorphic operations, but it is not a real public key.

See [technical report][Kul15] for details.


## Installation

### Requirements

- [boost](http://www.boost.org/) >= 1.57
- [GMP](https://gmplib.org/) >= 6.x.x

### Building

Run tests

```
make test BOOST_DIR=/path/to/boost/libraries
```

Produce an `so` library in build folder:

```
make BOOST_DIR=/path/to/boost/libraries
```

_Note_. Default value of `BOOST_DIR` is `/usr/local/lib`.

## Roadmap

- [ ] Documentation
- [ ] Include size and timing estimates
- [ ] Comparison against helib



[DGHV10]: http://eprint.iacr.org/2009/616.pdf
[CNT11]: http://eprint.iacr.org/2011/440.pdf
[Kul15]: http://bogdankulynych.me/papers/vdghv.pdf