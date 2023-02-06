# README for Multithreaded Httpserver

This program was created by Kaho Tran. Help was received from TAs, slides, piazza, Geeks for geeks, Jacob Sorber on youtube, discussion and stack overflow.

## Multithreaded Httpserver v1.0
The purpose of this server is to be able to have multiple threads handle requests and a logging feature. I did not finish the logging feature.

To run this program you will need two terminals.
One is for the server, the other is for the client. On one terminal start the server by typing in "./httpserver [-l filename] [-N numThreads] localhost 8081" or "./httpserver [-l filename] [-N numThreads] ipaddress".

If there is no port specified it will be defaulted to 80.
If there is no numThread specified then default is 4 threads.

Then on the client side you can use curl to test the functionality of the server.

To do this I used these two commands and a combined it with a script.

	- curl -s -T localfile http://localhost:8080 --request-target filename_27_character
	- curl -s http://localhost:8080 --request-target filename_27_character

Files submitted in asgn1 are:

	- README.md
	- DESIGN.pdf
	- Makefile
	- httpserver.cpp
    - httpserver.h
	- WRITEUP.pdf
	- queue.h
	- queue.cpp
