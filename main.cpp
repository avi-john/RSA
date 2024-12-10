#include <cstdint>
#include <iostream>
#include <fstream>
#include <numeric>
#include <random>
#include <sys/stat.h>

#include "codec64.h"
using namespace std;
bool isPrime(uint64_t n) {
	if (n < 2)
		return false;
	if (n == 2)
		return true;
	if (n % 2 == 0)
		return false;
	for (uint64_t f=3; f*f<=n; f+=2)
		if (n % f == 0)
			return false;
	return true;
}
int64_t modInverse(int64_t a,int64_t n) {

    int64_t t = 0, tNew = 1;
    int64_t r = n, rNew = a;

    while (rNew != 0) {

        int64_t q = r / rNew;
        int64_t
            tmp = tNew;
        tNew = t - q * tNew;
        t = tmp;
        int64_t
            tmpR = rNew;
            rNew = r - q * rNew;
            r = tmpR;
   }
    if (r > 1)
         return -1;
    if (t < 0)
          t = t + n;
    return t;
}
uint64_t modExp(uint64_t base,uint64_t exp,uint64_t m) {
    uint64_t ans = 1;
    while (exp != 0) {
        if (exp % 2 == 1)
             ans = (ans * base) % m;
        base = (base * base) % m;
        exp = exp / 2;
    }
    return ans;
}
int64_t getFileSize(char *fn) {
    struct stat
        fs{};
    stat(fn,&fs);
    return fs.st_size;
}
void keyGen() {
    random_device
        rd;
    mt19937
        mt(rd());
    uniform_int_distribution<>
        dis(4096,65535);
    int64_t p, q,e;
    do{
        p = dis(mt);
    }  while (!isPrime(p));
    do{
        q = dis(mt);
    } while (!isPrime(q) || q == p);
    int64_t n = p * q;
    int64_t f =(p-1) * (q-1);
    do {
        e = dis(mt);
    } while (gcd(e,f) != 1);
    int64_t d = modInverse(e, f);
    ofstream pubKeyFile("public.key");
    if (!pubKeyFile){
        cout << "Error: Could not open the file."  << endl;
        exit (EXIT_FAILURE);
    }
    pubKeyFile << n << " " << e << endl;
    pubKeyFile.close();
    ofstream privKeyFile("private.key");

    if (!privKeyFile){
        cout << "Error: Could not open the file" << endl;
        exit(EXIT_FAILURE);
    }

    // write n and d to file
    privKeyFile << n << " " << d << endl;

    // close file
    privKeyFile.close();

    cout << "Key Generation Complete." << endl;

}

//=============================================================================
// void encrypt(char *inFileName,char *outFileName,int64_t n,int64_t e)
//  encrypt the given file, place encrypted result in output file
//
// Parameter
//  inFileName  - name of file to be encrypted
//  outFileName - name of file to hold encrypted output
//  n,e         - the public (encryption) key
//

void encrypt(char *inFileName,char *outFileName,int64_t n,int64_t e) {
    Codec64
        codec;

    // open the input file
    ifstream inputFile (inFileName, ios::binary);

    // make sure the open worked
    if (!inputFile.is_open()){
        cout << "Error: Could not open the input FIle." << endl;
        exit(EXIT_FAILURE);
    }

    // prepare codec object for writing
    codec.beginEncode(outFileName);

    // get the file size
    uint32_t fileSize = getFileSize(inFileName);

    // write file size to codec
    codec.put32(fileSize);

    // loop over all bytes in input file, grouped into three-byte chunks
    uint8_t buffer[3];
   for (int i=0; i< fileSize; i+=3){

        // read three bytes. don't worry if three bytes aren't available
        buffer[0] = inputFile.get();
        buffer[1] = inputFile.get();
        buffer[2] = inputFile.get();

        // convert bytes into a single number
        int64_t plainText = buffer[0] + 256 * buffer[1] + 65536 * buffer[2];

        // encrypt using modExp
        int64_t cipherText = modExp(plainText, e, n);

        codec.put32(cipherText);
    // }
    }

    // close input file
    inputFile.close();
    // tell codec we're done
    codec.endEncode();

}

//=============================================================================
// void decrypt(char *inFileName,char *outFileName,int64_t n,int64_t d)
//  decrypt the given file, place decrypted result in output file
//
// Parameter
//  inFileName  - name of file to be decrypted
//  outFileName - name of file to hold decrypted output
//  n,d         - the private (decryption) key
//

