// Autor: Calyel Junio de Andrade dos Santos
// Descrição: Implementação da classe VideoPlayer

#include "ffmpeg_player/VideoPlayer.hpp"
#include "ffmpeg_player/ProcessRunner.hpp"

#include <filesystem>
#include <sstream>

namespace ffmpeg_player {

void VideoPlayer::validateFileExists(const std::string& filePath) {
    if (filePath.empty()) {
        throw FileNotFoundException(filePath);
    }
    std::error_code ec;
    if (!std::filesystem::exists(filePath, ec) || !std::filesystem::is_regular_file(filePath, ec)) {
        throw FileNotFoundException(filePath);
    }
}

std::string VideoPlayer::buildProbeCommand(const std::string& filePath) {
    return "ffprobe -v error -show_format -show_streams " + ProcessRunner::escapeShellArg(filePath);
}

std::string VideoPlayer::buildPlayCommand(const std::string& filePath, const PlaybackOptions& options) {
    std::ostringstream cmd;
    cmd << "ffplay -hide_banner -loglevel error";

    if (options.autoExit) {
        cmd << " -autoexit";
    }
    if (options.fullscreen) {
        cmd << " -fs";
    }
    if (options.noAudio) {
        cmd << " -an";
    }
    if (options.noVideo) {
        cmd << " -vn";
    }
    if (options.loopCount != 1 && options.loopCount >= 0) {
        cmd << " -loop " << options.loopCount;
    }
    if (!options.windowTitle.empty()) {
        cmd << " -window_title " << ProcessRunner::escapeShellArg(options.windowTitle);
    }

    cmd << " -i " << ProcessRunner::escapeShellArg(filePath);
    return cmd.str();
}

MediaInfo VideoPlayer::probe(const std::string& filePath) const {
    validateFileExists(filePath);

    std::string command = buildProbeCommand(filePath);
    CommandResult res = ProcessRunner::capture(command);

    if (res.exitCode != 0 || res.output.empty()) {
        throw MediaException("Falha ao extrair metadados com 'ffprobe' para o arquivo '" + filePath + "'.");
    }

    MediaInfo info = MediaInfo::parseFfprobeOutput(res.output, filePath);
    if (!info.hasVideo && !info.hasAudio && info.formatName.empty()) {
        throw MediaException("Nenhuma stream de midia valida encontrada no arquivo '" + filePath + "'.");
    }

    return info;
}

std::string VideoPlayer::getVideoCodec(const std::string& filePath) const {
    MediaInfo info = probe(filePath);
    if (!info.hasVideo || info.videoCodec.empty()) {
        throw MediaException("Nenhum codec de video encontrado para o arquivo '" + filePath + "'.");
    }
    return info.videoCodec;
}

void VideoPlayer::printCodec(const std::string& filePath, std::ostream& out) const {
    MediaInfo info = probe(filePath);
    if (info.hasVideo) {
        out << "Codec de Video : " << info.videoCodec;
        if (!info.videoCodecLong.empty()) {
            out << " (" << info.videoCodecLong << ")";
        }
        out << "\n";
    } else {
        out << "Codec de Video : N/A (sem stream de video)\n";
    }

    if (info.hasAudio) {
        out << "Codec de Audio : " << info.audioCodec;
        if (!info.audioCodecLong.empty()) {
            out << " (" << info.audioCodecLong << ")";
        }
        out << "\n";
    }
}

void VideoPlayer::printMediaInfo(const std::string& filePath, bool asJson, std::ostream& out) const {
    MediaInfo info = probe(filePath);
    if (asJson) {
        out << info.toJsonString() << "\n";
    } else {
        out << info.toPrettyString() << "\n";
    }
}

void VideoPlayer::play(const std::string& filePath, const PlaybackOptions& options) const {
    validateFileExists(filePath);

    std::string command = buildPlayCommand(filePath, options);
    int result = ProcessRunner::execute(command);

    if (result != 0) {
        throw PlaybackException(
            "Erro ao reproduzir o video '" + filePath +
            "'. Verifique se o 'ffplay' esta instalado e se ha um servidor grafico ativo.");
    }
}

void VideoPlayer::handleException(const std::exception& e, std::ostream& errOut) {
    errOut << "[ERRO] " << e.what() << "\n";
}

} // namespace ffmpeg_player
