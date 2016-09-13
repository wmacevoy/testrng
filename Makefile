CFLAGS=-O -fPIC -Iinclude -std=c11 -mrdrnd

all : libs bins

libs : lib/libstats_max64.so

bins : bin/test_reader bin/test_bits bin/testrng

tmp/path_to_self.o : src/path_to_self.c include/path_to_self.h
	$(CC) -c -o $@ $(CFLAGS) $<

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

tmp/stats_load.o : src/stats_load.c include/stats_load.h include/stats.h
	$(CC) -c -o $@ $(CFLAGS) $<

lib/libstats_max64.so : tmp/stats_max64.o tmp/bits.o
	$(CC) -shared -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/test_stats_max64.o : src/test_stats_max64.c include/stats_max64.h include/stats.h include/reader.h include/bits.h include/rng_rdrand.h
	$(CC) -c -o $@ $(CFLAGS) $<

bin/test_stats_max64 : tmp/test_stats_max64.o tmp/stats_max64.o tmp/reader.o tmp/bits.o tmp/rng_rdrand.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

tmp/testrng.o : src/main_testrng.c include/reader.h include/rng_rdrand.h include/stats_load.h
	$(CC) -c -o $@ $(CFLAGS) $<


tmp/rng_rdrand.o : src/rng_rdrand.c include/rng_rdrand.h include/reader.h
	$(CC) -c -o $@ $(CFLAGS) $<


bin/testrng : tmp/testrng.o tmp/reader.o tmp/rng_rdrand.o tmp/stats_load.o tmp/path_to_self.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)
