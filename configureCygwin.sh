#Configure the libraries
devroot=/cygdrive/c/dev
mkdir $devroot
cd $devroot/
cd ..
mkdir compile
if [ -f $devroot/wireshark-win32-libs-1.4 ]
then
  cd wireshark-win32-libs-1.4
  if [ -f $devroot/wireshark-win32-libs-1.4/gtk2 ]
  then
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
  else
    echo "gtk2 not found in wireshark-win32-libs-1.4"
    echo "Make sure you compiled wireshark."
  fi
else
  echo "wireshark-win32-libs-1.4 does not exist."
  echo "Make sure you compiled wireshark."
  echo "You may have to change the version number in this file."
fi
