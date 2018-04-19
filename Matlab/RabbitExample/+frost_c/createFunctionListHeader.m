function createFunctionListHeader(funcs, nums)
    % Generating h
    fileID = fopen(fullfile('local', 'code', 'functionlist.hh'), 'w');
    
    fprintf(fileID, '#ifndef FUNCTIONLIST_H\n');
    fprintf(fileID, '#define FUNCTIONLIST_H\n\n');
    fprintf(fileID, '#include "frost/allincludes.hh"\n\n');
    fprintf(fileID, 'namespace frost {\n');
    fprintf(fileID, '\ttypedef void (*fn_type)(double *, const double *);\n');
    fprintf(fileID, '\textern const fn_type functions[%d];\n', length(funcs));
    fprintf(fileID, '}\n');
    fprintf(fileID, '\n#endif\n');
    
    fclose(fileID);
    
    % Generating C
    fileID = fopen(fullfile('local', 'code', 'functionlist.cc'), 'w');
    
    fprintf(fileID, '#include "frost/functionlist.hh"\n\n');
    fprintf(fileID, 'const frost::fn_type frost::functions[%d] = {\n', length(funcs));
    for i = 1:length(funcs)
        if i ~= length(funcs)
            fprintf(fileID, '\tfrost::gen::%s,\n', funcs{i});
        else
            fprintf(fileID, '\tfrost::gen::%s};\n', funcs{i});
        end
    end
    fclose(fileID);
end
