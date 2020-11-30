# NAME: Kevin Tang
# EMAIL: kevintang2023@ucla.edu
# ID: 805419480

default: 
	gcc -g -Wall -Wextra lab4b.c -o lab4b -lmraa -lm

buildtest:
	gcc -g -Wall -Wextra lab4b.c -o lab4b -D PC -lm

dist: lab4b.tgz
submission_files =  Makefile README

lab4b.tgz: $(submission_files)
	tar -czf lab4b-805419480.tar.gz $(submission_files)

.PHONY: default dist clean

clean:
	rm -f *.tar.gz lab4b *.test
