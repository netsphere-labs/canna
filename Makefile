# Makefile generated by imake - do not edit!
# $TOG: imake.c /main/97 1997/06/20 20:23:51 kaleb $

# ----------------------------------------------------------------------
# Makefile generated from "Imake.tmpl" and <Imakefile>
# $TOG: Imake.tmpl /main/245 1997/05/20 10:05:47 kaleb $
#
#
#
#
# $XFree86: xc/config/cf/Imake.tmpl,v 3.32.2.5 1997/07/06 07:27:59 dawes Exp $
# ----------------------------------------------------------------------

all::

.SUFFIXES: .i

# $TOG: Imake.cf /main/28 1997/06/25 08:31:36 barstow $
# $XFree86: xc/config/cf/Imake.cf,v 3.34.2.3 1997/07/27 02:41:02 dawes Exp $

# -----------------------------------------------------------------------
# site-specific configuration parameters that need to come before
# the platform-specific parameters - edit site.def to change

# site:  $XConsortium: site.def /main/revisionist/4 1996/12/31 08:02:07 kaleb $
# site:  $XFree86: xc/config/cf/site.def,v 3.17.2.1 1997/06/22 10:32:21 dawes Exp $

# $XFree86: xc/config/cf/xf86site.def,v 3.101.2.11 1997/06/22 10:32:22 dawes Exp $

# ----------------------------------------------------------------------
# platform-specific configuration parameters - edit linux.cf to change

# platform:  $TOG: linux.cf /main/36 1997/06/16 22:21:03 kaleb $
# platform:  $XFree86: xc/config/cf/linux.cf,v 3.57.2.10 1997/07/28 14:17:25 dawes Exp $

# operating system:  Linux 2.0.33 i586 [ELF] (2.0.33)
# libc:	(6.4.0)
# binutils:	(28)

# $XConsortium: lnxLib.rules /main/13 1996/09/28 16:11:01 rws $
# $XFree86: xc/config/cf/lnxLib.rules,v 3.28.2.3 1997/06/22 10:32:20 dawes Exp $

# $XFree86: xc/config/cf/xfree86.cf,v 3.129.2.14 1997/07/06 07:28:00 dawes Exp $

# $XConsortium: xfree86.cf /main/34 1996/12/06 11:45:18 rws $

LINKKITDIR = $(USRLIBDIR)/Server
XF98LINKKITDIR = $(USRLIBDIR)/Server

       XF86SRC = $(SERVERSRC)/hw/xfree86
  XF86ACCELSRC = $(XF86SRC)/accel
    XF86COMSRC = $(XF86SRC)/common
 XF86CONFIGSRC = $(XF86COMSRC)
     XF86HWSRC = $(XF86SRC)/common_hw
     XF86OSSRC = $(XF86SRC)/os-support
  VGADRIVERSRC = $(XF86SRC)/vga256/drivers
VGA16DRIVERSRC = $(XF86SRC)/vga16/drivers
 VGA2DRIVERSRC = $(XF86SRC)/vga2/drivers
 MONODRIVERSRC = $(XF86SRC)/mono/drivers
   S3DRIVERSRC = $(XF86SRC)/accel/s3/drivers
  S3VDRIVERSRC = $(XF86SRC)/accel/s3_virge/drivers

       XF68SRC = $(SERVERSRC)/hw/xfree68
    XF68COMSRC = $(XF68SRC)/common
 XF68CONFIGSRC = $(XF68COMSRC)
     XF68OSSRC = $(XF68SRC)/os-support

           XF98SRC = $(SERVERSRC)/hw/xfree98
      XF98ACCELSRC = $(XF98SRC)/accel
        XF98COMSRC = $(XF98SRC)/common
     XF98CONFIGSRC = $(XF98COMSRC)
         XF98HWSRC = $(XF98SRC)/common_hw/generic
      XF98HWNECSRC = $(XF98SRC)/common_hw/nec
    XF98HWPWSKBSRC = $(XF98SRC)/common_hw/pwskb
     XF98HWPWLBSRC = $(XF98SRC)/common_hw/pwlb
    XF98HWGA968SRC = $(XF98SRC)/common_hw/ga968
         XF98OSSRC = $(XF98SRC)/os-support
  XF98VGADRIVERSRC = $(XF98SRC)/vga256/drivers
XF98VGA16DRIVERSRC = $(XF98SRC)/vga16/drivers
 XF98VGA2DRIVERSRC = $(XF98SRC)/vga2/drivers
 XF98MONODRIVERSRC = $(XF98SRC)/mono/drivers
XF98NECS3DRIVERSRC = $(XF98SRC)/accel/s3nec/drivers
XF98PWSKBDRIVERSRC = $(XF98SRC)/accel/s3pwskb/drivers
 XF98PWLBDRIVERSRC = $(XF98SRC)/accel/s3pwlb/drivers
