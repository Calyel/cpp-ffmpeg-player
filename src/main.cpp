// Autor: Calyel Junio de Andrade dos Santos
// Descrição: Ponto de entrada CLI e modo interativo do cpp-ffmpeg-player

#include "ffmpeg_player/ProcessRunner.hpp"
#include "ffmpeg_player/VideoPlayer.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr const char* kAppVersion = "1.0.0";
constexpr const char* kDefaultSamplePath = "assets/sample.mp4";

enum class ActionMode {
    Interactive,
    Help,
    Version,
    InfoOnly,
    JsonOnly,
    CodecOnly,
    InspectAndPlay
};

void printBanner() {
    std::cout << "============================================================\n"
              << "             CPP-FFMPEG-PLAYER v" << kAppVersion << "\n"
              << "     Inspecao de Metadados e Reproducao de Video (FFmpeg)   \n"
              << "============================================================\n";
}

void printHelp(const char* progName) {
    printBanner();
    std::cout << "\nUso:\n"
              << "  " << progName << " [OPCOES] <arquivo_de_video>\n"
              << "  " << progName << " (sem argumentos para iniciar o modo interativo)\n\n"
              << "Modos de Operacao:\n"
              << "  -m, --info              Exibe o relatorio completo de metadados (sem reproduzir)\n"
              << "  -j, --json              Exporta os metadados do arquivo em formato JSON\n"
              << "  -c, --codec             Exibe apenas os codecs de video e audio\n"
              << "  -p, --play              Exibe metadados e inicia a reproducao (padrao ao informar arquivo)\n"
              << "  -i, --interactive       Inicia o menu interativo no terminal\n"
              << "  -h, --help              Exibe esta mensagem de ajuda\n"
              << "  -v, --version           Exibe a versao do programa\n\n"
              << "Opcoes de Reproducao (ffplay):\n"
              << "  -fs, --fullscreen       Reproduz o video em tela cheia\n"
              << "  -an, --no-audio         Desativa o audio durante a reproducao\n"
              << "  -vn, --no-video         Desativa o video (reproduz apenas audio)\n"
              << "  -l,  --loop <N>         Repete a reproducao N vezes (0 = loop infinito)\n"
              << "  -t,  --title <TITULO>   Define um titulo customizado para a janela do player\n\n"
              << "Exemplos:\n"
              << "  " << progName << " assets/sample.mp4\n"
              << "  " << progName << " --info assets/sample.mp4\n"
              << "  " << progName << " --json assets/sample.mp4\n"
              << "  " << progName << " --codec assets/sample.mp4\n"
              << "  " << progName << " --play --fullscreen --no-audio assets/sample.mp4\n";
}

