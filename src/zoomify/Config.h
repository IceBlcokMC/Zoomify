#pragma once

namespace zoomify {

struct Config {
    int version = 1;

    // 按键(Windows VK Codes,0x43 = 'C';allowRemap 开启后也可在游戏内按键设置中重映射)
    int  zoomKeyCode    = 0x43;
    bool zoomToggleMode = false; // false:按住放大,松开恢复;true:按一次切换

    // 缩放
    float zoom          = 4.0f;  // 默认缩放倍率
    float maxZoom       = 30.0f; // 滚轮能调到的最大倍率
    float wheelZoomStep = 1.2f;  // 每格滚轮对倍率的乘数

    // 手感
    float zoomSmoothness      = 0.1f;  // 缩放动画时间常数(秒),越大动画越长越柔,0 = 瞬间
    float relativeSensitivity = 1.0f;  // 相对灵敏度:1.0 = 灵敏度随倍率等比降低(原版望远镜手感),0.0 = 不调整
    bool  allowScrollHotbarWhileZooming = false; // 缩放时滚轮是否仍然切换快捷栏
};

} // namespace zoomify
