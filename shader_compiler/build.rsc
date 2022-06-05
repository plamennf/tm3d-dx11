outputdir ..\run_tree
objdir ..\run_tree\obj\shader_compiler
exename shader_compiler
	
configurations {
    debug: {
        
    },
    release: {
            
    },
}

headers {
    ..\src\core.h
    ..\src\array.h
}

files {
    ..\shader_compiler\main.cpp
}
