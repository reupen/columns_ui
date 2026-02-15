#pragma once

namespace cui::dark {

[[nodiscard]] bool is_active_ui_dark(bool allow_cui_fallback = true);
[[nodiscard]] mmh::EventToken::Ptr add_status_callback(std::function<void()> callback, bool allow_cui_fallback = true);

} // namespace cui::dark
