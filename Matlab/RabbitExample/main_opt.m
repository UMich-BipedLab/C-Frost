%% Setup
clear; clc;
cur = pwd;
addpath(genpath(cur));

addpath('C:\Users\oharib\Documents\GitHub\frost-dev');
frost_addpath;
export_path = fullfile(cur, 'gen/');
addpath('../');
% if load_path is empty, it will not load any expression.
% if non-empty and assigned valid directory, then symbolic expressions will
% be loaded  from the MX binary files from the given directory.
load_path = [];%fullfile(cur, 'gen/sym');
delay_set = false;
COMPILE = true;
GENERATE_C = true;

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
    if ~exist([export_path, 'opt/'])
        mkdir([export_path, 'opt/'])
    end
    rabbit.ExportKinematics([export_path,'kinematics/']);
    compileConstraint(nlp,[],[],[export_path, 'opt/']);
    compileObjective(nlp,[],[],[export_path, 'opt/']);
end

% Example constraint removal
% removeConstraint(nlp.Phase(1),'u_friction_cone_RightToe');

%% Create Ipopt solver 
addpath(genpath(export_path));
nlp.update;
solver = IpoptApplication(nlp);

%% Create files depending on solver
if GENERATE_C
    c_code_path = 'c_code';
    src_path = 'c_code/src';
    src_gen_path = 'c_code/src/gen';
    include_dir = 'c_code/include';
    data_path = 'c_code/res';
    
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
    if exist(data_path, 'dir')
        mkdir(data_path);
    end
    
    [funcs] = frost_c.getAllFuncs(solver);
    frost_c.createFunctionListHeader(funcs, src_path, include_dir);
    frost_c.createIncludeHeader(funcs, include_dir);
    if COMPILE
        frost_c.createConstraints(nlp,[],[],src_gen_path, include_dir)
        frost_c.createObjectives(nlp,[],[],src_gen_path, include_dir)
    end
    frost_c.createDataFile(solver, funcs, data_path);
    frost_c.createInitialGuess(solver, data_path);
    
    copyfile('CMakeLists.txt', 'c_code/CMakeLists.txt');
    copyfile('default-ipopt.opt', 'c_code/ipopt.opt');
end

%% Run Optimization
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
