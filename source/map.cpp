/*******************************************************************************
 * Loads and saves raw world map data ot and from the CSV file format.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

// CSV parsing taken from here <https://stackoverflow.com/a/30338543>
TEINAPI std::vector<std::vector<std::string>> internal__read_csv (std::istream& in)
{
    enum class CSVState
    {
        UnquotedField,
        QuotedField,
        QuotedQuote
    };

    std::vector<std::vector<std::string>> table;
    std::string row;

    while (!in.eof())
    {
        std::getline(in, row);
        if (in.bad() || in.fail())
        {
            break;
        }
        CSVState state = CSVState::UnquotedField;
        std::vector<std::string> fields {""};
        size_t i = 0; // index of the current field
        for (char c : row)
        {
            switch (state)
            {
                case CSVState::UnquotedField:
                    switch (c)
                    {
                        case ',': // end of field
                                  fields.push_back(""); i++;
                                  break;
                        case '"': state = CSVState::QuotedField;
                                  break;
                        default:  fields[i].push_back(c);
                                  break;
                    }
                    break;
                case CSVState::QuotedField:
                    switch (c)
                    {
                        case '"': state = CSVState::QuotedQuote;
                                  break;
                        default:  fields[i].push_back(c);
                                  break;
                    }
                    break;
                case CSVState::QuotedQuote:
                    switch (c)
                    {
                        case ',': // , after closing quote
                                  fields.push_back(""); i++;
                                  state = CSVState::UnquotedField;
                                  break;
                        case '"': // "" -> "
                                  fields[i].push_back('"');
                                  state = CSVState::QuotedField;
                                  break;
                        default:  // end of quote
                                  state = CSVState::UnquotedField;
                                  break;
                    }
                    break;
            }
        }
        table.push_back(fields);
    }
    return table;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__load_map (Tab& tab, std::istream&& stream)
{
    // Convert raw CSV values into our internal map format.
    auto csv = internal__read_csv(stream);
    for (int iy=0; iy<static_cast<int>(csv.size()); ++iy)
    {
        const auto& row = csv.at(iy);
        for (int ix=0; ix<static_cast<int>(row.size()); ++ix)
        {
            const auto& field = row.at(ix);
            if (!field.empty())
            {
                tab.map.push_back({ ix,iy, field });
            }
        }
    }

    return true;
}

TEINAPI bool internal__save_map (const Tab& tab, FILE* file)
{
    // If the map is empty just save an empty file.
    if (tab.map.empty()) return true;

    int x = get_map_x_pos (tab.map);
    int y = get_map_y_pos (tab.map);
    int w = get_map_width (tab.map);
    int h = get_map_height(tab.map);

    // Write out the CSV formatted fields for the map.
    for (int iy=y; iy<y+h; ++iy)
    {
        for (int ix=x; ix<x+w; ++ix)
        {
            for (auto& node: tab.map)
            {
                if (node.x == ix && node.y == iy)
                {
                    std::string txt(node.lvl);
                    if (tab.map_node_info.active && (node.x == tab.map_node_info.active->x && node.y == tab.map_node_info.active->y))
                    {
                        txt = tab.map_node_info.cached_lvl_text;
                    }
                    // Need to wrap in quotes if it contains a comma.
                    if (txt.find(',') == std::string::npos)
                    {
                        fprintf(file, "%s", txt.c_str());
                    }
                    else
                    {
                        fprintf(file, "\"%s\"", txt.c_str());
                    }
                    break;
                }
            }
            if (ix != (x+w-1))
            {
                fprintf(file, ",");
            }
        }
        fprintf(file, "\n");
    }

    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_map (Tab& tab, std::string file_name)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.

    if (!DoesFileExist(file_name))
    {
        LOG_ERROR(ERR_MED, "CSV file '%s' does not exist!", file_name.c_str());
        return false;
    }

    return internal__load_map(tab, std::ifstream(file_name));
}

TEINAPI bool save_map (const Tab& tab, std::string file_name)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.

    FILE* file = fopen(file_name.c_str(), "wb");
    if (!file)
    {
        LOG_ERROR(ERR_MED, "Failed to save level file '%s'!", file_name.c_str());
        return false;
    }
    Defer { fclose(file); };

    return internal__save_map(tab, file);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_restore_map (Tab& tab, std::string file_name)
{
    std::string data(ReadEntireFile(file_name));

    // Read until the null-terminator to get the name of the map.
    std::string map_name;
    for (auto c: data)
    {
        if (!c) break;
        map_name += c;
    }

    // Set the name of the map for the tab we are loading into.
    tab.name = map_name;

    return internal__load_map(tab, std::istringstream(data.substr(map_name.length()+1)));
}

TEINAPI bool save_restore_map (const Tab& tab, std::string file_name)
{
    FILE* file = fopen(file_name.c_str(), "wb");
    if (!file)
    {
        LOG_ERROR(ERR_MED, "Failed to save restore file '%s'!", file_name.c_str());
        return false;
    }
    Defer { fclose(file); };

    // Write the name of the map + null-terminator for later restoration.
    if (tab.name.empty())
    {
        char null_terminator = '\0';
        fwrite(&null_terminator, sizeof(char), 1, file);
    }
    else
    {
        const char* name = tab.name.c_str();
        fwrite(name, sizeof(char), strlen(name)+1, file);
    }

    return internal__save_map(tab, file);
}

/* -------------------------------------------------------------------------- */

TEINAPI int get_map_x_pos (const Map& map)
{
    if (map.empty()) return 0;
    int min_x = INT_MAX;
    for (auto node: map)
    {
        min_x = std::min(node.x, min_x);
    }
    return min_x;
}

TEINAPI int get_map_y_pos (const Map& map)
{
    if (map.empty()) return 0;
    int min_y = INT_MAX;
    for (auto node: map)
    {
        min_y = std::min(node.y, min_y);
    }
    return min_y;
}

TEINAPI int get_map_width (const Map& map)
{
    if (map.empty()) return 1;
    int min_x = INT_MAX, max_x = INT_MIN;
    for (auto node: map)
    {
        min_x = std::min(node.x, min_x);
        max_x = std::max(node.x, max_x);
    }
    return abs(max_x-min_x)+1;
}

TEINAPI int get_map_height (const Map& map)
{
    if (map.empty()) return 1;
    int min_y = INT_MAX, max_y = INT_MIN;
    for (auto node: map)
    {
        min_y = std::min(node.y, min_y);
        max_y = std::max(node.y, max_y);
    }
    return abs(max_y-min_y)+1;
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
