# NLP Example

## Initial setup

Before starting the example, you'll have to setup some variables.

In the file `create_problem.m`, set the value of `FROST_PATH` to be the path to Frost installation.

In the file `CMakeLists.txt`, set the value of `C_FROST_ROOT_PATH` to be the path to C-Frost root folder.
*Note that if you're using WSL the path would look like `/mnt/c/...`.*

## Running the example

- Run `create_problem.m`.
- Afterwards you'll see a new folder created called `c_code`.
  This folder will contain the c++ Ipopt code.
- In a bash directory, go to the folder `c_code`.
- Run the following:
  ```
  mkdir build
  cd build
  cmake ..
  make
  make install
  cd ..
  ./program --initial 'res/init.json' --options '../ipopt.opt' --data 'res/data.json' --bounds 'res/bounds.json' --output '../output.json'
  ```
- You'll see a file called `output.json` created in the main example folder.
- In matlab, run `sol = loadjson('output.json');`. Now you have the solution from the problem in the variable `sol`.

## Explanation of the code

*TO BE DONE*
