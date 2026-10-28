// Autor: Calyel Junio de Andrade dos Santos
// Descrição: Suíte de testes unitários e de integração do cpp-ffmpeg-player

#include "ffmpeg_player/MediaInfo.hpp"
#include "ffmpeg_player/ProcessRunner.hpp"
#include "ffmpeg_player/VideoPlayer.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace {

int gTestsPassed = 0;
int gTestsFailed = 0;

void check(bool condition, const std::string& testName, const std::string& detail = "") {
    if (condition) {
        std::cout << "  [PASS] " << testName << "\n";
        ++gTestsPassed;
    } else {
        std::cerr << "  [FAIL] " << testName;
        if (!detail.empty()) {
            std::cerr << " -> " << detail;
        }
        std::cerr << "\n";
        ++gTestsFailed;
    }
}

void testShellEscaping() {
    using ffmpeg_player::ProcessRunner;

#ifndef _WIN32
    std::string escapedSimple = ProcessRunner::escapeShellArg("assets/sample.mp4");
    check(escapedSimple == "'assets/sample.mp4'", "Escape de caminho simples POSIX");

    std::string escapedSpaces = ProcessRunner::escapeShellArg("Meus Videos/aula 01.mp4");
    check(escapedSpaces == "'Meus Videos/aula 01.mp4'", "Escape de caminho com espacos POSIX");

    std::string escapedSingleQuote = ProcessRunner::escapeShellArg("video's_test.mp4");
    check(escapedSingleQuote == "'video'\\''s_test.mp4'", "Escape de aspas simples POSIX");
#else
    std::string escapedSpaces = ProcessRunner::escapeShellArg("Meus Videos\\aula 01.mp4");
    check(escapedSpaces == "\"Meus Videos\\aula 01.mp4\"", "Escape de caminho com espacos Windows");
#endif
}

void testFrameRateParsing() {
    using ffmpeg_player::MediaInfo;

    check(std::abs(MediaInfo::parseFrameRateFraction("30/1") - 30.0) < 0.001,
          "Parse de frame rate inteiro (30/1)");
    check(std::abs(MediaInfo::parseFrameRateFraction("30000/1001") - 29.97) < 0.01,
          "Parse de frame rate NTSC (30000/1001)");
    check(MediaInfo::parseFrameRateFraction("0/0") == 0.0,
          "Parse de frame rate invalido (0/0)");
    check(MediaInfo::parseFrameRateFraction("N/A") == 0.0,
          "Parse de frame rate N/A");
}

void testFfprobeOutputParser() {
    using ffmpeg_player::MediaInfo;

    const std::string mockOutput = R"(
[STREAM]
index=0
codec_name=h264
codec_long_name=H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10
profile=High
codec_type=video
width=1920
height=1080
display_aspect_ratio=16:9
pix_fmt=yuv420p
avg_frame_rate=60/1
bit_rate=4500000
nb_frames=600
[/STREAM]
[STREAM]
index=1
codec_name=aac
codec_long_name=AAC (Advanced Audio Coding)
codec_type=audio
sample_rate=48000
channels=2
channel_layout=stereo
bit_rate=192000
[/STREAM]
[FORMAT]
filename=mock_video.mp4
nb_streams=2
format_name=mov,mp4,m4a,3gp,3g2,mj2
format_long_name=QuickTime / MOV
duration=10.500000
size=6291456
bit_rate=4793490
[/FORMAT]
)";

    MediaInfo info = MediaInfo::parseFfprobeOutput(mockOutput, "mock_video.mp4");

    check(info.hasVideo, "Parser detecta stream de video");
    check(info.videoCodec == "h264", "Parser extrai videoCodec h264");
    check(info.width == 1920 && info.height == 1080, "Parser extrai resolucao 1920x1080");
    check(info.formatResolution() == "1920x1080 (16:9)", "Formatacao de resolucao com aspect ratio");
    check(std::abs(info.frameRate - 60.0) < 0.001, "Parser extrai 60 fps");
    check(info.hasAudio, "Parser detecta stream de audio");
    check(info.audioCodec == "aac", "Parser extrai audioCodec aac");
    check(info.sampleRate == 48000 && info.channels == 2, "Parser extrai sampleRate e canais");
    check(info.formatDuration() == "00:00:10.50", "Formatacao de duracao HH:MM:SS.cc");
    check(info.formatFileSize() == "6.00 MB", "Formatacao de tamanho em MB");

    std::string json = info.toJsonString();
    check(json.find("\"codec\": \"h264\"") != std::string::npos, "Exportacao JSON contem codec h264");
    check(json.find("\"width\": 1920") != std::string::npos, "Exportacao JSON contem largura 1920");
}

