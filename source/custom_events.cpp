TEINAPI void PushEditorEvent (EditorEvent id, void* data1, void* data2)
{
    SDL_Event event;
    SDL_zero(event);

    event.type = SDL_USEREVENT;
    event.user.type = SDL_USEREVENT;
    event.user.timestamp = SDL_GetTicks();
    event.user.code = id;
    event.user.data1 = data1;
    event.user.data2 = data2;

    SDL_PushEvent(&event);
}
