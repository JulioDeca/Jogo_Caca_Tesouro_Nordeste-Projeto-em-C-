# 🏜️ Tesouro do Nordeste

> Jogo de caça ao tesouro em C, em modo texto (terminal), ambientado no Nordeste brasileiro.

**Status:** 🚧 Em planejamento
**Product Owner (PO):** _[seu nome]_
**Time de desenvolvimento:** _[nomes dos amigos]_

---

## 📖 1. Sobre o jogo

O jogador é um(a) aventureiro(a) que descobre um mapa antigo, escondido num baú de uma casa de farinha no interior do sertão. O mapa leva a um tesouro lendário espalhado por pistas em vários cenários do Nordeste. Para chegar até ele, é preciso **explorar locais, coletar itens, resolver enigmas e administrar recursos** (água, energia e tempo).

### Pitch em uma frase
_Explore o Nordeste, siga as pistas do mapa e encontre o tesouro antes que a água acabe._

### Por que Nordeste?
O tema valoriza a cultura regional: cenários, personagens, comidas, lendas e o vocabulário nordestino fazem parte da experiência, não são só decoração.

---

## 🎯 2. Visão do produto

| Item | Descrição |
|------|-----------|
| **Objetivo do jogador** | Reunir as pistas e chegar ao tesouro final |
| **Plataforma** | Terminal (Linux, Windows e macOS) |
| **Linguagem** | C (padrão C11) |
| **Interface** | Texto, com menus numerados |
| **Duração de uma partida** | ~15 a 30 minutos _(a validar)_ |
| **Público** | Amigos, projeto acadêmico e portfólio |

### O que está FORA do escopo (por enquanto)
- Gráficos ou interface gráfica
- Multiplayer online
- Sons e música

---

## 🗺️ 3. Mundo do jogo (proposta inicial)

Cada local é uma "sala" do mapa, com descrição, itens, personagens e saídas.

| # | Local | Inspiração | Ideia de desafio |
|---|-------|------------|------------------|
| 1 | Casa de Farinha (início) | Interior nordestino | Tutorial: encontrar o mapa e a primeira pista |
| 2 | Feira de Caruaru | Pernambuco | Barganhar com feirantes e trocar itens |
| 3 | Sertão / Caatinga | Semiárido | Gerenciar água; enigma do mandacaru |
| 4 | Pelourinho | Salvador, BA | Enigma histórico e charada com baiana de acarajé |
| 5 | Lençóis Maranhenses | Maranhão | Atravessar as dunas e lagoas com o item certo |
| 6 | Chapada Diamantina | Bahia | Explorar a caverna e achar a chave |
| 7 | Olinda / Recife | Pernambuco | Decifrar a pista do carnaval |
| 8 | Jericoacoara | Ceará | Cenário de transição e recompensa |
| 9 | 🏆 Local do Tesouro | _[a definir]_ | Desafio final |

> ✏️ **PO:** ajustem a lista conforme o escopo. Menos locais bem feitos valem mais que muitos locais pela metade.

### Itens (exemplos)
🗺️ Mapa · 💧 Cantil · 🔦 Lamparina · 🧵 Corda · 🔑 Chave de ferro · 🪗 Sanfona · 🥥 Água de coco · 🍬 Rapadura

### Personagens (exemplos)
Cordelista, vaqueiro, baiana do acarajé, jangadeiro, mestre de capoeira, rendeira.

---

## 🎮 4. Regras e mecânicas

1. O jogador começa na **Casa de Farinha** com um cantil cheio.
2. A cada movimento entre locais, **gasta 1 unidade de água** (exceto em locais com fonte).
3. Se a água chegar a 0, **fim de jogo**.
4. Itens ficam no **inventário** (capacidade máxima: _N itens_).
5. Alguns locais exigem um **item específico** para avançar.
6. Enigmas dão **pistas**; as pistas formam o caminho até o tesouro.
7. Vitória: chegar ao local do tesouro com todas as pistas necessárias.

### Comandos do jogador (proposta)

| Comando | Ação |
|---------|------|
| `ir <direção>` | Move-se (norte, sul, leste, oeste) |
| `olhar` | Descreve o local atual |
| `pegar <item>` | Pega um item |
| `usar <item>` | Usa um item do inventário |
| `inventario` | Lista os itens |
| `falar <personagem>` | Conversa com um personagem |
| `ajuda` | Mostra os comandos |
| `sair` | Encerra o jogo |

---

## 🧱 5. Estrutura do projeto

