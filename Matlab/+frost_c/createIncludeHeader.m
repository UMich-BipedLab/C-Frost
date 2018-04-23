function createIncludeHeader(funcs, include_dir)
    if ~exist(include_dir, 'dir')
        mkdir(include_dir);
    end
    if ~exist(fullfile(include_dir, 'frost'), 'dir')
        mkdir(fullfile(include_dir, 'frost'));
    end
    
    fileID = fopen(fullfile(include_dir, 'frost', 'allincludes.hh'), 'w');
    
    fprintf(fileID, '#ifndef ALLINCLUDES_H\n');
    fprintf(fileID, '#define ALLINCLUDES_H\n\n');
    for i = 1:length(funcs)
        fprintf(fileID, '#include "frost/gen/%s.hh"\n', funcs{i});
    end
    fprintf(fileID, '\n#endif\n');
    
    fclose(fileID);
end
