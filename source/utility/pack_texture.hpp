/*******************************************************************************
 * Debug utility for adding a padding border to the variosu graphics.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

FILDEF void internal__do_tile_padding (u8* in, u8* out, int w, int h, int xpad, int ypad, int padding)
{
    constexpr int BPP = 4;

    // determine information regarding the tiles
    int tile_w = w+(padding*2);
    int tile_h = h+(padding*2);

    // create a buffer for handling each tile's padding
    u8* buffer = cstd_malloc(u8, (tile_w*tile_h)*BPP);
    if (!buffer)
    {
        LOG_ERROR(ERR_MIN, "Failed to allocate output padding buffer!");
        return;
    }
    defer { cstd_free(buffer); };

    for (int j=0,k=0; j<h; ++j,++k)
    {
        memcpy(&buffer[(padding+k)*(tile_w*BPP)+(padding*BPP)], &in[j*(w*BPP)], (w*BPP));
    }

    int xpix = 0;
    int ypix = 0;

    for (int iy=0; iy<tile_h; ++iy)
    {
        for (int ix=0; ix<(tile_w*BPP); ix+=BPP)
        {
            xpix = (ix/BPP)-padding;
            ypix =  iy     -padding;

            if (xpix < 0) xpix = 0; else if (xpix >= w-1) xpix = w-1;
            if (ypix < 0) ypix = 0; else if (ypix >= h-1) ypix = h-1;

            xpix += padding;
            ypix += padding;

            buffer[iy*(tile_w*BPP)+(ix+0)] = buffer[ypix*(tile_w*BPP)+((xpix*BPP)+0)];
            buffer[iy*(tile_w*BPP)+(ix+1)] = buffer[ypix*(tile_w*BPP)+((xpix*BPP)+1)];
            buffer[iy*(tile_w*BPP)+(ix+2)] = buffer[ypix*(tile_w*BPP)+((xpix*BPP)+2)];
            buffer[iy*(tile_w*BPP)+(ix+3)] = buffer[ypix*(tile_w*BPP)+((xpix*BPP)+3)];
        }
    }

    for (int j=0,k=0; j<tile_h; ++j,++k)
    {
        memcpy(&out[j*((w+xpad)*BPP)], &buffer[k*(tile_w*BPP)], (tile_w*BPP));
    }
}

FILDEF void internal__pack_textures (std::string in, std::string out)
{
    constexpr int PADDING = 12;
    constexpr int BPP = 4;

    std::vector<std::string> files;
    list_path_files(make_path_absolute(in).c_str(), files, true);

    for (auto f: files)
    {
        int w, h;
        u8* in_pixels = stbi_load(f.c_str(), &w,&h,NULL,BPP);
        if (!in_pixels)
        {
            LOG_ERROR(ERR_MIN, "Failed to pad tile image \"%s\"", f.c_str());
            continue;
        }
        defer { stbi_image_free(in_pixels); };

        int xpad = PADDING*2;
        int ypad = PADDING*2;

        u8* out_pixels = cstd_malloc(u8, ((w + xpad) * (h + ypad)) * BPP);
        if (!out_pixels)
        {
            LOG_ERROR(ERR_MIN, "Failed to allocate padded tile image \"%s\"", f.c_str());
            continue;
        }
        defer { cstd_free(out_pixels); };

        internal__do_tile_padding(in_pixels, out_pixels, w,h, xpad,ypad, PADDING);

        std::string file_name(make_path_absolute(out) + strip_file_path(f));
        stbi_write_png(file_name.c_str(), w+xpad,h+ypad, BPP, out_pixels, (w+xpad)*BPP);
    }

    LOG_DEBUG("Padded Tiles: %s", in.c_str());
}

/* -------------------------------------------------------------------------- */

FILDEF void pack_textures ()
{
    if (main_event.type == SDL_KEYDOWN)
    {
        if (main_event.key.keysym.sym == SDLK_F10)
        {
            internal__pack_textures("../dev/editor_icons/new_large/", "../dev/editor_icons_padded/new_large/");
            internal__pack_textures("../dev/editor_icons/new_small/", "../dev/editor_icons_padded/new_small/");
            internal__pack_textures("../dev/editor_icons/old_large/", "../dev/editor_icons_padded/old_large/");
            internal__pack_textures("../dev/editor_icons/old_small/", "../dev/editor_icons_padded/old_small/");
        }
    }
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
