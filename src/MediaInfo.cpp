// Autor: Calyel Junio de Andrade dos Santos
// Descrição: Implementação do parser e formatador de metadados (MediaInfo)

#include "ffmpeg_player/MediaInfo.hpp"

#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>

namespace ffmpeg_player {

namespace {

std::string trim(const std::string& str) {
    const auto first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

int safeStoi(const std::string& val, int defaultVal = 0) {
    if (val.empty() || val == "N/A") {
        return defaultVal;
    }
    try {
        return std::stoi(val);
    } catch (...) {
        return defaultVal;
    }
}

std::int64_t safeStoll(const std::string& val, std::int64_t defaultVal = 0) {
    if (val.empty() || val == "N/A") {
        return defaultVal;
    }
    try {
        return std::stoll(val);
    } catch (...) {
        return defaultVal;
    }
}

double safeStod(const std::string& val, double defaultVal = 0.0) {
    if (val.empty() || val == "N/A") {
        return defaultVal;
    }
    try {
        return std::stod(val);
    } catch (...) {
        return defaultVal;
    }
}

std::string escapeJson(const std::string& input) {
    std::ostringstream ss;
    for (char c : input) {
        switch (c) {
            case '"':  ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default:   ss << c; break;
        }
    }
    return ss.str();
}

void applyStreamMap(const std::map<std::string, std::string>& kv, MediaInfo& info) {
    auto itType = kv.find("codec_type");
    if (itType == kv.end()) {
        return;
    }

    const std::string& type = itType->second;
    auto get = [&](const std::string& key) -> std::string {
        auto it = kv.find(key);
        return (it != kv.end() && it->second != "N/A") ? it->second : "";
    };

    if (type == "video" && !info.hasVideo) {
        info.hasVideo = true;
        info.videoCodec = get("codec_name");
        info.videoCodecLong = get("codec_long_name");
        info.videoProfile = get("profile");
        info.width = safeStoi(get("width"));
        info.height = safeStoi(get("height"));
        info.displayAspectRatio = get("display_aspect_ratio");
        info.pixelFormat = get("pix_fmt");

        std::string avgFps = get("avg_frame_rate");
        std::string rFps = get("r_frame_rate");
        info.frameRate = MediaInfo::parseFrameRateFraction(!avgFps.empty() ? avgFps : rFps);

        info.videoBitrate = safeStoll(get("bit_rate"));
        info.totalFrames = safeStoll(get("nb_frames"));
        if (info.durationSeconds <= 0.0) {
            info.durationSeconds = safeStod(get("duration"));
        }
    } else if (type == "audio" && !info.hasAudio) {
        info.hasAudio = true;
        info.audioCodec = get("codec_name");
        info.audioCodecLong = get("codec_long_name");
        info.sampleRate = safeStoi(get("sample_rate"));
        info.channels = safeStoi(get("channels"));
        info.channelLayout = get("channel_layout");
        info.audioBitrate = safeStoll(get("bit_rate"));
    }
}

void applyFormatMap(const std::map<std::string, std::string>& kv, MediaInfo& info) {
    auto get = [&](const std::string& key) -> std::string {
        auto it = kv.find(key);
        return (it != kv.end() && it->second != "N/A") ? it->second : "";
    };

    if (info.filePath.empty()) {
        info.filePath = get("filename");
    }
    info.formatName = get("format_name");
    info.formatLongName = get("format_long_name");
    double fmtDuration = safeStod(get("duration"));
    if (fmtDuration > 0.0) {
        info.durationSeconds = fmtDuration;
    }
    info.fileSizeBytes = safeStoll(get("size"));
    info.totalBitrate = safeStoll(get("bit_rate"));
    info.streamCount = safeStoi(get("nb_streams"));
}

} // namespace

double MediaInfo::parseFrameRateFraction(const std::string& fraction) {
    if (fraction.empty() || fraction == "0/0" || fraction == "N/A") {
        return 0.0;
    }
    auto slashPos = fraction.find('/');
    if (slashPos == std::string::npos) {
        return safeStod(fraction);
    }
    double num = safeStod(fraction.substr(0, slashPos));
    double den = safeStod(fraction.substr(slashPos + 1));
    if (den <= 0.0) {
        return 0.0;
    }
    return num / den;
}

MediaInfo MediaInfo::parseFfprobeOutput(const std::string& rawOutput, const std::string& filePath) {
    MediaInfo info;
    info.filePath = filePath;

    std::istringstream stream(rawOutput);
    std::string line;
    std::string currentSection;
    std::map<std::string, std::string> currentMap;

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        if (line == "[STREAM]") {
            currentSection = "STREAM";
            currentMap.clear();
        } else if (line == "[/STREAM]") {
            if (currentSection == "STREAM") {
                applyStreamMap(currentMap, info);
            }
            currentSection.clear();
            currentMap.clear();
        } else if (line == "[FORMAT]") {
            currentSection = "FORMAT";
            currentMap.clear();
        } else if (line == "[/FORMAT]") {
            if (currentSection == "FORMAT") {
                applyFormatMap(currentMap, info);
            }
            currentSection.clear();
            currentMap.clear();
        } else if (!currentSection.empty()) {
            auto eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = trim(line.substr(0, eqPos));
                std::string val = trim(line.substr(eqPos + 1));
                currentMap[key] = val;
            }
        }
    }

