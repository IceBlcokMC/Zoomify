#include "zoomify/ZoomManager.h"

#include "ll/api/mod/NativeMod.h"
#include "ll/api/service/TargetedBedrock.h"

#include "mc/client/game/ClientInstance.h"

#include "zoomify/Config.h"

#include <algorithm>
#include <cmath>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace zoomify {

namespace {
auto& logger() { return ll::mod::NativeMod::current()->getLogger(); }

float clampZoom(float zoom, float maxZoom) {
    return std::clamp(zoom, 1.0f, std::max(maxZoom, 1.0f));
}
} // namespace

ZoomManager& ZoomManager::getInstance() {
    static ZoomManager instance;
    return instance;
}

void ZoomManager::setConfig(std::shared_ptr<const Config> config) {
    mConfig = std::move(config);
    if (mConfig) {
        mTargetZoom.store(clampZoom(mConfig->zoom, mConfig->maxZoom), std::memory_order_relaxed);
    }
}

void ZoomManager::setZooming(bool zooming) {
    if (mZooming.exchange(zooming, std::memory_order_relaxed) == zooming) return;

    logger().debug("[Zoomify] zoom {}", zooming ? "on" : "off");
    if (zooming && mConfig) {
        // 每次开启缩放都重置为基础倍率,不沿用上一次滚轮调整的结果。
        float const base = clampZoom(mConfig->zoom, mConfig->maxZoom);
        mTargetZoom.store(base, std::memory_order_relaxed);
        logger().debug("[Zoomify] zoom level reset to {:.2f}x", base);
    }
}

void ZoomManager::adjustZoom(int direction) {
    float const step    = mConfig ? mConfig->wheelZoomStep : 1.2f;
    float const maxZoom = mConfig ? mConfig->maxZoom : 30.0f;
    if (step <= 0.0f) return;

    float target = mTargetZoom.load(std::memory_order_relaxed);
    target       = direction >= 0 ? target * step : target / step;
    mTargetZoom.store(clampZoom(target, maxZoom), std::memory_order_relaxed);
    logger().debug("[Zoomify] zoom level -> {:.2f}x", mTargetZoom.load(std::memory_order_relaxed));
}

void ZoomManager::updateInputState() {
    // 直接轮询 Win32 按键状态,不依赖游戏输入系统(映射系统/事件链都可能有时序或平台问题)。
    int const  vk   = mConfig ? mConfig->zoomKeyCode : 0x43;
    bool const down = (GetAsyncKeyState(vk) & 0x8000) != 0;

    // 界面门控:currentScreenShouldStealMouse() 表示当前界面"收走"鼠标——
    // 游戏 HUD(相机捕获鼠标)时为 true,聊天/背包/暂停菜单(界面占用鼠标)时为 false。
    // 因此仅在 HUD 持有鼠标时响应缩放;客户端不可用时放行(游戏内才有 FOV 调用)。
    bool gateOpen = true;
    if (auto client = ll::service::getClientInstance()) {
        gateOpen = client->currentScreenShouldStealMouse();
    }

    if (mConfig && mConfig->zoomToggleMode) {
        if (down && !mPrevKeyDown && gateOpen) {
            toggleZooming();
        }
    } else {
        setZooming(down && gateOpen);
    }
    mPrevKeyDown = down;
}

float ZoomManager::applyFov(float fov) {
    auto const now = std::chrono::steady_clock::now();
    double     dt  = std::chrono::duration<double>(now - mLastFrame).count();
    mLastFrame     = now;
    if (!(dt > 0.0) || dt > 0.1) dt = 0.1;

    updateInputState();

    // 在 FOV 修饰系数空间(= 1/倍率)做指数缓动:
    // 任意倍率下 FOV 的百分比变化速度都相同,高倍率不会出现生猛的俯冲。
    float const targetMod = isZooming() ? 1.0f / mTargetZoom.load(std::memory_order_relaxed) : 1.0f;
    float       mod       = mCurrentMod.load(std::memory_order_relaxed);

    if (mod != targetMod) {
        float const  smoothness = mConfig ? mConfig->zoomSmoothness : 0.1f;
        double const k         = smoothness > 0.0f ? 1.0 - std::exp(-dt / smoothness) : 1.0;
        mod += (targetMod - mod) * static_cast<float>(k);
        if (!isZooming() && std::abs(mod - 1.0f) < 1e-4f) mod = 1.0f;
        mCurrentMod.store(mod, std::memory_order_relaxed);
    }

    return fov * mod;
}

float ZoomManager::turnScale() const {
    float const mod = mCurrentMod.load(std::memory_order_relaxed);
    if (mod >= 1.0f) return 1.0f;

    float const relative = mConfig ? mConfig->relativeSensitivity : 1.0f;
    if (relative <= 0.0f) return 1.0f;
    return std::pow(mod, relative);
}

} // namespace zoomify
