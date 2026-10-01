# Heurísticas e Meta-heurísticas

Este projeto contém todos os arquivos disponibilizados pelo professor para a execução do trabalho, incluindo artigos, códigos, instâncias e soluções.

## Organização do projeto

- `Trabalho/`: pasta principal onde será realizado o desenvolvimento do trabalho.
  - `artigo/`: contém um template LaTeX da IEEE. Esse template pode ser compilado localmente com a extensão LaTeX Workshop, caso uma distribuição LaTeX esteja instalada no computador. Também é possível enviar essa pasta para o Overleaf para compilar o documento.
  - `implementação/`: pasta destinada ao código que será produzido ao longo do estudo e desenvolvimento do trabalho.

## Objetivo

O repositório foi estruturado para centralizar os materiais do curso e facilitar a organização do projeto, desde a escrita do artigo até a implementação das soluções propostas.

## Compilador C

Os códigos em C podem ser compilados com o **GCC**. No Windows, recomenda-se instalar o GCC pelo [MSYS2](https://www.msys2.org/), usando o ambiente **UCRT64** (MinGW-w64), e adicionar `C:\msys64\ucrt64\bin` ao `PATH`. Para conferir se a instalação está disponível no terminal:

```sh
gcc --version
```

Os scripts de compilação do projeto usam o padrão GNU C89 (`-std=gnu89`). Para compilar um arquivo manualmente:

```sh
gcc -std=gnu89 arquivo.c -o programa.exe
```

Se o código usar funções da biblioteca matemática, acrescente `-lm` ao comando, como nos scripts do projeto:

```sh
gcc -O2 -std=gnu89 arquivo.c -o programa.exe -lm
```

Os arquivos `launch.json` e `tasks.json` já estão configurados para compilar e executar os programas pelos recursos **Run** e **Debug** do VS Code.