    return info;
}

std::string MediaInfo::formatDuration() const {
    if (durationSeconds <= 0.0) {
        return "00:00:00.00";
    }
    int totalCentiseconds = static_cast<int>(std::round(durationSeconds * 100.0));
    int hours = totalCentiseconds / 360000;
    int minutes = (totalCentiseconds % 360000) / 6000;
    int seconds = (totalCentiseconds % 6000) / 100;
    int centiseconds = totalCentiseconds % 100;

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << seconds << "."
        << std::setw(2) << centiseconds;
    return oss.str();
}

std::string MediaInfo::formatFileSize() const {
    if (fileSizeBytes <= 0) {
        return "0 B";
    }
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(fileSizeBytes);

    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        ++unitIndex;
    }

    std::ostringstream oss;
    if (unitIndex == 0) {
        oss << fileSizeBytes << " B";
    } else {
        oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    }
    return oss.str();
}

std::string MediaInfo::formatResolution() const {
    if (width <= 0 || height <= 0) {
        return "N/A";
    }
    std::ostringstream oss;
    oss << width << "x" << height;
    if (!displayAspectRatio.empty() && displayAspectRatio != "0:1") {
        oss << " (" << displayAspectRatio << ")";
    }
    return oss.str();
}

std::string MediaInfo::toPrettyString() const {
    std::ostringstream oss;
    oss << "============================================================\n";
    oss << "                 RELATORIO DE MIDIA (FFPROBE)               \n";
    oss << "============================================================\n";
    oss << " [Container / Arquivo]\n";
    oss << "  Arquivo       : " << (filePath.empty() ? "N/A" : filePath) << "\n";
    oss << "  Formato       : " << (formatLongName.empty() ? formatName : formatLongName + " (" + formatName + ")") << "\n";
    oss << "  Duracao       : " << formatDuration() << " (" << std::fixed << std::setprecision(2) << durationSeconds << " s)\n";
    oss << "  Tamanho       : " << formatFileSize() << " (" << fileSizeBytes << " bytes)\n";
    if (totalBitrate > 0) {
        oss << "  Bitrate Total : " << (totalBitrate / 1000) << " kbps\n";
    }
    oss << "  Streams       : " << streamCount << "\n";

    if (hasVideo) {
        oss << "\n [Stream de Video]\n";
        oss << "  Codec         : " << videoCodec;
        if (!videoCodecLong.empty()) {
            oss << " (" << videoCodecLong << ")";
        }
        oss << "\n";
        if (!videoProfile.empty()) {
            oss << "  Perfil        : " << videoProfile << "\n";
        }
        oss << "  Resolucao     : " << formatResolution() << "\n";
        if (!pixelFormat.empty()) {
            oss << "  Pixel Format  : " << pixelFormat << "\n";
        }
        if (frameRate > 0.0) {
            oss << "  Taxa Quadros  : " << std::fixed << std::setprecision(2) << frameRate << " fps\n";
        }
        if (videoBitrate > 0) {
            oss << "  Bitrate Video : " << (videoBitrate / 1000) << " kbps\n";
        }
        if (totalFrames > 0) {
            oss << "  Total Quadros : " << totalFrames << "\n";
        }
    }

    if (hasAudio) {
        oss << "\n [Stream de Audio]\n";
        oss << "  Codec         : " << audioCodec;
        if (!audioCodecLong.empty()) {
            oss << " (" << audioCodecLong << ")";
        }
        oss << "\n";
        if (sampleRate > 0) {
            oss << "  Sample Rate   : " << sampleRate << " Hz\n";
        }
        if (channels > 0) {
            oss << "  Canais        : " << channels;
            if (!channelLayout.empty()) {
                oss << " (" << channelLayout << ")";
            }
            oss << "\n";
        }
        if (audioBitrate > 0) {
            oss << "  Bitrate Audio : " << (audioBitrate / 1000) << " kbps\n";
        }
    }

    oss << "============================================================";
    return oss.str();
}

