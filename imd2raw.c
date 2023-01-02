/*
 * Fixes to sort sector list so that images with skews come out correct
 * Verified by actually comparing IMDU /B output byte for byte
 *
 * David Schmidt
 * December, 2022
 */

/* originally from the bitsavers/convergent directory
 *
 * Used stdin/stdout that won't work in Windows.  Files
 * must be opened explicity in binary mode for both input and output
 *
 * Testing: Converts *.IMD file identical to IMDU /B as verified
 * by both size and an MD5 cryptographic hash.
 *
 * Tom Burnett
 * December, 2013
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/*
 * IMD image format
 * 
 * IMD v.v: dd/mm/yyy hh:mm:ss (ascii header)
 * comment terminated with 0x1a
 * for each track
 *  char mode (0-5)
 *  char cylinder
 *  char head
 *  char sector count
 *  char sector size (0-6)
 *  sector numbering map
 *  optional cylinder map
 *  optional head map
 *  sector data records (type) (val, or data)
 */

char *modetbl[] = { "500K FM", "300K FM", "250K FM", "500K MFM", "300K MFM", "250K MFM" };

FILE *fpin, *fpout;

int comp (const void * elem1, const void * elem2) 
{
    unsigned char f = *((int*)elem1);
    unsigned char s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

static void leave (int reason)
{
    fflush (stdout);
    exit (reason);
} /* leave */

int main(int argc, char *argv[])
{
    unsigned char sectormap[32];
    unsigned char sectormapsorted[32];
    unsigned char secdisp[32];
    unsigned char secdata[64][8192];
    unsigned int mode, cyl, hd, seccnt, headflags;
    unsigned int secsiz = 0;
    int i, j;
    unsigned char c, value, fill;

    if (argc != 3) {
        fprintf(stderr, "Usage: imd2raw <infile.imd> <outfile.dsk>\n");
        leave(1);
    }

    if ((fpin = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Open failure on %s for read.\n", argv[1]);
        leave(2);
    }

    if ((fpout = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr, "Open failure on %s for write.\n", argv[2]);
        leave(3);
    }

    if( (fgetc(fpin) != 'I') || (fgetc(fpin) != 'M') || (fgetc(fpin) != 'D') ) {
        fprintf(stderr,"File doesn't start with 'IMD'\n");
        leave(4);
    }    

    // Lop off initial comments
    while(1) {
        c = fgetc(fpin);
        if(c == 0x1a)
            break;
    }

    while(1) {
        c = fgetc(fpin);
        if(feof(fpin)) break;
        mode   = c;
        if(mode > 6) {
            fprintf(stderr,"Stream out of sync at mode, got 0x%02x\n", mode);
            leave(EXIT_FAILURE);
        }
        cyl    = fgetc(fpin);
        if(cyl > 80) {
            fprintf(stderr,"Stream out of sync at cyl, got 0x%02x\n", cyl);
            leave(5);
        }
        c      = fgetc(fpin);
        hd = c & 0x0f;
        headflags = c & 0xf0;
        if(hd > 1) {
            fprintf(stderr,"Stream out of sync at hd, got 0x%02x\n", hd);
            leave(6);
        }
        seccnt = fgetc(fpin);

        c = fgetc(fpin);

        switch(c) {
            case 0:
                secsiz = 128;
                break;
            case 1:
                secsiz = 256;
                break;
            case 2:
                secsiz = 512;
                break;
            case 3:
                secsiz = 1024;
                break;
            case 4:
                secsiz = 2048;
                break;
            case 5:
                secsiz = 4096;
                break;
            case 6:
                secsiz = 8192;
                break;
            default:
                fprintf (stderr, "Unknown sector size indicator %d\n", c);
                break;
        }

        // fprintf(stderr,"Cyl:%d Hd:%d %s %d sectors size %d\n", cyl, hd, modetbl[mode], seccnt, secsiz);

        // copy sector/interleave map
        for (i=0; i < seccnt; i++) {
            sectormap[i] = fgetc(fpin);
            sectormapsorted[i] = sectormap[i];
        }
        // Sort the sectors in case of skew
        qsort(sectormapsorted, seccnt, 1, comp);
        // fprintf(stderr,"Tbl ");
        // for (i=0; i < seccnt; i++) fprintf(stderr,"%d ",sectormap[i]);
        // fprintf(stderr, "\n");
        // fprintf(stderr,"Srt ");
        // for (i=0; i < seccnt; i++) fprintf(stderr,"%d ",sectormapsorted[i]);
        // fprintf(stderr, "\n");

        if ((headflags & 64) == 64)
        {
          // Pull out "optional" cylinder map, discard
          for (int i = 0; i < seccnt; i++)
            c = fgetc(fpin);
        }
        if ((headflags & 128) == 128)
        {
          // Pull out "optional" head map, discard
          for (int i = 0; i < seccnt; i++)
            c = fgetc(fpin);
        }

        // copy sector information indexed by the sector number
        for (i=0; i < seccnt; i++) {
            c = fgetc(fpin);

            switch(c) {
                case 0:            // Sector data unavailable - could not be read
                case 5:            // Deleted address marks
                case 7:            // Bad sector
                    secdisp[i] = 'X';
                    fill = 0xE5;
                    for(j=0; j < secsiz; j++) {
                        if (c > 0)
                          fill = fgetc(fpin); // Grab whatever IMD wrote
                        secdata[sectormap[i]][j] = fill;
                    }
                    // fprintf(stderr,"Cyl %d Hd %d Sec %d bad, type %d\n",cyl,hd,sectormap[i],c);
                    break;
    
                case 1:            // normal data 'secsiz' bytes follow
                    secdisp[i] = '.';
                    for(j=0; j < secsiz; j++)
                        secdata[sectormap[i]][j] = fgetc(fpin);
                    break;

                case 3:            // data with 'deleted data' address mark
                    secdisp[i] = 'd';    
                    for(j=0; j < secsiz; j++)
                        secdata[sectormap[i]][j] = fgetc(fpin);
                    break;

                case 2:            // compressed with value in next byte
                case 4:
                case 6:
                case 8:
                    secdisp[i] = 'C';
                    value = fgetc(fpin);
                    for(j=0; j < secsiz; j++)
                        secdata[sectormap[i]][j] = value;
                    // fprintf(stderr,"Cyl %d Hd %d Sec %d all %x\n",cyl,hd,sectormap[i],value);
                    break;
            }
        }
        fprintf(stderr,"Cyl %02d Hd %d %-4d ", cyl, hd, secsiz );
        for(i=0; i<seccnt; i++) {
            fprintf(stderr,"%c",secdisp[i]);
            for(j=0; j<secsiz; j++) {
                fputc(secdata[sectormapsorted[i]][j], fpout);
            }
        }
        for(i=0; i<seccnt; i++) {
            fprintf(stderr," %-2d",sectormap[i]);
        }
        fprintf(stderr,"\n");
    } 
    fclose(fpin);
    fclose(fpout);
    return(0);
}
