%% Setup
clear; clc;
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

warning('off', 'MATLAB:MKDIR:DirectoryExists');

addpath(FROST_PATH);
frost_addpath;
addpath(C_FROST_PATH);

export_path = fullfile(cur, 'gen');
mkdir(export_path);


addpath(fullfile(cur, 'domains'));
addpath(fullfile(cur, 'utils'));

% if load_path is empty, it will not load any expression.
% if non-empty and assigned valid directory, then symbolic expressions will
% be loaded  from the MX binary files from the given directory.
load_path = []; %fullfile(cur, 'gen/sym');
delay_set = false;

% Load model
rabbit = RABBIT('urdf/five_link_walker.urdf');
if isempty(load_path)
    rabbit.configureDynamics('DelayCoriolisSet',delay_set);
else
    % load symbolic expression for the dynamics equations
    rabbit.loadDynamics(load_path, delay_set);
end


% Define domains
r_stance = RightStance(rabbit, load_path);
% l_stance = LeftStance(rabbit, load_path);
r_impact = RightImpact(r_stance, load_path);
% l_impact = LeftImpact(l_stance, load_path);

% Define hybrid system
rabbit_1step = HybridSystem('Rabbit_1step');
rabbit_1step = addVertex(rabbit_1step, 'RightStance', 'Domain', r_stance);

srcs = {'RightStance'};
tars = {'RightStance'};

rabbit_1step = addEdge(rabbit_1step, srcs, tars);
rabbit_1step = setEdgeProperties(rabbit_1step, srcs, tars, ...
    'Guard', {r_impact});

%% Define User Constraints
r_stance.UserNlpConstraint = str2func('right_stance_constraints');
r_impact.UserNlpConstraint = str2func('right_impact_constraints');

%% Define User Costs
u = r_stance.Inputs.Control.u;
u2r = tovector(norm(u).^2);
u2r_fun = SymFunction(['torque_' r_stance.Name],u2r,{u});

%% Create optimization problem
num_grid.RightStance = 10;
num_grid.LeftStance = 10;
nlp = HybridTrajectoryOptimization('Rabbit_1step',rabbit_1step,num_grid,[],'EqualityConstraintBoundary',1e-4);

% Configure bounds 
setBounds;

% load some optimization related expressions here
if ~isempty(load_path)
    nlp.configure(bounds, 'LoadPath',load_path);
else
    nlp.configure(bounds);
end

 
% Add costs
addRunningCost(nlp.Phase(getPhaseIndex(nlp,'RightStance')),u2r_fun,'u');

% Update
nlp.update;


% save expressions after you run the optimization. It will save all required
% expressions
% do not need to save expressions if the model configuration is not
% changed. Adding custom constraints does not require saving any
% expressions.
% load_path = fullfile(cur, 'gen/sym');
% rabbit_1step.saveExpression(load_path);
%% Compile
if COMPILE
    mkdir(fullfile(export_path, 'opt'));
    rabbit.ExportKinematics(fullfile(export_path, 'kinematics'));
    compileConstraint(nlp,[],[],fullfile(export_path, 'opt'));
    compileObjective(nlp,[],[],fullfile(export_path, 'opt'));
end

% Example constraint removal
% removeConstraint(nlp.Phase(1),'u_friction_cone_RightToe');

%% Create Ipopt solver 
addpath(genpath(export_path));
nlp.update;
solver = IpoptApplication(nlp);

%% (C-Frost specific code) Create c ipopt problem
if GENERATE_C
    c_code_path = 'c_code';
    src_path = 'c_code/src';
    src_gen_path = 'c_code/src/gen';
    include_dir = 'c_code/include';
    res_path = 'c_code/res';
    
    mkdir(c_code_path);
    mkdir(src_path);
    mkdir(src_gen_path);
    mkdir(include_dir);
    mkdir(res_path);
    
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

%% Run Optimization
if RUN_MATLAB_FROST
    tic
    % old = load('x0');
    % [sol, info] = optimize(solver, old.sol);
    [sol, info] = optimize(solver);
    toc

    [tspan, states, inputs, params] = exportSolution(nlp, sol);
    gait = struct(...
        'tspan',tspan,...
        'states',states,...
        'inputs',inputs,...
        'params',params);
end
