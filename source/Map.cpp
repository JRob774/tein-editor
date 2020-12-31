namespace Internal
{
    // CSV parsing taken from here <https://stackoverflow.com/a/30338543>
    TEINAPI std::vector<std::vector<std::string>> ReadCSV (std::istream& in)
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
                    case (CSVState::UnquotedField):
                    {
                        switch (c)
                        {
                            case (','): // end of field
                            {
                                fields.push_back(""); i++;
                            } break;
                            case ('"'):
                            {
                                state = CSVState::QuotedField;
                            } break;
                            default:
                            {
                                fields[i].push_back(c);
                            } break;
                        }
                    } break;
                    case (CSVState::QuotedField):
                    {
                        switch (c)
                        {
                            case ('"'):
                            {
                                state = CSVState::QuotedQuote;
                            } break;
                            default:
                            {
                                fields[i].push_back(c);
                            } break;
                        }
                    } break;
                    case (CSVState::QuotedQuote):
                    {
                        switch (c)
                        {
                            case (','): // , after closing quote
                            {
                                fields.push_back(""); i++;
                                state = CSVState::UnquotedField;
                            } break;
                            case ('"'): // "" -> "
                            {
                                fields[i].push_back('"');
                                state = CSVState::QuotedField;
                            } break;
                            default: // end of quote
                            {
                                state = CSVState::UnquotedField;
                            } break;
                        }
                    } break;
                }
            }
            table.push_back(fields);
        }
        return table;
    }

    TEINAPI bool LoadMap (Tab& tab, std::istream&& stream)
    {
        // Convert raw CSV values into our internal map format.
        auto csv = ReadCSV(stream);
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

    TEINAPI bool SaveMap (const Tab& tab, FILE* file)
    {
        // If the map is empty just save an empty file.
        if (tab.map.empty()) return true;

        int x = GetMapXPos(tab.map);
        int y = GetMapYPos(tab.map);
        int w = GetMapWidth(tab.map);
        int h = GetMapHeight(tab.map);

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
                        if (tab.mapNodeInfo.active && (node.x == tab.mapNodeInfo.active->x && node.y == tab.mapNodeInfo.active->y))
                        {
                            txt = tab.mapNodeInfo.cachedLevelText;
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
}

TEINAPI bool LoadMap (Tab& tab, std::string fileName)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.
    if (!DoesFileExist(fileName))
    {
        LogError(ErrorLevel::Med, "CSV file '%s' does not exist!", fileName.c_str());
        return false;
    }
    return Internal::LoadMap(tab, std::ifstream(fileName));
}

TEINAPI bool SaveMap (const Tab& tab, std::string fileName)
{
    // We don't make the path absolute or anything becuase if that is needed
    // then it should be handled by a higher-level than this internal system.
    FILE* file = fopen(fileName.c_str(), "wb");
    if (!file)
    {
        LogError(ErrorLevel::Med, "Failed to save level file '%s'!", fileName.c_str());
        return false;
    }
    Defer { fclose(file); };
    return Internal::SaveMap(tab, file);
}

TEINAPI bool LoadRestoreMap (Tab& tab, std::string fileName)
{
    std::string data(ReadEntireFile(fileName));

    // Read until the null-terminator to get the name of the map.
    std::string mapName;
    for (auto c: data)
    {
        if (!c) break;
        mapName += c;
    }

    // Set the name of the map for the tab we are loading into.
    tab.name = mapName;

    return Internal::LoadMap(tab, std::istringstream(data.substr(mapName.length()+1)));
}

TEINAPI bool SaveRestoreMap (const Tab& tab, std::string fileName)
{
    FILE* file = fopen(fileName.c_str(), "wb");
    if (!file)
    {
        LogError(ErrorLevel::Med, "Failed to save restore file '%s'!", fileName.c_str());
        return false;
    }
    Defer { fclose(file); };

    // Write the name of the map + null-terminator for later restoration.
    if (tab.name.empty())
    {
        char nullTerminator = '\0';
        fwrite(&nullTerminator, sizeof(char), 1, file);
    }
    else
    {
        const char* name = tab.name.c_str();
        fwrite(name, sizeof(char), strlen(name)+1, file);
    }

    return Internal::SaveMap(tab, file);
}

TEINAPI int GetMapXPos (const Map& map)
{
    if (map.empty()) return 0;
    int minX = INT_MAX;
    for (auto node: map) minX = std::min(node.x, minX);
    return minX;
}

TEINAPI int GetMapYPos (const Map& map)
{
    if (map.empty()) return 0;
    int minY = INT_MAX;
    for (auto node: map) minY = std::min(node.y, minY);
    return minY;
}

TEINAPI int GetMapWidth (const Map& map)
{
    if (map.empty()) return 1;
    int minX = INT_MAX, maxX = INT_MIN;
    for (auto node: map)
    {
        minX = std::min(node.x, minX);
        maxX = std::max(node.x, maxX);
    }
    return abs(maxX-minX)+1;
}

TEINAPI int GetMapHeight (const Map& map)
{
    if (map.empty()) return 1;
    int minY = INT_MAX, maxY = INT_MIN;
    for (auto node: map)
    {
        minY = std::min(node.y, minY);
        maxY = std::max(node.y, maxY);
    }
    return abs(maxY-minY)+1;
}
