enum class DialogType { Lvl, Csv, LvlCsv, Gpak, Exe };

// Platform-agnostic open/save/path dialog operations.
TEINAPI std::vector<std::string> OpenDialog (DialogType type, bool multiselect = true);
TEINAPI std::string              SaveDialog (DialogType type);
TEINAPI std::vector<std::string> PathDialog (                 bool multiselect = true);
