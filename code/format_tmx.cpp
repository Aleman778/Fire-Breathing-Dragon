
// NOTE(Alexander): hardcoded and may be invalidated whenever tilesets is changed
const s32 player_gid = 36;

typedef u8 Tile;

struct Box {
    v2 min_p;
    v2 max_p;
}

struct Loaded_Tmx {
    Entity* entities;
    //Collider* colliders;
    
    []Tile tile_map;
    s32 tile_map_width;
    s32 tile_map_height;
    s32 tile_width;
    s32 tile_height;
    
    s32 entity_count;
    
    //s32 collider_count;
    
    s32 is_loaded;
};

struct Read_File_Result {
    void* contents;
    u32 contents_size;
};

bool
string_equals(string a, string b) {
    if (a.count != b.count) {
        return false;
    }
    
    for (smm index = 0; index < a.count; index++) {
        if (a.data[index] != b.data[index]) {
            return false;
        }
    }
    
    return true;
}


Read_File_Result
read_entire_file(cstring filename) {
    Read_File_Result result = {};
    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(file_handle, &file_size)) {
            result.contents_size = (u32) file_size.QuadPart;
            result.contents = VirtualAlloc(0, result.contents_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            if (result.contents) {
                if (!ReadFile(file_handle, result.contents, result.contents_size, 0, 0)) {
                    // TODO(alexander): logging, failed read file
                }
                
            } else {
                // TODO(alexander): logging, failed to allocate memory
            }
            
        } else {
            // TODO(alexander): logging, failed to get file size
        }
        
        CloseHandle(file_handle);
    } else {
        // TODO(alexander): logging, file not found
    }
    
    return result;
}

