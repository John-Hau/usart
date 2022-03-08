#!/bin/bash

#echo -n -e '\x76\x49\xab\xac\x81\x19\xb2\x46\xce\xe9\x8e\x9b\x12\xe9\x19\x7d' \
#	| openssl enc -d -aes-128-cbc -K 2b7e151628aed2a6abf7158809cf4f3c\




cat eee.bin \
	| openssl enc -d -aes-128-cbc -K 2b7e151628aed2a6abf7158809cf4f3c\
	-iv 000102030405060708090a0b0c0d0e0f | od -Ax -tx1 -iv 000102030405060708090a0b0c0d0e0f | od -Ax -tx1 
