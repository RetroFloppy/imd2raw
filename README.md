# imd2raw
Dave Dunfield created the excellent [ImageDisk](http://dunfield.classiccmp.org/img/index.htm)
toolset that can take basically any disk created with Intel-alike disk controller chips and, given
the right hardware, create a digital image of that disk.
Along with the in-band data, there are several pieces of metadata that are stored inside the resulting
file (typically suffixed with .IMD) that keep track of many geometric details of the original disk such 
that it could even be reconstituted again on the appropriate hardware.

When using a disk image in emulation or with other disk manipulation tools, they typically expect a "raw"
disk image that is a linear reprsentation of exactly and only the in-band sector-by-sector data.
Dave Dunfield includes the tool `IMDU.COM` for the purpose - it untangles an ImageDisk-created disk image 
and outputs a linear disk image with no metadata whatsoever (typically suffixed with .IMG or .DSK or
whatever... there is no standard).

`IMDU.COM` itself requires a pretty early form of PC/MS-DOS in order to run, so it can be inconvenient to 
run if all you have is an .IMD file you want unpacked.  The subject of this repo is the utility "imd2raw" that 
does much the same thing as an `"IMDU /b IN.IMD OUT.IMG"` command line might do in DOS, but you get to 
compile it for your own operating system.

# Usage

Build the program with the C compiler of your choice; a `Makefile` is supplied for Posix-like operating
systems.

To build:  
`make`

To test:  
`make test`

To use:  
`./imd2raw in.imd out.img`

# Background
This code descended from an entry in bitsavers.org under the Convergent/ngen directory; other people 
such as Tom Burnett added file input and output so it didn't rely on standard input/output to function.
All of the versions of `imd2raw.c` that have been out in the world (so far) have suffered from a fatal
flaw, though:

**Any disk image that has any kind of sector skew (i.e. sectors aren't in 1, 2, 3, ... order) are not
converted correctly.  That is: unless your disk image happens to have a 1-1 sector skew, it will be
incorrectly decoded by other `imd2raw.c` programs.  This is becuse the sectors aren't sorted before 
writing them back out.**
