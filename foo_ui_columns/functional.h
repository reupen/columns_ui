#pragma once

namespace std {

template <>
struct hash<GUID> {
    size_t operator()(const GUID& value) const noexcept { return mmh::GUIDHasher()(value); }
};

} // namespace std
