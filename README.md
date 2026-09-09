# Zoomify

A hold-to-zoom mod for Minecraft Bedrock (Windows, LeviLamina client). Hold **C** to zoom in, release to zoom out, scroll wheel to change the zoom level while zooming.

Minecraft 基岩版(Windows,LeviLamina 客户端)的按住缩放模组:按住 **C** 放大视角,松开恢复,缩放时滚轮调整倍率。

## Features / 功能

- Hold a key to zoom, release to restore; optional toggle mode / 按住缩放、松开恢复,可选切换模式
- Smooth animated zoom; the animation speed feels the same at every zoom level / 平滑的缩放动画,任意倍率下动画节奏一致
- Scroll wheel adjusts the zoom level while zooming, without switching hotbar slots / 缩放时滚轮调整倍率,不会切换快捷栏;每次按下缩放键,倍率重置为基础值
- Mouse sensitivity scales down with the zoom level, so aiming stays precise / 鼠标灵敏度随倍率等比降低,高倍率下依然好瞄准
- Ignored while chat, inventory or the pause menu is open / 聊天、背包、暂停菜单打开时不响应
- Compatible with the vanilla spyglass and other FOV effects / 与原版望远镜及其他 FOV 效果兼容

## Config / 配置

`mods/Zoomify/config/config.json`, created on first launch. / 首次启动自动生成。

| Field | Default | Description |
| --- | --- | --- |
| `zoomKeyCode` | `67` | Zoom key as a Windows VK code (`0x43` = C). Restart the game after changing. / 缩放键,Windows VK 码,改后重启生效 |
| `zoomToggleMode` | `false` | `false`: zoom while held. `true`: press once to toggle. / 按住缩放或按一次切换 |
| `zoom` | `4.0` | Base zoom level, applied every time zoom starts. / 基础倍率,每次开启缩放时重置 |
| `maxZoom` | `30.0` | Upper limit reachable with the scroll wheel. / 滚轮可调的最大倍率 |
| `wheelZoomStep` | `1.2` | Multiplier applied per scroll wheel step. / 每格滚轮的倍率乘数 |
| `zoomSmoothness` | `0.1` | Animation time constant in seconds; higher = softer, `0` = instant. / 动画时间常数(秒),越大越柔,0 为瞬间 |
| `relativeSensitivity` | `1.0` | `1.0` sensitivity scales fully with zoom, `0.0` no scaling. / 1.0 为灵敏度完全随倍率缩放,0.0 不缩放 |
| `allowScrollHotbarWhileZooming` | `false` | Whether the wheel still switches hotbar slots while zooming. / 缩放时滚轮是否仍切换快捷栏 |
