// Autor: Calyel Junio de Andrade dos Santos
// Descrição: Implementação do executor de processos externos (ProcessRunner)

#include "ffmpeg_player/ProcessRunner.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace ffmpeg_player {

namespace {

int normalizeExitStatus(int rawStatus) {
    if (rawStatus == -1) {
        return -1;
    }
#ifdef _WIN32
    return rawStatus;
#else
    if (WIFEXITED(rawStatus)) {
        return WEXITSTATUS(rawStatus);
    }
    return rawStatus;
#endif
}

} // namespace

std::string ProcessRunner::escapeShellArg(const std::string& arg) {
#ifdef _WIN32
    std::string escaped = "\"";
    for (char c : arg) {
        if (c == '"') {
            escaped += "\\\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
#else
    std::string escaped = "'";
    for (char c : arg) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped += c;
        }
    }
    escaped += "'";
    return escaped;
#endif
}

CommandResult ProcessRunner::capture(const std::string& command) {
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif

    if (!pipe) {
        throw std::runtime_error("Falha ao abrir pipe para executar o comando: " + command);
    }

    std::array<char, 256> buffer{};
    std::string result;

    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }

#ifdef _WIN32
    int rawStatus = _pclose(pipe);
#else
    int rawStatus = pclose(pipe);
#endif

    return CommandResult{normalizeExitStatus(rawStatus), result};
}

int ProcessRunner::execute(const std::string& command) {
    int rawStatus = std::system(command.c_str());
    return normalizeExitStatus(rawStatus);
}

bool ProcessRunner::isToolAvailable(const std::string& toolName) {
    for (char c : toolName) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) {
            return false;
        }
    }
#ifdef _WIN32
    std::string checkCmd = toolName + " -version >NUL 2>&1";
#else
    std::string checkCmd = toolName + " -version >/dev/null 2>&1";
#endif
    return execute(checkCmd) == 0;
}

} // namespace ffmpeg_player
