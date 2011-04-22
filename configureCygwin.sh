#Configure the libraries
devroot=/cygdrive/c/dev
mkdir $devroot
cd $devroot/
cd ..
mkdir compile
cd wireshark-win32-libs-1.4
cp -r gtk2/* ../dev
cd $devroot/include
mv glib-2.0/* ./
rmdir glib-2.0
#Install libiconv
cd $devroot
cd ..
cd compile
libiconv=iconv-1.9.2.win32
wget http://xmlsoft.org/sources/win32/$libiconv.zip
unzip $libiconv.zip
cp -r $libiconv/* ../dev
unset libiconv
#Get libxml2
cd $devroot
cd ..
cd compile
libxml=libxml2-2.7.6
#Note: We've had problems with newer versions.
wget ftp://xmlsoft.org/libxml2/$libxml.tar.gz
tar xf $libxml.tar.gz
unset libxml
