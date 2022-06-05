#include <stdio.h>
#include <stdarg.h>
#include <direct.h>

#include "../src/array.h"
#include "../src/core.h"

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("No file name provided\n");
        return 1;
    }

    Array <char *> file_names;
    for (int i = 1; i < argc; i++) {
        char *name = argv[i];
        file_names.add(name);
    }

    system("if not exist src\\compiled mkdir src\\compiled");
    
    char directory[1024] = {};
    _getcwd(directory, sizeof(directory));

    char *path = mprintf("%s\\src\\compiled", directory);
    defer { delete [] path; };
    chdir(path);
    
    for (int i = 0; i < file_names.count; i++) {
        char *name = file_names[i];
        
        char *line = mprintf("call fxc /nologo /T ps_5_0 /E pixel_main /O3 /WX /Fh %s_ps.h /Vn %s_ps_shader_bytes /Qstrip_reflect /Qstrip_debug /Qstrip_priv ..\\..\\run_tree\\data\\shaders\\%s.hlsl", name, name, name);
        system(line);
        delete [] line;

        line = mprintf("call fxc /nologo /T vs_5_0 /E vertex_main /O3 /WX /Fh %s_vs.h /Vn %s_vs_shader_bytes /Qstrip_reflect /Qstrip_debug /Qstrip_priv ..\\..\\run_tree\\data\\shaders\\%s.hlsl", name, name, name);
        system(line);
        delete [] line;        
    }
    
    FILE *file = fopen("init.h", "wt");
    defer { fclose(file); };

    fprintf(file, "#ifndef INIT_H\n");
    fprintf(file, "#define INIT_H\n\n");
    
    fprintf(file, "inline void init_shaders() {\n");
    
    for (int i = 0; i < file_names.count; i++) {
        char *name = file_names[i];

        char *path = mprintf("data/shaders/%s.hlsl", name);
        defer { delete [] path; };
        
        fprintf(file, "    shader_%s = CompileShader(\"%s\", %s);\n", name, path, name);
    }

    fprintf(file, "}\n\n");
    
    fprintf(file, "#endif\n");
    
    return 0;
}
