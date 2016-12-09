#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <allegro5/allegro.h>

#define MEMSIS 4096

const int LARGURA_TELA = 640;
const int ALTURA_TELA = 320;

struct maquina_t {
    uint8_t mem[MEMSIS];     // Banco de memoria disponivel pela CPU
    uint16_t pc;             // Contador de programa

    uint16_t pilha[16];      // Pilha de 16 registradores
    uint16_t sp;             // Ponteiro da pilha

    uint8_t v[16];           // 16 registradores para uso geral (Vx...VF)
    uint8_t i;               // Registrador especial de direcionamento I
    uint8_t dt, st;          // Temporizadores

    char tela[2048];
};



void inic_maquina(struct maquina_t* maquina)
{
    maquina->sp = maquina->i = maquina->dt = maquina->st = 0x00;
    maquina->pc = 0x200;

    memset(maquina->mem, 0, MEMSIS);
    memset(maquina->pilha, 0, 32);
    memset(maquina->v, 0, 16);

}

void carrega_rom(struct maquina_t* maquina)
{
  FILE* fp = fopen("PONG", "r");
  if (fp == NULL){
    fprintf(stderr, "Não foi possível abrir o arquivo de ROM especificado.\n");
    exit(1);
  }
  // Obtendo o tamanho do arquivo
  fseek(fp, 0, SEEK_END);
  int length = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  fread(maquina->mem + 0x200, length, 1, fp);

  fclose(fp);
}


