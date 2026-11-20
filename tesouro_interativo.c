/*
 * Tesouro do Nordeste - versão interativa completa (itens, cenários, ciclo dia/noite, eventos e dicas)
 * Compilar: gcc -Wall -Wextra -std=c11 -o tesouro_interativo tesouro_interativo.c
 * Rodar:    ./tesouro_interativo           (com cores)
 *           ./tesouro_interativo --sem-cor (sem cores; ou use 'cores' no jogo)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_ITENS       6
#define AGUA_INICIAL    8
#define SEM_SAIDA      -1
#define SEM_ITEM       -1
#define LARGURA_ARTE   40
#define PASSOS_POR_FASE 3

/* ---------- 1. Modelagem dos dados ---------- */

typedef enum { NORTE, SUL, LESTE, OESTE, NUM_DIRECOES } Direcao;
typedef enum { MAPA, CORDA, CHAVE, LAMPIAO, CANTIL, NUM_ITENS } ItemId;
typedef enum { MANHA, TARDE, NOITE, NUM_FASES } Fase;

static const char *NOMES_DIRECOES[NUM_DIRECOES] = { "norte", "sul", "leste", "oeste" };
static const char *NOMES_ITENS[NUM_ITENS]       = { "mapa", "corda", "chave", "lampiao", "cantil" };
static const char *NOMES_FASES[NUM_FASES]       = { "Manha", "Tarde", "Noite" };
static const char *AVISO_FASE[NUM_FASES] = {
    "\n[TEMPO] O sol desponta no horizonte. A manha traz clima ameno (1 agua/passo).",
    "\n[TEMPO] Sol a pino! A tarde castiga o viajante (2 aguas/passo fora de fontes).",
    "\n[TEMPO] A noite cai. O frio alivia a sede, mas o escuro dificulta explorar."
};

typedef struct {
    const char *nome;
    const char *descricao;
    const char *detalhe_explorar;
    int saidas[NUM_DIRECOES];
    int item;                 /* item visível no local */
    int item_oculto;          /* item encontrado ao 'explorar' */
    int exige;                /* item necessário para entrar */
    int tem_fonte;            /* 1 = reabastece água */
    int explorado;            /* 1 = já vasculhado */
} Local;

typedef struct {
    int local;
    int agua;
    int agua_max;
    int passos;
    int inventario[MAX_ITENS];
    int qtd_itens;
    int bau_aberto;
    int atalho_chapada;
} Jogador;

enum { CASA, SERTAO, FEIRA, CHAPADA, TESOURO, NUM_LOCAIS };

/* ---- ARTE ASCII (largura fixa de 40 colunas) ---- */
static const char *CEU[NUM_FASES][3] = {
    [MANHA] = {
        "   ~~~~                       \\  |  /   ",
        "          ~~~~~              -- (O) --  ",
        "                              /  |  \\   ",
    },
    [TARDE] = {
        "                         ~~~~~    ~~~   ",
        "   \\  |  /                              ",
        "  -- (O) --                 ~~~~        ",
    },
    [NOITE] = {
        "  .      *          .-.        .    *   ",
        "       .           (   )    *        .  ",
        "   *         .      `-'         .  *    ",
    },
};

static const char *TETO[NUM_FASES][3] = {
    [MANHA] = {
        "##################    ##################",
        " V    V    V    V  ||V    V    V    V   ",
        "      v         v  ||     v         v   ",
    },
    [TARDE] = {
        "##################    ##################",
        " V    V    V    V  \\\\V    V    V    V   ",
        "      v         v   \\\\    v         v   ",
    },
    [NOITE] = {
        "###################*####################",
        " V    V    V    V    V    V    V    V   ",
        "      v         v         v         v   ",
    },
};

