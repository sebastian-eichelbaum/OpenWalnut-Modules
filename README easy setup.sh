# First navigate Dolphin to this file. Click the left button on no file/folder 
# but the white surface and then on Actions and run terminal here.
# Then type this to execute the following script: sh *easy*
# And read the content because some interaction is necessary.

sudo pkill -9 openwalnut
rm -r -f ../OpenWalnut

#Installing requirements:
sudo apt-get update
sudo apt-get purge virtualbox-ose
sudo apt-get autoremove -y
sudo apt-get install -y cmake libboost-all-dev libqt4-dev libqtwebkit-dev openscenegraph libopenscenegraph-dev libeigen3-dev libnifti-dev libbiosig-dev build-essential openjdk-7-jre

# Linux Mint required this:
sudo apt-get install -y libqt4-opengl-dev

#Repository support:
sudo apt-get install -y mercurial

#For Doxygen-documentation:
sudo apt-get install -y libxdot4 graphviz doxygen


# Getting and building OpenWalnut:
cd ..
ls -l
hg clone http://source.openwalnut.org OpenWalnut
cd OpenWalnut
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../src
make -j4

cd ../..
cd OpenWalnut-modules
export OPENWALNUT_LIBDIR="./../OpenWalnut/build/lib/"
export OPENWALNUT_INCLUDEDIR="./../OpenWalnut/src/"
cd LiDARToolbox/build
cmake ../src
make -j8


echo 
echo 
echo 
echo 
echo 
echo Now go to the directory OpenWalnut/build/bin and launch openwalnut-qt4
echo How to create an easy shortcut:
echo Create a file with the ending ".sh" and mark it as executable in the 
echo file properties. Edit it and add following lines to it:
echo cd '/full/path/to/the/openwalnut-qt4/file'
echo # You can encapsullate paths using the ' sign in order not to escape space signs
echo ./openwalnut-qt4