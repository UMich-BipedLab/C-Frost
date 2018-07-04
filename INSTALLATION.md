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

## *(OPTIONAL)* Install Intel Math Kernal Library for Linux

Using Intel MKL results in better performance.
Instructions on installing it is [here](https://software.intel.com/en-us/articles/installing-intel-free-libs-and-python-apt-repo).

Once you're done with the _Setting up the Repository_ steps, install the `intel-mkl` package.

Then, once done installing, in `~/.profile`, add the line (adjusting to to whatever version of the toolset you're using):
```
source /opt/intel/parallel_studio_xe_2018.3.051/psxevars.sh intel64 -platform linux
```

Finally either restart or run `source ~/.profile` to continue.

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
- *(OPTIONAL)* Next, aquire [HSL](http://www.hsl.rl.ac.uk/ipopt/).
  - Download the source code for the Linux version.
  - Extract the folder
  - Rename the folder to `coinhsl/` and place such that it looks like `ThirdParty/HSL/coinhsl/`
  - _HSL is used in a particular linear solver that we're found to work well. If you cannot get HSL, you'll have to change the solver in our examples._
- Next, in the root folder of the Ipopt run:
  ```
  mkdir build
  cd build
  ```
- If you're using the Intel compiler, run:
  ```
  ../configure --prefix=/usr/local/ ADD_CFLAGS=-fopenmp ADD_FFLAGS=-fopenmp ADD_CXXFLAGS=-fopenmp --with-blas="-L$MKLROOT/lib/intel64 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lm"
  ```
- Otherwise, run:
  ```
  ../configure
  ```
- Then run `make` or `make -j4` for 4 threads (or however many you want).
- Then, run `sudo make install`
- Then, run `sudo ldconfig`

## Install JSONlab

Go [here](https://www.mathworks.com/matlabcentral/fileexchange/33381-jsonlab--a-toolbox-to-encode-decode-json-files) to isntall JSONlab on for Matlab.
