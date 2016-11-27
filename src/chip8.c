#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MEMSIS 4096

struct maquina_t {
    uint8_t mem[MEMSIS];     // Banco de memoria disponivel pela CPU
    uint16_t pc;             // Contador de programa

    uint16_t pilha[16];      // Pilha de 16 registradores
    uint16_t sp;             // Ponteiro da pilha

    uint8_t v[16];           // 16 registradores para uso geral (Vx...VF)
    uint8_t i;               // Registrador especial de direcionamento I
    uint8_t dt, st;          // Temporizadores
};

void inic_maquina(struct maquina_t* maquina)
{
    maquina->sp = maquina->i = maquina->dt = maquina->st = 0x00;
    maquina->pc = 0x200;

    memset(maquina->mem, 0, MEMSIS);
    memset(maquina->pilha, 0, 32);
    memset(maquina->v, 0, 16);


    /*for (int i = 0; i < MEMSIS; i++)
        maquina->mem[i] = 0x00;
    for (int i = 0; i < 16; i++) {
        maquina->pilha[i] = 0;
        maquina->v[i] = 0;
    }*/
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
    inic_maquina(&maq);
    carrega_rom(&maq);
    int sair = 0;
    while (!sair) {
        // Ler o próximo opcode da memoria
        uint16_t opcode = (maq.mem[maq.pc] << 8) | maq.mem[maq.pc + 1];
        maq.pc = (maq.pc + 2) & 0xFFF;

        /*printf("%x\n", opcode);
        maq.pc += 2;
        if(maq.pc == MEMSIS)
           maq.pc = 0;*/

        uint16_t nnn = opcode & 0x0FFF;
        uint8_t kk = opcode & 0xFF;
        uint8_t n = opcode & 0xF;
        uint8_t x = (opcode >> 8) & 0xF;
        uint8_t y = (opcode >> 4) & 0xF;
        uint8_t p = (opcode >> 12);

        switch (p) {
          case 0:
              if (opcode == 0x00E0) {
                printf("CLS\n");
              } else if (opcode == 0x00EE) {
               printf("RET\n");
              }
              break;
          case 1:
              // JP nnn: contador para nnn
              maq.pc = nnn;
              break;
          case 2:
              printf("CALL %x\n", nnn);
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
                printf("LD I, %x\n", nnn);
                break;
            case 0xB:
                //JP V0, nnn: pc = v[0] + nnn
                maq.pc = maq.v[0] + nnn;
                break;
            case 0xC:
                printf("RND %x, %x\n", x, kk);
                break;
            case 0xD:
                printf("DRW %x, %x, %x\n", x, y, n);
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

  return 0;
}
