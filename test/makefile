# Jzon Test File

outdir = bin
outfile = test


all: setup main

clean:
	rm -f $(outdir)/$(outfile)

setup:
	mkdir -p $(outdir)

main:
	$(CXX) test.cpp ../Jzon.cpp -o $(outdir)/$(outfile)

test:
	./test.sh $(outdir)/$(outfile)