```
tesouro-do-nordeste/
├── README.md
├── Makefile
├── src/
│   ├── main.c          # Loop principal do jogo
│   ├── jogo.c / jogo.h         # Estado do jogo e regras
│   ├── mapa.c / mapa.h         # Locais e conexões
│   ├── jogador.c / jogador.h   # Inventário, água, posição
│   ├── itens.c / itens.h       # Definição dos itens
│   └── enigmas.c / enigmas.h   # Enigmas e pistas
├── data/                # Textos e descrições (opcional)
├── docs/                # Diagramas, design, ideias
└── tests/               # Testes
```

> Cada dupla/pessoa fica responsável por um módulo (veja a seção 8).

---

## ⚙️ 6. Como compilar e rodar

### Requisitos
- Compilador **GCC** (ou Clang)
- **Make** (opcional)
- **Git**

### Compilando

```bash
# Com Makefile
make

# Ou manualmente
gcc -Wall -Wextra -std=c11 -o tesouro src/*.c
```

### Executando

```bash
./tesouro        # Linux / macOS
tesouro.exe      # Windows
```

---

## 👥 7. Papéis

| Papel | Quem | Responsabilidade |
|-------|------|------------------|
| **Product Owner** | _[você]_ | Define a visão, prioriza o backlog, aceita (ou não) as entregas |
| **Desenvolvedores** | _[amigos]_ | Implementam, testam e dão sugestões |

**Combinados:**
- O PO decide **o quê** e **em que ordem**. O time decide **como** implementar.
- Dúvida sobre o escopo? Pergunte ao PO antes de codar.
- Ideias novas são bem-vindas, mas vão para o backlog primeiro (abra uma _issue_).

---

## 🔄 8. Como trabalhamos

### Backlog inicial (por prioridade)

| Prioridade | História de usuário | Responsável |
|:---:|---|---|
| 🔴 Alta | Como jogador, quero ver um menu inicial para começar ou sair | _—_ |
| 🔴 Alta | Como jogador, quero me mover entre locais para explorar o mapa | _—_ |
| 🔴 Alta | Como jogador, quero ver a descrição do local atual | _—_ |
| 🟠 Média | Como jogador, quero pegar e usar itens | _—_ |
| 🟠 Média | Como jogador, quero ver meu inventário | _—_ |
| 🟠 Média | Como jogador, quero controlar minha água para ter desafio | _—_ |
| 🟡 Baixa | Como jogador, quero resolver enigmas para ganhar pistas | _—_ |
| 🟡 Baixa | Como jogador, quero salvar e carregar o progresso | _—_ |

### Definição de "Pronto" ✅
Uma tarefa só está pronta quando:
- [ ] Compila sem warnings (`-Wall -Wextra`)
- [ ] Foi testada manualmente rodando o jogo
- [ ] Não tem vazamento de memória (`valgrind`, se usar alocação dinâmica)
- [ ] Foi revisada por outra pessoa do time
- [ ] O PO aprovou

### Fluxo com Git

```bash
git checkout -b feature/nome-da-tarefa
# ...trabalhe...
git add .
git commit -m "feat: adiciona sistema de inventário"
git push origin feature/nome-da-tarefa
# abra um Pull Request para a branch main
```

- **Nunca** faça commit direto na `main`.
- Commits curtos, com mensagem clara (`feat:`, `fix:`, `docs:`, `refactor:`).
- Todo Pull Request precisa de pelo menos **1 revisão**.

---

## 🧑‍💻 9. Convenções de código

- **Nomes** em português, `snake_case` para funções e variáveis (`mover_jogador`, `agua_restante`).
- **Constantes e macros** em `MAIUSCULAS` (`MAX_ITENS`).
- Cada módulo tem seu `.h` e seu `.c`.
- Funções pequenas, com uma responsabilidade só.
- Comente o **porquê**, não o óbvio.
- Sempre libere o que alocar (`malloc` → `free`).
- Sem variáveis globais, salvo justificativa combinada com o time.

---

## 🗓️ 10. Roadmap

| Marco | Entrega |
|-------|---------|
| **v0.1** | Menu, mapa com 3 locais e movimentação |
| **v0.2** | Inventário e itens |
| **v0.3** | Sistema de água e condições de derrota |
| **v0.4** | Enigmas e pistas |
| **v1.0** | Mapa completo, tesouro final e polimento |

_As datas ficam a cargo do grupo._

---

## 💡 11. Ideias para o futuro

- Salvar e carregar partida
- Níveis de dificuldade
- Eventos aleatórios (chuva no sertão, cangaceiros)
- Modo "cordel": narração em versos
- Placar de melhores tempos

---

## 📚 12. Referências

- Livro _The C Programming Language_ (Kernighan & Ritchie)
- Documentação do GCC: https://gcc.gnu.org/onlinedocs/
- _[outras referências do grupo]_

---

## 📜 Licença

_[Defina: MIT, uso acadêmico etc.]_

---

*Feito com ☀️, cuscuz e muito código. Bora achar esse tesouro!*
