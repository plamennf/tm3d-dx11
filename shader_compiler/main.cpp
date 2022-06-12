#include <stdio.h>
#include <stdarg.h>
#include <direct.h>
#include <windows.h>

#include "../src/array.h"
#include "../src/core.h"

u64 get_last_write_time(char *file_path) {
    HANDLE file = CreateFileA(file_path, 0, FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return 0;

    FILETIME ft_create, ft_access, ft_write;
    GetFileTime(file, &ft_create, &ft_access, &ft_write);

    LARGE_INTEGER li;
    li.LowPart = ft_write.dwLowDateTime;
    li.HighPart = ft_write.dwHighDateTime;

    return li.QuadPart;
}

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

    bool has_to_rewrite_init = false;
    for (int i = 0; i < file_names.count; i++) {
        char *name = file_names[i];

        char *path = mprintf("..\\..\\run_tree\\data\\shaders\\%s.hlsl", name);
        u64 shader_file_last_write_time = get_last_write_time(path);
        delete [] path;
        
        path = mprintf("%s_vs.h", name);
        u64 vertex_header_file_last_write_time = get_last_write_time(path);
        delete [] path;

        path = mprintf("%s_ps.h", name);
        u64 pixel_header_file_last_write_time = get_last_write_time(path);
        delete [] path;

        if (shader_file_last_write_time <= vertex_header_file_last_write_time ||
            shader_file_last_write_time <= pixel_header_file_last_write_time) {
            continue;
        }
        
        char *line = mprintf("call fxc /nologo /T ps_5_0 /E pixel_main /O3 /WX /Fh %s_ps.h /Vn %s_ps_shader_bytes /Qstrip_reflect /Qstrip_debug /Qstrip_priv ..\\..\\run_tree\\data\\shaders\\%s.hlsl", name, name, name);
        system(line);
        delete [] line;

        line = mprintf("call fxc /nologo /T vs_5_0 /E vertex_main /O3 /WX /Fh %s_vs.h /Vn %s_vs_shader_bytes /Qstrip_reflect /Qstrip_debug /Qstrip_priv ..\\..\\run_tree\\data\\shaders\\%s.hlsl", name, name, name);
        system(line);
        delete [] line;

        has_to_rewrite_init = true;
    }

    if (has_to_rewrite_init) {
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
    }
        
    return 0;
}