void*
allocate_file_memory(u32 size) {
    return VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

void
free_file_memory(void* memory) {
    VirtualFree(memory, 0, MEM_RELEASE);
}

bool
write_entire_file(cstring filename, void* memory, u32 size) {
    bool result = false;
    
    DeleteFileA(filename);
    HANDLE file_handle = CreateFileA(filename, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD num_bytes_written;
        if (WriteFile(file_handle, memory, size, &num_bytes_written, 0)) {
            result = (num_bytes_written == size);
        } else {
            // TODO(alexander): logging, failed to write file
        }
        
        CloseHandle(file_handle);
    } else {
        // TODO(alexander): logging, file not found
    }
    
    return result;
}

Loaded_Tmx
read_tmx_map_data(string filename,
                  Memory_Arena* arena) {
    
    
    cstring cfilename = string_to_cstring(filename);
    Read_File_Result file = read_entire_file(cfilename);
    cstring_free(cfilename);
    
    
    Loaded_Tmx result = read_tmx_map_data((u8*) file.contents, arena);
    //free_file_memory(file.contents);
    return result;
}

// TODO(Alexander): we are currently storing resulting entities and colliders
// in contiguous array, what if the arena runs out of memory, how do we handle this?
// Either we set a hard constraint that a level can only be 10kB or if we decide to
// stream chunks instead we have to store the entites in a more sophisticated manner.

Loaded_Tmx
read_tmx_map_data(u8* scan, Memory_Arena* arena) {
    Loaded_Tmx result = {};
    
    // NOTE(Alexander): first loads general information about the map
    for (; *scan; scan++) {
        if (eat_string(&scan, "<map")) {
            for (; *scan; scan++) {
                if (eat_string(&scan, ">")) {
                    break;
                }
                if (eat_string(&scan, " width=\"")) {
                    result.tile_map_width = eat_integer(&scan);
                } else if (eat_string(&scan, " height=\"")) {
                    result.tile_map_height = eat_integer(&scan);
                } else if (eat_string(&scan, " tilewidth=\"")) {
                    result.tile_width = eat_integer(&scan);
                } else if (eat_string(&scan, " tileheight=\"")) {
                    result.tile_height = eat_integer(&scan);
                }
            }
            
            break;
        }
    }
    
    if (result.tile_map_width == 0 || result.tile_map_height == 0 || 
        result.tile_width == 0 || result.tile_height == 0) {
        pln("Tiled map is corrupt");// TODO(Alexander): logging system
        return result;
    }
    
    int tile_count = result.tile_map_width * result.tile_map_height;
    result.tile_map.count = tile_count;
    result.tile_map.data = push_array_of_structs(arena, tile_count, Tile);
    //result.tile_map.data = (Tile*) push_size(arena, tile_count*sizeof(Tile));
    
    // NOTE(Alexander): Load all the layers
    for (; *scan; scan++) {
        if (eat_string(&scan, "<layer")) {
            for (; *scan; scan++) {
                if (eat_string(&scan, "</layer>")) {
                    break;
                }
                if (eat_string(&scan, "<data encoding=\"csv\">")) {
                    read_tmx_tile_map(&scan, &result);
                }
            }
        }
        
        if (eat_string(&scan, "<objectgroup")) {
            for (; *scan; scan++) {
                if (eat_string(&scan, " name=\"")) {
                    string name = eat_until(&scan,'"');
                    
                    if (string_equals(name, "Entities")) {
                        read_tmx_entities(&scan, arena, &result);
                    } else if (string_equals(name, "Collisions")) {
                        read_tmx_colliders(&scan, arena, &result);
                    } else {
                        pln("Invalid object group: ", name);
                        assert(0 && "invalid objectgroup found");
                    }
                    break;
                }
            }
        }
    }
    
    result.is_loaded = true;
    return result;
}

internal void
read_tmx_tile_map(u8** scanner, Loaded_Tmx* result) {
    s32 tile_index = 0;
    s32 chunk_width = 0;
    
    u8* scan = *scanner;
    for (; *scan; scan++) {
        if (eat_string(&scan, "</data>")) {
            break;
        }
        
        if (*scan == ',') {
            scan++;
            tile_index++;
            
            assert(tile_index < result->tile_map.count && "number of tiles exceeds its limit");
            
            if (chunk_width > 0) {
                if ((tile_index % chunk_width) == 0) {
                    tile_index += result->tile_map_width - chunk_width;
                }
            }
            
            result->tile_map[tile_index] = 0;
        }
        
        if (*scan == ' ' || *scan == '\n' || *scan == '\t' || *scan == '\r' || 
            eat_string(&scan, "</chunk>")) {
            continue;
        }
        
        if (eat_string(&scan, "<chunk")) {
            int chunk_x = 0;
            int chunk_y = 0;
            for (; *scan; scan++) {
                if (eat_string(&scan, ">")) {
                    assert(chunk_x >= 0 && chunk_y >= 0 && "chunks needs to first be normalized");
                    tile_index = chunk_y*result->tile_map_width + chunk_x;
                    break;
                }
                if (eat_string(&scan, " x=\"")) {
                    chunk_x = eat_integer(&scan);
                } else if (eat_string(&scan, " y=\"")) {
                    chunk_y = eat_integer(&scan);
                } else if (eat_string(&scan, " width=\"")) {
                    chunk_width = eat_integer(&scan);
                }
            }
            
            result->tile_map[tile_index] = 0;
            //pln("parsed chunk: x=%, y=%, width=%, tile_index = %", chunk_x, chunk_y, chunk_width, tile_index);
            continue;
        }
        
        int number = (int) (*scan - '0');
        if (number >= 0 && number <= 9) {
            Tile data = result->tile_map[tile_index];
            result->tile_map[tile_index] = (Tile) (data*10 + number);
            //pln("number: %, tile: % = %", number, tile_index, (int) result->tile_map[tile_index]);
        }
    }
}


internal void
read_tmx_colliders(u8** scanner, Memory_Arena* arena, Loaded_Tmx* result) {
    for (u8* scan = *scanner; *scan; scan++) {
        if (eat_string(&scan, "</objectgroup>")) {
            break;
        }
        
        if (eat_string(&scan, "<object")) {
            Entity* collider = push_struct(arena, Entity);
            collider->type = Entity_Type.Box_Collider;
            result->entity_count++;
            
            v2 p = {};
            
            if (!result->entities) {
                result->entities = collider;
            }
            
            for (; *scan; scan++) {
                if (eat_string(&scan, "/>") || eat_string(&scan, "</object>")) {
                    break;
                }
                
                if (eat_string(&scan, " x=\"")) {
                    f32 x = (f32) eat_integer(&scan);
                    collider->p.x = x/(f32) result->tile_width;
                } else if (eat_string(&scan, " y=\"")) {
                    f32 y = (f32) eat_integer(&scan);
                    collider->p.y = y/(f32) result->tile_height;
                } else if (eat_string(&scan, " width=\"")) {
                    f32 width  = (f32) eat_integer(&scan);
                    collider->size.width = width/(f32) result->tile_width;
                } else if (eat_string(&scan, " height=\"")) {
                    f32 height = (f32) eat_integer(&scan);
                    collider->size.height = height/(f32) result->tile_height;
                }
                
#if 0 
                if (eat_string(&scan, "<polygon points=\"")) {
                    v2 origin = collider->aabb.min;
                    v2 p0;
                    p0.x = origin.x + (f32) eat_integer(&scan)/(f32) result->tile_width;
                    assert(*scan++ == ',');
                    p0.y = origin.y + (f32) eat_integer(&scan)/(f32) result->tile_height;
                    v2 first_p = p0;
                    if (*scan++ == '"') {
                        break;
                    }
                    
                    for (; *scan && *scan != '"'; scan++) {
                        if (*scan == ' ') continue;
                        
                        v2 p1;
                        p1.x = origin.x + (f32) eat_integer(&scan)/(f32) result->tile_width;
                        assert(*scan++ == ',');
                        p1.y = origin.y + (f32) eat_integer(&scan)/(f32) result->tile_height;
                        
                        collider->type = Collider_Type.Line;
                        collider->line.p = p0;
                        collider->line.r = p1 - p0;
                        p0 = p1;
                        
                        collider = push_struct(arena, Collider);
                        result->collider_count++;
                        
                        if (*scan == '"') {
                            scan++;
                            break;
                        }
                    }
                    
                    collider->type = Collider_Type.Line;
                    collider->line.p = p0;
                    collider->line.r = first_p - p0;
                }
#endif
            }
        }
    }
}

internal void
read_tmx_entities(u8** scanner, Memory_Arena* arena, Loaded_Tmx* result) {
    for (u8* scan = *scanner; *scan; scan++) {
        if (eat_string(&scan, "</objectgroup>")) {
            break;
        }
        
        if (eat_string(&scan, "<object")) {
            Entity* entity = push_struct(arena, Entity);
            result->entity_count++;
            
            if (!result->entities) {
                result->entities = entity;
            }
            
            for (; *scan; scan++) {
                if (eat_string(&scan, "/>") || eat_string(&scan, "</object>")) {
                    break;
                }
                
                if (eat_string(&scan, "gid=\"")) {
                    s32 gid = eat_integer(&scan);
                    //entity->graphics_id = gid; // TODO(Alexander): store using props
                    
                    // NOTE(Alexander): this creates different entity types
                    if (gid == player_gid) {
                        entity->type = Entity_Type.Player;
                    } else {
                        entity->type = Entity_Type.None;
                    }
                } else if (eat_string(&scan, " x=\"")) {
                    f32 x = (f32) eat_integer(&scan);
                    entity->p.x = x/(f32) result->tile_width;
                } else if (eat_string(&scan, " y=\"")) {
                    f32 y = (f32) eat_integer(&scan);
                    entity->p.y = y/(f32) result->tile_height;
                }
                else if (eat_string(&scan, " width=\"")) {
                    f32 width = (f32) eat_integer(&scan);
                    entity->size.width = width/(f32) result->tile_width;
                } else if (eat_string(&scan, " height=\"")) {
                    f32 height = (f32) eat_integer(&scan);
                    entity->size.height = height/(f32) result->tile_height; 
                }
            }
        }
    }
}

#if 0
void
write_polygon_object(String_Builder* sb, int object_id, []v2 points) {
    string_builder_push_format(sb, "  <object id=\"%\" x=\"0\" y=\"0\">\n", object_id);
    string_builder_push(sb, "   <polygon points=\"");
    
    for (s32 index = 0; index < points.count; index++) {
        v2 p = points[index];
        string_builder_push_format(sb, "%,% ", (s32) (p.x * TILE_SIZE), (s32) (p.y * TILE_SIZE));
    }
    
    string_builder_push(sb, "   \"/>\n");
    string_builder_push(sb, "  </object>\n");
}

void
write_tmx_map(string filename, Tile_Map* tile_map, []Entity_Data entities, Platform_Write_Entire_File* write_entire_file) {
    rect2 bounds = { 0.0f, 0.0f, (f32) tile_map->width, (f32) tile_map->height };
    
    String_Builder sb;
    string_builder_push(&sb, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    string_builder_push(&sb, "<map version=\"1.5\" tiledversion=\"1.7.2\" orientation=\"orthogonal\" renderorder=\"right-down\" width=\"48\" height=\"32\" tilewidth=\"32\" tileheight=\"32\" infinite=\"1\" nextlayerid=\"4\" nextobjectid=\"23\">\n");
    string_builder_push(&sb, " <tileset firstgid=\"1\" source=\"../sprites/grass_tileset.tsx\"/>\n");
    string_builder_push(&sb, " <tileset firstgid=\"36\" source=\"../sprites/player.tsx\"/>\n");
    string_builder_push(&sb, " <tileset firstgid=\"37\" source=\"../sprites/red_slime.tsx\"/>\n");
    
    // Tile map
    string_builder_push(&sb, " <layer id=\"1\" name=\"Tile map\" width=\"48\" height=\"32\">\n");
    string_builder_push(&sb, "  <data encoding=\"csv\">\n");
    for (int i = 0; i < tile_map.tiles.count; i++) {
        string_builder_push_format(&sb, " %", tile_map.tiles[i]);
        if (i < tile_map.tiles.count - 1) {
            string_builder_push_char(&sb, ',');
        }
    }
    string_builder_push(&sb, "\n  </data>\n");
    string_builder_push(&sb, " </layer>\n");
    
    // Collisions
    string_builder_push(&sb, " <objectgroup id=\"2\" name=\"Collisions\">\n");
    Collision_Builder* builder = (Collision_Builder*) calloc(1, sizeof(Collision_Builder)); //temporary
    builder.visited_size = (tile_map->width + 1) * (tile_map->height + 1);
    
    int next_unique_id = 1;
    for (s32 x = 0; x < tile_map->width; x++) {
        for (s32 y = 0; y < tile_map->height; y++) {
            
            builder.num_points = 0;
            create_tile_map_collision(builder, tile_map, x, y, x, y);
            
            if (builder.num_points > 1) {
                //pln("pts: %, x = %, y = %", builder.num_points, x, y);
                
                []v2 points;
                points.data = builder.points.data;
                points.count = builder.num_points;
                
                write_polygon_object(&sb, next_unique_id, points);
                next_unique_id++;
            }
        }
    }
    
    
    []v2 borders = {
        bounds.min, 
        vec2(bounds.min.x, bounds.max.y),
    };
    write_polygon_object(&sb, next_unique_id, ([]v2) borders);
    next_unique_id++;
    borders = {
        vec2(bounds.max.x, bounds.min.y),
        bounds.max,
        
    };
    write_polygon_object(&sb, next_unique_id, ([]v2) borders);
    next_unique_id++;
    string_builder_push(&sb, " </objectgroup>\n");
    
    // Entities
    string_builder_push(&sb, " <objectgroup id=\"3\" name=\"Entities\">\n");
    for (int i = 0; i < entities.count; i++) {
        Entity_Data data = entities[i];
        v2s p;
        p.x = (s32) (data.target.p.x * TILE_SIZE);
        p.y = (s32) (data.target.p.y * TILE_SIZE);
        v2s size;
        size.x = (s32) (data.target.size.x * TILE_SIZE);
        size.y = (s32) (data.target.size.y * TILE_SIZE);
        
        s32 gid = 0;
        switch (data.target.type) {
            case Entity_Type.Player: {
                gid = player_gid;
            }
            
            case Entity_Type.Enemy_Red_Slime: {
                gid =  enemy_red_slime_gid;
            }
        }
        
        string_builder_push_format(&sb, " <object id=\"%\" gid=\"%\" x=\"%\" y=\"%\" width=\"%\" height=\"%\"/>\n",
                                   next_unique_id, gid, p.x, p.y, size.x, size.y);
        
        next_unique_id++;
    }
    string_builder_push(&sb, " </objectgroup>\n");
    
    string_builder_push(&sb, "</map>\n");
    
    string test = string_builder_to_string_nocopy(&sb);
    //pln("%", test);
    
    cstring output_file = string_to_cstring(filename);
    write_entire_file(output_file, test.data, (s32) test.count);
    cstring_free(output_file);
    
    pln("Level was written to `%`", filename);
}

#endif