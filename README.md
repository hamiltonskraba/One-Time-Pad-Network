# One-Time-Pad-Network

This program exlpores network programming through TCP/IP protocol by passing plain text and key files from client to server in order to be encrypted and decrypted through the One Time Pad method. 

First, keygen is used to generate a random sequence of letter set the to the same length as the plain text message intended to be encrypted. Otp_enc_d and Otp_dec_d are daemons that run in the background on specified ports, waiting to receive data from the client. Otp_enc can then send the plain text and key to the daemon, which generates a cipher text file. Accordingly, the cipher text and key can then be sent to otp_dec_d for decryption. There is also a short bash script to comiple all of the files. 

Example usage:

$ cat plaintext1
THE RED GOOSE FLIES AT MIDNIGHT
$ otp_enc_d 57171 &
$ otp_dec_d 57172 &
$ keygen 10 > mykey
$ cat mykey
VAONWOYVXP
$ otp_enc plaintext1 mykey 57171 > ciphertext
$ cat ciphertext
GU WIRGEWOMGRIFOENBYIWUG T WOFL
$ otp_dec_d ciphertext mykey 57172
THE RED GOOSE FLIES AT MIDNIGHT
