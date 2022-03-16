HEADER	=	$(wildcard *.hh)
SOURCE	=	$(wildcard *.cc)

OBJECT_DEBUG	=	$(addprefix obj/debug/,$(addsuffix .o,$(SOURCE)))
OBJECT_RELEASE	=	$(addprefix obj/release/,$(addsuffix .o,$(SOURCE)))

debug:	obj/ $(OBJECT_DEBUG)
	make -C ../../chess debug
	make -C optionparser
	@g++ -o x $(OBJECT_DEBUG) ../../chess/obj/debug/*.cc.o optionparser/obj/debug/*.cc.o

release:	obj/ $(OBJECT_RELEASE)
	make -C ../../chess release
	make -C optionparser
	@g++ -o x $(OBJECT_RELEASE) ../../chess/obj/release/*.cc.o optionparser/obj/debug/*.cc.o

obj/debug/%.cc.o:	%.cc $(HEADER)
	@echo "Building $< in debug mode"
	@g++ -c -o $@ $< -std=c++17 -pedantic -Wall -Wextra -g -D DEBUG

obj/release/%.cc.o:	%.cc $(HEADER)
	@echo "Building $< in release mode"
	@g++ -c -o $@ $< -std=c++17 -pedantic -Wall -Wextra -O3

obj/:
	@mkdir -p obj/debug
	@mkdir -p obj/release
