# Dieharder Example Runs

## Why

The results of these runs might be valuable for comparison to
other random number generators or tests of randomness.

Because of the time involved in generating these tests, it seems useful to host them where they can be obtained in minutes instead of days of CPU time.

## What

This project contains 10 example runs of the deterministic
pseudo random number generators in the dieharder suite against
each of the tests listed as good on intel (little endian) hardware.

Runs were made on different systems with unknown CPU loads (but all intel little endian architecture), so the rands/sec results have no comparable meaning.

### Random number generators

These are generally listed by `dieharder -g -1`.  Runs that would be different on different systems are not represented here.

|gid|name          |gid|name          |gid|name|
|---|--------------|---|--------------|---|--------------|
|000|borosh13      |001|cmrg          |002|coveyou       |
|003|fishman18     |004|fishman20     |005|fishman2x     |
|006|gfsr4         |007|knuthran      |008|knuthran2     |
|009|knuthran2002  |010|lecuyer21     |011|minstd        |
|012|mrg           |013|mt19937       |014|mt19937_1999  |
|015|mt19937_1998  |016|r250          |017|ran0          |
|018|ran1          |019|ran2          |020|ran3          |
|021|rand          |022|rand48        |023|random128-bsd |
|024|random128-glibc2|025|random128-libc5|026|random256-bsd|
|027|random256-glibc2|028|random256-libc5|029|random32-bsd|
|030|random32-glibc2|031|random32-libc5|032|random64-bsd|
|033|random64-glibc2|034|random64-libc5|035|random8-bsd|
|036|random8-glibc2|037|random8-libc5|038|random-bsd|
|039|random-glibc2 |040|random-libc5  |041|randu         |
|042|ranf          |043|ranlux        |044|ranlux389     |
|045|ranlxd1       |046|ranlxd2       |047|ranlxs0       |
|048|ranlxs1       |049|ranlxs2       |050|ranmar        |
|051|slatec        |052|taus          |053|taus2         |
|054|taus113       |055|transputer    |056|tt800         |
|057|uni           |058|uni32         |059|vax           |
|060|waterman14    |061|zuf           |   |              |
|<del>200</del>|<del>stdin_input_raw</del>|<del>201</del>|<del>file_input_raw</del>|<del>202</del>|<del>file_input</del>|
|203|ca            |204|uvag          |205|AES_OFB       |
|206|Threefish_OFB |<del>207</del>|<del>XOR (supergenerator)</del>|208|kiss|
|209|superkiss     |   |              |   |              |
|400|R_wichmann_hill|401|R_marsaglia_multic.|402|R_super_duper|
|403|R_mersenne_twister|404|R_knuth_taocp|405|R_knuth_taocp2|
|<del>500</del>|<del>/dev/random</del>|<del>501</del>|<del>/dev/urandom</del>|   |               |

### Dieharder Tests

These are generally listed by `dieharder -l`.  Only tests marked as good are represented here.

|Test Number|                        Test Name|Test Reliability|
|-----------|---------------------------------|----------------|
|       -d 0|           Diehard Birthdays Test|    Good        |
|       -d 1|              Diehard OPERM5 Test|	   Good        |
|       -d 2|   Diehard 32x32 Binary Rank Test|    Good        |
|       -d 3|     Diehard 6x8 Binary Rank Test|    Good        |
|       -d 4|           Diehard Bitstream Test|    Good        |
|<del>-d 5</del>|   <del>Diehard OPSO</del>| <del>Suspect</del>|
|<del>-d 6</del>|<del>Diehard OQSO Test</del>|<del>Suspect</del>|
|<del>-d 7</del>|<del>Diehard DNA Test</del>|<del>Suspect</del>|
|       -d 8|Diehard Count the 1s (stream) Test|   Good        |
|       -d 9| Diehard Count the 1s Test (byte)|    Good        |
|      -d 10|         Diehard Parking Lot Test|    Good        |
|      -d 11|Diehard Minimum Distance (2d Circle) Test|  Good  |
|      -d 12|Diehard 3d Sphere (Minimum Distance) Test|  Good  |
|      -d 13|             Diehard Squeeze Test|    Good        |
|<del>-d 14</del>|<del>Diehard Sums Test</del>|<del>Do Not Use</del>|
|      -d 15|                Diehard Runs Test|	   Good        |
|      -d 16|               Diehard Craps Test|    Good        |
|      -d 17|     Marsaglia and Tsang GCD Test|    Good        |
|     -d 100|                 STS Monobit Test|    Good        |
|     -d 101|                    STS Runs Test|    Good        |
|     -d 102|    STS Serial Test (Generalized)|    Good        |
|     -d 200|        RGB Bit Distribution Test|    Good        |
|     -d 201|RGB Generalized Minimum Distance Test|  Good      |
|     -d 202|            RGB Permutations Test|    Good        |
|     -d 203|              RGB Lagged Sum Test|    Good        |
|     -d 204| RGB Kolmogorov-Smirnov Test Test|    Good        |
|     -d 205|                Byte Distribution|	   Good        |
|     -d 206|                          DAB DCT|    Good        |
|     -d 207|               DAB Fill Tree Test|    Good        |
|     -d 208|             DAB Fill Tree 2 Test|    Good        |
|     -d 209|               DAB Monobit 2 Test|    Good        |

The directory structure is

```bash
HERE/d<test>/g<generator>/d<test>_g<generator>_S<seed>.log
```

The dieharder tests ids and random generator ids are zero-padded in the names so they list in numerical order.

The output is the result of

```bash
dieharder -d <test> -g <generator> -S <seed>
```

There is a script file with a `.sh` extention which should reproduce the output.  Unfortunately dieharder is not currently honoring the `-S` option, so only new runs can be produced this way.

# Copyright

These results are in the public domain.

If you use this work please refer to the resource so others might
take advantage of it.



