#include "glfw_shell.h"
#include "flutter_texture_registrar.h"

#include <iostream>

void FlutterDesktopMessengerSetCallback(FlutterDesktopMessengerRef messenger,
                                        const char *channel,
                                        FlutterDesktopMessageCallback callback,
                                        void *user_data)
{
    messenger->GetEngine()->message_dispatcher->SetMessageCallback(
        channel, callback, user_data);
}

void FlutterDesktopMessengerRelease(FlutterDesktopMessengerRef messenger)
{
    messenger->Release();
}

int64_t FlutterDesktopTextureRegistrarRegisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    const FlutterDesktopTextureInfo *texture_info)
{
    std::cerr << "GLFW Texture support is not implemented yet." << std::endl;
    return -1;
}

void FlutterDesktopTextureRegistrarUnregisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id,
    void (*callback)(void *user_data),
    void *user_data)
{
    std::cerr << "GLFW Texture support is not implemented yet." << std::endl;
}

bool FlutterDesktopTextureRegistrarMarkExternalTextureFrameAvailable(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id)
{
    std::cerr << "GLFW Texture support is not implemented yet." << std::endl;
    return false;
}


void FlutterDesktopMessengerUnlock(FlutterDesktopMessengerRef messenger)
{
    messenger->GetMutex().unlock();
}

bool FlutterDesktopMessengerIsAvailable(FlutterDesktopMessengerRef messenger)
{
    return messenger->GetEngine() != nullptr;
}

void FlutterDesktopMessengerSendResponse(
    FlutterDesktopMessengerRef messenger,
    const FlutterDesktopMessageResponseHandle *handle,
    const uint8_t *data,
    size_t data_length)
{
    FlutterEngineSendPlatformMessageResponse(
        messenger->GetEngine()->flutter_engine, handle, data, data_length);
}

FlutterDesktopMessengerRef FlutterDesktopMessengerAddRef(
    FlutterDesktopMessengerRef messenger)
{
    messenger->AddRef();
    return messenger;
}

FlutterDesktopMessengerRef FlutterDesktopMessengerLock(
    FlutterDesktopMessengerRef messenger)
{
    messenger->GetMutex().lock();
    return messenger;
}

bool FlutterDesktopMessengerSendWithReply(FlutterDesktopMessengerRef messenger,
                                          const char *channel,
                                          const uint8_t *message,
                                          const size_t message_size,
                                          const FlutterDesktopBinaryReply reply,
                                          void *user_data)
{
    FlutterPlatformMessageResponseHandle *response_handle = nullptr;
    if (reply != nullptr && user_data != nullptr) {
        FlutterEngineResult result = FlutterPlatformMessageCreateResponseHandle(
            messenger->GetEngine()->flutter_engine, reply, user_data,
            &response_handle);
        if (result != kSuccess) {
            std::cout << "Failed to create response handle\n";
            return false;
        }
    }

    FlutterPlatformMessage platform_message = {
        sizeof(FlutterPlatformMessage),
        channel,
        message,
        message_size,
        response_handle,
    };

    FlutterEngineResult message_result = FlutterEngineSendPlatformMessage(
        messenger->GetEngine()->flutter_engine, &platform_message);

    if (response_handle != nullptr) {
        FlutterPlatformMessageReleaseResponseHandle(
            messenger->GetEngine()->flutter_engine, response_handle);
    }

    return message_result == kSuccess;
}

bool FlutterDesktopMessengerSend(FlutterDesktopMessengerRef messenger,
                                 const char *channel,
                                 const uint8_t *message,
                                 const size_t message_size)
{
    return FlutterDesktopMessengerSendWithReply(messenger, channel, message,
                                                message_size, nullptr, nullptr);
}
