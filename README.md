# mcrypt
This is a quick program to deminstrate the usage of mcrypt to encrypt a string.

This is only the encryption not the decryption.  Decryption can be done the same way only using `mdecrypt_generic`.



## Compile Instructions
1. Download and install libmcrypt
..* From source: http://sourceforge.net/projects/mcrypt/files/Libmcrypt/2.5.8/libmcrypt-2.5.8.tar.gz/download
..* From yum: `yum install libmcrypt-devel`
..* From apt: `sudo apt-get install libmcrypt-dev` 
2. Compile: `gcc -lmcrypt aes.c -o aes`

There is also a `./build.sh` for convenience.

## Things Learned
1. IV's need to be 16 char lengths.
2. Key should also be 16 char length.
3. The encryption text should be a mod of 16. So if it is under there should be padding.
4. `rijndael-128` is the same as AES-128, but `rijndael-256` is not the same as AES-256`.  AES was built from `rijndael-128`.
5. When printing HEX text use unsigned char or else you'll get a bunch of `FFFFFF`'s :P

## Resources
https://gist.github.com/bricef/2436364 - Very nice example.
http://www.cplusplusdevelop.com/6336_10264784/