XF98GA968DRIVERSRC = $(XF98SRC)/accel/s3ga968/drivers

        XFREE86DOCDIR = $(LIBDIR)/doc
      XFREE86PSDOCDIR = $(XFREE86DOCDIR)/PostScript
    XFREE86HTMLDOCDIR = $(XFREE86DOCDIR)/html
XFREE86JAPANESEDOCDIR = $(XFREE86DOCDIR)/Japanese

# $XConsortium: xf86.rules /main/9 1996/10/31 14:54:26 kaleb $
# $XFree86: xc/config/cf/xf86.rules,v 3.16.2.1 1997/05/18 12:00:01 dawes Exp $

# ----------------------------------------------------------------------
# site-specific configuration parameters that go after
# the platform-specific parameters - edit site.def to change

# site:  $XConsortium: site.def /main/revisionist/4 1996/12/31 08:02:07 kaleb $
# site:  $XFree86: xc/config/cf/site.def,v 3.17.2.1 1997/06/22 10:32:21 dawes Exp $

# ---------------------------------------------------------------------
# Imake rules for building libraries, programs, scripts, and data files
# rules:  $TOG: Imake.rules /main/222 1997/07/17 20:04:40 kaleb $
# rules:  $XFree86: xc/config/cf/Imake.rules,v 3.33.2.5 1997/07/19 04:59:07 dawes Exp $

 _NULLCMD_ = @ echo -n

TKLIBNAME = tk4.2

TKLIBDIR = /usr/lib

TCLLIBNAME = tcl7.6

TCLIBDIR = /usr/lib

          PATHSEP = /
            SHELL = /bin/sh

              TOP = .
      CURRENT_DIR = .

            IMAKE = imake
           DEPEND = gccmakedep
        MKDIRHIER = mkdir -p
    EXPORTLISTGEN =
        CONFIGSRC = $(TOP)/config
         IMAKESRC = $(CONFIGSRC)/imake
        DEPENDSRC = $(CONFIGSRC)/util

          INCROOT = /usr/X11R6/include
        USRLIBDIR = /usr/X11R6/lib
        VARLIBDIR = /var/lib
         SHLIBDIR = /usr/X11R6/lib
       LINTLIBDIR = $(USRLIBDIR)/lint
          MANPATH = /usr/X11R6/man
    MANSOURCEPATH = $(MANPATH)/man
           MANDIR = $(MANSOURCEPATH)1
        LIBMANDIR = $(MANSOURCEPATH)3
       FILEMANDIR = $(MANSOURCEPATH)5

               AR = ar clq
  BOOTSTRAPCFLAGS =
               CC = gcc
               AS = as

.SUFFIXES: .cc

              CXX = c++
          CXXFILT = c++filt
           CXXLIB =
    CXXDEBUGFLAGS = -O2 -fno-strength-reduce
CXXDEPENDINCLUDES =
 CXXEXTRA_DEFINES =
