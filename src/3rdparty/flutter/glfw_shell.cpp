#include "glfw_shell.h"

namespace kdab {
void FlutterDesktopMessengerSendResponse(
    FlutterDesktopMessengerRef messenger,
    const FlutterDesktopMessageResponseHandle *handle,
    const uint8_t *data,
    size_t data_length)
{
    FlutterEngineSendPlatformMessageResponse(
        messenger->GetEngine()->flutter_engine, handle, data, data_length);
}
}
