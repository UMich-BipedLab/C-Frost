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

## *OPTIONAL*: Install Intel Parallel Studio XE for Linux

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
- Either restart the bash or run `sudo ~/.profile` to continue.

## Install Ipopt