CXXEXTRA_INCLUDES =
   CXXSTD_DEFINES = -Dlinux -D__i386__ -D_POSIX_C_SOURCE=199309L -D_POSIX_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_REENTRANT $(CXXPROJECT_DEFINES)
       CXXOPTIONS =
      CXXINCLUDES = $(INCLUDES) $(TOP_INCLUDES) $(CXXEXTRA_INCLUDES)
       CXXDEFINES = $(CXXINCLUDES) $(CXXSTD_DEFINES) $(THREADS_CXXDEFINES) $(CXXEXTRA_DEFINES) $(DEFINES)
         CXXFLAGS = $(CXXDEBUGFLAGS) $(CXXOPTIONS) $(THREADS_CXXFLAGS) $(CXXDEFINES)

         COMPRESS = compress
          GZIPCMD = gzip
              CPP = /lib/cpp $(STD_CPP_DEFINES)
    PREPROCESSCMD = gcc -E $(STD_CPP_DEFINES)
          INSTALL = install
     INSTALLFLAGS = -c
               LD = ld
              LEX = flex -l
           LEXLIB = -lfl
             YACC = bison -y
           CCYACC = bison -y
             LINT = lint
      LINTLIBFLAG = -C
         LINTOPTS = -axz
               LN = ln -s
             MAKE = make
               MV = mv -f
               CP = cp

           RANLIB = ranlib
  RANLIBINSTFLAGS =

               RM = rm -f
        MANSUFFIX = 1x
     LIBMANSUFFIX = 3x
    FILEMANSUFFIX = 5x
            TROFF = psroff
            NROFF = nroff
         MSMACROS = -ms
        MANMACROS = -man
              TBL = tbl
              EQN = eqn
             NEQN = neqn
              COL = col

            DVIPS = dvips
            LATEX = latex

     STD_INCLUDES =
  STD_CPP_DEFINES = -traditional -Dlinux -D__i386__ -D_POSIX_C_SOURCE=199309L -D_POSIX_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_REENTRANT $(PROJECT_DEFINES)
      STD_DEFINES = -Dlinux -D__i386__ -D_POSIX_C_SOURCE=199309L -D_POSIX_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_REENTRANT $(PROJECT_DEFINES)
 EXTRA_LOAD_FLAGS =
  EXTRA_LDOPTIONS =
  EXTRA_LIBRARIES =
             TAGS = ctags

   PARALLELMFLAGS =

    SHAREDCODEDEF =
         SHLIBDEF =

     SHLIBLDFLAGS = -shared

         PICFLAGS = -fPIC

      CXXPICFLAGS = -fPIC

    PROTO_DEFINES = -DFUNCPROTO=15 -DNARROWPROTO

     INSTPGMFLAGS = -s

     INSTBINFLAGS = -m 0755
     INSTUIDFLAGS = -m 4711
     INSTLIBFLAGS = -m 0644
     INSTINCFLAGS = -m 0444
     INSTMANFLAGS = -m 0444
     INSTDATFLAGS = -m 0444
    INSTKMEMFLAGS = -m 4711

      PROJECTROOT = /usr/X11R6

      CDEBUGFLAGS = -O2 -fno-strength-reduce
        CCOPTIONS =

      ALLINCLUDES = $(INCLUDES) $(EXTRA_INCLUDES) $(TOP_INCLUDES) $(STD_INCLUDES)
       ALLDEFINES = $(ALLINCLUDES) $(STD_DEFINES) $(EXTRA_DEFINES) $(PROTO_DEFINES) $(THREADS_DEFINES) $(DEFINES)
           CFLAGS = $(CDEBUGFLAGS) $(CCOPTIONS) $(THREADS_CFLAGS) $(ALLDEFINES)
        LINTFLAGS = $(LINTOPTS) -DLINT $(ALLDEFINES) $(DEPEND_DEFINES)
         LDPRELIB = -L$(USRLIBDIR)
        LDPOSTLIB =
        LDOPTIONS = $(CDEBUGFLAGS) $(CCOPTIONS)  $(EXTRA_LDOPTIONS) $(THREADS_LDFLAGS) $(LOCAL_LDFLAGS) $(LDPRELIBS)
     CXXLDOPTIONS = $(CXXDEBUGFLAGS) $(CXXOPTIONS) $(EXTRA_LDOPTIONS) $(THREADS_CXXLDFLAGS) $(LOCAL_LDFLAGS) $(LDPRELIBS)

           LDLIBS = $(LDPOSTLIBS) $(THREADS_LIBS) $(SYS_LIBRARIES) $(EXTRA_LIBRARIES)

           CCLINK = $(CC)

          CXXLINK = $(CXX)

     LDSTRIPFLAGS = -x
   LDCOMBINEFLAGS = -r
      DEPENDFLAGS =

# Not sure this belongs here
         TKLIBDIR = /usr/lib
         TKINCDIR = /usr/include/tk
        TKLIBNAME = tk4.2
        TKLIBRARY = -L$(TKLIBDIR) -l$(TKLIBNAME)
        TCLLIBDIR = /usr/lib
        TCLINCDIR = /usr/include/tcl
       TCLLIBNAME = tcl7.6
       TCLLIBRARY = -L$(TCLLIBDIR) -l$(TCLLIBNAME)

        MACROFILE = linux.cf
           RM_CMD = $(RM)

    IMAKE_DEFINES =

         IRULESRC = $(CONFIGDIR)
        IMAKE_CMD = $(IMAKE) -DUseInstalled -I$(IRULESRC) $(IMAKE_DEFINES)

     ICONFIGFILES = $(IRULESRC)/Imake.tmpl $(IRULESRC)/X11.tmpl 			$(IRULESRC)/site.def $(IRULESRC)/$(MACROFILE) 			$(IRULESRC)/xfree86.cf $(IRULESRC)/xf86.rules $(IRULESRC)/xf86site.def $(IRULESRC)/host.def $(EXTRA_ICONFIGFILES)

# $TOG: X11.rules /main/4 1997/04/30 15:23:24 kaleb $

# ----------------------------------------------------------------------
# X Window System Build Parameters and Rules
# $TOG: X11.tmpl /main/292 1997/05/20 10:05:59 kaleb $
#
#
#
#
# $XFree86: xc/config/cf/X11.tmpl,v 1.8.2.3 1997/05/21 15:02:13 dawes Exp $

