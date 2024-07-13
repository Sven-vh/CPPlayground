#pragma once

static inline bool ImGuiUintColorEdit(const char* label, uint& color) {
	bool changed = false;
	float4 fBackgroundColor = RGB8_to_RGBF32(color);
	changed |= ImGui::ColorEdit4(label, &fBackgroundColor.x);
	color = RGBF32_to_RGB8(&fBackgroundColor);
	return changed;
}