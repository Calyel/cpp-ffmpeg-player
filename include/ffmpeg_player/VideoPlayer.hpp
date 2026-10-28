// Autor: Calyel Junio de Andrade dos Santos
// Descrição: Classe principal para inspeção e reprodução de vídeos com FFmpeg

#ifndef FFMPEG_PLAYER_VIDEO_PLAYER_HPP
#define FFMPEG_PLAYER_VIDEO_PLAYER_HPP

#include "ffmpeg_player/MediaInfo.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace ffmpeg_player {

/**
 * @brief Exceção base para erros de manipulação de mídia.
 */
class MediaException : public std::runtime_error {
public:
    explicit MediaException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Exceção lançada quando o arquivo de mídia não existe ou é inacessível.
 */
class FileNotFoundException : public MediaException {
public:
    explicit FileNotFoundException(const std::string& filePath)
        : MediaException("O arquivo '" + filePath + "' nao foi encontrado ou nao e um arquivo regular."),
          filePath_(filePath) {}

    const std::string& filePath() const noexcept { return filePath_; }

private:
    std::string filePath_;
};

/**
 * @brief Exceção lançada quando ffprobe ou ffplay falha na execução.
 */
class PlaybackException : public MediaException {
public:
    explicit PlaybackException(const std::string& message)
        : MediaException(message) {}
};

/**
 * @brief Opções configuráveis para reprodução de mídia via ffplay.
 */
struct PlaybackOptions {
    bool autoExit = true;       ///< Fecha o player automaticamente ao fim do vídeo (-autoexit)
    bool fullscreen = false;    ///< Inicia em tela cheia (-fs)
    bool noAudio = false;       ///< Desativa a reprodução de áudio (-an)
    bool noVideo = false;       ///< Desativa a exibição de vídeo (-vn)
    int loopCount = 1;          ///< Número de repetições (0 = infinito) (-loop)
    std::string windowTitle;    ///< Título personalizado da janela (-window_title)
};

/**
 * @brief Classe responsável pela inspeção de metadados (ffprobe) e reprodução (ffplay).
 */
class VideoPlayer {
public:
    VideoPlayer() = default;

    /**
     * @brief Valida se o arquivo existe no sistema de arquivos.
     * @throws FileNotFoundException se o arquivo não existir.
     */
    static void validateFileExists(const std::string& filePath);

    /**
     * @brief Constrói o comando ffprobe seguro para inspeção completa do arquivo.
     */
    static std::string buildProbeCommand(const std::string& filePath);

    /**
     * @brief Constrói o comando ffplay seguro com base nas opções de reprodução.
     */
    static std::string buildPlayCommand(const std::string& filePath, const PlaybackOptions& options = {});

    /**
     * @brief Extrai todos os metadados do arquivo de mídia usando ffprobe.
     * @param filePath Caminho para o arquivo de vídeo/áudio.
     * @return Estrutura MediaInfo preenchida.
     */
    MediaInfo probe(const std::string& filePath) const;

    /**
     * @brief Retorna o nome do codec de vídeo principal do arquivo.
     */
    std::string getVideoCodec(const std::string& filePath) const;

    /**
     * @brief Exibe os codecs de vídeo e áudio na saída informada.
     */
    void printCodec(const std::string& filePath, std::ostream& out = std::cout) const;

    /**
     * @brief Exibe o relatório completo de metadados (texto formatado ou JSON).
     */
    void printMediaInfo(const std::string& filePath, bool asJson = false, std::ostream& out = std::cout) const;

    /**
     * @brief Reproduz o arquivo de mídia utilizando ffplay.
     */
    void play(const std::string& filePath, const PlaybackOptions& options = {}) const;

    /**
     * @brief Exibe uma mensagem amigável e detalhada para exceções de mídia.
     */
    static void handleException(const std::exception& e, std::ostream& errOut = std::cerr);
};

} // namespace ffmpeg_player

#endif // FFMPEG_PLAYER_VIDEO_PLAYER_HPP