# -----------------------------------------------------------------------
# X Window System make variables; these need to be coordinated with rules

             XTOP = $(TOP)
           BINDIR = /usr/X11R6/bin
     BUILDINCROOT = $(TOP)/exports
      BUILDINCDIR = $(BUILDINCROOT)/include
      BUILDINCTOP = ../..
      BUILDLIBDIR = $(TOP)/exports/lib
      BUILDLIBTOP = ../..
      BUILDBINDIR = $(TOP)/exports/bin
      BUILDBINTOP = ../..
    XBUILDINCROOT = $(XTOP)/exports
     XBUILDINCDIR = $(XBUILDINCROOT)/include/X11
     XBUILDINCTOP = ../../..
     XBUILDBINDIR = $(XBUILDINCROOT)/bin
           INCDIR = $(INCROOT)
           ADMDIR = /usr/adm
           LIBDIR = $(USRLIBDIR)/X11
   TOP_X_INCLUDES = -I$(XPROJECTROOT)/include

          FONTDIR = $(LIBDIR)/fonts
         XINITDIR = $(LIBDIR)/xinit
           XDMDIR = $(LIBDIR)/xdm
        XDMVARDIR = $(VARLIBDIR)/xdm
           TWMDIR = $(LIBDIR)/twm
           XSMDIR = $(LIBDIR)/xsm
           NLSDIR = $(LIBDIR)/nls
       XLOCALEDIR = $(LIBDIR)/locale
        PEXAPIDIR = $(LIBDIR)/PEX
      LBXPROXYDIR = $(LIBDIR)/lbxproxy
  PROXYMANAGERDIR = $(LIBDIR)/proxymngr
        XPRINTDIR = $(LIBDIR)
      XAPPLOADDIR = $(LIBDIR)/app-defaults
       FONTCFLAGS = -t

     INSTAPPFLAGS = $(INSTDATFLAGS)

              RGB = rgb
            FONTC = bdftopcf
        MKFONTDIR = mkfontdir

       DOCUTILSRC = $(XTOP)/doc/util
        CLIENTSRC = $(TOP)/clients
          DEMOSRC = $(TOP)/demos
       XDOCMACROS = $(DOCUTILSRC)/macros.t
       XIDXMACROS = $(DOCUTILSRC)/indexmacros.t
       PROGRAMSRC = $(TOP)/programs
           LIBSRC = $(XTOP)/lib
          FONTSRC = $(XTOP)/fonts
       INCLUDESRC = $(BUILDINCROOT)/include
      XINCLUDESRC = $(INCLUDESRC)/X11
        SERVERSRC = $(XTOP)/programs/Xserver
       CONTRIBSRC = $(XTOP)/../contrib
   UNSUPPORTEDSRC = $(XTOP)/unsupported
           DOCSRC = $(XTOP)/doc
           RGBSRC = $(XTOP)/programs/rgb
      BDFTOPCFSRC = $(PROGRAMSRC)/bdftopcf
     MKFONTDIRSRC = $(PROGRAMSRC)/mkfontdir
    FONTSERVERSRC = $(PROGRAMSRC)/xfs
       FONTINCSRC = $(XTOP)/include/fonts
        EXTINCSRC = $(XTOP)/include/extensions
     TRANSCOMMSRC = $(LIBSRC)/xtrans
   TRANS_INCLUDES = -I$(TRANSCOMMSRC)

       XENVLIBDIR = $(USRLIBDIR)
   CLIENTENVSETUP = LD_LIBRARY_PATH=$(XENVLIBDIR)

# $XConsortium: lnxLib.tmpl,v 1.5 95/01/11 21:44:44 kaleb Exp $
# $XFree86: xc/config/cf/lnxLib.tmpl,v 3.9 1996/02/24 04:32:52 dawes Exp $

          XLIBSRC = $(LIBSRC)/X11

SOXLIBREV = 6.1
DEPXONLYLIB =
XONLYLIB =  -lX11

LINTXONLY = $(LINTLIBDIR)/llib-lX11.ln

         XLIBONLY = $(XONLYLIB)

      XEXTLIBSRC = $(LIBSRC)/Xext

SOXEXTREV = 6.3
DEPEXTENSIONLIB =
EXTENSIONLIB =  -lXext

LINTEXTENSION = $(LINTLIBDIR)/llib-lXext.ln

LINTEXTENSIONLIB = $(LINTEXTENSION)
          DEPXLIB = $(DEPEXTENSIONLIB) $(DEPXONLYLIB)
             XLIB = $(EXTENSIONLIB) $(XONLYLIB)
         LINTXLIB = $(LINTXONLYLIB)

    XSSLIBSRC = $(LIBSRC)/Xss

DEPXSSLIB = $(USRLIBDIR)/libXss.a
XSSLIB =  -lXss

LINTXSS = $(LINTLIBDIR)/llib-lXss.ln

    XXF86MISCLIBSRC = $(LIBSRC)/Xxf86misc

DEPXXF86MISCLIB = $(USRLIBDIR)/libXxf86misc.a
XXF86MISCLIB =  -lXxf86misc

LINTXXF86MISC = $(LINTLIBDIR)/llib-lXxf86misc.ln

    XXF86VMLIBSRC = $(LIBSRC)/Xxf86vm

DEPXXF86VMLIB = $(USRLIBDIR)/libXxf86vm.a
XXF86VMLIB =  -lXxf86vm