void decrypt(char *inFileName,char *outFileName,int64_t n,int64_t d) {
    Codec64
        codec;

    // open output file
    ofstream outputFile(outFileName);

    // make sure open worked
    if (!outputFile.is_open()) {
        cout << "Error: Could not open output file." << endl;
        return;
    }

    // prepare codec for decoding
    codec.beginDecode(inFileName);

    // get file size from codec
    int64_t fileSize = codec.get32();

    // loop over all bytes in file
    for (int i = 0; i< fileSize; i+= 3) {

        // get 32-bit encrypted value from codec
        int64_t cipher = codec.get32();

        // decrypt using modExp
        int64_t plain = modExp(cipher,d,n);

        // split decrypted value into three bytes
        uint8_t buffer[3];
        buffer[0] = plain & 0xFF;  // to extract the least significant byte
        buffer[1] = (plain >> 8) & 0xFF;  // to extract the middle byte
        buffer[2] = (plain >> 16) & 0xFF;   // to extract the most significant byte

        // output first byte
        outputFile.put(buffer[0]);

        // if necessary, output second byte
        if (i < fileSize - 1)
            outputFile.put(buffer[1]);

        // if necessary, output third byte
        if (i < fileSize - 2)
            outputFile.put(buffer[2]);

    // }
    }

    // close output file
    outputFile.close();

    // tell codec we're done
    codec.endDecode();
    cout << "Decryption complete. Output written to " << outFileName << endl;

}

//=============================================================================
// void usage(char *progName)
//  display a proper usage message and stop the program
//
// Parameter
//  progName - name of the program (from main's argv[0])
//

void usage(char *progName) {

    cout << "Usage: " << progName << "-k" << endl
         << "       " << progName << "-e input-file output-file" << endl
         << "       " << progName << "-d input-file output-file" << endl;

    exit(EXIT_FAILURE);
}

//=============================================================================
// int main(int argc,char *argv[])
//  the main function
//
// Parameters
//  argc - number of command-line strings
//  argv - array of command-line strings
//
// Returns
//  0, although usage will call exit(1) if called
//

int main(int argc,char *argv[]) {
    // some variables
    // make sure argc is at least 2
    //if (argc == 1)
      //  usage(argv[0]);
    if (argc < 2)
        usage(argv[0]);

    // make sure argv[1][0] is a -
    if (argv[1][0] != '-')
        usage (argv[0]);

    // if argv[1][1] == k, do... {
    if (argv[1][1] == 'k') {
        // make sure argv[1][2] == 0
        if (argv[1][2] != 0)
            usage (argv[0]);

        // make sure argc == 2
        if (argc !=2)
            usage (argv[0]);

        // call keygen()
        keyGen();
    // }
    }

    // else if argv[1][1] == e, do... {
    else if (argv[1][1] == 'e') {

        // make sure argv[1][2] == 0
        if (argv[1][2] != 0)
            usage (argv[0]);

        // make sure argc == 4
        if (argc != 4)
            usage (argv[0]);

        // read n and e from public.key
        int64_t n, e;
        ifstream publicKeyFile ("public.key");
        if (!publicKeyFile) {
            cout << "Error: Could not open public.key" << endl;
            exit (EXIT_FAILURE);
        }
        publicKeyFile >> n>> e;
        publicKeyFile.close();

        // call encrypt(argv[2],argv[3],n,e)
        encrypt (argv[2], argv[3], n, e );
    // }
    }

    // else if argv[1][1] == d, do... {
    else if (argv [1][1] == 'd') {

        // make sure argv[1][2] == 0
        if (argv[1][2] != 0)
            usage (argv[0]);

        // make sure argc == 4
        if (argc != 4)
            usage (argv[0]);

        // read n and d from private.key
        int64_t n, d;
        ifstream privteKeyFile ("private.key");
        if (!privteKeyFile){
            cout << "Error: Coulf not open private.key" << endl;
            exit(EXIT_FAILURE);
        }
        privteKeyFile >> n >> d;
        privteKeyFile.close();

        // call decrypt(argv[2],argv[3],n,d)
        decrypt(argv[2],argv[3],n,d);
    // }
    }

    // else display an error message
    else
        usage (argv[0]);

    return 0;
}