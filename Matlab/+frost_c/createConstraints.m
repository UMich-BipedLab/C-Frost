function createConstraints(nlp, phase, constr, src_path, include_dir, exclude, varargin)
    if ~exist('exclude', 'var')
        exclude = [];
    end
    
    if ~exist(src_path, 'dir')
        mkdir(src_path)
    end
    if ~exist(fullfile(include_dir), 'dir')
        mkdir(fullfile(include_dir))
    end
    if ~exist(fullfile(include_dir, 'frost'), 'dir')
        mkdir(fullfile(include_dir, 'frost'))
    end
    if ~exist(fullfile(include_dir, 'frost', 'gen'), 'dir')
        mkdir(fullfile(include_dir, 'frost', 'gen'))
    end
    
    local_path = fullfile(frost_c.getRootPath(), 'local');
    if ~exist(local_path, 'dir')
        mkdir(local_path);
    else
        delete(fullfile(local_path, '*'));
    end
    local_temp_path = fullfile(local_path, sprintf('%.0f', now*100000000000));
    if ~exist(local_temp_path, 'dir')
        mkdir(local_temp_path);
    else
        delete(fullfile(local_temp_path, '*'));
    end
    
    switch class(nlp)
        case 'NonlinearProgram'
            compileConstraint(nlp,local_temp_path,...
                'ForceExport', true,...
                'StackVariable', true,...
                'BuildMex', false,...
                'TemplateFile', fullfile(frost_c.getRootPath, 'res', 'templateMin.c'),...
                'TemplateHeader', fullfile(frost_c.getRootPath, 'res', 'templateMin.h'),varargin{:});
            
        case 'HybridTrajectoryOptimization'
            compileConstraint(nlp,phase,constr,local_temp_path, exclude,...
                'ForceExport', true,...
                'StackVariable', true,...
                'BuildMex', false,...
                'TemplateFile', fullfile(frost_c.getRootPath, 'res', 'templateMin.c'),...
                'TemplateHeader', fullfile(frost_c.getRootPath, 'res', 'templateMin.h'),varargin{:});
            
        case 'TrajectoryOptimization'
            compileConstraint(nlp,constr,local_temp_path, exclude,...
                'ForceExport', true,...
                'StackVariable', true,...
                'BuildMex', false,...
                'TemplateFile', fullfile(frost_c.getRootPath, 'res', 'templateMin.c'),...
                'TemplateHeader', fullfile(frost_c.getRootPath, 'res', 'templateMin.h'),varargin{:});
            
        otherwise
            error('NLP type is not supported');
    end
    
    movefile(fullfile(local_temp_path, '*.hh'), fullfile(include_dir, 'frost', 'gen'));
    movefile(fullfile(local_temp_path, '*.cc'), src_path);
    
    delete(fullfile(local_temp_path, '*'));
    rmdir(local_temp_path);
end
