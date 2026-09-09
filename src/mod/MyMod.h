#pragma once

#include <memory>

#include "ll/api/event/ListenerBase.h"
#include "ll/api/mod/NativeMod.h"

namespace zoomify {
struct Config;
}

namespace my_mod {

class MyMod {

public:
    static MyMod& getInstance();

    MyMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

private:
    ll::mod::NativeMod& mSelf;

    std::shared_ptr<zoomify::Config> mConfig;
    ll::event::ListenerPtr           mKeyListener;
    ll::event::ListenerPtr           mMouseWheelListener;
    ll::event::ListenerPtr           mJoinLevelListener;
    ll::event::ListenerPtr           mExitLevelListener;
};

} // namespace my_mod
