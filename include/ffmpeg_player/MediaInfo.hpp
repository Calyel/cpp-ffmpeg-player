// Autor: Calyel Junio de Andrade dos Santos
// Descrição: Estrutura e parser de metadados de mídia extraídos via ffprobe

#ifndef FFMPEG_PLAYER_MEDIA_INFO_HPP
#define FFMPEG_PLAYER_MEDIA_INFO_HPP

#include <cstdint>
#include <string>

namespace ffmpeg_player {

/**
 * @brief Estrutura que armazena metadados detalhados de container, vídeo e áudio.
 */
struct MediaInfo {
    // Informações de Container / Formato
    std::string filePath;
    std::string formatName;
    std::string formatLongName;
    double durationSeconds = 0.0;
    std::int64_t fileSizeBytes = 0;
    std::int64_t totalBitrate = 0;
    int streamCount = 0;

    // Informações da Stream de Vídeo
    bool hasVideo = false;
    std::string videoCodec;
    std::string videoCodecLong;
    std::string videoProfile;
    int width = 0;
    int height = 0;
    std::string displayAspectRatio;
    std::string pixelFormat;
    double frameRate = 0.0;
    std::int64_t videoBitrate = 0;
    std::int64_t totalFrames = 0;

    // Informações da Stream de Áudio
    bool hasAudio = false;
    std::string audioCodec;
    std::string audioCodecLong;
    int sampleRate = 0;
    int channels = 0;
    std::string channelLayout;
    std::int64_t audioBitrate = 0;

    /**
     * @brief Analisa a saída bruta do comando `ffprobe -show_format -show_streams`.
     * @param rawOutput Saída textual do ffprobe.
     * @param filePath Caminho original do arquivo analisado.
     * @return Objeto MediaInfo preenchido.
     */
    static MediaInfo parseFfprobeOutput(const std::string& rawOutput, const std::string& filePath = "");

    /**
     * @brief Converte uma fração de taxa de quadros (ex.: "30000/1001" ou "30/1") em double.
     */
    static double parseFrameRateFraction(const std::string& fraction);

    /**
     * @brief Formata a duração em segundos para o padrão HH:MM:SS.cc.
     */
    std::string formatDuration() const;

    /**
     * @brief Formata o tamanho do arquivo em bytes para unidade legível (B, KB, MB, GB).
     */
    std::string formatFileSize() const;

    /**
     * @brief Formata a resolução de vídeo (ex.: "640x360 (16:9)").
     */
    std::string formatResolution() const;

    /**
     * @brief Gera um relatório formatado para exibição no terminal.
     */
    std::string toPrettyString() const;

    /**
     * @brief Gera uma representação em formato JSON dos metadados.
     */
    std::string toJsonString() const;
};

} // namespace ffmpeg_player

#endif // FFMPEG_PLAYER_MEDIA_INFO_HPP
