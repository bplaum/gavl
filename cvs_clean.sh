#!/bin/sh

CVSCLEANFILES="config.guess config.status config.sub configure gavl.spec gavl.pc libtool ltmain.sh depcomp install-sh missing  mkinstalldirs"

SUBDIRS="src gavl gavl/c gavl/mmx gavl/mmxext include"

make distclean

echo "Cleaning up..."
for i in $CVSCLEANFILES; do
echo "Removing $i"
rm -rf $i
done
echo Removing autom4te*
rm -rf autom4te*
for i in $SUBDIRS; do
echo Removing $i/Makefile
rm -f $i/Makefile 
echo Removing $i/Makefile.in
rm -f $i/Makefile.in
echo Removing $i/.libs $i/.deps
rm -rf $i/.libs $i/.deps
done
echo "You can now rerun autogen.sh"
