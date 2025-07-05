#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
Assume-se que:
 - a CPU tem 32 bits de endereco virtual e paginas de 4Kbytes: 20 bits para
   o numero da pagina e 12 bits para o deslocamento  (Intel x/86 like)
 - logo, o numero de entradas da tabela de paginas e' 2^20=1024*1024
*/
#define PAGESIZE 4096
#define OFFSET_BITS 12
#define MAX_VIRTUAL_PAGES (1024 * 1024)
#define MAX_FRAMES 64

#define FALSE 0
#define TRUE 1

/* Tipo base da tabela de paginas */
typedef struct pagetable_entry {
  unsigned int frame;
  int valid;
} pagetable_entry;

/* Tipo base da tabela de paginas invertida */
typedef struct frametable_entry {
  unsigned int page;
  int dirty;
  int referenced;
  unsigned int load_time;
  unsigned int last_reference_time;
  // novo campo: quantidade de vezes que e' referenciado
  unsigned int reference_num;
} frametable_entry;

/* Tabela de paginas inicializada a "todas as entradas invalidas" */
pagetable_entry pageTable[MAX_VIRTUAL_PAGES];

/* Indicador de que o frame está livre.  Valor a usar no campo "page". */
#define FREE -1

/* Tabela de paginas invertida inicializada a "todas as frames livres" */
frametable_entry frameTable[MAX_FRAMES];

/* Contador de referencias de memoria */
int step;

/* Contador de faltas de pagina */
int numberOfPageFaults;

/* Contador de paginas reutilizadas que estavam 'dirty' */
int numberOfPagesWritten;

/* Numero de frames = numero de paginas de memoria fisica */
int n_frames;

/* Tipo das funcoes de seleccao de frame vitima */
typedef int (*selectVictimF)(void);

/* Tipo base da tabela com as strings identificadoras dos algoritmos e
   apontadores para as respectivas funcoes */
typedef struct algorithms_t {
  char *name;
  selectVictimF function;
} algorithms_t;

/* Protótipos das funções de selecção de frame vítima */
int selectVictimRandom(void);
int selectVictimFIFO(void);
int selectVictimLRU(void);
int selectVictimSC(void);
int selectVictimLFU(void);
int selectVictimMFU(void);

/* Tabela com as strings identificadoras dos algoritmos e apontadores
   para as respectivas funcoes */
algorithms_t algorithms[] = {
    {"random", selectVictimRandom},
    {"fifo", selectVictimFIFO},
    {"lru", selectVictimLRU},
    {"sc", selectVictimSC},
    {"lfu", selectVictimLFU},
    {"mfu", selectVictimMFU},
    {NULL, NULL} /* Mark the end of the vector */
};

/* Aponta para a funcao apropriada para a seleccao de um frame vitima */
selectVictimF selectVictim = NULL;

/* As funcoes "initPageTable()" e "initFrames()" nao sao necessarias.
   As variaveis sao inicializadas estaticamente na declaracao! */
void initPageTable(void) {
  /* todas as entradas invalidas */
  int i;
  for (i = 0; i < MAX_VIRTUAL_PAGES; i++)
    pageTable[i].valid = FALSE;
}

void initFrames(void) {
  /* todas as frames livres */
  int i;
  for (i = 0; i < n_frames; i++)
    frameTable[i].page = FREE;
}

selectVictimF findAlgorithm(const char *name) {
  /* Devolver um apontador para a funcao associada ao algoritmo cujo
         nome e dado.  Devolve NULL se o nome do algoritmo e invalido,
         i.e., nao consta da tabela.
   */
  int i;
  int len = strlen(name);
  for (i = 0; algorithms[i].name != NULL; i++)
    if (strncmp(name, algorithms[i].name, len) == 0)
      return algorithms[i].function;
  return NULL;
}

int getFreeFrame(void) {
  /* devolve a primeira frame livre.
     devolve -1 se nao ha nenhuma livre
   */
  int i;

  for (i = 0; i < n_frames; i++)
    if (frameTable[i].page == FREE)
      return i;
  return -1;
}

int selectVictimRandom(void) {
  /* estrategia de substituicao que escolhe como vitima
     uma frame ao acaso
   */
  int victim;

  victim = random() % n_frames;

  return victim;
}

int selectVictimFIFO(void) {
  /* estrategia de substituicao que escolhe como vitima a
     frame que esta ha mais tempo carregada em memoria
   */

  int victim;

  /* ... */

  return victim;
}

int selectVictimLRU(void) {
  /* estrategia de substituicao que escolhe como vitima a
     frame que esta ha mais tempo sem ser referenciada
   */

  int victim;

  /* ... */

  return victim;
}

int selectVictimSC(void) {
  /* estrategia de substituicao que a vitima conforme a
     estrategia do algoritmo de segunda chance
   */

  int victim;

  /* ... */

  return victim;
}

