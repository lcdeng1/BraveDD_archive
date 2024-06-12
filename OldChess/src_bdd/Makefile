
EXES= build_bdd test_parser read_bdd read_bdds test_query

all: $(EXES)

BUILD_SRC= bdd.cc build_bdd.cc buffer.cc ct.cc operation.cc parser.cc \
          switches.cc resolv.cc alg_restr.cc alg_osm.cc alg_tsm.cc

READ_SRC= fixed_dd.cc read_bdd.cc
READ2_SRC= fixed_dd.cc read_bdds.cc
TP_SRC = test_parser.cc parser.cc
Q_SRC = test_query.cc fixed_dd.cc parser.cc

BUILD_OBJ= $(BUILD_SRC:.cc=.o)
READ_OBJ= $(READ_SRC:.cc=.o)
READ2_OBJ= $(READ2_SRC:.cc=.o)
TP_OBJ= $(TP_SRC:.cc=.o)
Q_OBJ= $(Q_SRC:.cc=.o)

# CFLAGS=-Wall -O3
#CFLAGS=-O3 -Weverything -Wno-padded -Wno-suggest-override -Wno-suggest-destructor-override -Wno-weak-vtables -Wno-poison-system-directories -Wno-global-constructors
# CFLAGS=-Wall -ggdb

build_bdd: $(BUILD_OBJ)
	g++ -std=c++11 -o build_bdd $(BUILD_OBJ)

test_parser: $(TP_OBJ)
	g++ -std=c++11 -o test_parser $(TP_OBJ)

test_query: $(Q_OBJ)
	g++ -std=c++11 -o test_query $(Q_OBJ)

read_bdd: $(READ_OBJ)
	g++ -std=c++11 -o read_bdd $(READ_OBJ)

read_bdds: $(READ2_OBJ)
	g++ -std=c++11 -o read_bdds $(READ2_OBJ)

%.o: %.cc
	g++ -std=c++11 -c $(CFLAGS) $<

clean:
	echo "$(BUILD_OBJ) $(READ_OBJ) $(READ2_OBJ) $(TP_OBJ) $(Q_OBJ) $(EXES) reader.tar.gz" | xargs -n 1 | sort -u | xargs rm

depend:
	makedepend -DSKIP_SYSTEM_INCLUDES $(BUILD_SRC) $(READ_SRC) $(READ2_SRC) $(TP_SRC) $(Q_SRC)

reader.tar.gz: $(READ_SRC) fixed_dd.h
	tar czvf reader.tar.gz $(READ_SRC) fixed_dd.h

# DO NOT DELETE

bdd.o: bdd.h defines.h ct.h
build_bdd.o: parser.h bdd.h defines.h buffer.h ct.h resolv.h switches.h
buffer.o: buffer.h resolv.h bdd.h defines.h
ct.o: ct.h defines.h operation.h bdd.h
operation.o: operation.h bdd.h defines.h ct.h
parser.o: parser.h
switches.o: switches.h
resolv.o: resolv.h
alg_restr.o: bdd.h defines.h ct.h operation.h resolv.h
alg_osm.o: bdd.h defines.h ct.h operation.h resolv.h
alg_tsm.o: bdd.h defines.h ct.h operation.h resolv.h
fixed_dd.o: fixed_dd.h exception_neutral_macros.hpp
read_bdd.o: fixed_dd.h
fixed_dd.o: fixed_dd.h exception_neutral_macros.hpp
read_bdds.o: fixed_dd.h
test_parser.o: parser.h
test_quey.o: parser.h fixed_dd.h
parser.o: parser.h