static const char *CHAO[NUM_LOCAIS][6] = {
    [CASA] = {
        "           ) (                          ",
        "       ___(___)___                      ",
        "      /____________\\       \\|/   \\|/    ",
        "      | [] |##| [] |        |     |     ",
        "      |____|__|____|        |     |     ",
        "========================================",
    },
    [SERTAO] = {
        "       _                ~v~             ",
        "   _  | |  _                            ",
        "  | |_| |_| |                           ",
        "  |_  | |  _|                  _        ",
        "      | |                     | |       ",
        ",.-'`-.,_,.-'`-.,_,.-'`-.,_,.-'`-.,_,.-'",
    },
    [FEIRA] = {
        "^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v",
        " /\\/\\/\\/\\/\\        '.'       /\\/\\/\\/\\/\\ ",
        " |~~~~~~~~~~|    .-~~~-.     |~~~~~~~~~~",
        " | [] []    |   (  ~~~  )    |  o  o    ",
        " |__________|   \\_______/    |__________",
        "========================================",
    },
    [CHAPADA] = {
        "          ____________________          ",
        " |        \\      |::::|      /        | ",
        " |         \\     |::::|     /         | ",
        " |          |    |::::|    |    .--.  | ",
        " |          |    |::::|    |   /    \\ | ",
        "===========~~~~~~~~~~~~~~~~~~===========",
    },
    [TESOURO] = {
        "                                        ",
        "                 _______                ",
        "                /_______\\               ",
        "     ^         |  [$$$]  |       ^      ",
        "    / \\        |_________|      / \\     ",
        "==========o O o 0 o O o 0 o O o=========",
    },
};

static const char *COR_CEU[NUM_FASES]   = { "33", "91", "97" };
static const char *COR_CHAO[NUM_LOCAIS] = {
    [CASA] = "33", [SERTAO] = "93", [FEIRA] = "95", [CHAPADA] = "31", [TESOURO] = "36"
};
static int usar_cor = 1;

static Local mundo[NUM_LOCAIS] = {
    [CASA] = {
        "Casa de Farinha",
        "Uma casa de farinha antiga. O cheiro de mandioca ainda esta no ar.",
        "Atras dos sacos de farinha, voce encontra um velho lampiao a querosene!",
        { SERTAO, SEM_SAIDA, FEIRA, SEM_SAIDA }, MAPA, LAMPIAO, SEM_ITEM, 0, 0
    },
    [SERTAO] = {
        "Sertao",
        "Caatinga a perder de vista. O sol racha a terra e o mandacaru vigia.",
        "Perto de um juazeiro frondoso, voce acha um pote de barro com agua fresca (+2 agua)!",
        { SEM_SAIDA, CASA, CHAPADA, SEM_SAIDA }, SEM_ITEM, SEM_ITEM, SEM_ITEM, 0, 0
    },
    [FEIRA] = {
        "Feira de Caruaru",
        "Barracas coloridas, cordel pendurado e uma fonte no meio da praca.",
        "Um cordelista recita: 'Na Chapada use a corda; na Gruta, a chave abre o ouro.'",
        { SEM_SAIDA, SEM_SAIDA, SEM_SAIDA, CASA }, CORDA, SEM_ITEM, SEM_ITEM, 1, 0
    },
    [CHAPADA] = {
        "Chapada Diamantina",
        "Paredoes de pedra e uma caverna funda. Ha uma fenda onde cabe uma corda.",
        "Voce observa marcas de escalada descendo por uma fenda estreita no paredao.",
        { TESOURO, SEM_SAIDA, SEM_SAIDA, SERTAO }, CHAVE, SEM_ITEM, CORDA, 0, 0
    },
    [TESOURO] = {
        "Gruta do Tesouro",
        "Um bau enorme, coberto de ouro e rapadura petrificada, trancado a cadeado!",
        "O cadeado do bau tem uma fechadura antiga. Parece pedir uma chave.",
        { SEM_SAIDA, CHAPADA, SEM_SAIDA, SEM_SAIDA }, SEM_ITEM, SEM_ITEM, CHAVE, 0, 0
    },
};

/* ---------- 2. Funções auxiliares ---------- */

static int tem_item(const Jogador *j, int item) {
    for (int i = 0; i < j->qtd_itens; i++)
        if (j->inventario[i] == item) return 1;
    return 0;
}

static int adicionar_item(Jogador *j, int item) {
    if (j->qtd_itens >= MAX_ITENS) return 0;
    j->inventario[j->qtd_itens++] = item;
    return 1;
}

static int buscar_nome(const char *nome, const char *lista[], int tamanho) {
    for (int i = 0; i < tamanho; i++)
        if (strcmp(nome, lista[i]) == 0) return i;
    return -1;
}

