#pragma once

#include <cstddef>

namespace worldanalysis::sdk::offsets {

namespace VTable {
// dot60 primary ClientInstance vtable; see compatibility/dot60-bindings.json.
inline constexpr std::size_t ClientInstance_getRegion = 30;
inline constexpr std::size_t ClientInstance_getCamera = 203;
inline constexpr std::size_t ClientInstance_getGuiData = 233;
// Slot 19 forwards to Dimension's getId accessor (now slot 4).
inline constexpr std::size_t BlockSource_getDimensionId = 19;
// Minecraft 26.45 Level::getTime() is virtual slot 133 (byte offset 0x428).
// The exact target contains multiple call sites that invoke this slot and
// immediately normalize the returned integer against the 24,000-tick day.
inline constexpr std::size_t Level_getTime = 133;
inline constexpr std::size_t RenderMaterialGroup_getMaterial = 2;
inline constexpr std::size_t HoverTextRendererRenderHoverBox = 17;
inline constexpr std::size_t MinecraftUIRenderContextGetLineLength = 2;
inline constexpr std::size_t MinecraftUIRenderContextDrawText = 6;
// A second drawText overload was inserted at slot 7 in dot60.
inline constexpr std::size_t MinecraftUIRenderContextFlushText = 8;
inline constexpr std::size_t MinecraftUIRenderContextDrawImage = 9;
inline constexpr std::size_t MinecraftUIRenderContextFlushImages = 11;
inline constexpr std::size_t MinecraftUIRenderContextFillRectangle = 17;
inline constexpr std::size_t MinecraftUIRenderContextGetTexture = 33;
inline constexpr std::size_t ClientInstanceGetMinecraftGame = 82;
inline constexpr std::size_t ItemGetMaxDamage = 37;
}

namespace ClientInstance {
inline constexpr std::size_t mLevelRenderer = 0x1B8;
}

namespace GuiData {
// Minecraft 26.45 keeps the physical and GUI-scaled viewport sizes in the
// modern GuiData layout.  UiOverlay also validates the older adjacent layout
// at runtime before using either pair, so a bad/stale pointer is never trusted.
inline constexpr std::size_t mScreenSize = 0x40;
inline constexpr std::size_t mScreenSizeScaled = 0x50;
inline constexpr std::size_t mLegacyScreenSize = 0x30;
inline constexpr std::size_t mLegacyScreenSizeScaled = 0x40;
}

}
