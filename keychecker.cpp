#include <iostream>
#include <fstream>

using namespace std;

bool isPrime(int64_t n) {

    if (n < 2)
        return false;

    if (n == 2)
        return true;

    if ((n & 1) == 0)
        return false;

    for (int64_t f=3;f*f<=n;f+=2)
        if (n % f == 0)
            return false;

    return true;
}

int main() {
    ifstream
        inFile;
    uint32_t
        n1,n2,
        d,e,f,
        p,q;

    // open public.key
    inFile.open("public.key");

    // make sure open worked
    if (!inFile) {
        cout << "open: Cannot open public.key" << endl;
        return 1;
    }

    // read n and e
    inFile >> n1 >> e;

    // close file
    inFile.close();

    // open private.key
    inFile.open("private.key");

    // make sure open worked
    if (!inFile) {
        cout << "open: Cannot open private.key";
        return 1;
    }

    // read n and d
    inFile >> n2 >> d;

    // close file
    inFile.close();

    // make sure n values are the same
    if (n1 != n2) {
        cout << "n values are different" << endl;
        return 1;
    }

    // find a prime factor of n
    for (p=3;p<65536;p+=2)
        if (isPrime(p) && n1 % p == 0)
            break;
    if (p > 65535) {
        cout << "no prime factor of n found" << endl;
        return 1;
    }

    // split n into two factors
    q = n1 / p;

    // verify factors are prime
    if (!isPrime(q)) {
        cout << "n is not semiprime" << endl;
        return 1;
    }

    // verify that factors are different
    if (p == q) {
        cout << "p and q are identical" << endl;
        return 1;
    }

    // compute f
    f = (p - 1) * (q - 1);

    // make sure d and e are inverses mod f
    if ((d * (uint64_t)e) % f != 1) {
        cout << "d and e are not inverses mod f" << endl;
        return 1;
    }

    cout << "All tests pass.\np = " << p << endl << "q = " << q << endl << "d = "
        << d << endl << "e = " << e << endl << "f = " << f << endl;

    return 0;
}