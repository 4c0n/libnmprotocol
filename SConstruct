src = 'amidi.c ackmessage.cc activepidlistener.cc alsadriver.cc getpatchlistmessage.cc getpatchmessage.cc iammessage.cc lightmessage.cc metermessage.cc mididriver.cc midimessage.cc netmididriver.cc newpatchinslotmessage.cc nmprotocol.cc parametermessage.cc patchlistmessage.cc patchmessage.cc requestpatchmessage.cc slotactivatedmessage.cc slotsselectedmessage.cc synth.cc voicecountmessage.cc'

inc = 'ackmessage.h activepidlistener.h getpatchlistmessage.h getpatchmessage.h iammessage.h lightmessage.h metermessage.h midiexception.h midimessage.h newpatchinslotmessage.h nmprotocol.h nmprotocollistener.h parametermessage.h patchlistmessage.h patchmessage.h requestpatchmessage.h slotactivatedmessage.h slotsselectedmessage.h synth.h synthlistener.h voicecountmessage.h'

pkg = 'midi.pdl'

import os
withJava = int(ARGUMENTS.get('JAVA', 0)) == 1;
java_home = os.environ.get('JAVA_HOME','')

# check is JAVA compilation is requested and JAVA_HOME is set
if not os.environ.has_key('JAVA_HOME') and (withJava == 1):
    print "compilation with JAVA requested, but JAVA_HOME not set, aborting!"
    Exit(1)

if withJava:
  print 'Compiling with java support (JAVA_HOME = "%s")' % java_home
  src = 'nmprotocol.i %s' % src


nmprotocol = Environment(SWIGFLAGS = '-c++ -java -package nmcom.swig.nmprotocol -outdir ../../java-nmcom/src/nmcom/swig/nmprotocol',
                     LIBPATH = ['../libppf','../libnmpatch', '../libpdl','/lib', '/usr/lib', '/usr/local/lib'],
                         CPPPATH = ['.', '../libpdl', '../libnmpatch',java_home+'/include/'])

opts = Options('nmprotocol.conf')
opts.Add(PathOption('PREFIX',
                    'Install dir (Defult: /usr/local)',
                    '/usr/local'))
opts.Add(BoolOption('JAVA', 'Set to 1 to compile with java support', 0))
opts.Add(EnumOption('RTMIDI','If set, compile using RtMidi and specified OS', 'DISABLED',
    allowed_values = ('MACOSX', 'LINUX', 'IRIX', 'WINDOWS', 'DISABLED')))

opts.Update(nmprotocol)
opts.Save('nmprotocol.conf', nmprotocol)

idir_prefix = '$PREFIX'
idir_lib    = '$PREFIX/lib'
idir_pkg    = '$PREFIX/lib/nmprotocol'
idir_inc    = '$PREFIX/include/nmprotocol'


conf = Configure(nmprotocol)

conf.env.Append(CPPFLAGS='-DLIBPATH=\'"' + idir_pkg + '"\'')

if withJava:
  # java include path
  java_include=java_home+'/include'
  nmprotocol.Append(CPPPATH=[java_include])

  # check for 'jni_md.h'
  for subdirInclude in os.listdir(java_include):
    mdfile=subdirInclude+'/jni_md.h'
    if os.path.isfile(java_include+'/'+mdfile):
      if conf.CheckCHeader(mdfile):
        mdpath=java_include+'/'+subdirInclude
        nmprotocol.Append(CPPPATH=[mdpath])
        break

  # if 'jni_md.h' is not on the path, this will fail
  if not conf.CheckCHeader('jni.h'):
    print 'jni.h not found!'
    Exit(1)

if not ARGUMENTS.has_key('RTMIDI') or ARGUMENTS['RTMIDI'] == "DISABLED":
    if conf.CheckLib('asound'):
       conf.env.Append(CPPFLAGS=' -DALSA')

    if conf.CheckFunc('socket'):
       conf.env.Append(CPPFLAGS=' -DNETMIDI')
else:
    # compile using RtMidi
    src = '%s RtMidi.cpp rtmididriver.cc' % src
    inc = '%s RtMidi.h RtError.h rtmididriver.h' % inc
    conf.env.Append(CPPFLAGS = ' -DRTMIDI')
    if ARGUMENTS['RTMIDI'] == 'MACOSX':
        conf.env.Append(CPPFLAGS = ' -D__MACOSX_CORE__')
        conf.env.Append(LINKFLAGS = ' -framework CoreMidi -framework CoreAudio -framework CoreFoundation')
    elif ARGUMENTS['RTMIDI'] == 'LINUX':
        if not conf.CheckLib('asound'):
            print "libasound missing!"
            Exit(1)
        if not conf.CheckLib('pthread'):
            print "libpthread missing!"
            Exit(1)
        conf.env.Append(CPPFLAGS = ' -D__LINUX_ALSASEQ__')
        conf.env.Append(LIBS = ['asound', 'pthread'])

if not conf.CheckLib('pdl'):
   print "libpdl missing!"
   Exit(1)

if not conf.CheckLib('ppf'):
   print "libnmpatch missing!"
   Exit(1)

if not conf.CheckLib('nmpatch'):
   print "libnmpatch missing!"
   Exit(1)

nmprotocol = conf.Finish()

srcfiles = [('src/' + x)        for x in Split(src)]
incfiles = [('nmprotocol/' + x) for x in Split(inc)]
pkgfiles = [('src/' + x)        for x in Split(pkg)]

lib = nmprotocol.SharedLibrary('nmprotocol', srcfiles)

nmprotocol.Install(idir_lib, lib)
nmprotocol.Install(idir_inc, incfiles)
nmprotocol.Install(idir_pkg, pkgfiles)
nmprotocol.Alias('install', idir_prefix)

