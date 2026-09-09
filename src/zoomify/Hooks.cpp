#include "zoomify/Hooks.h"

#include "ll/api/memory/Hook.h"
#include "ll/api/mod/NativeMod.h"

#include "mc/client/player/LocalPlayer.h"
#include "mc/client/renderer/game/LevelRendererPlayer.h"
#include "mc/deps/core/math/Vec2.h"

#include "zoomify/ZoomManager.h"

#include <atomic>

namespace zoomify {
namespace {

auto& logger() { return ll::mod::NativeMod::current()->getLogger(); }

std::atomic<bool> getFovDiagLogged{false};
std::atomic<bool> turnDiagLogged{false};

// 缩放 FOV:在游戏自身的 FOV 计算(含望远镜、疾跑、死亡镜头等效果)之后
// 乘以当前的 FOV 修饰系数。修饰系数由 ZoomManager 按帧时间做指数缓动,得到平滑的缩放动画。
LL_TYPE_INSTANCE_HOOK(
    GetFovHook,
    ll::memory::HookPriority::Normal,
    LevelRendererPlayer,
    &LevelRendererPlayer::getFov,
    float,
    float alpha,
    bool enableVariableFov
) {
    float const fov = origin(alpha, enableVariableFov);
    if (!getFovDiagLogged.exchange(true)) {
        logger().debug(
            "[Zoomify][diag] getFov hook fired: alpha={:.3f} useFovOption={} fov={:.2f}",
            alpha,
            enableVariableFov,
            fov
        );
    }
    return ZoomManager::getInstance().applyFov(fov);
}

// 相对灵敏度:缩放时按比例降低鼠标转向灵敏度,倍率越高灵敏度越低,
// 手感与原版望远镜一致。系数 = pow(FOV 修饰系数, relativeSensitivity)。
LL_TYPE_INSTANCE_HOOK(
    LocalPlayerTurnHook,
    ll::memory::HookPriority::Normal,
    LocalPlayer,
    &LocalPlayer::localPlayerTurn,
    void,
    ::Vec2 const& deltaRot
) {
    if (!turnDiagLogged.exchange(true)) {
        logger().debug("[Zoomify][diag] localPlayerTurn hook fired");
    }
    float const scale = ZoomManager::getInstance().turnScale();
    if (scale == 1.0f) {
        origin(deltaRot);
    } else {
        origin(::Vec2{deltaRot.x * scale, deltaRot.y * scale});
    }
}

using HookList = ll::memory::HookRegistrar<GetFovHook, LocalPlayerTurnHook>;

std::unique_ptr<HookList> hooks;

} // namespace

void installHooks() {
    hooks = std::make_unique<HookList>();
    logger().debug("[Zoomify] hooks installed (getFov + localPlayerTurn)");
}
void uninstallHooks() {
    hooks.reset();
    logger().debug("[Zoomify] hooks uninstalled");
}

} // namespace zoomify
