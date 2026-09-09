#include "mod/MyMod.h"

#include "ll/api/Config.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/client/ClientExitLevelEvent.h"
#include "ll/api/event/client/ClientJoinLevelEvent.h"
#include "ll/api/event/input/KeyInputEvent.h"
#include "ll/api/event/input/MouseInputEvent.h"
#include "ll/api/mod/RegisterHelper.h"

#include "mc/deps/input/MouseAction.h"

#include "zoomify/Config.h"
#include "zoomify/Hooks.h"
#include "zoomify/ZoomManager.h"

#include <atomic>

namespace my_mod {

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() {
    auto& logger = getSelf().getLogger();

    mConfig = std::make_shared<zoomify::Config>();
    if (!ll::config::loadConfig(*mConfig, getSelf().getConfigDir() / u8"config.json")) {
        logger.warn("Failed to load config, using defaults");
    }
    return true;
}

bool MyMod::enable() {
    auto& logger = getSelf().getLogger();
    auto& bus    = ll::event::EventBus::getInstance();

    zoomify::ZoomManager::getInstance().setConfig(mConfig);
    zoomify::installHooks();

    // 原始键盘事件(诊断用,缩放状态由渲染帧内的 Win32 轮询驱动)。
    mKeyListener = bus.emplaceListener<ll::event::KeyInputEvent>([this](ll::event::KeyInputEvent& ev) {
        static std::atomic<int> count{0};
        if (count.fetch_add(1) < 3) {
            getSelf().getLogger().debug(
                "[Zoomify][diag] KeyInputEvent code={} down={}",
                ev.keyCode(),
                ev.isDown()
            );
        }
    });

    // 缩放时用滚轮调整倍率:滚轮以 ActionWheel 事件上报,data>0 为上滚、<0 为下滚。
    mMouseWheelListener = bus.emplaceListener<ll::event::MouseInputEvent>(
        [this](ll::event::MouseInputEvent& ev) {
            static std::atomic<bool> diagLogged{false};
            if (ev.actionButtonId() == ::MouseAction::ActionWheel && !diagLogged.exchange(true)) {
                getSelf().getLogger().debug("[Zoomify][diag] wheel event: data={}", ev.buttonData());
            }
            if (ev.actionButtonId() != ::MouseAction::ActionWheel) return;
            auto& manager = zoomify::ZoomManager::getInstance();
            if (!manager.isZooming()) return;
            if (!mConfig->allowScrollHotbarWhileZooming) ev.setCancelled(true);
            manager.adjustZoom(ev.buttonData() > 0 ? 1 : -1);
        }
    );

    // 关卡生命周期:退出关卡时复位缩放状态。
    mJoinLevelListener = bus.emplaceListener<ll::event::ClientJoinLevelEvent>(
        [](ll::event::ClientJoinLevelEvent&) {
            ll::mod::NativeMod::current()->getLogger().debug("[Zoomify] ClientJoinLevel");
            zoomify::ZoomManager::getInstance().setZooming(false);
        }
    );
    mExitLevelListener = bus.emplaceListener<ll::event::ClientExitLevelEvent>(
        [](ll::event::ClientExitLevelEvent&) {
            ll::mod::NativeMod::current()->getLogger().debug("[Zoomify] ClientExitLevel");
            zoomify::ZoomManager::getInstance().setZooming(false);
        }
    );

    logger.info("Zoomify enabled: press C to zoom, scroll wheel to adjust zoom level");
    return true;
}

bool MyMod::disable() {
    auto& bus = ll::event::EventBus::getInstance();
    for (auto listener : {std::ref(mKeyListener), std::ref(mMouseWheelListener), std::ref(mJoinLevelListener), std::ref(mExitLevelListener)}) {
        if (listener.get()) {
            bus.removeListener(listener.get());
            listener.get().reset();
        }
    }
    zoomify::uninstallHooks();
    zoomify::ZoomManager::getInstance().setConfig(nullptr);
    zoomify::ZoomManager::getInstance().setZooming(false);
    return true;
}

} // namespace my_mod

LL_REGISTER_MOD(my_mod::MyMod, my_mod::MyMod::getInstance());
