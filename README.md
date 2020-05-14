# PyCAD
Experiments in making a CAD software on the Raspberry Pi, using Python, with wxPython boost-python, OpenGL and my geometry derived from HeeksCNC project.\
This will involve building a CAD python module and a GEOM python module\
The plan is that all the wx will be done with wxPython scripts.\
The gl canvas will be created by wxPython.\
The gl commands will be done in the c++ by my CAD module.\
Cad object classes will be defined in Python, derived from a cad object base class defined in the cad module.\
Cad objects will implement GetProperties, GetProperty, SetProperty, GetLines, GetTriangles, GetName, GetIcon, etc.

## How to build ##
Start with a clean Raspbian installation

### build wxPython ###
https://wiki.wxpython.org/BuildWxPythonOnRaspberryPi\
or if you have already built it copied it's whl to your Windows laptop ( this is for me )\
on you laptop open a command line\
pscp c:\tmp\wxPython-4.1.0-cp37-cp37m-linux_armv7l.whl pi@192.168.3.110\
( type in password )\
on pi\
pip3 install wxPython-4.1.0-cp37-cp37m-linux_armv7l.whl\

### Get essentials ###
```
sudo apt-get install git build-essential debhelper cmake python3-dev freeglut3-dev
```

### Build boost python for python3 ###
download the latest boost\
unzip\
cd into boost folder\
./bootstrap.sh --with-python-version=3.7\
sudo nano project-config.jam
change
libraries = ;
to
libraries = --with-python ;

./b2\
sudo ./b2 install\
cd ../\


### Fetch sources ###
```
git clone https://github.com/danheeks/PyCAD.git
```

### build CAD python module ###
cd PyCAD/CAD\
mkdir build\
cd build\
cmake ..\
make\
cp cad.so ../../\

### build Geom python module ###
cd ../../Geom\
mkdir build\
cd build\
cmake ..\
make\
cp geom.so ../../\

### try the test.py ###
cd ../../\
python test.py

### Development Plan ###
To use Property Grid on the raspberry pi, I need to start using Python3, so I need to get boost-python working on the raspberry pi, probably need to build boost python\

The plan is to get back to the same functionality as HeeksCNC 1.3, but working from python 3, using PyCAD.
I need to think through the mechanism of adding new functionality to PyCAD.
It doesn't need to be automatic. The idea is for it to be hackable, so that you just download an add-on to PyCAD, then hack the UI in Frame.py, I guess.
