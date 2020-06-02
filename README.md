## Build for Kognition

You need to create a symlink path/to/libetpan/kog_imap_lcb_code that points to
the folder /path/to/kognition/repo/extensions/io.kognition.imap/code/x86_64-mac

This ensures that after the dynamic lib is compiled, it is directly copied into
the code section of the LCB library.

Note if the folder cannot be found, the build will fail (for Mac builds at
the moment, at least).

Instructions on how to build can be found further down in the
**Build Instructions**

## LibEtPan

The purpose of this mail library is to provide a portable, efficient framework for different kinds of mail access: IMAP, SMTP, POP and NNTP.

It provides an API for C language.

[![Build Status](https://travis-ci.org/dinhviethoa/libetpan.png?branch=master)](https://travis-ci.org/dinhviethoa/libetpan)
[![Code Quality: Cpp](https://img.shields.io/lgtm/grade/cpp/g/dinhviethoa/libetpan.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/dinhviethoa/libetpan/context:cpp)
[![Total Alerts](https://img.shields.io/lgtm/alerts/g/dinhviethoa/libetpan.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/dinhviethoa/libetpan/alerts)

## Features

- IMAP
- SMTP
- POP
- NNTP
- RFC822/MIME message builder
- RFC822/MIME message parser
- Maildir
- mbox
- MH

## Build instructions

### Unix

You need to install autoconf, automake and libtool.
They can be installed using [brew](http://brew.sh/).

    $ ./autogen.sh
    $ make

You can use flag --with-poll for using poll() instead of select() for checking connection status

### How to link with it

    $ gcc -c -o sample.o sample.c `pkg-config libetpan --cflags`
    $ gcc -o sample sample.o `pkg-config libetpan --libs`

### Mac / iOS

- Download Xcode
- Open `build-mac/libetpan.xcodeproj`
- Choose the correct target "static libetpan" for Mac or "libetpan ios" for iOS.
- Build either the target _Dynamic libetpan_ or _test_: either will build the dylib

The target _test_ will run the script _tests/imap-wraper.c_, which is intended
to check that segfaults and memory leaks are not happening.

### Setup a Mac project

- Add `libetpan.xcodeproj` as sub-project
- Link with libetpan.a

### Setup an iOS project

- Add `libetpan.xcodeproj` as sub-project
- Link with libetpan-ios.a
- Set "Other Linker Flags": `-lsasl2`

### Build on Windows

- See README and Visual Studio Solution in build-windows folder

## More information

See http://etpan.org/libetpan.html for more information and examples.
