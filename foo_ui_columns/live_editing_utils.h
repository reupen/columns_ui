#pragma once

namespace cui::layout {

using LowLevelMouseHandler = std::function<bool(WPARAM msg, const MSLLHOOKSTRUCT& mllhs)>;
mmh::EventToken::Ptr add_low_level_mouse_handler(LowLevelMouseHandler handler);

} // namespace cui::layout