LINTXXF86VM = $(LINTLIBDIR)/llib-lXxf86vm.ln

    XXF86DGALIBSRC = $(LIBSRC)/Xxf86dga

DEPXXF86DGALIB = $(USRLIBDIR)/libXxf86dga.a
XXF86DGALIB =  -lXxf86dga

LINTXXF86DGA = $(LINTLIBDIR)/llib-lXxf86dga.ln

    XDPMSLIBSRC = $(LIBSRC)/Xdpms

DEPXDPMSLIB = $(USRLIBDIR)/libXdpms.a
XDPMSLIB =  -lXdpms

LINTXDPMS = $(LINTLIBDIR)/llib-lXdpms.ln

         XAUTHSRC = $(LIBSRC)/Xau

DEPXAUTHLIB = $(USRLIBDIR)/libXau.a
XAUTHLIB =  -lXau

LINTXAUTH = $(LINTLIBDIR)/llib-lXau.ln

      XDMCPLIBSRC = $(LIBSRC)/Xdmcp

DEPXDMCPLIB = $(USRLIBDIR)/libXdmcp.a
XDMCPLIB =  -lXdmcp

LINTXDMCP = $(LINTLIBDIR)/llib-lXdmcp.ln

           XMUSRC = $(LIBSRC)/Xmu

SOXMUREV = 6.0
DEPXMULIB =
XMULIB =  -lXmu

LINTXMU = $(LINTLIBDIR)/llib-lXmu.ln

       OLDXLIBSRC = $(LIBSRC)/oldX

DEPOLDXLIB = $(USRLIBDIR)/liboldX.a
OLDXLIB =  -loldX

LINTOLDX = $(LINTLIBDIR)/llib-loldX.ln

         XPLIBSRC = $(LIBSRC)/Xp

SOXPREV = 6.2
DEPXPLIB =
XPLIB =  -lXp

LINTXP = $(LINTLIBDIR)/llib-lXp.ln

       TOOLKITSRC = $(LIBSRC)/Xt

SOXTREV = 6.0
DEPXTOOLONLYLIB =
XTOOLONLYLIB =  -lXt

LINTXTOOLONLY = $(LINTLIBDIR)/llib-lXt.ln

      DEPXTOOLLIB = $(DEPXTOOLONLYLIB) $(DEPSMLIB) $(DEPICELIB)
         XTOOLLIB = $(XTOOLONLYLIB) $(SMLIB) $(ICELIB)
     LINTXTOOLLIB = $(LINTXTOOLONLYLIB)

       XALIBSRC = $(LIBSRC)/Xa

SOXAREV = 1.0
DEPXALIB =
XALIB =  -lXa

LINTXA = $(LINTLIBDIR)/llib-lXa.ln

       AWIDGETSRC = $(LIBSRC)/Xaw

SOXAWREV = 6.1
DEPXAWLIB =
XAWLIB =  -lXaw

LINTXAW = $(LINTLIBDIR)/llib-lXaw.ln

         XILIBSRC = $(LIBSRC)/Xi

SOXINPUTREV = 6.0
DEPXILIB =
XILIB =  -lXi

LINTXI = $(LINTLIBDIR)/llib-lXi.ln

      XTESTLIBSRC = $(LIBSRC)/Xtst

SOXTESTREV = 6.1
DEPXTESTLIB =
XTESTLIB =  -lXtst

LINTXTEST = $(LINTLIBDIR)/llib-lXtst.ln

        PEXLIBSRC = $(LIBSRC)/PEX5

SOPEXREV = 6.0
DEPPEXLIB =
PEXLIB =  -lPEX5

LINTPEX = $(LINTLIBDIR)/llib-lPEX5.ln

        XIELIBSRC = $(LIBSRC)/XIE

SOXIEREV = 6.0
DEPXIELIB =
XIELIB =  -lXIE

LINTXIE = $(LINTLIBDIR)/llib-lXIE.ln

      PHIGSLIBSRC = $(LIBSRC)/PHIGS

DEPPHIGSLIB = $(USRLIBDIR)/libphigs.a
PHIGSLIB =  -lphigs

LINTPHIGS = $(LINTLIBDIR)/llib-lphigs.ln

DEPXBSDLIB = $(USRLIBDIR)/libXbsd.a
XBSDLIB =  -lXbsd

LINTXBSD = $(LINTLIBDIR)/llib-lXbsd.ln

           ICESRC = $(LIBSRC)/ICE

SOICEREV = 6.3
DEPICELIB =
ICELIB =  -lICE

LINTICE = $(LINTLIBDIR)/llib-lICE.ln

            SMSRC = $(LIBSRC)/SM

SOSMREV = 6.0
DEPSMLIB =
SMLIB =  -lSM

LINTSM = $(LINTLIBDIR)/llib-lSM.ln

           XKEYSRC = $(LIBSRC)/Xkey

SOXKEYREV = 6.0
DEPXKEYLIB =
XKEYLIB =  -lXkey

