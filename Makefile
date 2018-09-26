CFILES=$(sort $(wildcard *.c))
OFILES=$(patsubst %.c,%.o,$(CFILES))

OKFILES=$(sort $(wildcard *.ok))
RESULTS=$(patsubst %.ok,%.result,$(OKFILES))

PROG = p8
LINKS = to capture

CPPFLAGS=
CFLAGS=-O3
LDFLAGS=

all : $(PROG) $(LINKS);

$(PROG) : % : $(OFILES) Makefile
	gcc -o $@ $(OFILES)

$(LINKS) : % : $(PROG)
	ln -f $(PROG) $@

%.o : %.c Makefile
	gcc -c -MD $(CPPFLAGS) $(CFLAGS) $*.c

t0.out : all
	(./p8 ls -d /tmp ; echo $$?) > t0.out

t1.out : all
	-rm -f *.stdout *.stderr
	-(./capture x1 ls -d /tmp ; echo $$? ; cat x1.stdout ; echo $$?; cat x1.stderr; echo $$?) > t1.out 2>&1
	-rm -f *.stdout *.stderr

t2.out : all
	-rm -f *.stdout *.stderr
	-(./p8 -t 1000 -o x2.stderr ls -d /tmp ; echo $$?; cat x2.stdout ; echo $$?; cat x2.stderr ; echo $$?) > t2.out 2>&1
	-rm -f *.stdout *.stderr

t3.out : all
	-rm -f *.stdout *.stderr
	-(./to 1 -e x3.stderr sleep 10 ; echo $$?; cat x3.stdout ; echo $$?; cat x3.stderr ; echo $$?) > t3.out 2>&1
	-rm -f *.stdout *.stderr

t4.out : all
	-rm -f *.stdout *.stderr
	-(env PATH=$$PATH capture x4 to 1 sleep 10 ; echo $$?; cat x4.stdout ; echo $$?; cat x4.stderr ; echo $$?) > t4.out 2>&1
	-rm -f *.stdout *.stderr

t5.out : all
	-rm -f *.stdout *.stderr
	-(env PATH=$$PATH to 1 to 10 to 100 sleep 1000 ; echo $$?; cat x5.stdout ; echo $$?; cat x5.stderr ; echo $$?) > t5.out 2>&1
	-rm -f *.stdout *.stderr

t6.out : all
	-rm -f *.stdout *.stderr
	-(./p8 ./p8 ./p8 -f 10; echo $$?) > t6.out 2>&1
	-rm -f *.stdout *.stderr

t7.out : all
	-rm -f *.stdout *.stderr
	-(./p8 ./p8 ./p8 -s; echo $$?) > t7.out 2>&1
	-rm -f *.stdout *.stderr

$(RESULTS) : %.result : %.out %.ok
	-@echo -n "$* ..."
	-@(diff $*.out $*.ok > /dev/null 2>&1 && echo pass) || echo fail

test : $(RESULTS);

clean :
	rm -f $(PROG) $(LINKS)
	rm -f *.o
	rm -f *.d
	rm -f *.stderr
	rm -f *.stdout
	rm -f *.out

-include *.d
