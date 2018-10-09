#!/bin/sh

# Set up AutoTools directory structure.
# Pretty blunt - just deletes whatever is there and starts over

Dev=Development
Rel=Release

curdir=`pwd`

if test -d $HOME/$Rel/Inodeum
then
    rm -rf $HOME/$Rel/Inodeum
fi

mkdir -p $HOME/$Rel/Inodeum
cd $HOME/$Rel/Inodeum
mkdir src
mkdir -p data/icons

ln -s $HOME/$Dev/Inodeum/README .
ln -s $HOME/$Dev/Inodeum/COPYING .
ln -s $HOME/$Dev/Inodeum/AutoTools/AUTHORS .
ln -s $HOME/$Dev/Inodeum/AutoTools/ChangeLog .
ln -s $HOME/$Dev/Inodeum/AutoTools/INSTALL .
ln -s $HOME/$Dev/Inodeum/AutoTools/NEWS .
ln -s $HOME/$Dev/Inodeum/AutoTools/configure.ac .
ln -s $HOME/$Dev/Inodeum/AutoTools/Makefile.am .

cd src
ln -s $HOME/$Dev/Inodeum/AutoTools/src/Makefile.am .
cp -p $HOME/$Dev/Inodeum/src/*.c .
cp -p $HOME/$Dev/Inodeum/src/*.h .

cd ../data
ln -s $HOME/$Dev/Inodeum/AutoTools/data/inodeum.1 .
ln -s $HOME/$Dev/Inodeum/AutoTools/data/inodeum.desktop .
ln -s $HOME/$Dev/Inodeum/AutoTools/data/Makefile.am .

cd icons
ln -s $HOME/$Dev/Inodeum/AutoTools/data/icons/inodeum.png .
ln -s $HOME/$Dev/Inodeum/AutoTools/data/icons/Makefile.am .

cd ../..

autoreconf -i

cd $curdir

exit 0
