#include "loader.h"

#include "mesh.h"
#include "geometry.h"
#include "os.h"
#include "array.h"

#include <stdio.h>

Mesh *make_mesh(u32 num_vertices, Vector3 *positions, Vector2 *uvs, Vector3 *normals,
                u32 num_indices, u32 *indices) {
    Mesh_Vertex *dest_buffer = new Mesh_Vertex[num_vertices];
    defer { delete [] dest_buffer; };
    for (u32 i = 0; i < num_vertices; i++) {
        dest_buffer[i].position = positions[i];

        if (uvs) {
            dest_buffer[i].uv = uvs[i];
        } else {
            dest_buffer[i].uv = make_vector2(0.0f, 0.0f);
        }

        if (normals) {
            dest_buffer[i].normal = normals[i];
        } else {
            dest_buffer[i].normal = make_vector3(0.0f, 1.0f, 0.0f);
        }
    }

    Mesh *result = new Mesh();

    result->vertex_count = num_indices;

    extern void make_buffers_for_mesh(Mesh *mesh, u32 num_vertices, Mesh_Vertex *buffer, u32 num_indices, u32 *indices);
    make_buffers_for_mesh(result, num_vertices, dest_buffer, num_indices, indices);
    
    return result;
}

Mesh *load_obj(char *filename) {
    char *full_path = mprintf("data/models/%s.obj", filename);
    defer { delete [] full_path; };

    char *data = os_read_entire_file(full_path);
    if (!data) {
        fprintf(stderr, "Failed to read file '%s'\n", full_path);
        return nullptr;
    }
    defer { delete [] data; };

    Array <Vector3> vertices;
    Array <Vector2> uvs;
    Array <Vector3> normals;
    Array <u32> indices;

    Vector3 *vertex_array = nullptr;
    Vector2 *uv_array = nullptr;
    Vector3 *normal_array = nullptr;
    u32 *index_array = nullptr;

    char *at = data;
    while (true) {
        char *line = consume_next_line(&at);
        if (!line) break;

        char line_header[128] = {};
        sscanf(line, "%s", line_header);

        if (line_header[0] == 'v' && line_header[1] == 't') {
            Vector2 uv = make_vector2(0.0f, 0.0f);
            sscanf(line, "vt %f %f", &uv.x, &uv.y);
            uvs.add(uv);
        } else if (line_header[0] == 'v' && line_header[1] == 'n') {
            Vector3 normal = make_vector3(0.0f, 0.0f, 0.0f);
            sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
            normals.add(normal);
        } else if (line_header[0] == 'v') {
            Vector3 vertex = make_vector3(0.0f, 0.0f, 0.0f);
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            vertices.add(vertex);
        } else if (line_header[0] == 'f') {
            if (!uv_array) uv_array = new Vector2[vertices.count * 2];
            if (!normal_array) normal_array = new Vector3[vertices.count * 3];

            int vertex_index[3], uv_index[3], normal_index[3];
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                   &vertex_index[0], &uv_index[0], &normal_index[0],
                   &vertex_index[1], &uv_index[1], &normal_index[1],
                   &vertex_index[2], &uv_index[2], &normal_index[2]);

            for (int i = 0; i < 3; i++) {
                int current_vertex_pointer = vertex_index[i] - 1;
                indices.add(current_vertex_pointer);
                Vector2 current_uv = uvs[uv_index[i] - 1];
                uv_array[current_vertex_pointer] = current_uv;
                Vector3 current_normal = normals[normal_index[i] - 1];
                normal_array[current_vertex_pointer] = current_normal;
            }
        }
    }

    vertex_array = vertices.data;
    index_array = indices.data;

    Mesh *result = make_mesh(vertices.count, vertex_array, uv_array, normal_array,
                             indices.count, index_array);
    return result;
}
