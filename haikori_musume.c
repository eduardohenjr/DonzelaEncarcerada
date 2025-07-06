#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINHAS 50
#define MAX_COLUNAS 50
#define MAX_CONFIGURACOES 10
#define MAX_MOVIMENTOS 1000
#define MAX_HISTORICO 1000

typedef struct {
    char nome[50];
    char tabuleiro[MAX_LINHAS][MAX_COLUNAS];
    int linhas;
    int colunas;
} Configuracao;

typedef struct {
    int linha;
    int coluna;
    char direcao;
} Movimento;

Configuracao configuracoes[MAX_CONFIGURACOES];
Movimento movimentos[MAX_MOVIMENTOS];
char historico_tabuleiros[MAX_HISTORICO][MAX_LINHAS][MAX_COLUNAS];
int total_configuracoes = 0;
int total_movimentos = 0;
int total_historico = 0;

void carregar_configuracoes(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo %s\n", nome_arquivo);
        exit(1);
    }

    char linha[100];
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (total_configuracoes >= MAX_CONFIGURACOES) break;
        Configuracao *config = &configuracoes[total_configuracoes++];
        strcpy(config->nome, linha);
        config->nome[strcspn(config->nome, "\n")] = '\0';
        config->linhas = 0;
        int max_colunas = 0;
        while (fgets(linha, sizeof(linha), arquivo) && linha[0] != '\n') {
            size_t len = strlen(linha);
            if (len > 0 && linha[len-1] == '\n') linha[--len] = '\0';
            strcpy(config->tabuleiro[config->linhas], linha);
            if ((int)len > max_colunas) max_colunas = (int)len;
            config->linhas++;
        }
        for (int i = 0; i < config->linhas; i++) {
            int linelen = strlen(config->tabuleiro[i]);
            if (linelen < max_colunas) {
                for (int j = linelen; j < max_colunas; j++)
                    config->tabuleiro[i][j] = ' ';
                config->tabuleiro[i][max_colunas] = '\0';
            }
        }
        config->colunas = max_colunas;
    }

    fclose(arquivo);
}

void imprimir_tabuleiro_enunciado(char tabuleiro[MAX_LINHAS][MAX_COLUNAS], int linhas, int colunas) {
    int tem_linha_interna = 0;
    for (int i = 0; i < linhas; i++) {
        if (tabuleiro[i][0] != '*') { tem_linha_interna = 1; break; }
    }
    if (tem_linha_interna) {
        printf("  ");
        for (int j = 0; j < colunas; j++) printf("%d ", j+1);
        printf("\n");
    }
    for (int i = 0; i < linhas; i++) {
        if (tabuleiro[i][0] == '*') printf("  ");
        else printf("%d ", i+1);
        for (int j = 0; j < colunas; j++) printf("%c ", tabuleiro[i][j]);
        printf("\n");
    }
}

void salvar_tabuleiro_historico(Configuracao *config) {
    if (total_historico < MAX_HISTORICO) {
        for (int i = 0; i < config->linhas; i++)
            strcpy(historico_tabuleiros[total_historico][i], config->tabuleiro[i]);
        total_historico++;
    }
}

void marcar_bloco_dfs(char tabuleiro[MAX_LINHAS][MAX_COLUNAS], int linhas, int colunas, int x, int y, char letra, int visitado[MAX_LINHAS][MAX_COLUNAS]) {
    if (x < 0 || x >= linhas || y < 0 || y >= colunas) return;
    if (tabuleiro[x][y] != letra || visitado[x][y]) return;
    visitado[x][y] = 1;
    marcar_bloco_dfs(tabuleiro, linhas, colunas, x+1, y, letra, visitado);
    marcar_bloco_dfs(tabuleiro, linhas, colunas, x-1, y, letra, visitado);
    marcar_bloco_dfs(tabuleiro, linhas, colunas, x, y+1, letra, visitado);
    marcar_bloco_dfs(tabuleiro, linhas, colunas, x, y-1, letra, visitado);
}

