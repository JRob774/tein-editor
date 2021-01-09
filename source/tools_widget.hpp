struct ToolsWidget
{
    // @Incomplete: ...
    bool open;
};

Global ToolsWidget gToolsWidget;

EditorAPI bool InitToolsWidget ();
EditorAPI void QuitToolsWidget ();
EditorAPI void DoToolsWidget ();
EditorAPI bool& GetToolsWidgetOpen ();
