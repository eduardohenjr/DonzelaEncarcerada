#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>

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

typedef struct NoArvore {
    Configuracao config;
    Movimento mov;
    struct NoArvore *pai;
    struct NoArvore **filhos;
    int n_filhos;
} NoArvore;

Configuracao configuracoes[MAX_CONFIGURACOES];
Movimento movimentos[MAX_MOVIMENTOS];
char historico_tabuleiros[MAX_HISTORICO][MAX_LINHAS][MAX_COLUNAS];
int total_configuracoes = 0;
int total_movimentos = 0;
int total_historico = 0;

NoArvore* define_noarvore(NoArvore *pai, Configuracao *config, Movimento mov) {
    NoArvore *novo = (NoArvore*)malloc(sizeof(NoArvore));
    novo->config = *config;
    novo->mov = mov;
    novo->pai = pai;
    novo->filhos = NULL;
    novo->n_filhos = 0;
    return novo;
}

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
    printf("   ");
    for (int j = 0; j < colunas; j++) printf("%d ", j+1);
    printf("\n");
    for (int i = 0; i < linhas; i++) {
        printf("%2d ", i+1);
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

int mover_bloco_simples(char tabuleiro[MAX_LINHAS][MAX_COLUNAS], int linhas, int colunas, int x, int y, char direcao, int simulacao) {
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
        if (letra != 'D' && (ni == 0 || ni == linhas-1 || nj == 0 || nj == colunas-1)) return 0;
        if (ni < 0 || ni >= linhas || nj < 0 || nj >= colunas) {
            if (letra == 'D') {
                if (simulacao) return 2;
                printf("Parabéns! Você venceu!\n");
                exit(0);
            } else {
                return 0;
            }
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
    if (letra == 'D' && !simulacao) {
        for (int i = 0; i < linhas; i++) {
            for (int j = 0; j < colunas; j++) {
                if (tabuleiro[i][j] == 'D' && (i == 0 || i == linhas-1 || j == 0 || j == colunas-1)) {
                    imprimir_tabuleiro_enunciado(tabuleiro, linhas, colunas);
                    printf("Parabéns! Você venceu!\n");
                    exit(0);
                }
            }
        }
    }
    return 1;
}

int tab_iguais(char t1[MAX_LINHAS][MAX_COLUNAS], char t2[MAX_LINHAS][MAX_COLUNAS], int l, int c) {
    for (int i = 0; i < l; i++)
        if (strncmp(t1[i], t2[i], c) != 0) return 0;
    return 1;
}

#define MAX_VISITADOS 10000
Configuracao visitados[MAX_VISITADOS];
int n_visitados = 0;

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "pt_BR.UTF-8");
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
            printf("s: Gerar e explorar todas as configurações possíveis a partir da atual\n");
            printf("R: Resolver automaticamente o quebra-cabeça (busca automática)\n");
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
                int x = linha - 1, y = coluna - 1;
                if (mover_bloco_simples(configuracoes[config_atual].tabuleiro, configuracoes[config_atual].linhas, configuracoes[config_atual].colunas, x, y, direcao, 0)) {
                    if (total_movimentos < MAX_MOVIMENTOS) {
                        movimentos[total_movimentos].linha = linha;
                        movimentos[total_movimentos].coluna = coluna;
                        movimentos[total_movimentos].direcao = direcao;
                        total_movimentos++;
                    }
                    salvar_tabuleiro_historico(&configuracoes[config_atual]);
                    printf("Movimento %d\n", total_movimentos);
                    imprimir_tabuleiro_enunciado(configuracoes[config_atual].tabuleiro, configuracoes[config_atual].linhas, configuracoes[config_atual].colunas);
                } else {
                    char *dir_str;
                    switch (direcao) {
                        case 'T': dir_str = "para cima"; break;
                        case 'B': dir_str = "para baixo"; break;
                        case 'E': dir_str = "para a esquerda"; break;
                        case 'D': dir_str = "para a direita"; break;
                        default: dir_str = "em direção desconhecida"; break;
                    }
                    printf("Impossível movimentar peça em %d,%d %s.\n", linha, coluna, dir_str);
                }
            }
        } else if (comando == 'p') {
            for (int h = 0; h < total_historico; h++)
                imprimir_tabuleiro_enunciado(historico_tabuleiros[h], configuracoes[config_atual].linhas, configuracoes[config_atual].colunas);
        } else if (comando == 's') {
            if (config_atual == -1) {
                printf("Nenhuma configuração carregada!\n");
            } else {
                NoArvore *raiz = define_noarvore(NULL, &configuracoes[config_atual], (Movimento){0,0,' '});
                int n_filhos = 0;
                NoArvore **filhos = (NoArvore**)malloc(MAX_LINHAS * MAX_COLUNAS * 4 * sizeof(NoArvore*));
                for (int i = 0; i < configuracoes[config_atual].linhas; i++) {
                    for (int j = 0; j < configuracoes[config_atual].colunas; j++) {
                        char letra = configuracoes[config_atual].tabuleiro[i][j];
                        if (letra == ' ' || letra == '*') continue;
                        char dirs[4] = {'T','B','E','D'};
                        for (int d = 0; d < 4; d++) {
                            Configuracao nova = configuracoes[config_atual];
                            int res = mover_bloco_simples(nova.tabuleiro, nova.linhas, nova.colunas, i, j, dirs[d], 1);
                            if (res == 1) {
                                Movimento mov = {i+1, j+1, dirs[d]};
                                filhos[n_filhos] = define_noarvore(raiz, &nova, mov);
                                n_filhos++;
                            }
                        }
                    }
                }
                if (n_filhos == 0) {
                    printf("Nenhum movimento possível a partir da configuração atual.\n");
                } else {
                    printf("Configurações possíveis:\n");
                    for (int f = 0; f < n_filhos; f++) {
                        printf("%d) Movimento: (%d,%d) %c\n", f+1, filhos[f]->mov.linha, filhos[f]->mov.coluna, filhos[f]->mov.direcao);
                        imprimir_tabuleiro_enunciado(filhos[f]->config.tabuleiro, filhos[f]->config.linhas, filhos[f]->config.colunas);
                    }
                    printf("Escolha uma configuração (1-%d) para expandir ou 0 para cancelar: ", n_filhos);
                    int escolha;
                    scanf("%d", &escolha);
                    if (escolha >= 1 && escolha <= n_filhos) {
                        configuracoes[config_atual] = filhos[escolha-1]->config;
                        salvar_tabuleiro_historico(&configuracoes[config_atual]);
                        printf("Configuração escolhida carregada.\n");
                        imprimir_tabuleiro_enunciado(configuracoes[config_atual].tabuleiro, configuracoes[config_atual].linhas, configuracoes[config_atual].colunas);
                    } else {
                        printf("Operação cancelada.\n");
                    }
                }
                for (int f = 0; f < n_filhos; f++) free(filhos[f]);
                free(filhos);
                free(raiz);
            }
        } else if (comando == 'R') {
            if (config_atual == -1) {
                printf("Nenhuma configuração carregada!\n");
            } else {
                clock_t inicio = clock();
                typedef struct NoFila {
                    NoArvore *no;
                    struct NoFila *prox;
                } NoFila;
                NoFila *ini = NULL, *fim = NULL;
                NoArvore *raiz = define_noarvore(NULL, &configuracoes[config_atual], (Movimento){0,0,' '});
                ini = fim = (NoFila*)malloc(sizeof(NoFila));
                ini->no = raiz; ini->prox = NULL;
                int achou = 0;
                NoArvore *sol = NULL;
                while (ini && !achou) {
                    NoArvore *atual = ini->no;
                    if (n_visitados < MAX_VISITADOS) visitados[n_visitados++] = atual->config;
                    for (int i = 0; i < atual->config.linhas; i++) {
                        for (int j = 0; j < atual->config.colunas; j++) {
                            char letra = atual->config.tabuleiro[i][j];
                            if (letra == ' ' || letra == '*') continue;
                            char dirs[4] = {'T','B','E','D'};
                            for (int d = 0; d < 4; d++) {
                                Configuracao nova = atual->config;
                                int res = mover_bloco_simples(nova.tabuleiro, nova.linhas, nova.colunas, i, j, dirs[d], 1);
                                if (res == 1) {
                                    int repetido = 0;
                                    for (int v = 0; v < n_visitados; v++) {
                                        if (tab_iguais(nova.tabuleiro, visitados[v].tabuleiro, nova.linhas, nova.colunas)) { repetido = 1; break; }
                                    }
                                    if (repetido) continue;
                                    Movimento mov = {i+1, j+1, dirs[d]};
                                    NoArvore *filho = define_noarvore(atual, &nova, mov);
                                    int venceu = 0;
                                    for (int x = 0; x < nova.linhas; x++) for (int y = 0; y < nova.colunas; y++)
                                        if (nova.tabuleiro[x][y] == 'D' && (x == 0 || x == nova.linhas-1 || y == 0 || y == nova.colunas-1)) venceu = 1;
                                    if (venceu) {
                                        achou = 1; sol = filho; break;
                                    }
                                    NoFila *novo = (NoFila*)malloc(sizeof(NoFila));
                                    novo->no = filho; novo->prox = NULL;
                                    if (fim) {
                                        fim->prox = novo;
                                    }
                                    fim = novo;
                                    if (!ini) ini = novo;
                                }
                            }
                            if (achou) break;
                        }
                        if (achou) break;
                    }
                    NoFila *tmp = ini;
                    ini = ini->prox;
                    free(tmp);
                }
                clock_t fim_tempo = clock();
                double tempo_gasto = (double)(fim_tempo - inicio) / CLOCKS_PER_SEC;
                if (achou && sol) {
                    NoArvore *caminho[1000];
                    int tam = 0;
                    while (sol) { caminho[tam++] = sol; sol = sol->pai; }
                    printf("Caminho da solução (do início ao fim):\n");
                    for (int i = tam-1; i >= 0; i--) {
                        if (i != tam-1) printf("Movimento: (%d,%d) %c\n", caminho[i]->mov.linha, caminho[i]->mov.coluna, caminho[i]->mov.direcao);
                        imprimir_tabuleiro_enunciado(caminho[i]->config.tabuleiro, caminho[i]->config.linhas, caminho[i]->config.colunas);
                    }
                    printf("Tempo para encontrar a solução: %.4f segundos\n", tempo_gasto);
                } else {
                    printf("Não foi encontrada solução a partir da configuração atual.\n");
                    printf("Tempo de busca: %.4f segundos\n", tempo_gasto);
                }
            }
        } else if (comando == 'q') break;
        else {
            printf("Comando inválido!\n");
            printf("Comandos válidos: l, c <n>, m <linha> <coluna> <direção>, p, q\n");
        }
    }
    return 0;
}
