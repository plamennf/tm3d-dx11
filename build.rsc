outputdir run_tree
objdir run_tree\obj
exename tm3d-dx11
	
configurations {
    debug: {
        
    },
    release: {
            
    },
}

defines {
    RENDER_D3D11
}

includedirs {
    external\include
}

libdirs {
    external\lib
}

libs {
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    freetype.lib
}

headers {
    src\general.h
    src\array.h
    src\os.h
    src\display.h
    src\draw.h
    src\geometry.h
    src\mesh.h
    src\loader.h
    src\bitmap.h
    src\font.h
    src\catalog.h
    src\debug.h
    src\input.h
    src\entities.h
    src\camera.h
    src\hash_table.h
    src\config.h
    src\text_file_handler.h
}

files {
    src\main.cpp
    src\os_windows.cpp
    src\display_windows.cpp
    src\draw_d3d11.cpp
    src\loader.cpp
    src\draw.cpp
    src\font.cpp
    src\catalog.cpp
    src\debug.cpp
    src\input.cpp
    src\entities.cpp
    src\camera.cpp
    src\menu.cpp
    src\config.cpp
    src\text_file_handler.cpp
}

prebuildcmd: compile_shaders.bat