std::string MediaInfo::toJsonString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "{\n";
    oss << "  \"file_path\": \"" << escapeJson(filePath) << "\",\n";
    oss << "  \"format\": {\n";
    oss << "    \"name\": \"" << escapeJson(formatName) << "\",\n";
    oss << "    \"long_name\": \"" << escapeJson(formatLongName) << "\",\n";
    oss << "    \"duration_seconds\": " << durationSeconds << ",\n";
    oss << "    \"duration_formatted\": \"" << formatDuration() << "\",\n";
    oss << "    \"size_bytes\": " << fileSizeBytes << ",\n";
    oss << "    \"bitrate_bps\": " << totalBitrate << ",\n";
    oss << "    \"stream_count\": " << streamCount << "\n";
    oss << "  },\n";
    oss << "  \"video\": {\n";
    oss << "    \"present\": " << (hasVideo ? "true" : "false") << ",\n";
    oss << "    \"codec\": \"" << escapeJson(videoCodec) << "\",\n";
    oss << "    \"codec_long\": \"" << escapeJson(videoCodecLong) << "\",\n";
    oss << "    \"profile\": \"" << escapeJson(videoProfile) << "\",\n";
    oss << "    \"width\": " << width << ",\n";
    oss << "    \"height\": " << height << ",\n";
    oss << "    \"aspect_ratio\": \"" << escapeJson(displayAspectRatio) << "\",\n";
    oss << "    \"pixel_format\": \"" << escapeJson(pixelFormat) << "\",\n";
    oss << "    \"frame_rate\": " << std::setprecision(2) << frameRate << ",\n";
    oss << "    \"bitrate_bps\": " << videoBitrate << ",\n";
    oss << "    \"total_frames\": " << totalFrames << "\n";
    oss << "  },\n";
    oss << "  \"audio\": {\n";
    oss << "    \"present\": " << (hasAudio ? "true" : "false") << ",\n";
    oss << "    \"codec\": \"" << escapeJson(audioCodec) << "\",\n";
    oss << "    \"codec_long\": \"" << escapeJson(audioCodecLong) << "\",\n";
    oss << "    \"sample_rate_hz\": " << sampleRate << ",\n";
    oss << "    \"channels\": " << channels << ",\n";
    oss << "    \"channel_layout\": \"" << escapeJson(channelLayout) << "\",\n";
    oss << "    \"bitrate_bps\": " << audioBitrate << "\n";
    oss << "  }\n";
    oss << "}";
    return oss.str();
}

} // namespace ffmpeg_player
