# Installation Instructions

## Enabling WSL on Windows 10

(Skip this step if you're on Ubuntu).

Run on Powershell with administrator rights:
```
Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux
```
Then restart when prompted.

## Required Packages

Install the following packages:
```
sudo apt-get install gcc g++ gfortran subversion patch wget cmake build-essential
```

## *(OPTIONAL)* Install Intel Parallel Studio XE for Linux

We've noticed significant speed improvements when using this framework to build Ipopt.

You can obtain the tools [here](https://software.intel.com/en-us/parallel-studio-xe). Then:

- Download the full package. 
- If on WSL, copy the compressed file to your linux home directory `~/`.
- Extract the file.
- Browse into the extracted folder and run `sudo install.sh`.
  - On WSL, when selecting the pachages to install, remove any package that includes "Graphical User Interface".
  - For all other options, use default ones.
- Once done installing, run `sudo ldconfig`.
- In `~/.profile`, add the line (adjusting to to whatever version of the toolset you're using):
  ```
  source /opt/intel/parallel_studio_xe_2018.3.051/psxevars.sh intel64 -platform linux
  ```
- Either restart the bash or run `source ~/.profile` to continue.

## Install Ipopt

- We'll download Ipopt:
  ```
  svn co https://projects.coin-or.org/svn/Ipopt/stable/3.12 CoinIpopt
  ```
- Next, we'll download third party licenses. From within the downloaded folder, run:
  ```
  cd ThirdParty/ASL
  ./get.ASL
  cd ../Blas
  ./get.Blas
  cd ../Lapack
  ./get.Lapack
  cd ../Metis
  ./get.Metis
  cd ../Mumps
  ./get.Mumps
  cd ../../
  ```
- Next, aquire [HSL](http://www.hsl.rl.ac.uk/ipopt/).
  - Download the source code for the Linux version.
  - Extract the folder
  - Rename the folder to `coinhsl/` and place it in `ThirdParty/HSL/coinhsl/`
- Next, in the root folder of the Ipopt run:
  ```
  mkdir build
  cd build
  ```
- If you're using the Intel compiler, run:
  ```
  ../configure CXX=icpc CC=icc F77=ifort --prefix=/usr/local/ ADD_CFLAGS=-fopenmp ADD_FFLAGS=-fopenmp ADD_CXXFLAGS=-fopenmp --with-blas="-L$MKLROOT/lib/intel64 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lm"
  ```
- Otherwise, run:
  ```
  ../configure
  ```
- Then run `make` or `make -j4` for 4 threads (or however many you want).
- Then, run `sudo env "PATH=$PATH" make install`
- Then run `sudo ldconfig`

## Install JSONlab

Go [here](https://www.mathworks.com/matlabcentral/fileexchange/33381-jsonlab--a-toolbox-to-encode-decode-json-files) to isntall JSONlab on for Matlab.
