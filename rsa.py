#!/usr/bin/env python3
import argparse

def gcd(a, b):
    if b == 0:
        return a
    else:
        return gcd(b, a % b)

def inverse(e, n):
    """Return the multiplicative inverse of e mod n, which is d.

    See https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm#Computing_multiplicative_inverses_in_modular_structures
    The code here uses variable names for calculating the private key d for RSA encryption algorithm.
    This inverse calculates d to satisfy the equation e*d = 1 mod n, where n = (p-1)*(q-1)
    p and q are very large primes, and 1 < e < n, and e and n are coprimes
    """
    d, new_d, r, new_r = 0, 1, n, e

    while new_r != 0:
        quotient = r // new_r
        d, new_d = new_d, d - quotient * new_d
        r, new_r = new_r, r - quotient * new_r
    if r > 1: raise Exception("e is not invertible")
    if d < 0: d += n
    return d

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='RSA Example')
    parser.add_argument('--e', help='Coprime e, default is 3', default=3)
    parser.add_argument('p', help='Large prime p')
    parser.add_argument('q', help='Large prime q')
    args = parser.parse_args()

    # See https://www.di-mgt.com.au/rsa_alg.html
    # A very simple example of RSA encryption

    # 1. Select primes from command line input (example: 11, 3 or 173, 149)
    p, q = int(args.p), int(args.q)

    # 2. n = pq = 11.3 = 33
    n = p*q # the modulus

    # 3. Choose the coprime e from the command line, 3 is a common choice
    e = int(args.e)

    # Check preconditions
    assert gcd(e, p-1) == 1, "%d and %d should have no common factors except 1" % (e, p-1)
    assert gcd(e, q-1) == 1, "%d and %d should have no common factors except 1" % (e, q-1)

    # 4. Compute d such that ed ≡ 1 mod((p-1)(q-1))
    d = inverse(e, (p-1)*(q-1))

    print("Public key = (n, e) = (%d, %d)" % (n, e))
    print("Private key = (n, d) = (%d, %d)" % (n, d))

