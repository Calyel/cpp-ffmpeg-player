# Guia de Contribuição (`CONTRIBUTING.md`)

Obrigado pelo interesse em contribuir com o **cpp-ffmpeg-player**! Este documento reúne as diretrizes de desenvolvimento, padrões de código e fluxo de trabalho do projeto.

---

## Pré-requisitos de Desenvolvimento

- Compilador C++ com suporte a **C++17** (`g++ >= 9`, `clang++ >= 10` ou `MSVC 2019+`)
- **FFmpeg** instalado e disponível no `PATH` (`ffprobe` e `ffplay`)
- **GNU Make** ou **CMake 3.16+**

---

## Como Compilar e Testar Localmente

1. **Compilar o projeto e rodar todos os testes:**
   ```bash
   make clean && make test
   ```
2. **Testar os modos de linha de comando com o vídeo de exemplo:**
   ```bash
   make info
   make json
   make codec
   ```

---

## Padrões de Código

- **Padrão da linguagem:** C++17 estrito (`-std=c++17 -Wall -Wextra -Wpedantic`). Nenhum *warning* deve ser introduzido.
- **Formatação:** O projeto inclui um arquivo `.clang-format` (baseado no estilo Google com indentação de 4 espaços e limite de 100 colunas).
- **Segurança:** Qualquer argumento repassado a processos externos (`ffprobe`/`ffplay`) **deve** passar por `ProcessRunner::escapeShellArg()`.
- **Testes:** Novas funcionalidades de parsing ou opções de CLI devem ser acompanhadas de testes em `tests/test_main.cpp`.

---

## Fluxo para Pull Requests

1. Faça um *fork* do repositório e crie uma *branch* descritiva (`git checkout -b feat/nova-funcionalidade` ou `fix/correcao-bug`).
2. Certifique-se de que `make test` passa com 100% de sucesso.
3. Faça commits claros seguindo o padrão *Conventional Commits* (`feat:`, `fix:`, `docs:`, `refactor:`, `test:`).
4. Abra um *Pull Request* descrevendo as alterações realizadas.
