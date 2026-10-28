// Autor: Calyel Junio de Andrade dos Santos
// Descrição: Utilitário para execução segura de processos externos (ffprobe / ffplay)

#ifndef FFMPEG_PLAYER_PROCESS_RUNNER_HPP
#define FFMPEG_PLAYER_PROCESS_RUNNER_HPP

#include <string>

namespace ffmpeg_player {

/**
 * @brief Resultado da execução de um comando externo com captura de saída.
 */
struct CommandResult {
    int exitCode = -1;
    std::string output;
};

/**
 * @brief Classe utilitária responsável por escapar argumentos de shell e
 *        executar binários externos do FFmpeg de forma segura e multiplataforma.
 */
class ProcessRunner {
public:
    /**
     * @brief Escapa um argumento para uso seguro em comandos de shell,
     *        prevenindo falhas com espaços em branco ou caracteres especiais.
     * @param arg Argumento bruto (ex.: caminho de arquivo).
     * @return Argumento devidamente entre aspas e escapado para o SO atual.
     */
    static std::string escapeShellArg(const std::string& arg);

    /**
     * @brief Executa um comando e captura sua saída padrão (stdout).
     * @param command Comando completo a ser executado.
     * @return CommandResult contendo o código de saída e o texto capturado.
     */
    static CommandResult capture(const std::string& command);

    /**
     * @brief Executa um comando interativo no terminal (ex.: ffplay).
     * @param command Comando completo a ser executado.
     * @return Código de retorno normalizado do processo.
     */
    static int execute(const std::string& command);

    /**
     * @brief Verifica se uma ferramenta de linha de comando está instalada no PATH.
     * @param toolName Nome do executável (ex.: "ffprobe", "ffplay").
     * @return true se a ferramenta estiver disponível, false caso contrário.
     */
    static bool isToolAvailable(const std::string& toolName);
};

} // namespace ffmpeg_player

#endif // FFMPEG_PLAYER_PROCESS_RUNNER_HPP
