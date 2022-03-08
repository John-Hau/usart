#!/bin/bash

iv="000102030405060708090a0b0c0d0e00"
 k="2b7e151628aed2a6abf7158809cf4f"


echo -n -e '\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a\xfa' \
	| openssl enc -v -e -aes-128-cbc -K $k\
	-iv $iv | od -Ax -tx1 




#echo -n -e '\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a\xfa' \
#	| openssl enc -v -e -aes-128-cbc -K 2b7e151628aed2a6abf7158809cf4f\
#	-iv 000102030405060708090a0b0c0d0e | od -Ax -tx1 



#echo -n -e '\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a' \
#	| openssl enc -v -e -aes-128-cbc -K 2b7e151628aed2a6abf7158809cf4f3c\
#	-iv 000102030405060708090a0b0c0d0e0f -nopad > eee.bin 



#aes128 cbc decryption
#openssl enc -v -d -aes-128-cbc -K 2b7e151628aed2a6abf7158809cf4f3c\
#	-iv 000102030405060708090a0b0c0d0e0f -nopad -in eee.bin | od -Ax -tx1 



