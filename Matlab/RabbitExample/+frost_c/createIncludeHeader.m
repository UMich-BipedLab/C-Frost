function createIncludeHeader(funcs, nums)
    fileID = fopen(fullfile('local', 'code', 'allincludes.hh'), 'w');
    
    fprintf(fileID, '#ifndef ALLINCLUDES_H\n');
    fprintf(fileID, '#define ALLINCLUDES_H\n\n');
    for i = 1:length(funcs)
        fprintf(fileID, '#include "frost/gen/%s.hh"\n', funcs{i});
    end
    fprintf(fileID, '\n#endif\n');
    
    fclose(fileID);
end
