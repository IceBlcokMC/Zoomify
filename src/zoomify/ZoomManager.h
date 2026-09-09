#pragma once

#include <atomic>
#include <chrono>
#include <memory>

namespace zoomify {

struct Config;

class ZoomManager {
public:
    static ZoomManager& getInstance();

    void setConfig(std::shared_ptr<const Config> config);

    void setZooming(bool zooming);
    void toggleZooming() { setZooming(!isZooming()); }
    [[nodiscard]] bool isZooming() const { return mZooming.load(std::memory_order_relaxed); }

    /// @param direction +1 = 滚轮向上(放大),-1 = 滚轮向下(缩小)。
    void adjustZoom(int direction);

    /// 由渲染线程在 LevelRendererPlayer::getFov 每帧调用:
    /// 轮询按键状态(Win32,不依赖游戏输入系统)→ 推进缩放缓动 → 返回缩放后的 FOV。
    [[nodiscard]] float applyFov(float fov);

    /// 由 LocalPlayer::localPlayerTurn 调用:返回鼠标转向增量的缩放系数。
    [[nodiscard]] float turnScale() const;

private:
    ZoomManager() = default;

    void updateInputState();

    std::shared_ptr<const Config> mConfig;

    std::atomic<bool>  mZooming{false};
    std::atomic<float> mTargetZoom{4.0f};  // 目标倍率(每次开启缩放重置为配置值,滚轮可临时调整)
    std::atomic<float> mCurrentMod{1.0f};  // 当前 FOV 修饰系数 = 1/倍率,在修饰系数空间缓动
    bool               mPrevKeyDown{false};
    std::chrono::steady_clock::time_point mLastFrame{std::chrono::steady_clock::now()};
};

} // namespace zoomify
