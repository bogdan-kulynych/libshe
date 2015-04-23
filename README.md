she
---

Symmetric Somewhat Homomorphic Encryption.


## Introduction

Homomorphic encryption is a kind of encryption that allows to execute functions over the ciphertexts without decrypting them. This library implements a symmetric variant of originally asymmetric homomorphic encryption scheme over the integers by van Dijk et al. [(DGHV10)][DGHV10] using ciphertext compression techniques from [(CNT11)][CNT11]. The symmetricity of the scheme means that only the private key is used to encrypt and decrypt ciphertexts. A relatively small public element, however, is used in homomorphic operations, but it is not a real public key.

Such scheme is useful in secure function evaluation setting, where a client encrypts an input to an algorithm using their private key, sends it to a server which executes an algorithm homorphically without ever deciphering the client's input, and sends the output back to the client. The client then obtains the output of the algorithm by decrypting server response using the private key.

See the following diagram for visual explanation.

- Let _f_ be an algorithm to be evaluated on a server.
- Let _a[1], a[2], ... a[n]_ be inputs of _f_ that client provides to the server.
- Let _b[1], b[2], ... b[n]_ be inputs of _f_ that server possesses.
- Let _p_ be the client's private key, and _x[0]_ be the corresponding public element

![SFE](misc/sfe.png)

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
- [ ] Include ciphertext size, encryption, decryption, and homomorphic evaluation timing estimates
- [ ] Comparison with helib



[DGHV10]: http://eprint.iacr.org/2009/616.pdf
[CNT11]: http://eprint.iacr.org/2011/440.pdf
[Kul15]: http://bogdankulynych.me/papers/vdghv.pdf