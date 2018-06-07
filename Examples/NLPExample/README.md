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

Anything prior to the line,
```
solver = IpoptApplication(nlp, extraOpts);
```
including the line itself, is the same as when using the typical Frost.

The section `(C-Frost specific code) Create c ipopt problem` includes the unique commands needed to be called when wanting to run the Ipopt problem natively on C++.

Here is a description of some important lines:
___
```
[funcs] = frost_c.getAllFuncs(solver);
frost_c.createFunctionListHeader(funcs, src_path, include_dir);
frost_c.createIncludeHeader(funcs, include_dir);
```
These three lines do the following,
`[funcs] = frost_c.getAllFuncs(solver);` returns the names of all the important mathematical expressions/functions that will be generated.
Then, `frost_c.createFunctionListHeader(funcs, src_path, include_dir);` and `frost_c.createIncludeHeader(funcs, include_dir);` generate some necessary C files that are dependent on the content of `funcs` (both the names of the functions and the order of the elements in the list).
Typically, if a new constraint/function is created, or if the index of an element in the list changes, run all three lines of code.
Then for convinence, save the content of `funcs` (just to make sure you're always using the same order of elements).
___
```
frost_c.createConstraints(nlp,[],[],src_gen_path, include_dir);
frost_c.createObjectives(nlp,[],[],src_gen_path, include_dir);
```
These two lines generate the actual mathematical expressions of all the constraints as C code.
Their usage is identical to:
```
nlp.compileConstraint(export_path);
nlp.compileObjective(export_path);
```
Except you have to provide two paths instead of one. Note that you can also ignore some functions or generate specific functions.
___
```
frost_c.createDataFile(solver, funcs, res_path, 'data');
frost_c.createBoundsFile(solver, funcs, res_path, 'bounds');
frost_c.createInitialGuess(solver, res_path);
```
These three lines generate three json files: a data file, a bounds file, and an init file.
- `data.json` contain information about all the constraint/objective functions. This includes the indices of the dependant varaibles and the list of functions (including which functions from the `funcs` list each constraint/objective must call).
- `bounds.json` contain the bounds for each variable and constraint. In addition, the file includes all Auxilary Data used in the problem.
- `init.json` is simply a vector containing the initial condition.

Generally, if only the bounds have changed, you only need new bounds files.
If a constraint expression was modified (or if you removed a constraint), you need to generate new a new data file (and a new bounds file).
If new constraints were created (or some were renamed), you'll need to run:
```
[funcs] = frost_c.getAllFuncs(solver);
frost_c.createFunctionListHeader(funcs, src_path, include_dir);
frost_c.createIncludeHeader(funcs, include_dir);
```
then generate a new data and bounds file.
