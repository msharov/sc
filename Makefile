-include Config.mk

SC	:= ${name}
PSC	:= p${name}

SCSRC	:= $(sort $(filter-out psc.c, $(wildcard *.c)) gram.c)
PSCSRC	:= psc.c xmalloc.c vmtbl.c

SCOBJ	:= $(addprefix .o/,$(SCSRC:.c=.o))
PSCOBJ	:= $(addprefix .o/,$(PSCSRC:.c=.o))
ALLOBJ	:= $(sort ${SCOBJ} ${PSCOBJ})

DOCS	:= doc/sc.1 doc/psc.1 doc/tutorial.sc
CONFS	:= config.status config.h Config.mk
GRAMS	:= gram.c gram.h experres.h statres.h

######## Compilation #################################################

all:	${SC} ${PSC}

${SC}:	${SCOBJ}
	@echo "Linking $@ ..."
	@${CC} ${LDFLAGS} -o $@ ${SCOBJ} ${SCLIBS}

${PSC}:	${PSCOBJ}
	@echo "Linking $@ ..."
	@${CC} ${LDFLAGS} -o $@ ${PSCOBJ} ${PSCLIBS}

.o/%.o:	%.c
	@echo "    Compiling $< ..."
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@${CC} ${CFLAGS} -MMD -MT "$(<:.c=.s) $@" -o $@ -c $<

%.s:	%.c
	@echo "    Assembling $< ..."
	@${CC} ${CFLAGS} -S -o $@ -c $<

gram.c:	gram.y
	@echo "    Compiling $< ..."
	@${YACC} -d $< -o $@
experres.h:	gram.y eres.sed
	@echo "    Generating $@ ..."
	@sed -f eres.sed < gram.y > experres.h
statres.h:	gram.y sres.sed
	@echo "    Generating $@ ..."
	@sed -f sres.sed < gram.y > statres.h
gram.h:	gram.c
lex.c:	experres.h statres.h gram.h

######## Installation ################################################

ifdef BINDIR
ISC	:= ${BINDIR}/${SC}
IPSC	:= ${BINDIR}/${PSC}
ISCMAN	:= ${MANDIR}/${SC}.${MANEXT}
IPSCMAN	:= ${MANDIR}/${PSC}.${MANEXT}
ISCTUT	:= ${DATADIR}/${SC}/tutorial.sc

${ISC}:		${SC}
	@echo "Installing $@ ..."
	@${INSTALLBIN} $< $@
${IPSC}:	${PSC}
	@echo "Installing $@ ..."
	@${INSTALLBIN} $< $@
${ISCMAN}:	doc/${SC}.1
	@echo "Installing $@ ..."
	@${INSTALLMAN} $< $@
${IPSCMAN}:	doc/${PSC}.1
	@echo "Installing $@ ..."
	@${INSTALLMAN} $< $@
${ISCTUT}:	doc/tutorial.sc
	@echo "Installing $@ ..."
	@${INSTALLDOC} $< $@

install:	${ISC} ${IPSC} ${ISCMAN} ${IPSCMAN} ${ISCTUT}
uninstall:
	@echo "Uninstalling ..."
	@rm -f ${ISC} ${IPSC} ${ISCMAN} ${IPSCMAN} ${ISCTUT}
	@rmdir -p ${DATADIR}/${SC} ${MANDIR} ${BINDIR} &> /dev/null || true
endif

######## Maintenance #################################################

clean:
	@rm -rf .o
	@rm -f ${SC} ${PSC} ${GRAMS}

distclean:	clean
	@rm -f config.h Config.mk config.status

maintainer-clean:	distclean

Config.mk config.h:	config.status
config.status:		configure Config.mk.in config.h.in
	@if [ -x config.status ]; then echo "Reconfiguring ..."; ./config.status; \
	else echo "Running configure ..."; ./configure; fi
${ALLOBJ} ${DOCS} ${GRAMS}:	Makefile ${CONFS}

-include ${ALLOBJ:.o=.d}
