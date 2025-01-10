// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the src/3rdparty/flutter/LICENSE file.

#pragma once

#include "incoming_message_dispatcher.h"
#include "embedder.h"

#include <mutex>
#include <atomic>
#include <memory>

namespace flutter {
class BinaryMessengerImpl;
}

struct FlutterDesktopMessenger;

typedef struct _FlutterEngineAOTData *FlutterEngineAOTData;

// Custom deleter for FlutterEngineAOTData.
struct AOTDataDeleter
{
    void operator()(FlutterEngineAOTData aot_data)
    {
        FlutterEngineCollectAOTData(aot_data);
    }
};

typedef struct
{
} Dart_LoadedElf;

extern void Dart_UnloadELF(Dart_LoadedElf *loaded);
struct LoadedElfDeleter
{
    void operator()(Dart_LoadedElf *elf)
    {
        if (elf) {
            ::Dart_UnloadELF(elf);
        }
    }
};

using UniqueLoadedElf = std::unique_ptr<Dart_LoadedElf, LoadedElfDeleter>;
struct _FlutterEngineAOTData
{
    UniqueLoadedElf loaded_elf = nullptr;
    const uint8_t *vm_snapshot_data = nullptr;
    const uint8_t *vm_snapshot_instrs = nullptr;
    const uint8_t *vm_isolate_data = nullptr;
    const uint8_t *vm_isolate_instrs = nullptr;
};

using UniqueAotDataPtr = std::unique_ptr<_FlutterEngineAOTData, AOTDataDeleter>;
/// Maintains one ref on the FlutterDesktopMessenger's internal reference count.
using FlutterDesktopMessengerReferenceOwner =
    std::unique_ptr<FlutterDesktopMessenger,
                    decltype(&FlutterDesktopMessengerRelease)>;

struct FlutterDesktopEngineState
{
    // The plugin messenger handle given to API clients.
    FlutterDesktopMessengerReferenceOwner messenger = {
        nullptr, [](FlutterDesktopMessengerRef ref) {}
    };

    _FlutterEngine *flutter_engine = nullptr;

    // Message dispatch manager for messages from the Flutter engine.
    std::unique_ptr<flutter::IncomingMessageDispatcher>
        message_dispatcher;

    flutter::BinaryMessengerImpl *binaryMessenger;

    // // The plugin registrar handle given to API clients.
    // std::unique_ptr<FlutterDesktopPluginRegistrar> plugin_registrar;

    // // The plugin registrar managing internal plugins.
    // std::unique_ptr<flutter::PluginRegistrar> internal_plugin_registrar;

    // // Handler for the flutter/platform channel.
    // std::unique_ptr<flutter::PlatformHandler> platform_handler;

    // // The controller associated with this engine instance, if any.
    // // This will always be null for a headless engine.
    // FlutterDesktopWindowControllerState *window_controller = nullptr;

    // AOT data for this engine instance, if applicable.
    UniqueAotDataPtr aot_data = nullptr;
};


// copied from flutter_glfw.cc
// State associated with the messenger used to communicate with the engine.
struct FlutterDesktopMessenger
{
    FlutterDesktopMessenger() = default;

    /// Increments the reference count.
    ///
    /// Thread-safe.
    void AddRef()
    {
        ref_count_.fetch_add(1);
    }

    /// Decrements the reference count and deletes the object if the count has
    /// gone to zero.
    ///
    /// Thread-safe.
    void Release()
    {
        int32_t old_count = ref_count_.fetch_sub(1);
        if (old_count <= 1) {
            delete this;
        }
    }

    /// Getter for the engine field.
    FlutterDesktopEngineState *GetEngine() const
    {
        return engine_;
    }

    /// Setter for the engine field.
    /// Thread-safe.
    void SetEngine(FlutterDesktopEngineState *engine)
    {
        std::scoped_lock lock(mutex_);
        engine_ = engine;
    }

    /// Returns the mutex associated with the |FlutterDesktopMessenger|.
    ///
    /// This mutex is used to synchronize reading or writing state inside the
    /// |FlutterDesktopMessenger| (ie |engine_|).
    std::mutex &GetMutex()
    {
        return mutex_;
    }

    FlutterDesktopMessenger(const FlutterDesktopMessenger &value) = delete;
    FlutterDesktopMessenger &operator=(const FlutterDesktopMessenger &value) =
        delete;

private:
    // The engine that backs this messenger.
    FlutterDesktopEngineState *engine_;
    std::atomic<int32_t> ref_count_ = 0;
    std::mutex mutex_;
};


// Converts a FlutterPlatformMessage to an equivalent FlutterDesktopMessage.
inline FlutterDesktopMessage ConvertToDesktopMessage(
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