void testBuildPlayCommand() {
    using ffmpeg_player::PlaybackOptions;
    using ffmpeg_player::VideoPlayer;

    PlaybackOptions opts;
    opts.fullscreen = true;
    opts.noAudio = true;
    opts.loopCount = 3;
    opts.windowTitle = "Meu Player";

    std::string cmd = VideoPlayer::buildPlayCommand("video com espaco.mp4", opts);
    check(cmd.find("-autoexit") != std::string::npos, "Comando ffplay inclui -autoexit por padrao");
    check(cmd.find("-fs") != std::string::npos, "Comando ffplay inclui -fs quando fullscreen ativo");
    check(cmd.find("-an") != std::string::npos, "Comando ffplay inclui -an quando noAudio ativo");
    check(cmd.find("-loop 3") != std::string::npos, "Comando ffplay inclui -loop 3");
    check(cmd.find("-window_title") != std::string::npos, "Comando ffplay inclui -window_title");
}

void testFileValidationAndExceptions() {
    using ffmpeg_player::FileNotFoundException;
    using ffmpeg_player::VideoPlayer;

    VideoPlayer player;
    bool threwNotFound = false;
    try {
        player.probe("arquivo_que_nao_existe_12345.mp4");
    } catch (const FileNotFoundException&) {
        threwNotFound = true;
    } catch (...) {
        threwNotFound = false;
    }
    check(threwNotFound, "Lanca FileNotFoundException para arquivo inexistente");
}

void testLiveSampleVideoIntegration() {
    using ffmpeg_player::MediaInfo;
    using ffmpeg_player::ProcessRunner;
    using ffmpeg_player::VideoPlayer;

    if (!ProcessRunner::isToolAvailable("ffprobe")) {
        std::cout << "  [SKIP] ffprobe nao instalado no ambiente; pulando teste de integracao real.\n";
        return;
    }

    VideoPlayer player;
    const std::string samplePath = "assets/sample.mp4";

    MediaInfo info = player.probe(samplePath);
    check(info.hasVideo, "Integracao sample.mp4: possui stream de video");
    check(info.videoCodec == "h264", "Integracao sample.mp4: codec de video e h264");
    check(info.width == 640 && info.height == 360, "Integracao sample.mp4: resolucao 640x360");
    check(info.hasAudio && info.audioCodec == "aac", "Integracao sample.mp4: codec de audio e aac");
    check(player.getVideoCodec(samplePath) == "h264", "Integracao sample.mp4: getVideoCodec() retorna h264");

    std::ostringstream oss;
    player.printCodec(samplePath, oss);
    check(oss.str().find("h264") != std::string::npos, "Integracao sample.mp4: printCodec() exibe h264");
}

} // namespace

int main() {
    std::cout << "============================================================\n"
              << "        EXECUTANDO SUITE DE TESTES: CPP-FFMPEG-PLAYER       \n"
              << "============================================================\n";

    testShellEscaping();
    testFrameRateParsing();
    testFfprobeOutputParser();
    testBuildPlayCommand();
    testFileValidationAndExceptions();
    testLiveSampleVideoIntegration();

    std::cout << "------------------------------------------------------------\n"
              << " Resultado Final: " << gTestsPassed << " passaram, "
              << gTestsFailed << " falharam.\n"
              << "============================================================\n";

    return (gTestsFailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
