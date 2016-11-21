#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    for (int i = 0; i < MEMSIS; i++)
        maquina->mem[i] = 0x00;
    for (int i = 0; i < 16; i++) {
        maquina->pilha[i] = 0;
        maquina->v[i] = 0;
    }
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
        // Ler opcode
        uint16_t opcode = (maq.mem[maq.pc] << 8) | maq.mem[maq.pc + 1];
        //printf("%x\n", opcode);
        maq.pc += 2;
        if(maq.pc == MEMSIS)
           maq.pc = 0;
           
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
              printf("JP %x\n", nnn);
              break;
          case 2:
              printf("CALL %x\n", nnn);
              break;
          case 3:
              printf("SE %x, %x\n", x, kk);
              break;
          case 4:
              printf("SNE %x, %x\n", x, kk);
              break;
          case 5:
              printf("SE %x, %x\n", x, y);
              break;
          case 6:
              printf("LD %x, %x\n", x, kk);
              break;
          case 7:
              printf("ADD %x, %x\n", x, kk);
              break;
          case 8:
              switch (n) {
                case 0:
                    printf("LD %x, %x\n", x, y);
                    break;
                case 1:
                    printf("OR %x, %x\n", x, y);
                    break;
                case 2:
                    printf("AND %x, %x\n", x, y);
                    break;
                case 3:
                    printf("XOR %x, %x\n", x, y);
                    break;
                case 4:
                    printf("ADD %x, %x\n", x, y);
                    break;
                case 5:
                    printf("SUB %x, %x\n", x, y);
                    break;
                case 6:
                    printf("SHR %x\n", x);
                    break;
                case 7:
                    printf("SUBN %x, %x\n", x, y);
                    break;
                case 0xE:
                    printf("SHL %x\n", x);
                    break;
              }
              break;
            case 9:
                printf("SNE %x, %x\n", x, y);
                break;
            case 0xA:
                printf("LD I, %x\n", nnn);
                break;
            case 0xB:
                printf("JP V0, %x\n", nnn);
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
                      printf("LD %x, DT\n", x);
                      break;
                  case 0x0A:
                      printf("LD %x, K\n", x);
                      break;
                  case 0x15:
                      printf("LD DT, %x\n", x);
                      break;
                  case 0x18:
                      printf("LD ST, %x\n", x);
                      break;
                  case 0x1E:
                      printf("ADD I, %x\n", x);
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