LINTXKEY = $(LINTLIBDIR)/llib-lXkey.ln

         FSLIBSRC = $(LIBSRC)/FS

DEPFSLIB = $(USRLIBDIR)/libFS.a
FSLIB =  -lFS

LINTFS = $(LINTLIBDIR)/llib-lFS.ln

         FONTLIBSRC = $(LIBSRC)/font

DEPFONTLIB = $(USRLIBDIR)/libfont.a
FONTLIB =  -lfont

LINTFONT = $(LINTLIBDIR)/llib-lfont.ln

          XPMLIBSRC = $(LIBSRC)/Xpm

DEPXPMLIB = $(USRLIBDIR)/libXpm.a
XPMLIB =  -lXpm

LINTXPM = $(LINTLIBDIR)/llib-lXpm.ln

    XKBFILELIBSRC = $(LIBSRC)/xkbfile

DEPXKBFILELIB = $(USRLIBDIR)/libxkbfile.a
XKBFILELIB =  -lxkbfile

LINTXKBFILE = $(LINTLIBDIR)/llib-lxkbfile.ln

     XKBCOMPCMD = xkbcomp

    XKBUILIBSRC = $(LIBSRC)/xkbui

DEPXKBUILIB = $(USRLIBDIR)/libxkbui.a
XKBUILIB =  -lxkbui

LINTXKBUI = $(LINTLIBDIR)/llib-lxkbui.ln

          DEPLIBS = $(DEPXAWLIB) $(DEPXMULIB) $(DEPXTOOLLIB) $(DEPXLIB)

         DEPLIBS1 = $(DEPLIBS)
         DEPLIBS2 = $(DEPLIBS)
         DEPLIBS3 = $(DEPLIBS)
         DEPLIBS4 = $(DEPLIBS)
         DEPLIBS5 = $(DEPLIBS)
         DEPLIBS6 = $(DEPLIBS)
         DEPLIBS7 = $(DEPLIBS)
         DEPLIBS8 = $(DEPLIBS)
         DEPLIBS9 = $(DEPLIBS)
         DEPLIBS10 = $(DEPLIBS)

XMULIBONLY = -lXmu
XMULIB = $(XMULIBONLY) $(XTOOLLIB) $(XLIB)

        CONFIGDIR = $(LIBDIR)/config

    USRLIBDIRPATH = $(USRLIBDIR)
        LDPRELIBS = -L$(USRLIBDIR)
       LDPOSTLIBS =
     TOP_INCLUDES = -I$(INCROOT) $(TOP_X_INCLUDES)
  PROJECT_DEFINES =

CXXPROJECT_DEFINES =

# ----------------------------------------------------------------------
# start of Imakefile

cannaBinDir = /usr/bin
cannaSrvDir = /usr/sbin
cannaLibDir = /var/lib/canna

cannaManDir = /usr/man
cannaIncDir = /usr/include/canna
libCannaDir = /usr/lib

ErrDir  = /var/log/canna

wcharDefinition = -DCANNA_WCHAR
Wlib =
JapaneseLocale = japanese

cannaOwner = bin
cannaGroup = bin

cannaOwnerGroup = -o $(cannaOwner) -g $(cannaGroup)

pointerIntegerDef =

cannaDsoRev = 1.0

sharedLibExtension = so.$(cannaDsoRev)

pubdicDir = $(CANNAROOT)/dic/ideo/pubdic

CHOWN = chown
CHGRP = chgrp
CHMOD = chmod

cannaLight = 1

dontHaveRename = 0

DicDir   = $(cannaLibDir)/dic

    DEPCANNALIB = $(CANNASRC)/libcanna.$(sharedLibExtension)

       CANNALIB = -L$(CANNASRC) -lcanna $(DLLIB)

    DEPCANNALIB16 = $(CANNASRC)/libcanna16.$(sharedLibExtension)

       CANNALIB16 = -L$(CANNASRC) -lcanna16 $(DLLIB)

 CANNASERVER_DEFINES = $(wcharDefinition)
          RK_DEFINES = $(pointerIntegerDef)
         RKC_DEFINES = $(wcharDefinition)
       UILIB_DEFINES = $(wcharDefinition) $(pointerIntegerDef)                        $(cannaDsoRevDef)

          XN_DEFINES = $(wcharDefinition) $(pointerIntegerDef)
      SAMPLE_DEFINES = $(wcharDefinition) $(pointerIntegerDef)
     SCRIPTS_DEFINES =

MANUALSED = $(CANNAROOT)/misc/manual.sed

      WORLDOPTS = -k
        SUBDIRS = lib canna server cmd dic misc doc
        INCLUDE = ./include/canna

all:: cannaconf.h

all::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	echo "making" all "in $(CURRENT_DIR)/$$i..."; \
	$(MAKE) -C $$i $(MFLAGS) $(PARALLELMFLAGS)  all; \
	done