int selectVictimLFU(void) {
  /* estrategia de substituicao que escolhe como vitima a
     frame que foi referenciada menos vezes
   */

  int victim;

  /* ... */

  return victim;
}

int selectVictimMFU(void) {
  /* estrategia de substituicao que escolhe como vitima a
     frame que foi referenciada mais vezes
   */

  int victim;

  /* ... */

  return victim;
}

int findPage(int frameToUse) {
  /* retorna o numero da página carregada numa frame que e' dada
     como argumento;
     so e' chamada para frames que tem uma pagina carregada
   */
  return frameTable[frameToUse].page;
}

void initSystem(void) {
  initFrames();
  initPageTable();
  step = 0;
  numberOfPageFaults = 0;
  numberOfPagesWritten = 0;
  srandom(time(NULL)); /* pseudo-random seed */
}

void printResults(void) {
  printf("number of memory references = %u \n", step);
  printf("number of page faults = %u \n", numberOfPageFaults);
  printf("number of pages written on disk = %u\n", numberOfPagesWritten);
}

void simulateOneStep(unsigned int addr, char rw) {
  int page, new_frame, victim;

  step++;

  // obtendo o endereço da página a partir do endereço virtual passado
  page = addr >> OFFSET_BITS;

  /* pagina esta em RAM? */
  if (!pageTable[page].valid) {
    /* page fault! */
    numberOfPageFaults++;
    new_frame = getFreeFrame(); /* tenta obter uma frame livre */

    if (new_frame >= 0) { /* havia uma frame livre */
                          /* actualiza tabela de páginas */
      pageTable[page].frame = new_frame;
      pageTable[page].valid = TRUE;

      /* actualiza tabela de frames
       note que o passo da simulacao em que se esta
       é usado como relogio ... */
      frameTable[new_frame].page = page;
      frameTable[new_frame].load_time = step;
      frameTable[new_frame].last_reference_time = step;
      frameTable[new_frame].dirty = FALSE;
      frameTable[new_frame].reference_num = 0;
    } else {
      /* nao havia frames livres - chamar um algoritmo de substituição
       de paginas para escolher a frame que vai ser ocupada */
      new_frame = selectVictim();
      victim = findPage(new_frame);

      if (frameTable[new_frame].dirty)
        numberOfPagesWritten++;

      /* actualiza tabela de paginas  */
      pageTable[victim].valid = FALSE;
      pageTable[page].frame = new_frame;
      pageTable[page].valid = TRUE;

      /* actualiza tabela de frames */
      frameTable[new_frame].page = page;
      frameTable[new_frame].load_time = step;
      frameTable[new_frame].last_reference_time = step;
      frameTable[new_frame].dirty = FALSE;
      frameTable[new_frame].referenced = FALSE;
      frameTable[new_frame].reference_num = 0;
    }
  } else { /* caso em que nao houve falta de pagina */
    frameTable[pageTable[page].frame].referenced = TRUE;
    frameTable[pageTable[page].frame].last_reference_time = step;
    // novo: incrementa a quantidade de referencias
    frameTable[pageTable[page].frame].reference_num++;
  }

  if (rw == 'W')
    frameTable[pageTable[page].frame].dirty = TRUE;
}

void simulateAllTheSteps(FILE *tf) {
  unsigned addr;
  char rw;

  fscanf(tf, "%x %c", &addr, &rw);
  while (!feof(tf)) {
    simulateOneStep(addr, rw);
    fscanf(tf, "%x %c", &addr, &rw);
  }
}

int main(int argc, char *argv[]) {
  FILE *traceFile;
  const char *execname;

  /* Determinar o nome do executavel como elemento do pathname
         depois do ultimo caracter '/'  */
  execname = strrchr(argv[0], '/');
  if (execname == NULL)
    execname = argv[0];
  else
    execname++;

  if (argc != 4) { /* Garantir que foram passados 1+3
                      argumentos de linha de comando */
    printf("\n%s traceFile numero-de-frames algoritmo\n\
    'numero-de-frames'  nao pode exceder 64\n\
    'algoritmo'         pode ser random, fifo, lru, sc, lfu, mfu\n\n",
           argv[0]);
    exit(1);
  }

  /* arg1 = nome do ficheiro com o tracado */
  if ((traceFile = fopen(argv[1], "r")) == NULL) {
    printf("Trace-file %s doesn't exist\n", argv[1]);
    exit(1);
  }

  /* arg2 = numero de frames */
  n_frames = atoi(argv[2]);
  if (n_frames > MAX_FRAMES) {
    printf("n_frames greater than %d\n", MAX_FRAMES);
    exit(1);
  }

  /* arg3 = algoritmo a aplicar */
  selectVictim = findAlgorithm(argv[3]);
  if (selectVictim == NULL) {
    printf("Invalid algoritms '%s'\n", argv[3]);
    exit(1);
  }

  /* Corpo do programa */
  initSystem();
  simulateAllTheSteps(traceFile);
  printResults();

  return 0;
}
