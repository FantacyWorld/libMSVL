
# libMSVL
A library for the MSVL compiler to support  parallel statements

[install]  
	$ mkdir build && cd build  
	$ cmake /source/root/directory  
	$ cmake --build .  
	$ cmake --build . --target install # defaut install directory: /usr/local  
	
It is possible to set a different install prefix at installation time by invoking the cmake_install.cmake script generated in the build directory:  
	cmake -DCMAKE_INSTALL_PREFIX=/specifyied/directory -P cmake_install.cmake

