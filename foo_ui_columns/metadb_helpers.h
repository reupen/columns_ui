#pragma once

namespace cui::helpers {

double calculate_tracks_total_length(auto& tracks)
{
    const auto metadb_v2_api = metadb_v2::tryGet();

    if (!metadb_v2_api.is_valid())
        return metadb_handle_list_helper::calc_total_duration(tracks);

    std::vector<double> lengths(tracks.size());
    metadb_v2_api->queryMultiParallel_(tracks, [&lengths](size_t index, const metadb_v2_rec_t& rec) {
        if (rec.info.is_valid())
            lengths[index] = rec.info->info().get_length();
    });
    return std::reduce(lengths.begin(), lengths.end());
}

} // namespace cui::helpers
