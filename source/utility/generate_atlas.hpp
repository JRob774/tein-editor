/*******************************************************************************
 * Debug utility for generating texture atlases for the various graphics.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr float TEXTURE_ATLAS_W_LARGE = 4096;
static constexpr float TEXTURE_ATLAS_H_LARGE = 4096;
static constexpr float TEXTURE_ATLAS_W_SMALL = 1024;
static constexpr float TEXTURE_ATLAS_H_SMALL = 1024;

/* -------------------------------------------------------------------------- */

TEINAPI void internal__generate_texture_atlas (std::string output_name, std::string path, float w, float h)
{
    // This is some old code ported over to here so I'm not sure how good it is.
    // Doesn't really matter though as it is not being shipped with the release.

    const int atlas_w = static_cast<int>(w);
    const int atlas_h = static_cast<int>(h);

    std::vector<std::string> files;
    list_path_files(make_path_absolute(path).c_str(), files, true);

    // We will be loading all textures in the same 8-bit component RGBA format.
    constexpr U32 SURFACE_FORMAT  = SDL_PIXELFORMAT_RGBA32;
    constexpr int BYTES_PER_PIXEL = 4;
    constexpr int BITS_PER_PIXEL  = BYTES_PER_PIXEL * 8;

    size_t surface_count = files.size();

    // It is important to use calloc to ensure all pointers are set to NULL!
    // We must also keep the raw texture data until we are done with the SDL
    // surfaces as they do not make a copy of the pixel data during creation.
    SDL_Surface** surfaces = Malloc(SDL_Surface*, surface_count);
    U8** raw_data = Malloc(U8*, surface_count);
    if (!surfaces || !raw_data)
    {
        LOG_ERROR(ERR_MIN, "Failed to allocate texture atlas surfaces!");
        return;
    }

    memset(surfaces, 0, sizeof(SDL_Surface*)*surface_count);
    memset(raw_data, 0, sizeof(U8*)*surface_count);

    Defer
    {
        for (size_t i=0; i<surface_count; ++i)
        {
            SDL_FreeSurface(surfaces[i]);
            stbi_image_free(raw_data[i]);
        }
        Free(surfaces); surfaces = NULL;
        Free(raw_data); raw_data = NULL;
    };

    // Go through each of the textures to be packed and load as surfaces.
    for (size_t i=0; i<surface_count; ++i)
    {
        const char* file_name = files[i].c_str();
        int iw, ih, bpp;
        raw_data[i] = stbi_load(file_name, &iw, &ih, &bpp, BYTES_PER_PIXEL);
        if (!raw_data[i])
        {
            LOG_ERROR(ERR_MIN, "Failed to load atlas image file '%s'!", file_name);
            return;
        }

        int pitch = iw * BYTES_PER_PIXEL;
        surfaces[i] = SDL_CreateRGBSurfaceWithFormatFrom(raw_data[i], iw, ih, BITS_PER_PIXEL, pitch, SURFACE_FORMAT);
        if (!surfaces[i])
        {
            LOG_ERROR(ERR_MIN, "Failed to create atlas surface '%s'!", file_name);
            return;
        }

        // We need to set the blendmode to none because when we later make the
        // final atlas surface it will automatically filled with RGBA data of
        // all zeroes. So, if we had SDL_BLENDMODE_BLEND then the blit surface
        // calls would attempt to blend the semi-transparent pixels of each of
        // the surfaces with the black of the atlas surface -- leading to the
        // semi-transparent parts of the texture clips being slightly darker.
        SDL_SetSurfaceBlendMode(surfaces[i], SDL_BLENDMODE_NONE);
    }

    // Allocate memory for the necessary components used by the rect packer.
    int node_count = atlas_w;
    stbrp_node* nodes = Malloc(stbrp_node, node_count);
    if (!nodes)
    {
        LOG_ERROR(ERR_MIN, "Failed to allocate atlas packing nodes!");
        return;
    }
    Defer { Free(nodes); };

    int rect_count = static_cast<int>(surface_count);
    stbrp_rect* rects = Malloc(stbrp_rect, rect_count);
    if (!rects)
    {
        LOG_ERROR(ERR_MIN, "Failed to allocate atlas packing rects!");
        return;
    }
    Defer { Free(rects); };

    // We need to store the size of each texture into the rects so that stb
    // can know the dimensions when packing the rects into a texture atlas.
    for (int i=0; i<rect_count; ++i)
    {
        rects[i].w = static_cast<stbrp_coord>(surfaces[i]->w);
        rects[i].h = static_cast<stbrp_coord>(surfaces[i]->h);
    }

    // Basic initialization needed to begin using the stb_rect_pack library.
    stbrp_context context;
    stbrp_init_target(&context, atlas_w,atlas_h, nodes, node_count);
    if (!stbrp_pack_rects(&context, rects, rect_count))
    {
        LOG_ERROR(ERR_MIN, "Failed to pack texture atlas images!");
        return;
    }

    SDL_Surface* atlas_surface = SDL_CreateRGBSurfaceWithFormat(0, atlas_w,atlas_h, BITS_PER_PIXEL, SURFACE_FORMAT);
    if (!atlas_surface)
    {
        LOG_ERROR(ERR_MIN, "Failed to create final atlas surface! (%s)", SDL_GetError());
        return;
    }
    Defer { SDL_FreeSurface(atlas_surface); atlas_surface = NULL; };

    std::string output_txt_name(build_resource_string(output_name) + ".txt");
    FILE* file = fopen(output_txt_name.c_str(), "w");
    if (!file)
    {
        LOG_ERROR(ERR_MIN, "Failed to create atlas lookup GON!");
        return;
    }
    Defer { fclose(file); };

    fprintf(file, "texture \"%s\"\n", std::string(output_name + ".png").c_str());
    fprintf(file, "clips\n[\n");

    // Blit each of the individual surfaces to the final atlas surface and
    // cache the clip information for each texture in the lookup table.
    for (size_t i=0; i<surface_count; ++i)
    {
        SDL_Rect rect
        {
            static_cast<int>(rects[i].x),
            static_cast<int>(rects[i].y),
            static_cast<int>(rects[i].w),
            static_cast<int>(rects[i].h)
        };
        if (SDL_BlitSurface(surfaces[i], NULL, atlas_surface, &rect) != 0)
        {
            LOG_ERROR(ERR_MIN, "Failed to blit texture to the atlas! (%s)", SDL_GetError());
            return;
        }

        constexpr int PADDING = 12;

        int rx = rects[i].x +  PADDING;
        int ry = rects[i].y +  PADDING;
        int rw = rects[i].w - (PADDING * 2);
        int rh = rects[i].h - (PADDING * 2);

        std::string id_str(strip_file_path_and_ext(files[i].c_str()));
        int id = std::stoi(id_str);
        if (id_str.find("-alt") != std::string::npos) id += ALT_OFFSET;

        fprintf(file, "{ id %d  clip [ %d %d %d %d ] }\n", id, rx,ry,rw,rh);
    }

    fprintf(file, "]\n");

    std::string output_png_name(build_resource_string(output_name) + ".png");
    stbi_write_png(output_png_name.c_str(), atlas_w,atlas_h, BYTES_PER_PIXEL,
        atlas_surface->pixels, atlas_w*BYTES_PER_PIXEL);

    LOG_DEBUG("Created Texture Atlas: %s", strip_file_path(output_name).c_str());
}

/* -------------------------------------------------------------------------- */

TEINAPI void generate_texture_atlases ()
{
    if (main_event.type == SDL_KEYDOWN)
    {
        if (main_event.key.keysym.sym == SDLK_F11)
        {
            internal__generate_texture_atlas("textures/editor_icons/new_large", "../dev/editor_icons_padded/new_large/",
                TEXTURE_ATLAS_W_LARGE,TEXTURE_ATLAS_H_LARGE);
            internal__generate_texture_atlas("textures/editor_icons/new_small", "../dev/editor_icons_padded/new_small/",
                TEXTURE_ATLAS_W_SMALL,TEXTURE_ATLAS_H_SMALL);
            internal__generate_texture_atlas("textures/editor_icons/old_large", "../dev/editor_icons_padded/old_large/",
                TEXTURE_ATLAS_W_LARGE,TEXTURE_ATLAS_H_LARGE);
            internal__generate_texture_atlas("textures/editor_icons/old_small", "../dev/editor_icons_padded/old_small/",
                TEXTURE_ATLAS_W_SMALL,TEXTURE_ATLAS_H_SMALL);
        }
    }
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