int main(int argc, const char * argv[])
{

    struct maquina_t maq;
    int sair = 0;

    ALLEGRO_DISPLAY *janela = NULL;
    ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;

    if (!al_init())
    {
      fprintf(stderr, "Falha ao inicializar a Allegro.\n");
      return -1;
    }


    janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if (!janela)
    {
      fprintf(stderr, "Falha ao criar janela.\n");
      return -1;
    }


    fila_eventos = al_create_event_queue();
    if (!fila_eventos)
    {
      fprintf(stderr, "Falha ao criar fila de eventos.\n");
      al_destroy_display(janela);
      return -1;
    }

    al_register_event_source(fila_eventos, al_get_display_event_source(janela));

    //Inicia Emulador
    inic_maquina(&maq);
    carrega_rom(&maq);
    srand(time(NULL));


    while (!sair) {

      //  configura uma localização aleatória
        int a = rand() % LARGURA_TELA;
        int b = rand() % ALTURA_TELA;
        //configura uma cor aleatória
        int red_color = rand() % 190;
        int green_color = rand() % 255;
        int blue_color = rand() % 1;
        al_put_pixel(a, b, al_map_rgb(red_color, green_color, blue_color ));
        

        al_flip_display();



        ALLEGRO_EVENT evento;
        ALLEGRO_TIMEOUT timeout;
        al_init_timeout(&timeout, 0.05);

        int tem_eventos = al_wait_for_event_until(fila_eventos, &evento, &timeout);

        if (tem_eventos && evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break;
        }


        // Ler o próximo opcode da memoria
        uint16_t opcode = (maq.mem[maq.pc] << 8) | maq.mem[maq.pc + 1];
        maq.pc = (maq.pc + 2) & 0xFFF;

        uint16_t nnn = opcode & 0x0FFF;
        uint8_t kk = opcode & 0xFF;
        uint8_t n = opcode & 0xF;
        uint8_t x = (opcode >> 8) & 0xF;
        uint8_t y = (opcode >> 4) & 0xF;
        uint8_t p = (opcode >> 12);

        switch (p) {
          case 0:
              if (opcode == 0x00E0) {
                //CLS
                memset(maq.tela,0,2048);
              } else if (opcode == 0x00EE) {
                if (maq.sp > 0)
                    maq.pc = maq.pilha[--maq.sp];
              }
              break;
          case 1:
              // JP nnn: contador para nnn
              maq.pc = nnn;
              break;
          case 2:
              if (maq.sp < 16)
                  maq.pilha[maq.sp++] = maq.pc;
              maq.pc = nnn;
              break;
          case 3:
              //SE x, kk: if v[x] == kk -> pc += 2
              if (maq.v[x] == kk)
                  maq.pc = (maq.pc + 2) & 0xFFF;
              break;
          case 4:
              // SNE x, kk: if v[x] != kk -> pc += 2
              if (maq.v[x] != kk)
                  maq.pc = (maq.pc + 2) & 0xFFF;
              break;
          case 5:
              //SE x, y: if v[x] == v[y] -> pc += 2
              if (maq.v[x] == maq.v[y])
                  maq.pc = (maq.pc + 2) & 0xFFF;
              break;
          case 6:
              //LD x, kk: v[x] = kk
              maq.v[x] = kk;
              break;
          case 7:
              //ADD x, kk: v[x] = (v[x] + kk) & 0xFF
              maq.v[x] = (maq.v[x] + kk) & 0xFF;
              break;
          case 8:
              switch (n) {
                case 0:
                    //LD x, y: v[x] = v[y]
                    maq.v[x] = maq.v[y];
                    break;
                case 1:
                    //OR x, y: v[x] = v[x] | v[y]
                    //   1011 0100
                    //   0100 0110
                    //   ---------
                    //   1111 0110
                    maq.v[x] |= maq.v[y];
                    break;
                case 2:
                    //AND x, y: v[x] = v[x] & v[y]
                    maq.v[x] &= maq.v[y];
                    break;
                case 3:
                    //XOR x, y: v[x] = v[x] ^^ v[y]
                    maq.v[x] ^= maq.v[y];
                    break;
                case 4:
                    //ADD x, y: v[x] += v[y]
                    maq.v[0xF] = (maq.v[x] > maq.v[x] + maq.v[y]);
                    maq.v[x] += maq.v[y];
                    break;
                case 5:
                    //SUB x, y: v[x] -= v[y]
                    maq.v[0xF] = (maq.v[x] > maq.v[y]);
                    maq.v[x] -= maq.v[y];
                    break;
                case 6:
                    //SHR x: v[x] = v[x] >> 1
                    maq.v[0xF] = (maq.v[x] & 1);
                    maq.v[x] >>= 1;
                    break;
                case 7:
                    //SUBN x, y: v[y] - v[x]
                    maq.v[0xF] = (maq.v[y] > maq.v[x]);
                    maq.v[x] = maq.v[y] - maq.v[x];
                    break;
                case 0xE:
                    //SHL x: v[x] = v[x] << 1
                    maq.v[0xF] = ((maq.v[x] & 0x80) != 0);
                    maq.v[x] <<= 1;
                    break;
              }
              break;
            case 9:
                //SNE x, y: v[x] != v[y] -> pc += 2
                if (maq.v[x] != maq.v[y])
                    maq.pc = (maq.pc + 2) & 0xFFF;
                  break;
            case 0xA:
                //LD I, x: I = nnn
                maq.i = nnn;
                break;
            case 0xB:
                //JP V0, nnn: pc = v[0] + nnn
                maq.pc = (maq.v[0] + nnn) & 0xFFF;
                break;
            case 0xC:
                maq.v[x] = rand() & kk;
                break;
            case 0xD:
                for (int j = 0; j < n; j++) {
                    uint8_t sprite = maq.mem[maq.i];
                    for (int i = 0; i < 7; i++) {
                        int px = (maq.v[x] + i) & 63;
                        int py = (maq.v[y] + j) & 31;
                        maq.tela[64 * py + px] = (sprite & (1 << (7-1))) != 0;
                    }
                }
                break;
            case 0xE:
                if (kk == 0x9E) {
                  printf("SKP %x\n", x);
                } else if (kk == 0xA1) {
                  printf("SKNP %x\n", x);
                }
                break;
            case 0xF:
                switch (kk) {
                  case 0x07:
                      //LD V[x], DT: v[x] = DT
                      maq.v[x] = maq.dt;
                      break;
                  case 0x0A:
                      printf("LD %x, K\n", x);
                      break;
                  case 0x15:
                      //LD DT: v[x] -> DT = v[x]
                      maq.dt = maq.v[x];
                      break;
                  case 0x18:
                      //LD ST, v[x] -> ST = v[x]
                      maq.st = maq.v[x];
                      break;
                  case 0x1E:
                      //ADD I, v[x] -> += v[x]
                      maq.i += maq.v[x];
                      break;
                  case 0x29:
                      printf("LD F, %x\n", x);
                      break;
                  case 0x33:
                      printf("LD B, %x\n", x);
                      break;
                  case 0x55:
                      printf("LD [I], %x\n", x);
                      break;
                  case 0x65:
                      printf("LD %x, [I]\n", x);
                      break;
              }
              break;
          }

        }

        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);

  return 0;
}
