%% Setting up example options
cur = fileparts(mfilename('fullpath'));

COMPILE = true;
GENERATE_C = true;
RUN_MATLAB_FROST = false;
FROST_PATH = 'INSERT_PATH_HERE';
C_FROST_PATH = fullfile(cur, '..', '..', 'Matlab');

%% Setting up paths
if strcmp(FROST_PATH, 'INSERT_PATH_HERE')
    error('Please set the path to Frost in the variable FROST_PATH');
end

addpath(FROST_PATH);
frost_addpath;

addpath(C_FROST_PATH);

export_path = fullfile(cur, 'export/');
if ~exist(export_path, 'dir')
    mkdir(export_path);
end
addpath(export_path);

%% Creating Problem
nlp = NonlinearProgram('nlp051');
nlp.setOption('DerivativeLevel',1);

x_s = SymVariable('x',[3,1]);
y_s = SymVariable('y',[2,1]);
x_var = NlpVariable('Name', 'x', 'Dimension', 3, ...
    'lb', -inf, 'ub', inf, 'x0', [2.5 0.5 2]);
y_var = NlpVariable('Name', 'y', 'Dimension', 2, ...
    'lb', -inf, 'ub', inf, 'x0', [-1 0.5]);
nlp = regVariable(nlp,x_var);
nlp = regVariable(nlp,y_var);

nlp = update(nlp);

f1 = (x_s(1)-x_s(2)).^2;
f2 = (x_s(2) + x_s(3) - 2).^2;
f3 = (y_s(1)-1).^2 + (y_s(2)-1).^2;

f_cost1 = SymFunction('f_cost1',f1,{x_s});
f_cost2 = SymFunction('f_cost2',f2,{x_s});
f_cost3 = SymFunction('f_cost3',f3,{y_s});

costs(1) = NlpFunction('Name', 'f_cost1',...
    'SymFun',f_cost1, 'Type', 'Nonlinear',...
    'DepVariables',x_var);
costs(2) = NlpFunction('Name', 'f_cost21',...
    'SymFun',f_cost2, 'Type', 'Nonlinear',...
    'DepVariables',x_var);

costs(3) = NlpFunction('Name', 'f_cost3',...
    'SymFun',f_cost3, 'Type', 'Nonlinear',...
    'DepVariables',y_var);

nlp = regObjective(nlp,costs);


c1 = x_s(1) + 3*x_s(2);
c2 = x_s(3) + y_s(1) - 2*y_s(2);
c3 = x_s(2) - y_s(2);

f_constr1 = SymFunction('f_constr1',c1,{x_s});
f_constr2 = SymFunction('f_constr2',c2,{x_s,y_s});
f_constr3 = SymFunction('f_constr3',c3,{x_s,y_s});

constrs(1) = NlpFunction('Name','f_constr1','Type','nonlinear',...
    'SymFun',f_constr1,'DepVariables',x_var,'lb',4,'ub',4);

constrs(2) = NlpFunction('Name','f_constr2','Type','nonlinear',...
    'SymFun',f_constr2,'DepVariables',[x_var;y_var],'lb',0,'ub',0);

constrs(3) = NlpFunction('Name','f_constr3','Type','nonlinear',...
    'SymFun',f_constr3,'DepVariables',[x_var;y_var],'lb',0,'ub',0);


nlp = regConstraint(nlp,constrs);

%% Compiling Constraints
nlp.update;

if COMPILE
    nlp.compileConstraint(export_path);
    nlp.compileObjective(export_path);
end

%% Createing the Solver Problem

extraOpts.fixed_variable_treatment = 'RELAX_BOUNDS';
extraOpts.point_perturbation_radius = 0;
extraOpts.jac_c_constant        = 'yes';
extraOpts.hessian_approximation = 'limited-memory';
extraOpts.derivative_test = 'first-order';
extraOpts.derivative_test_perturbation = 1e-7;

solver = IpoptApplication(nlp, extraOpts);

%% Create files depending on solver
if GENERATE_C
    c_code_path = 'c_code';
    src_path = 'c_code/src';
    src_gen_path = 'c_code/src/gen';
    include_dir = 'c_code/include';
    res_path = 'c_code/res';
    
    if exist(c_code_path, 'dir')
        mkdir(c_code_path);
    end
    if exist(src_path, 'dir')
        mkdir(src_path);
    end
    if exist(src_gen_path, 'dir')
        mkdir(src_gen_path);
    end
    if exist(include_dir, 'dir')
        mkdir(include_dir);
    end
    if exist(res_path, 'dir')
        mkdir(res_path);
    end
    
    if COMPILE
        [funcs] = frost_c.getAllFuncs(solver);
        frost_c.createFunctionListHeader(funcs, src_path, include_dir);
        frost_c.createIncludeHeader(funcs, include_dir);
        
        frost_c.createConstraints(nlp,[],[],src_gen_path, include_dir);
        frost_c.createObjectives(nlp,[],[],src_gen_path, include_dir);
        
        save('function_list.mat', 'funcs');
    end
    load('function_list.mat', 'funcs');
    frost_c.createDataFile(solver, funcs, res_path, 'data');
    frost_c.createBoundsFile(solver, funcs, res_path, 'bounds');
    frost_c.createInitialGuess(solver, res_path);
    
    copyfile('CMakeLists.txt', 'c_code/CMakeLists.txt');
end

%% Optimize
if RUN_MATLAB_FROST
    [sol, info] = optimize(solver);
end
