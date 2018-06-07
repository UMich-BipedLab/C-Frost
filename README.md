# C-Frost

C-Frost is a toolbox that helps convert [Frost](https://github.com/ayonga/frost-dev) in native C++ [Ipopt](https://projects.coin-or.org/Ipopt) problems.
It has been noticed that running on native C++ significantly improves running speeds compared to running the mexed Ipopt version provided by Frost.

Frost has been tested on native Ubuntu 16.04 and on Ubuntu 16.04 running on [WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10).

## Installation

C-Frost needs [Ipopt](https://projects.coin-or.org/Ipopt) to be installed on your system to run. You can follow the [official documentation](https://www.coin-or.org/Ipopt/documentation/node2.html), but we recommend you follow the steps we've used [here](INSTALLATION.md).

## Descriptions

[Linux/](Linux) contains source files to be used when building your Frost problem. For typical cases you won't need to change anything in that folder.

[Matlab/](Matlab) contains Matlab code used to create C++ files from your Frost problem on Matlab.

[Examples/](Examples) contains some examples including explanation and a small walkthrough of the code.

[ThirdPartyLicense/](ThirdPartyLicense) contains licenses and disclamers from third party code used.

## Tutorial

Once done installing all the prerequisits, we recommend going over [here](Examples/README.md) to go through the examples and how to use this tool.