depend::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	echo "depending" "in $(CURRENT_DIR)/$$i..."; \
	$(MAKE) -C $$i $(MFLAGS) $(PARALLELMFLAGS)  depend; \
	done

clean::
	$(RM) -r include cannaconf.h

includes::
	$(RM) -r include
	@if [ -d $(INCLUDE) ]; then set +x; \
	else (set -x; $(MKDIRHIER) $(INCLUDE)); fi

cannaconf.h:: Canna.conf
	(echo "/* for cannaserver */";\
	echo "#define DICHOME \"$(DicDir)\"";  \
	echo "#define ERRDIR \"$(ErrDir)\"";   \
	echo "#define USE_UNIX_SOCKET"; \
	echo "#define USE_INET_SOCKET"; \
	echo "/* for lib/RKC */";\
	echo "#define JAPANESE_LOCALE \"$(JapaneseLocale)\"";\
	echo "#define CANNAHOSTFILE \"$(cannaLibDir)/cannahost\"";\
	echo "/* for lib/canna */";\
	echo "#define CANNALIBDIR \"$(cannaLibDir)\""; \
	echo "/* for scripts */";\
	echo "#define CANNABINDIR $(cannaBinDir)"; \
	echo "/* others */";\
	echo "#define CANNA_LIGHT $(cannaLight)"; \
	echo "#define DONT_HAVE_RENAME $(dontHaveRename)"; \
	echo "#ifdef nec"; \
	echo "#undef nec"; \
	echo "#endif") > $@

CANNAROOT = .
SERVERDIR = server dic/phono dic/ideo
CLIENTDIR = cmd dic/phono misc
SGSDIR = canna lib

install:: mkbindir mklibdir mkdicdir mkerrdir
instserver:: mkdicdir mkerrdir
instclient:: mkbindir mkdicdir
instsgs:: mklibdir

mkbindir::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[i]*) set +e;; esac; done; \
	for i in  $(cannaBinDir); do if [ -d $(DESTDIR)$$i ]; then \
	set +x; else (set -x; $(MKDIRHIER) $(DESTDIR)$$i); fi; \
	done

mklibdir::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[i]*) set +e;; esac; done; \
	for i in  $(cannaLibDir); do if [ -d $(DESTDIR)$$i ]; then \
	set +x; else (set -x; $(MKDIRHIER) $(DESTDIR)$$i); fi; \
	done

mkdicdir::
	@case '${MFLAGS}' in *[i]*) set +e;; esac;
	@for i in  $(DicDir); do if [ -d $(DESTDIR)$$i ]; then \
	set +x; else (set -x; $(MKDIRHIER) $(DESTDIR)$$i;$(CHOWN)  $(cannaOwner) $(DESTDIR)$$i;$(CHGRP)  $(cannaGroup) $(DESTDIR)$$i); fi; \
	done

mkerrdir::
	@case '${MFLAGS}' in *[i]*) set +e;; esac;
	@for i in  $(ErrDir); do if [ -d $(DESTDIR)$$i ]; then \
	set +x; else (set -x; $(MKDIRHIER) $(DESTDIR)$$i;$(CHOWN)  $(cannaOwner) $(DESTDIR)$$i;$(CHGRP)  $(cannaGroup) $(DESTDIR)$$i); fi; \
	done

includes:: cannaconf.h
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[i]*) set +e;; esac; done; \
	echo "	cd" $(CANNAROOT)/include; cd $(CANNAROOT)/include && \
	for i in cannaconf.h; do (set -x; $(RM) $$i; $(LN) ../$$i .); done

canna::
	@echo ""
	@echo "Building canna"
	@echo ""
	@echo ""
	$(MAKE) Makefiles
	$(MAKE) clean
	$(MAKE) includes
	$(MAKE) -k depend
	$(MAKE) $(WORLDOPTS)
	@echo ""
	@date
	@echo ""

instserver::
	@case '${MFLAGS}' in *[ik]*) set +e;; esac; 	for i in $(SERVERDIR) ;	do 		(cd $$i ; echo "installing" "in $(CURRENT_DIR)/$$i..."; 		$(MAKE) $(MFLAGS) DESTDIR='$(DESTDIR)' install); 	done

instclient::
	@case '${MFLAGS}' in *[ik]*) set +e;; esac; 	for i in $(CLIENTDIR) ;	do 		(cd $$i ; echo "installing" "in $(CURRENT_DIR)/$$i..."; 		$(MAKE) $(MFLAGS) DESTDIR='$(DESTDIR)' install); 	done

instsgs::
	@case '${MFLAGS}' in *[ik]*) set +e;; esac; 	for i in $(SGSDIR) ;	do 		(cd $$i ; echo "installing" "in $(CURRENT_DIR)/$$i..."; 		$(MAKE) $(MFLAGS) DESTDIR='$(DESTDIR)' install); 	done

# ----------------------------------------------------------------------
# common rules for all Makefiles - do not edit

