TEINAPI void PushEditorEvent (EditorEvent id, void* data1, void* data2)
{
    SDL_Event event = {};
    event.type = SDL_USEREVENT;
    event.user.type = SDL_USEREVENT;
    event.user.timestamp = SDL_GetTicks();
    event.user.code = static_cast<U32>(id);
    event.user.data1 = data1;
    event.user.data2 = data2;
    SDL_PushEvent(&event);
}
