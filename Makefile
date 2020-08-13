-include Config.mk

################ Source files ##########################################

exe	:= $O${name}
pexe	:= $Op${name}
srcs	:= $(filter-out psc.c,$(wildcard *.c))
psrcs	:= psc.c xmalloc.c vmtbl.c
objs	:= $(addprefix $O,$(srcs:.c=.o)) $Ogram.o
pobjs	:= $(addprefix $O,$(psrcs:.c=.o))
deps	:= ${objs:.o=.d} $Opsc.d $Ogram.d
genf	:= $Ogram.c $Ogram.h $Oexperres.h $Ostatres.h
confs	:= Config.mk config.h
oname   := $(notdir $(abspath $O))

################ Compilation ###########################################

.SUFFIXES:
.PHONY: all clean distclean maintainer-clean

all:	${exe} ${pexe}

run:	${exe}
	@$<

${exe}:	${objs}
	@echo "Linking $@ ..."
	@${CC} ${ldflags} -o $@ $^ ${libs}

${pexe}:	${pobjs}
	@echo "Linking $@ ..."
	@${CC} ${ldflags} -o $@ $^

$O%.o:	%.c
	@echo "    Compiling $< ..."
	@${CC} ${cflags} -MMD -MT "$(<:.c=.s) $@" -o $@ -c $<

$Ogram.o:	$Ogram.c
	@echo "    Compiling $< ..."
	@${CC} ${cflags} -MMD -MT "$(<:.c=.s) $@" -I. -o $@ -c $<

%.s:	%.c
	@echo "    Compiling $< to assembly ..."
	@${CC} ${cflags} -S -o $@ -c $<

$Ogram.h:	| $Ogram.c
$Ogram.c $Ogram.h:	gram.y
	@echo "    Compiling $< ..."
	@${BISON} -d -o $Ogram.c $<
$Oexperres.h:	gram.y eres.sed
	@echo "    Generating $@ ..."
	@sed -f eres.sed < $< > $@
$Ostatres.h:	gram.y sres.sed
	@echo "    Generating $@ ..."
	@sed -f sres.sed < $< > $@
lex.c:	$Oexperres.h $Ostatres.h $Ogram.h
sc.c vi.c cmds.c:	$Ogram.h

################ Installation ##########################################

.PHONY:	install installdirs
.PHONY: uninstall uninstall-man uninstall-tutorial

ifdef bindir
exed	:= ${DESTDIR}${bindir}
exei	:= ${exed}/$(notdir ${exe})
pexei	:= ${exed}/$(notdir ${pexe})

${exed}:
	@echo "Creating $@ ..."
	@${INSTALL} -d $@
${exei}:	${exe} | ${exed}
	@echo "Installing $@ ..."
	@${INSTALL_PROGRAM} $< $@
${pexei}:	${pexe} | ${exed}
	@echo "Installing $@ ..."
	@${INSTALL_PROGRAM} $< $@

installdirs:	${exed}
install:	${exei} ${pexei}
uninstall:
	@if [ -f ${exei} -o -f ${pexei} ]; then\
	    echo "Removing ${exei} ...";\
	    rm -f ${exei} ${pexei};\
	fi
endif
ifdef man1dir
mand	:= ${DESTDIR}${man1dir}
mani	:= ${mand}/${name}.1
pmani	:= ${mand}/p${name}.1

${mand}:
	@echo "Creating $@ ..."
	@${INSTALL} -d $@
${mani}:	doc/${name}.1 | ${mand}
	@echo "Installing $@ ..."
	@${INSTALL_DATA} $< $@
${pmani}:	doc/p${name}.1 | ${mand}
	@echo "Installing $@ ..."
	@${INSTALL_DATA} $< $@

installdirs:	${mand}
install:	${mani} ${pmani}
uninstall:	uninstall-man
uninstall-man:
	@if [ -f ${mani} -o -f ${pmani} ]; then\
	    echo "Removing ${mani} ...";\
	    rm -f ${mani} ${pmani};\
	fi
endif
ifdef datadir
tutd	:= ${DESTDIR}${datadir}/${name}
tuti	:= ${tutd}/tutorial.sc

${tutd}:
	@echo "Creating $@ ..."
	@${INSTALL} -d $@
${tuti}:	doc/tutorial.sc | ${tutd}
	@echo "Installing $@ ..."
	@${INSTALL_DATA} $< $@

installdirs:	${tutd}
install:	${tuti}
uninstall:	uninstall-tutorial
uninstall-tutorial:
	@if [ -f ${tuti} ]; then\
	    echo "Removing ${tuti} ...";\
	    rm -f ${tuti};\
	    rmdir ${tutd};\
	fi
endif

################ Maintenance ###########################################

clean:
	@if [ -d ${builddir} ]; then\
	    rm -f ${exe} ${pexe} ${objs} ${pobjs} ${genf} ${deps} $O.d;\
	    rmdir ${builddir};\
	fi

distclean:	clean
	@rm -f ${oname} ${confs} config.status

maintainer-clean: distclean

${builddir}/.d:
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@touch $@
$O.d:	| ${builddir}/.d
	@[ -h ${oname} ] || ln -sf ${builddir} ${oname}

${objs} ${genf}:	Makefile ${confs} | $O.d
Config.mk:	Config.mk.in
config.h:	config.h.in | Config.mk
${confs}:	configure
	@if [ -x config.status ]; then echo "Reconfiguring ...";\
	    ./config.status;\
	else echo "Running configure ...";\
	    ./configure;\
	fi

-include ${deps}