.c.i:
	$(RM) $@
	 $(CC) -E $(CFLAGS) $(_NOOP_) $*.c > $@

emptyrule::

clean::
	$(RM) *.CKP *.ln *.BAK *.bak *.o core errs ,* *~ *.a .emacs_* tags TAGS make.log MakeOut  "#"*

Makefile::
	-@if [ -f Makefile ]; then set -x; \
	$(RM) Makefile.bak; $(MV) Makefile Makefile.bak; \
	else exit 0; fi
	$(IMAKE_CMD) -DTOPDIR=$(TOP) -DCURDIR=$(CURRENT_DIR)

tags::
	$(TAGS) -w *.[ch]
	$(TAGS) -xw *.[ch] > TAGS

man_keywords::

# ----------------------------------------------------------------------
# rules for building in SUBDIRS - do not edit

install::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	echo "installing" "in $(CURRENT_DIR)/$$i..."; \
	$(MAKE) -C $$i $(MFLAGS) $(PARALLELMFLAGS) DESTDIR=$(DESTDIR) install; \
	done

install.man::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	echo "installing man pages" "in $(CURRENT_DIR)/$$i..."; \
	$(MAKE) -C $$i $(MFLAGS) $(PARALLELMFLAGS) DESTDIR=$(DESTDIR) install.man; \
	done

install.linkkit::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	echo "installing link kit" "in $(CURRENT_DIR)/$$i..."; \
	$(MAKE) -C $$i $(MFLAGS) $(PARALLELMFLAGS) DESTDIR='$(DESTDIR)' install.linkkit; \
	done

clean::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	echo "cleaning" "in $(CURRENT_DIR)/$$i..."; \
	$(MAKE) -C $$i $(MFLAGS) $(PARALLELMFLAGS)  clean; \
	done

tags::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	echo "tagging" "in $(CURRENT_DIR)/$$i..."; \
	$(MAKE) -C $$i $(MFLAGS) $(PARALLELMFLAGS) TAGS='$(TAGS)' tags; \
	done

$(ONESUBDIR)/Makefile:
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[n]*) executeit="no";; esac; done; \
	cd $(ONESUBDIR) && \
	if [ "$$executeit" != "no" ]; then \
	$(IMAKE_CMD) -DTOPDIR=$(IMAKETOP) -DCURDIR=$(ONECURDIR)$(ONESUBDIR); \
	fi;

Makefiles::
	-@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[n]*) executeit="no";; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	case "$(CURRENT_DIR)" in \
	.) curdir= ;; \
	*) curdir=$(CURRENT_DIR)/ ;; \
	esac; \
	echo "making Makefiles in $$curdir$$i..."; \
	itmp=`echo $$i | sed -e 's;^\./;;g' -e 's;/\./;/;g'`; \
	curtmp="$(CURRENT_DIR)" \
	toptmp=""; \
	case "$$itmp" in \
	../?*) \
	while echo "$$itmp" | grep '^\.\./' > /dev/null;\
	do \
	toptmp="/`basename $$curtmp`$$toptmp"; \
	curtmp="`dirname $$curtmp`"; \
	itmp="`echo $$itmp | sed 's;\.\./;;'`"; \
	done \
	;; \
	esac; \
	case "$$itmp" in \
	*/?*/?*/?*/?*)	newtop=../../../../..;; \
	*/?*/?*/?*)	newtop=../../../..;; \
	*/?*/?*)	newtop=../../..;; \
	*/?*)		newtop=../..;; \
	*)		newtop=..;; \
	esac; \
	newtop="$$newtop$$toptmp"; \
	case "$(TOP)" in \
	/?*) imaketop=$(TOP) \
	imakeprefix= ;; \
	.) imaketop=$$newtop \
	imakeprefix=$$newtop/ ;; \
	*) imaketop=$$newtop/$(TOP) \
	imakeprefix=$$newtop/ ;; \
	esac; \
	$(RM) $$i/Makefile.bak; \
	if [ -f $$i/Makefile ]; then \
	echo "	$(MV) Makefile Makefile.bak"; \
	if [ "$$executeit" != "no" ]; then \
	$(MV) $$i/Makefile $$i/Makefile.bak; \
	fi; \
	fi; \
	$(MAKE) $(MFLAGS) $(MAKE_OPTS) ONESUBDIR=$$i ONECURDIR=$$curdir IMAKETOP=$$imaketop IMAKEPREFIX=$$imakeprefix $$i/Makefile; \
	if [ -d $$i ] ; then \
	cd $$i; \
	$(MAKE) $(MFLAGS) Makefiles; \
	cd $$newtop; \
	else \
	exit 1; \
	fi; \
	done

includes::
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
	echo including "in $(CURRENT_DIR)/$$i..."; \
	$(MAKE) -C $$i $(MFLAGS) $(PARALLELMFLAGS)  includes; \
	done

# ----------------------------------------------------------------------
# dependencies generated by makedepend