static void minusculas(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

static int ler_linha(char *cmd, char *arg) {
    char linha[64];
    cmd[0] = arg[0] = '\0';
    if (!fgets(linha, sizeof linha, stdin)) return 0;
    sscanf(linha, "%15s %31s", cmd, arg);
    minusculas(cmd);
    minusculas(arg);
    return 1;
}

static Fase fase_atual(const Jogador *j) {
    return (Fase)((j->passos / PASSOS_POR_FASE) % NUM_FASES);
}

static void mostrar_dica_fase(const Jogador *j) {
    Fase f = fase_atual(j);
    printf("[DICA - %s] ", NOMES_FASES[f]);

    if (j->agua <= 2 && !mundo[j->local].tem_fonte) {
        printf("Sua agua esta critica (%d/%d)! Va para a Feira (fonte) ou use 'descansar' em local seguro.\n",
               j->agua, j->agua_max);
        return;
    }

    if (j->local == CHAPADA && tem_item(j, CORDA) && !j->atalho_chapada) {
        printf("Voce esta na Chapada com a corda na mochila! Experimente 'usar corda'.\n");
        return;
    }

    if (j->local == TESOURO && tem_item(j, CHAVE) && !j->bau_aberto) {
        printf("O bau esta bem na sua frente! Digite 'usar chave' para abrir.\n");
        return;
    }

    if (f == MANHA) {
        if (!mundo[j->local].explorado)
            printf("O sol ainda esta fraco (1 agua/passo). Aproveite para usar 'explorar' aqui!\n");
        else
            printf("Clima ameno para viajar. Use 'usar mapa' para planejar sua rota antes da tarde.\n");
    } else if (f == TARDE) {
        if (j->agua <= 4)
            printf("Sol forte (2 aguas/passo)! Com pouca agua, prefira 'descansar' ate a noite.\n");
        else
            printf("Cada passo agora gasta 2 de agua. So caminhe se tiver cantil cheio ou rumo a uma fonte.\n");
    } else {
        if (!tem_item(j, LAMPIAO) && j->local != TESOURO)
            printf("Sem lampiao voce nao consegue 'explorar' no escuro. Viaje (1 agua/passo) ou use 'descansar'.\n");
        else
            printf("A noite refresca (1 agua/passo) e seu lampiao permite explorar normalmente!\n");
    }
}

static void ajustar_agua(Jogador *j, int delta) {
    j->agua += delta;
    if (j->agua > j->agua_max) j->agua = j->agua_max;
}

static void avancar_passos(Jogador *j, int qtd) {
    Fase antes = fase_atual(j);
    j->passos += qtd;
    Fase depois = fase_atual(j);
    if (depois != antes) {
        printf("%s\n", AVISO_FASE[depois]);
        mostrar_dica_fase(j);
    }
}

/* ---------- 3. Cenas em arte ASCII ---------- */

static void linha_arte(const char *cor, const char *texto) {
    if (usar_cor) printf("|\033[%sm%s\033[0m|\n", cor, texto);
    else          printf("|%s|\n", texto);
}

static void desenhar_cena(const Jogador *j) {
    Fase f = fase_atual(j);
    int interior = (j->local == TESOURO);
    const char *cor_ceu = interior ? "90" : COR_CEU[f];
    char cor_chao[16];
    snprintf(cor_chao, sizeof cor_chao, "%s%s",
             (f == NOITE && !interior) ? "2;" : "", COR_CHAO[j->local]);

    char titulo[64];
    snprintf(titulo, sizeof titulo, "%s | %s", mundo[j->local].nome, NOMES_FASES[f]);
    int tracos = LARGURA_ARTE - 4 - (int)strlen(titulo);
    printf("\n+-- %s ", titulo);
    for (int i = 0; i < tracos; i++) putchar('-');
    printf("+\n");

    for (int i = 0; i < 3; i++)
        linha_arte(cor_ceu, interior ? TETO[f][i] : CEU[f][i]);
    for (int i = 0; i < 6; i++)
        linha_arte(cor_chao, CHAO[j->local][i]);

    putchar('+');
    for (int i = 0; i < LARGURA_ARTE; i++) putchar('-');
    printf("+\n");
}

static void descrever_local(const Jogador *j) {
    const Local *l = &mundo[j->local];
    desenhar_cena(j);
    printf("== %s == [Agua: %d/%d]\n%s\n", l->nome, j->agua, j->agua_max, l->descricao);
    if (l->item != SEM_ITEM)
        printf("Voce ve aqui: %s\n", NOMES_ITENS[l->item]);
    printf("Saidas:");
    for (int d = 0; d < NUM_DIRECOES; d++)
        if (l->saidas[d] != SEM_SAIDA) printf(" %s", NOMES_DIRECOES[d]);
    printf("\n");
}

/* ---------- 4. Eventos e interações ---------- */

static void evento_viagem(Jogador *j) {
    int sorteio = rand() % 10;
    if (sorteio == 0 || sorteio == 1) {
        printf("[EVENTO] Voce encontrou um pe de umbu carregado! (+2 de agua)\n");
        ajustar_agua(j, 2);
    } else if (sorteio == 2) {
        printf("[EVENTO] Uma brisa fresca soprou do vale e poupou seu cantil! (+1 de agua)\n");
        ajustar_agua(j, 1);
    } else if (sorteio == 3) {
        if (tem_item(j, CORDA)) {
            printf("[EVENTO] Voce cruzou uma ribanceira usando sua corda sem se cansar!\n");
        } else {
            printf("[EVENTO] Terreno acidentado! O esforco extra consumiu 1 de agua a mais.\n");
            ajustar_agua(j, -1);
        }
    } else if (sorteio == 4 && fase_atual(j) == TARDE) {
        printf("[EVENTO] Miragem no horizonte! Voce se desviou um pouco sob o sol (-1 agua).\n");
        ajustar_agua(j, -1);
    }
}

static void mostrar_mapa(const Jogador *j) {
    char m[NUM_LOCAIS];
    for (int i = 0; i < NUM_LOCAIS; i++)
        m[i] = (j->local == i) ? '*' : ' ';

    printf("\n--- MAPA DO NORDESTE (* = voce) ---\n");
    printf("                  [Gruta Tesouro %c]\n", m[TESOURO]);
    printf("                          |\n");
    printf("   [Sertao %c] ------ [Chapada %c]\n", m[SERTAO], m[CHAPADA]);
    printf("       |\n");
    printf("[Casa Farinha %c] --- [Feira Caruaru %c]\n", m[CASA], m[FEIRA]);
}

/* ---------- 5. Comandos ---------- */

static void cmd_ir(Jogador *j, const char *arg) {
    int d = buscar_nome(arg, NOMES_DIRECOES, NUM_DIRECOES);
    if (d < 0) { printf("Ir para onde? (norte, sul, leste, oeste)\n"); return; }

    int destino = mundo[j->local].saidas[d];
    if (destino == SEM_SAIDA) { printf("Nao ha caminho para o %s.\n", arg); return; }

    int exige = mundo[destino].exige;
    if (exige != SEM_ITEM && !tem_item(j, exige)) {
        printf("Voce precisa de: %s\n", NOMES_ITENS[exige]);
        return;
    }

    j->local = destino;
    avancar_passos(j, 1);

    if (mundo[destino].tem_fonte) {
        j->agua = j->agua_max;
        printf("Voce encheu o cantil na fonte! (%d/%d)\n", j->agua, j->agua_max);
    } else {
        int gasto = (fase_atual(j) == TARDE) ? 2 : 1;
        ajustar_agua(j, -gasto);
        if (gasto > 1)
            printf("O calor da tarde consumiu %d de agua!\n", gasto);
        evento_viagem(j);
    }
    descrever_local(j);
}

static void cmd_pegar(Jogador *j, const char *arg) {
    Local *l = &mundo[j->local];
    int item = buscar_nome(arg, NOMES_ITENS, NUM_ITENS);

    if (item < 0 || l->item != item) { printf("Nao ha '%s' aqui.\n", arg); return; }
    if (!adicionar_item(j, item))    { printf("Inventario cheio!\n"); return; }

    l->item = SEM_ITEM;
    printf("Voce pegou: %s\n", NOMES_ITENS[item]);
}

static void cmd_explorar(Jogador *j) {
    Local *l = &mundo[j->local];
    Fase f = fase_atual(j);

    if (f == NOITE && j->local != TESOURO && !tem_item(j, LAMPIAO)) {
        printf("Esta escuro demais para explorar! Acenda um 'lampiao' ou use 'descansar'.\n");
        return;
    }

    printf("%s\n", l->detalhe_explorar);
    if (l->explorado) {
        printf("Voce ja vasculhou cada canto deste lugar.\n");
        return;
    }

    l->explorado = 1;
    if (l->item_oculto != SEM_ITEM) {
        if (adicionar_item(j, l->item_oculto)) {
            printf("Item obtido: %s!\n", NOMES_ITENS[l->item_oculto]);
            l->item_oculto = SEM_ITEM;
        } else {
            l->item = l->item_oculto;
            l->item_oculto = SEM_ITEM;
            printf("Sua mochila estava cheia; o item ficou no chao.\n");
        }
    } else if (j->local == SERTAO) {
        ajustar_agua(j, 2);
        printf("Agua atual: %d/%d.\n", j->agua, j->agua_max);
    }
}

static void cmd_usar(Jogador *j, const char *arg) {
    int item = buscar_nome(arg, NOMES_ITENS, NUM_ITENS);
    if (item < 0 || !tem_item(j, item)) {
        printf("Voce nao possui '%s'.\n", arg);
        return;
    }

    if (item == MAPA) {
        mostrar_mapa(j);
    } else if (item == LAMPIAO) {
        printf("Voce acende o lampiao. A chama ilumina ao redor mesmo a noite!\n");
    } else if (item == CANTIL) {
        printf("Seu cantil reforcado comporta ate %d doses de agua.\n", j->agua_max);
    } else if (item == CORDA) {
        if (j->local == CHAPADA && !j->atalho_chapada) {
            j->atalho_chapada = 1;
            j->agua_max += 3;
            ajustar_agua(j, 3);
            adicionar_item(j, CANTIL);
            printf("Voce prende a corda na fenda, desce ate uma nascente oculta e acha um CANTIL!\n");
            printf("Capacidade maxima de agua aumentada para %d!\n", j->agua_max);
        } else {
            printf("Voce verifica o no da corda. Pode ser util em paredoes ou ribanceiras.\n");
        }
    } else if (item == CHAVE) {
        if (j->local == TESOURO) {
            j->bau_aberto = 1;
            printf("CREC! A chave gira no cadeado enferrujado e a tampa do bau se abre!\n");
        } else {
            printf("Nao ha nenhuma fechadura aqui para usar a chave.\n");
        }
    }
}

static void cmd_descansar(Jogador *j) {
    int faltam = PASSOS_POR_FASE - (j->passos % PASSOS_POR_FASE);
    printf("Voce faz uma pausa a sombra ate o tempo mudar...\n");
    avancar_passos(j, faltam);
    if (mundo[j->local].tem_fonte || j->local == CASA) {
        ajustar_agua(j, 1);
        printf("O descanso recuperou +1 de agua (%d/%d).\n", j->agua, j->agua_max);
    }
}

static void cmd_inventario(const Jogador *j) {
    printf("Agua: %d/%d | Fase: %s | Itens:", j->agua, j->agua_max, NOMES_FASES[fase_atual(j)]);
    if (j->qtd_itens == 0) printf(" (vazio)");
    for (int i = 0; i < j->qtd_itens; i++)
        printf(" %s", NOMES_ITENS[j->inventario[i]]);
    printf("\n");
}

static void cmd_ajuda(void) {
    printf("Comandos: ir <dir> | olhar | explorar | pegar <item> | usar <item>\n");
    printf("          descansar | dica | inventario | tutorial | cores | ajuda | sair\n");
}

static void cmd_tutorial(void) {
    printf("\n--- COMO JOGAR ---\n");
    printf("Objetivo: ache a Gruta do Tesouro e use a chave para abrir o bau.\n");
    printf("Explorar: use 'explorar' em cada local para achar itens ocultos e dicas.\n");
    printf("Usar: 'usar mapa' mostra o mapa; 'usar corda' e 'usar chave' interagem com locais.\n");
    printf("Ciclo do dia: Manha (1 agua), Tarde (2 aguas/passo), Noite (pede lampiao p/ explorar).\n");
    printf("Descansar e Dica: 'descansar' avanca a fase do dia; 'dica' da orientacao contextual.\n");
    cmd_ajuda();
}

static void cmd_cores(void) {
    usar_cor = !usar_cor;
    printf("Cores %s.\n", usar_cor ? "ativadas" : "desativadas");
}

static int executar(Jogador *j, const char *cmd, const char *arg) {
    if      (strcmp(cmd, "ir") == 0)         cmd_ir(j, arg);
    else if (strcmp(cmd, "olhar") == 0)      descrever_local(j);
    else if (strcmp(cmd, "explorar") == 0)   cmd_explorar(j);
    else if (strcmp(cmd, "pegar") == 0)      cmd_pegar(j, arg);
    else if (strcmp(cmd, "usar") == 0)       cmd_usar(j, arg);
    else if (strcmp(cmd, "descansar") == 0)  cmd_descansar(j);
    else if (strcmp(cmd, "dica") == 0)       mostrar_dica_fase(j);
    else if (strcmp(cmd, "inventario") == 0) cmd_inventario(j);
    else if (strcmp(cmd, "tutorial") == 0)   cmd_tutorial();
    else if (strcmp(cmd, "cores") == 0)      cmd_cores();
    else if (strcmp(cmd, "ajuda") == 0)      cmd_ajuda();
    else if (strcmp(cmd, "sair") == 0)       { printf("Ate a proxima!\n"); return 0; }
    else if (cmd[0] != '\0')                 printf("Nao entendi. Digite 'ajuda'.\n");
    return 1;
}

/* ---------- 6. Tutorial guiado ---------- */

enum { TUT_SAIR = -1, TUT_PULAR = 0, TUT_OK = 1 };

static int passo(Jogador *j, const char *texto, const char *cmd_esp, const char *arg_esp) {
    char cmd[16], arg[32];
    printf("\n[TUTORIAL] %s\n", texto);
    while (1) {
        printf("\n> ");
        if (!ler_linha(cmd, arg)) return TUT_SAIR;
        if (strcmp(cmd, "sair") == 0) { printf("Ate a proxima!\n"); return TUT_SAIR; }
        if (strcmp(cmd, "pular") == 0) {
            printf("[TUTORIAL] Tutorial pulado. Boa sorte!\n");
            return TUT_PULAR;
        }
        if (strcmp(cmd, cmd_esp) == 0 && strcmp(arg, arg_esp) == 0) {
            executar(j, cmd, arg);
            return TUT_OK;
        }
        printf("[TUTORIAL] Quase! Neste passo, digite: %s%s%s (ou 'pular')\n",
               cmd_esp, arg_esp[0] ? " " : "", arg_esp);
    }
}

static int tutorial_guiado(Jogador *j) {
    int r;
    printf("\n[TUTORIAL] Bem-vindo(a)! Vamos praticar como explorar e usar itens.\n");
    printf("(Digite 'pular' a qualquer momento para sair do tutorial.)\n");

    if ((r = passo(j, "Passo 1/5: veja onde voce esta. Digite: olhar", "olhar", "")) != TUT_OK) return r;
    if ((r = passo(j, "Passo 2/5: pegue o mapa no chao. Digite: pegar mapa", "pegar", "mapa")) != TUT_OK) return r;
    if ((r = passo(j, "Passo 3/5: abra o mapa para ver a regiao. Digite: usar mapa", "usar", "mapa")) != TUT_OK) return r;
    if ((r = passo(j, "Passo 4/5: vasculhe a casa por itens escondidos. Digite: explorar", "explorar", "")) != TUT_OK) return r;
    if ((r = passo(j, "Passo 5/5: siga para a feira com fonte. Digite: ir leste", "ir", "leste")) != TUT_OK) return r;

    printf("\n[TUTORIAL] Boa! Use 'dica' sempre que quiser ajuda contextual. Boa sorte!\n");
    return TUT_OK;
}

/* ---------- 7. Loop principal ---------- */

int main(int argc, char **argv) {
    Jogador jogador = {
        .local = CASA,
        .agua = AGUA_INICIAL,
        .agua_max = AGUA_INICIAL,
        .passos = 0,
        .qtd_itens = 0,
        .bau_aberto = 0,
        .atalho_chapada = 0
    };
    char cmd[16], arg[32];

    srand((unsigned int)time(NULL));

    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--sem-cor") == 0) usar_cor = 0;

    printf("=== TESOURO DO NORDESTE ===\n");
    printf("Primeira vez jogando? Quer o tutorial guiado? (s/n) ");
    if (!ler_linha(cmd, arg)) return 0;

    if (cmd[0] == 's') {
        descrever_local(&jogador);
        mostrar_dica_fase(&jogador);
        if (tutorial_guiado(&jogador) == TUT_SAIR) return 0;
    } else {
        cmd_ajuda();
        descrever_local(&jogador);
        mostrar_dica_fase(&jogador);
    }

    while (1) {
        if (jogador.local == TESOURO && jogador.bau_aberto) {
            printf("\n*** PARABENS! Voce destrancou o bau e encontrou o tesouro! ***\n");
            break;
        }
        if (jogador.agua <= 0) {
            printf("\nSua agua acabou sob o sol do Nordeste... Fim de jogo.\n");
            break;
        }

        printf("\n> ");
        if (!ler_linha(cmd, arg)) break;
        if (!executar(&jogador, cmd, arg)) break;
    }
    return 0;
}
