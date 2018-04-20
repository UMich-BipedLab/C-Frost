%% Setup
clear; clc;
cur = pwd;
addpath(genpath(cur));

addpath('/home/ayonga/Projects/frost');
frost_addpath;
export_path = fullfile(cur, 'gen/');
% if load_path is empty, it will not load any expression.
% if non-empty and assigned valid directory, then symbolic expressions will
% be loaded  from the MX binary files from the given directory.
load_path = [];%fullfile(cur, 'gen/sym');
delay_set = false;
COMPILE = true;

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
    if ~exist([export_path, 'c/'])
        mkdir([export_path, 'c/'])
    end
    if ~exist([export_path, 'c/include/'])
        mkdir([export_path, 'c/include/'])
    end
    if ~exist([export_path, 'c/include/frost/'])
        mkdir([export_path, 'c/include/frost/'])
    end
    if ~exist([export_path, 'c/include/frost/gen/'])
        mkdir([export_path, 'c/include/frost/gen/'])
    end
    rabbit.ExportKinematics([export_path,'kinematics/']);
    compileConstraint(nlp,[],[],[export_path, 'opt/']);
    compileObjective(nlp,[],[],[export_path, 'opt/']);
    compileConstraint(nlp,[],[],[export_path, 'c/'], [],...
        'ForceExport', true,...
        'StackVariable', true,...
        'BuildMex', false,...
        'TemplateFile', fullfile(frost_c.getRootPath, 'templateMin.c'),...
        'TemplateHeader', fullfile(frost_c.getRootPath, 'templateMin.h'));
    compileObjective(nlp,[],[],[export_path, 'c/'], [],...
        'ForceExport', true,...
        'StackVariable', true,...
        'BuildMex', false,...
        'TemplateFile', fullfile(frost_c.getRootPath, 'templateMin.c'),...
        'TemplateHeader', fullfile(frost_c.getRootPath, 'templateMin.h'));
    movefile([export_path, 'c/*hh'], [export_path, 'c/include/frost/gen/']);
end

% Example constraint removal
% removeConstraint(nlp.Phase(1),'u_friction_cone_RightToe');

%% Create Ipopt solver 
addpath(genpath(export_path));
nlp.update;
solver = IpoptApplication(nlp);

%% Create files depending on solver
if ~exist(fullfile(frost_c.getRootPath, 'local'))
    mkdir(fullfile(frost_c.getRootPath, 'local'));
end
if ~exist(fullfile(frost_c.getRootPath, 'local', 'code'))
    mkdir(fullfile(frost_c.getRootPath, 'local', 'code'));
end

[funcs, nums] = frost_c.getAllFuncs(solver);
frost_c.createFunctionListHeader(funcs, nums);
frost_c.createIncludeHeader(funcs, nums);
frost_c.createJSONFile(solver, funcs, nums);

x0 = getInitialGuess(solver.Nlp, solver.Options.initialguess);
if iscolumn(x0)
    x0 = x0';
end
savejson('', x0, fullfile('local', 'code', 'init_condition.json'))

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
