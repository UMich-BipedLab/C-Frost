function createJSONFile(solver, funcs, nums)
    obj = frost_c.generateProblemStructure(solver, funcs, nums);
    
    savejson('', obj, fullfile(frost_c.getRootPath(), 'local', 'code', 'data.json'));
    
%     text = jsonencode(obj);
%     
%     fileID = fopen(fullfile('local', 'code', 'data.json'), 'w');
%     
%     fprintf(fileID, '%s\n', text);
%     
%     fclose(fileID);
end
