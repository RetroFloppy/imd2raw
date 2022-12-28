The test disk image `BLANK.IMD` is from SWTPC 6800 SSB DOS68 v5.1, and has a wide sector skew.
The linear image `BLANK.IMG` was created with the ImageDisk tool `IMDU.COM` and is considered correct.
The output of `./imd2raw BLANK.IMD test.img` should match byte-for-byte when comparing `test.img` with
`BLANK.IMG`.
