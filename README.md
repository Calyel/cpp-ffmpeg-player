# cpp-ffmpeg-player

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![FFmpeg](https://img.shields.io/badge/FFmpeg-ffprobe%20%7C%20ffplay-007808?style=for-the-badge&logo=ffmpeg&logoColor=white)
![Build](https://img.shields.io/badge/Build-Make%20%7C%20CMake-2496ED?style=for-the-badge&logo=cmake&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

Ferramenta de linha de comando (CLI), modo interativo e biblioteca modular em **C++17** para inspeção completa de metadados de mídia (`ffprobe`) e reprodução configurável de vídeo/áudio (`ffplay`).

---

## Principais Funcionalidades

- **Inspeção Completa de Metadados (`--info`)**: Extrai informações de container, duração formatada (`HH:MM:SS.cc`), tamanho legível (`KB`/`MB`/`GB`), bitrate total, resolução, aspect ratio, pixel format, taxa de quadros (`fps`), total de frames e parâmetros de áudio (`sample rate`, canais e layout).
- **Exportação Estruturada em JSON (`--json`)**: Gera saída JSON limpa e pronta para integração com scripts, pipelines de automação ou outras aplicações.
- **Consulta Rápida de Codecs (`--codec`)**: Identifica instantaneamente os codecs de vídeo e áudio do arquivo.
- **Reprodução Configurável (`--play`)**: Controle direto sobre o `ffplay` com suporte a tela cheia (`--fullscreen`), mudo (`--no-audio`), somente áudio (`--no-video`), repetição (`--loop`) e título de janela customizado (`--title`).
- **Modo Interativo Amigável**: Ao executar sem argumentos, apresenta um menu interativo completo no terminal.
- **Execução Segura e Multiplataforma**: Escapamento automático de argumentos de shell (suporte total a caminhos com espaços, acentos e caracteres especiais em Linux, macOS e Windows) e validação de arquivos via `std::filesystem`.

---

## Estrutura do Projeto

```text
cpp-ffmpeg-player/
├── .github/workflows/ci.yml        # Pipeline de Integração Contínua (GitHub Actions)
├── assets/
│   └── sample.mp4                  # Vídeo de exemplo para testes rápidos (H.264 / AAC)
├── docs/
│   └── ARCHITECTURE.md             # Documentação da arquitetura e decisões técnicas
├── include/ffmpeg_player/
│   ├── MediaInfo.hpp               # Estrutura e parser de metadados de mídia
│   ├── ProcessRunner.hpp           # Executor seguro de processos externos e escape de shell
│   └── VideoPlayer.hpp             # Classe principal de inspeção, reprodução e exceções
├── src/
│   ├── MediaInfo.cpp               # Implementação do parser ffprobe e formatadores (Texto/JSON)
│   ├── ProcessRunner.cpp           # Implementação de popen/system e sanitização de argumentos
│   ├── VideoPlayer.cpp             # Implementação da fachada VideoPlayer
│   └── main.cpp                    # Interface de linha de comando (CLI) e menu interativo
├── tests/
│   └── test_main.cpp               # Suíte de testes unitários e de integração (31 asserções)
├── .clang-format                   # Padronização de estilo de código C++17
├── .gitignore                      # Arquivos e diretórios ignorados pelo Git
├── CMakeLists.txt                  # Configuração de build moderna com CMake + CTest
├── CONTRIBUTING.md                 # Guia de contribuição e padrões de desenvolvimento
├── LICENSE                         # Licença MIT
├── Makefile                        # Automação rápida de compilação, execução e testes
└── README.md                       # Documentação principal do projeto
```

---

## Pré-requisitos

1. **Compilador compatível com C++17**:
   - GCC (`g++` 9+) ou Clang (`clang++` 10+) ou MSVC (Visual Studio 2019+)
2. **FFmpeg (`ffprobe` e `ffplay`) instalado no `PATH`**:
   - **Ubuntu / Debian**: `sudo apt update && sudo apt install ffmpeg build-essential`
   - **Arch Linux**: `sudo pacman -S ffmpeg base-devel`
   - **macOS (Homebrew)**: `brew install ffmpeg`
   - **Windows (Winget)**: `winget install Gyan.FFmpeg`

---

## Como Compilar e Testar

### Opção 1: Usando `Makefile` (Recomendado no Linux/macOS)

```bash
# Compilar o executável principal (bin/cpp-ffmpeg-player)
make

# Compilar e rodar a suíte de testes automatizados (31 testes)
make test
```

Atalhos úteis do `Makefile` utilizando o vídeo de demonstração (`assets/sample.mp4`):

```bash
make info    # Exibe o relatório completo de metadados
make json    # Exporta os metadados em formato JSON
make codec   # Exibe apenas os codecs de vídeo e áudio
make play    # Reproduz o vídeo de exemplo com ffplay
make run     # Abre o menu interativo no terminal
make clean   # Remove os diretórios build/ e bin/
```

### Opção 2: Usando `CMake` (Multiplataforma: Linux, macOS, Windows)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Guia de Uso (CLI)

```text
Uso:
  ./bin/cpp-ffmpeg-player [OPCOES] <arquivo_de_video>
  ./bin/cpp-ffmpeg-player (sem argumentos para iniciar o modo interativo)

Modos de Operacao:
  -m, --info              Exibe o relatorio completo de metadados (sem reproduzir)
  -j, --json              Exporta os metadados do arquivo em formato JSON
  -c, --codec             Exibe apenas os codecs de video e audio
  -p, --play              Exibe metadados e inicia a reproducao (padrao ao informar arquivo)
  -i, --interactive       Inicia o menu interativo no terminal
  -h, --help              Exibe esta mensagem de ajuda
  -v, --version           Exibe a versao do programa

Opcoes de Reproducao (ffplay):
  -fs, --fullscreen       Reproduz o video em tela cheia
  -an, --no-audio         Desativa o audio durante a reproducao
  -vn, --no-video         Desativa o video (reproduz apenas audio)
  -l,  --loop <N>         Repete a reproducao N vezes (0 = loop infinito)
  -t,  --title <TITULO>   Define um titulo customizado para a janela do player
```

### Exemplos de Saída

#### 1. Relatório Completo de Metadados (`--info`)

```bash
./bin/cpp-ffmpeg-player --info assets/sample.mp4
```

```text
============================================================
                 RELATORIO DE MIDIA (FFPROBE)               
============================================================
 [Container / Arquivo]
  Arquivo       : assets/sample.mp4
  Formato       : QuickTime / MOV (mov,mp4,m4a,3gp,3g2,mj2)
  Duracao       : 00:00:07.27 (7.27 s)
  Tamanho       : 370.63 KB (379526 bytes)
  Bitrate Total : 417 kbps
  Streams       : 2

 [Stream de Video]
  Codec         : h264 (H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10)
  Perfil        : Main
  Resolucao     : 640x360 (16:9)
  Pixel Format  : yuv420p
  Taxa Quadros  : 30.00 fps
  Bitrate Video : 284 kbps
  Total Quadros : 218

 [Stream de Audio]
  Codec         : aac (AAC (Advanced Audio Coding))
  Sample Rate   : 44100 Hz
  Canais        : 2 (stereo)
  Bitrate Audio : 128 kbps
============================================================
```

#### 2. Exportação em JSON (`--json`)

```bash
./bin/cpp-ffmpeg-player --json assets/sample.mp4
```

```json
{
  "file_path": "assets/sample.mp4",
  "format": {
    "name": "mov,mp4,m4a,3gp,3g2,mj2",
    "long_name": "QuickTime / MOV",
    "duration_seconds": 7.268,
    "duration_formatted": "00:00:07.27",
    "size_bytes": 379526,
    "bitrate_bps": 417758,
    "stream_count": 2
  },
  "video": {
    "present": true,
    "codec": "h264",
    "codec_long": "H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10",
    "profile": "Main",
    "width": 640,
    "height": 360,
    "aspect_ratio": "16:9",
    "pixel_format": "yuv420p",
    "frame_rate": 30.00,
    "bitrate_bps": 284173,
    "total_frames": 218
  },
  "audio": {
    "present": true,
    "codec": "aac",
    "codec_long": "AAC (Advanced Audio Coding)",
    "sample_rate_hz": 44100,
    "channels": 2,
    "channel_layout": "stereo",
    "bitrate_bps": 128204
  }
}
```

#### 3. Consulta Rápida de Codecs (`--codec`)

```bash
./bin/cpp-ffmpeg-player --codec assets/sample.mp4
```

```text
Codec de Video : h264 (H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10)
Codec de Audio : aac (AAC (Advanced Audio Coding))
```

---

## Uso como Biblioteca C++

Você também pode integrar a biblioteca `ffmpeg_player_core` diretamente em outros códigos C++:

```cpp
#include "ffmpeg_player/VideoPlayer.hpp"
#include <iostream>

int main() {
    ffmpeg_player::VideoPlayer player;

    try {
        // Extrai metadados estruturados
        ffmpeg_player::MediaInfo info = player.probe("assets/sample.mp4");
        std::cout << "Codec: " << info.videoCodec
                  << " | Resolucao: " << info.formatResolution() << "\n";

        // Reproduz o vídeo com opções customizadas
        ffmpeg_player::PlaybackOptions opts;
        opts.autoExit = true;
        opts.windowTitle = "Meu Player FFmpeg";
        player.play("assets/sample.mp4", opts);
    } catch (const ffmpeg_player::MediaException& e) {
        ffmpeg_player::VideoPlayer::handleException(e);
    }
}
```

---

## Documentação Adicional

- [Arquitetura Técnica (`docs/ARCHITECTURE.md`)](docs/ARCHITECTURE.md)
- [Guia de Contribuição (`CONTRIBUTING.md`)](CONTRIBUTING.md)

---

## Autor e Licença

Desenvolvido por **Calyel Junio de Andrade dos Santos**.  
Distribuído sob a [Licença MIT](LICENSE).