int runInteractiveMode(const ffmpeg_player::VideoPlayer& player) {
    printBanner();

    std::string filePath;
    std::cout << "\nInforme o caminho do arquivo de video\n"
              << "(ou pressione ENTER para usar '" << kDefaultSamplePath << "'): ";

    if (!std::getline(std::cin, filePath)) {
        return EXIT_SUCCESS;
    }

    if (filePath.empty()) {
        filePath = kDefaultSamplePath;
    }

    while (true) {
        std::cout << "\n------------------------------------------------------------\n"
                  << " Arquivo atual: " << filePath << "\n"
                  << "------------------------------------------------------------\n"
                  << " [1] Exibir relatorio completo de metadados (ffprobe)\n"
                  << " [2] Exibir apenas codecs de video e audio\n"
                  << " [3] Exportar metadados em formato JSON\n"
                  << " [4] Reproduzir video (ffplay)\n"
                  << " [5] Reproduzir video em tela cheia\n"
                  << " [6] Trocar arquivo de midia\n"
                  << " [0] Sair\n"
                  << "Escolha uma opcao: ";

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            break;
        }

        if (choice == "0" || choice == "sair" || choice == "exit") {
            std::cout << "Encerrando o cpp-ffmpeg-player.\n";
            break;
        }

        try {
            if (choice == "1") {
                std::cout << "\n";
                player.printMediaInfo(filePath, false);
            } else if (choice == "2") {
                std::cout << "\n";
                player.printCodec(filePath);
            } else if (choice == "3") {
                std::cout << "\n";
                player.printMediaInfo(filePath, true);
            } else if (choice == "4") {
                std::cout << "\nIniciando reproducao de '" << filePath << "'...\n";
                player.printCodec(filePath);
                player.play(filePath);
            } else if (choice == "5") {
                ffmpeg_player::PlaybackOptions opts;
                opts.fullscreen = true;
                std::cout << "\nIniciando reproducao em tela cheia de '" << filePath << "'...\n";
                player.play(filePath, opts);
            } else if (choice == "6") {
                std::cout << "Informe o novo caminho do arquivo de video: ";
                std::string newPath;
                if (std::getline(std::cin, newPath) && !newPath.empty()) {
                    filePath = newPath;
                }
            } else {
                std::cout << "Opcao invalida. Escolha um numero de 0 a 6.\n";
            }
        } catch (const std::exception& e) {
            ffmpeg_player::VideoPlayer::handleException(e);
        }
    }

    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char* argv[]) {
    ffmpeg_player::VideoPlayer player;

    if (argc == 1) {
        return runInteractiveMode(player);
    }

    ActionMode mode = ActionMode::InspectAndPlay;
    ffmpeg_player::PlaybackOptions playOptions;
    std::string filePath;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            mode = ActionMode::Help;
            break;
        } else if (arg == "-v" || arg == "--version") {
            mode = ActionMode::Version;
            break;
        } else if (arg == "-i" || arg == "--interactive") {
            mode = ActionMode::Interactive;
            break;
        } else if (arg == "-m" || arg == "--info") {
            mode = ActionMode::InfoOnly;
        } else if (arg == "-j" || arg == "--json") {
            mode = ActionMode::JsonOnly;
        } else if (arg == "-c" || arg == "--codec") {
            mode = ActionMode::CodecOnly;
        } else if (arg == "-p" || arg == "--play") {
            mode = ActionMode::InspectAndPlay;
        } else if (arg == "-fs" || arg == "--fullscreen") {
            playOptions.fullscreen = true;
        } else if (arg == "-an" || arg == "--no-audio") {
            playOptions.noAudio = true;
        } else if (arg == "-vn" || arg == "--no-video") {
            playOptions.noVideo = true;
        } else if (arg == "-l" || arg == "--loop") {
            if (i + 1 >= argc) {
                std::cerr << "[ERRO] A opcao '" << arg << "' requer um numero inteiro.\n";
                return EXIT_FAILURE;
            }
            try {
                playOptions.loopCount = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "[ERRO] Valor invalido para '--loop'.\n";
                return EXIT_FAILURE;
            }
        } else if (arg == "-t" || arg == "--title") {
            if (i + 1 >= argc) {
                std::cerr << "[ERRO] A opcao '" << arg << "' requer um texto para o titulo.\n";
                return EXIT_FAILURE;
            }
            playOptions.windowTitle = argv[++i];
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "[ERRO] Opcao desconhecida: " << arg << "\n"
                      << "Use '--help' para listar as opcoes disponiveis.\n";
            return EXIT_FAILURE;
        } else {
            filePath = arg;
        }
    }

    if (mode == ActionMode::Help) {
        printHelp(argv[0]);
        return EXIT_SUCCESS;
    }

    if (mode == ActionMode::Version) {
        std::cout << "cpp-ffmpeg-player v" << kAppVersion << "\n";
        return EXIT_SUCCESS;
    }

    if (mode == ActionMode::Interactive) {
        return runInteractiveMode(player);
    }

    if (filePath.empty()) {
        std::cerr << "[ERRO] Nenhum arquivo de midia foi informado.\n"
                  << "Use '" << argv[0] << " --help' para instrucoes de uso.\n";
        return EXIT_FAILURE;
    }

    try {
        switch (mode) {
            case ActionMode::InfoOnly:
                player.printMediaInfo(filePath, false);
                break;
            case ActionMode::JsonOnly:
                player.printMediaInfo(filePath, true);
                break;
            case ActionMode::CodecOnly:
                player.printCodec(filePath);
                break;
            case ActionMode::InspectAndPlay:
                player.printMediaInfo(filePath, false);
                std::cout << "\nIniciando reproducao com ffplay...\n";
                player.play(filePath, playOptions);
                break;
            default:
                break;
        }
    } catch (const std::exception& e) {
        ffmpeg_player::VideoPlayer::handleException(e);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
