CFLAGS=-O -fPIC -Iinclude -mrdrnd
#CFLAGS=-g -fPIC -Iinclude -mrdrnd
LDFLAGS=-ldl -lm
all : libs bins

libs : lib/libstats_max64.so lib/libstats_repeat.so lib/librng_rdrand.so lib/librng_reader.so lib/librng_skip.so 

bins : bin/test_reader bin/test_bits bin/testrng bin/test_stats_max64 bin/dieharder_to_binary bin/test_rng_skip bin/test_parse bin/test_path_to_self

tmp/dieharder_to_binary.o : src/dieharder_to_binary.c
	$(CC) -c -o $@ $(CFLAGS) $<

bin/dieharder_to_binary : tmp/dieharder_to_binary.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/path_to_self.o : src/path_to_self.c include/path_to_self.h
	$(CC) -c -o $@ $(CFLAGS) $<

tmp/parse.o : src/parse.c include/parse.h
	$(CC) -c -o $@ $(CFLAGS) $<

tmp/test_parse.o : src/test_parse.c include/parse.h
	$(CC) -c -o $@ $(CFLAGS) $<

bin/test_parse : tmp/test_parse.o tmp/parse.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/test_path_to_self.o : src/test_path_to_self.c include/path_to_self.h
	$(CC) -c -o $@ $(CFLAGS) $<

bin/test_path_to_self : tmp/test_path_to_self.o tmp/path_to_self.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/reader.o : src/reader.c include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<

tmp/test_reader.o : src/test_reader.c include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<

bin/test_reader : tmp/test_reader.o tmp/reader.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/bits.o : src/bits.c include/bits.h
	$(CC) -c -o $@ $(CFLAGS) $<

tmp/test_bits.o : src/test_bits.c include/bits.h include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<

bin/test_bits : tmp/test_bits.o tmp/bits.o tmp/reader.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)


tmp/stats_max64.o : src/stats_max64.c include/stats_max64.h include/stats.h include/reader.h include/bits.h
	$(CC) -c -o $@ $(CFLAGS) $<

lib/libstats_max64.so : tmp/stats_max64.o tmp/bits.o
	$(CC) -shared -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/stats_repeat.o : src/stats_repeat.c include/stats_repeat.h include/stats.h include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<

lib/libstats_repeat.so : tmp/stats_repeat.o tmp/stats_load.o tmp/load.o tmp/parse.o tmp/path_to_self.o
	$(CC) -shared -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/stats_load.o : src/stats_load.c include/stats_load.h include/load.h include/stats.h
	$(CC) -c -o $@ $(CFLAGS) $<

tmp/rng_load.o : src/rng_load.c include/rng_load.h include/load.h include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<

tmp/load.o : src/load.c include/load.h include/parse.h
	$(CC) -c -o $@ $(CFLAGS) $<

tmp/test_stats_max64.o : src/test_stats_max64.c include/stats_max64.h include/stats.h include/reader.h include/bits.h include/rng_rdrand.h
	$(CC) -c -o $@ $(CFLAGS) $<

bin/test_stats_max64 : tmp/test_stats_max64.o tmp/stats_max64.o tmp/reader.o tmp/bits.o tmp/rng_rdrand.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/testrng.o : src/main_testrng.c include/reader.h include/rng_rdrand.h include/stats_load.h
	$(CC) -c -o $@ $(CFLAGS) $<


tmp/rng_rdrand.o : src/rng_rdrand.c include/rng_rdrand.h include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<

lib/librng_rdrand.so : tmp/rng_rdrand.o
	$(CC) -shared -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/rng_reader.o : src/rng_reader.c include/rng_reader.h include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<

lib/librng_reader.so : tmp/rng_reader.o tmp/reader.o
	$(CC) -shared -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/rng_skip.o : src/rng_skip.c include/bits.h include/reader.h include/parse.h
	$(CC) -c -o $@ $(CFLAGS) $<

lib/librng_skip.so : tmp/rng_skip.o tmp/bits.o tmp/reader.o tmp/rng_load.o tmp/load.o tmp/parse.o tmp/path_to_self.o tmp/parse.o
	$(CC) -shared -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/test_rng_skip.o : src/test_rng_skip.c include/parse.h include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<

bin/test_rng_skip : tmp/test_rng_skip.o tmp/rng_skip.o tmp/rng_load.o tmp/load.o tmp/parse.o tmp/path_to_self.o tmp/parse.o tmp/bits.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

bin/testrng : tmp/testrng.o tmp/reader.o tmp/rng_rdrand.o tmp/stats_load.o tmp/rng_load.o tmp/load.o tmp/parse.o tmp/path_to_self.o tmp/parse.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

run : bin/testrng
	bin/testrng --rng /dev/urandom --stats "repeat samples=100 stats=(max64)"

show_dieharder_rngs :
	dieharder -g -1

STATS0=--stats "repeat samples=1e6 limit=10 stats=(max64 samples=3 use0=24 skip0=8 use1=24 skip1=8 offset=0)"
STATS1=--stats "repeat samples=1e7 limit=10 progress=100000 stats=(max64 samples=3 use0=24 skip0=8 use1=24 skip1=8 offset=0)"
STATS2=--stats "repeat samples=1e7 limit=10 progress=100000 stats=(max64 samples=3 use0=21 skip0=3 use1=21 skip1=1 offset=0)"
STATS=$(STATS2)

RNG0=--rng rdrand


GOOD=\
	test-rdrand \
	test-urandom \
	test-dh-AES_OFB

POOR=\
	test-dh-borosh13 \
	test-dh-rand \
	test-dh-coveyou \
	test-dh-knuthran \
	test-dh-ran3 \
	test-dh-rand \
	test-dh-ranlux \
	test-dh-ranlux389 \
	test-dh-ranlxs0 \
	test-dh-ranlxs1 \
	test-dh-ranlxs2 \
	test-dh-ranmar \
	test-dh-slatec \
	test-dh-transputer \
	test-dh-uni \
	test-dh-vax \
	test-dh-waterman14 \
	test-dh-zuf \
	test-dh-R_knuth_taocp \
	test-dh-R_knuth_taocp2

test-pass : $(GOOD)
test-fail : $(POOR)

test-rdrand : libs bins
	bin/testrng --rng "rdrand" $(STATS)

test-urandom : libs bins
	bin/testrng --rng "reader /dev/urandom" $(STATS)

test-dh-% : libs bins
	bin/testrng \
		--rng "skip skip=1 keep=31 rng=(reader src/rng_dieharder $*|)" \
		--stats "repeat samples=1e7 limit=10 progress=100000 stats=(max64 samples=3 use0=21 skip0=3 use1=21 skip1=1 offset=0)"

test-skip : libs bins
	bin/testrng \
		--rng "skip skip=1 keep=7 rng=(rdrand)" \
		--stats "repeat samples=1e7 limit=10 progress=1000 stats=(max64 samples=3 use0=21 skip0=3 use1=21 skip1=1 offset=0)"


dieharder-list-tests:
	dieharder -l

dieharder-list-rngs:
	dieharder -g -1