int mover_bloco_simples(char tabuleiro[MAX_LINHAS][MAX_COLUNAS], int linhas, int colunas, int x, int y, char direcao) {
    char letra = tabuleiro[x][y];
    if (letra == ' ' || letra == '*') return 0;
    int dx = 0, dy = 0;
    if (direcao == 'T') dx = -1;
    else if (direcao == 'B') dx = 1;
    else if (direcao == 'E') dy = -1;
    else if (direcao == 'D') dy = 1;
    else return 0;
    int visitado[MAX_LINHAS][MAX_COLUNAS] = {0};
    marcar_bloco_dfs(tabuleiro, linhas, colunas, x, y, letra, visitado);
    for (int i = 0; i < linhas; i++) for (int j = 0; j < colunas; j++) if (visitado[i][j]) {
        int ni = i + dx, nj = j + dy;
        if (ni < 0 || ni >= linhas || nj < 0 || nj >= colunas) {
            if (letra == 'D') { printf("Parabéns! Você venceu!\n"); exit(0); }
            else return 0;
        }
        if (tabuleiro[ni][nj] != ' ' && !visitado[ni][nj]) return 0;
    }
    if (dx == 1 || dy == 1) {
        for (int i = linhas-1; i >= 0; i--) for (int j = colunas-1; j >= 0; j--)
            if (visitado[i][j]) { tabuleiro[i+dx][j+dy] = letra; tabuleiro[i][j] = ' '; }
    } else {
        for (int i = 0; i < linhas; i++) for (int j = 0; j < colunas; j++)
            if (visitado[i][j]) { tabuleiro[i+dx][j+dy] = letra; tabuleiro[i][j] = ' '; }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    const char *nome_arquivo = "haikori.txt";
    if (argc > 2 && strcmp(argv[1], "-f") == 0) nome_arquivo = argv[2];
    carregar_configuracoes(nome_arquivo);
    char comando;
    int config_atual = -1;
    while (1) {
        printf("Digite um comando (h para ajuda): ");
        scanf(" %c", &comando);
        if (comando == 'h') {
            printf("Comandos disponíveis:\n");
            printf("l: Listar configurações\n");
            printf("c <n>: Escolher configuração\n");
            printf("m <linha> <coluna> <direção>: Movimentar peça\n");
            printf("p: Imprimir movimentos realizados\n");
            printf("q: Sair\n");
        } else if (comando == 'l') {
            for (int i = 0; i < total_configuracoes; i++) {
                printf("%d\n%s\n", i+1, configuracoes[i].nome);
                imprimir_tabuleiro_enunciado(configuracoes[i].tabuleiro, configuracoes[i].linhas, configuracoes[i].colunas);
            }
        } else if (comando == 'c') {
            int n;
            scanf("%d", &n);
            if (n < 1 || n > total_configuracoes) printf("Configuração inválida!\n");
            else {
                if (config_atual != -1 && total_movimentos > 0) {
                    printf("Você perderá todos os movimentos realizados na configuração atual. Deseja continuar? (s/n): ");
                    char resp;
                    scanf(" %c", &resp);
                    if (resp != 's' && resp != 'S') { printf("Operação cancelada.\n"); continue; }
                }
                config_atual = n - 1;
                total_movimentos = 0;
                total_historico = 0;
                salvar_tabuleiro_historico(&configuracoes[config_atual]);
                printf("Configuração %s carregada:\n", configuracoes[config_atual].nome);
                imprimir_tabuleiro_enunciado(configuracoes[config_atual].tabuleiro, configuracoes[config_atual].linhas, configuracoes[config_atual].colunas);
            }
        } else if (comando == 'm') {
            int linha, coluna; char direcao;
            scanf("%d %d %c", &linha, &coluna, &direcao);
            if (config_atual == -1) printf("Nenhuma configuração carregada!\n");
            else {
                int x = linha, y = coluna;
                if (mover_bloco_simples(configuracoes[config_atual].tabuleiro, configuracoes[config_atual].linhas, configuracoes[config_atual].colunas, x, y, direcao)) {
                    if (total_movimentos < MAX_MOVIMENTOS) {
                        movimentos[total_movimentos].linha = linha;
                        movimentos[total_movimentos].coluna = coluna;
                        movimentos[total_movimentos].direcao = direcao;
                        total_movimentos++;
                    }
                    salvar_tabuleiro_historico(&configuracoes[config_atual]);
                    printf("Movimento %d\n", total_movimentos);
                    imprimir_tabuleiro_enunciado(configuracoes[config_atual].tabuleiro, configuracoes[config_atual].linhas, configuracoes[config_atual].colunas);
                } else printf("Erro ao executar movimento.\n");
            }
        } else if (comando == 'p') {
            for (int h = 0; h < total_historico; h++)
                imprimir_tabuleiro_enunciado(historico_tabuleiros[h], configuracoes[config_atual].linhas, configuracoes[config_atual].colunas);
        } else if (comando == 'q') break;
        else {
            printf("Comando inválido!\n");
            printf("Comandos válidos: l, c <n>, m <linha> <coluna> <direção>, p, q\n");
        }
    }
    return 0;
}
