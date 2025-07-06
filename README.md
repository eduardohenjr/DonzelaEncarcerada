# Donzela Encarcerada

Jogo de lógica inspirado no clássico "Hakoiri Musume" (Donzela Encarcerada), implementado em C.

## Objetivo

Mova os blocos do tabuleiro para libertar a donzela (bloco 'D'), levando-a até a saída do tabuleiro.

## Como jogar

1. Compile o programa:
   ```sh
   gcc haikori_musume.c -o donzela
   ```
2. Execute o jogo:
   ```sh
   ./donzela
   ```
   Ou especifique um arquivo de configuração:
   ```sh
   ./donzela -f haikori.txt
   ```

3. Comandos disponíveis:
   - `l` : Listar configurações disponíveis
   - `c <n>` : Carregar configuração número n
   - `m <linha> <coluna> <direção>` : Mover bloco na posição (linha, coluna) na direção desejada
     - Direções: `T` (cima), `B` (baixo), `E` (esquerda), `D` (direita)
     - Exemplo: `m 1 1 D` (move o bloco da linha 1, coluna 1 para a direita)
   - `p` : Imprimir histórico de movimentos/tabuleiros
   - `q` : Sair do jogo
   - `h` : Ajuda

## Arquivo de configuração

O arquivo `haikori.txt` contém as configurações dos tabuleiros. Cada configuração deve ter um nome na primeira linha, seguida pelo desenho do tabuleiro, e uma linha em branco separando as configurações.

Exemplo:
```
Fase 1
****
*DD*
*AA*
****

Fase 2
****
*DA*
*AD*
****
```

## Regras
- O tabuleiro é indexado a partir de (1,1) para o usuário, conforme mostrado na impressão do jogo.
- Só é possível mover blocos inteiros (blocos conectados de mesma letra).
- O objetivo é mover o bloco 'D' para fora do tabuleiro.
- Se um movimento não for possível, o programa exibirá: `Impossível movimentar peça em <linha>,<coluna> <direção>`, detalhando a posição e direção.

## Créditos
Desenvolvido por Eduardo Henrique Marques Gomes Junior.

## Licença
Este projeto está licenciado sob a licença MIT.
