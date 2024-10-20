#pragma once

#include "flutter_messenger.h"
#include "flutter_embedder.h"

// Converts a FlutterPlatformMessage to an equivalent FlutterDesktopMessage.
static FlutterDesktopMessage ConvertToDesktopMessage(
    const FlutterPlatformMessage &engine_message)
{
    FlutterDesktopMessage message = {};
    message.struct_size = sizeof(message);
    message.channel = engine_message.channel;
    message.message = engine_message.message;
    message.message_size = engine_message.message_size;
    message.response_handle = engine_message.response_handle;
    return message;
}
