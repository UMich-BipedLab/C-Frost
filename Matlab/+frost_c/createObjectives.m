function createObjectives(nlp, phase, constr, src_path, include_dir, exclude, varargin)
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
    
    switch class(nlp)
        case 'NonlinearProgram'
            compileObjective(nlp,local_path, exclude,...
                'ForceExport', true,...
                'StackVariable', true,...
                'BuildMex', false,...
                'TemplateFile', fullfile(frost_c.getRootPath, 'res', 'templateMin.c'),...
                'TemplateHeader', fullfile(frost_c.getRootPath, 'res', 'templateMin.h'),varargin{:});
            
        case 'HybridTrajectoryOptimization'
            compileObjective(nlp,phase,constr,local_path, exclude,...
                'ForceExport', true,...
                'StackVariable', true,...
                'BuildMex', false,...
                'TemplateFile', fullfile(frost_c.getRootPath, 'res', 'templateMin.c'),...
                'TemplateHeader', fullfile(frost_c.getRootPath, 'res', 'templateMin.h'),varargin{:});
            
        case 'TrajectoryOptimization'
            compileObjective(nlp,constr,local_path, exclude,...
                'ForceExport', true,...
                'StackVariable', true,...
                'BuildMex', false,...
                'TemplateFile', fullfile(frost_c.getRootPath, 'res', 'templateMin.c'),...
                'TemplateHeader', fullfile(frost_c.getRootPath, 'res', 'templateMin.h'),varargin{:});
            
        otherwise
            error('NLP type is not supported');
    end
    
    movefile(fullfile(local_path, '*.hh'), fullfile(include_dir, 'frost', 'gen'));
    movefile(fullfile(local_path, '*.cc'), src_path);
end
