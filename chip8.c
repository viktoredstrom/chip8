#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <time.h>

/*
 * Based on the documentation described in:
 * http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
*/

#define SCALE 10
#define ADDR(x)(x&0x0FFF)
#define GET_X(x)((x>>8)&0xF)
#define GET_Y(x)((x>>4)&0x00F)
#define BYTE(x)(x&0x0FF)
#define NIBBLE(x)(x&0x000F)

typedef struct CPU {
    //General purpose registers 0x0 -> 0xF (VF is a carry flag)
    uint8_t V[0xF];
    //'address register'
    uint16_t I;
    uint16_t PC;
    uint16_t SP;
    uint16_t stack[16];
    uint8_t sys_delay;
    uint8_t audio_delay;
    unsigned char memory[4096];
}CPU;

CPU *cpu;


unsigned char chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
};


void init_font() {
    int i;
    for (i=0; i<80; i++) {
        cpu->memory[i] = chip8_fontset[i];
    }

}

void init_cpu() {
    //calloc instead of malloc for easy zero initialization.   
    if ((cpu = calloc(1, sizeof(struct CPU))) == NULL) {
        printf("Calloc error\n");
        exit(0);
    }
    cpu->PC = 0x200;
    init_font();
}

//todo
void push(uint16_t val) {
    cpu->stack[(cpu->SP++)] = val;
}
uint16_t pop() {
    return cpu->stack[(--cpu->SP)];
}


typedef struct Display {
    uint8_t pixels[64][32];
    int should_render;
    SDL_Window *window;
    SDL_Renderer *renderer;
}Display;

Display *display;

void init_display() {
    if ((display = calloc(1, sizeof(struct Display))) == NULL) {
        printf("Calloc error\n");
        exit(0);
    }
    //init SDL
    SDL_Init(SDL_INIT_VIDEO);
    display->window = SDL_CreateWindow("chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64 * SCALE, 32 * SCALE, SDL_WINDOW_SHOWN);
    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
}

void render() {
    if (!display->should_render)
        return;
    
    SDL_SetRenderDrawColor(display->renderer, 0,0,0,0);
    SDL_RenderClear(display->renderer);
    int x, y;
    for (y=0; y<32; y++) {
        for (x=0; x<64; x++) {
            if (display->pixels[x][y] != 0) {
                SDL_Rect r;
                r.x = x * SCALE;
                r.y = y * SCALE;
                r.w = SCALE;
                r.h = SCALE;
                SDL_SetRenderDrawColor(display->renderer, 255,255,255,255);
                SDL_RenderFillRect(display->renderer, &r);
            }
        }
    }
    SDL_RenderPresent(display->renderer);
    display->should_render = 0;
    SDL_Delay(15);
}

int quit;

uint8_t keyboard[0xF];
void check_keys() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type) {
        case SDL_QUIT:
            quit = 1;
        case SDL_KEYDOWN:
            switch(event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = 1;
                    break;
                case SDLK_0:
                    keyboard[0x0] = 1;
                    break;
                case SDLK_1:
                    keyboard[0x1] = 1;
                    break;
                case SDLK_2:
                    keyboard[0x2] = 1;
                    break;
                case SDLK_3:
                    keyboard[0x3] = 1;
                    break;
                case SDLK_4:
                    keyboard[0x4] = 1;
                    break;
                case SDLK_5:
                    keyboard[0x5] = 1;
                    break;
                case SDLK_6:
                    keyboard[0x6] = 1;
                    break;
                case SDLK_7:
                    keyboard[0x7] = 1;
                    break;
                case SDLK_8:
                    keyboard[0x8] = 1;
                    break;
                case SDLK_9:
                    keyboard[0x9] = 1;
                    break;
                case SDLK_a:
                    keyboard[0xA] = 1;
                    break;
                case SDLK_b:
                    keyboard[0xB] = 1;
                    break;
                case SDLK_c:
                    keyboard[0xC] = 1;
                    break;
                case SDLK_d:
                    keyboard[0xD] = 1;
                    break;
                case SDLK_e:
                    keyboard[0xE] = 1;
                    break;
                case SDLK_f:
                    keyboard[0xF] = 1;
                    break;
            }
            break;
        case SDL_KEYUP:
            switch(event.key.keysym.sym) {
                case SDLK_0:
                    keyboard[0x0] = 0;
                    break;
                case SDLK_1:
                    keyboard[0x1] = 0;
                    break;
                case SDLK_2:
                    keyboard[0x2] = 0;
                    break;
                case SDLK_3:
                    keyboard[0x3] = 0;
                    break;
                case SDLK_4:
                    keyboard[0x4] = 0;
                    break;
                case SDLK_5:
                    keyboard[0x5] = 0;
                    break;
                case SDLK_6:
                    keyboard[0x6] = 0;
                    break;
                case SDLK_7:
                    keyboard[0x7] = 0;
                    break;
                case SDLK_8:
                    keyboard[0x8] = 0;
                    break;
                case SDLK_9:
                    keyboard[0x9] = 0;
                    break;
                case SDLK_a:
                    keyboard[0xA] = 0;
                    break;
                case SDLK_b:
                    keyboard[0xB] = 0;
                    break;
                case SDLK_c:
                    keyboard[0xC] = 0;
                    break;
                case SDLK_d:
                    keyboard[0xD] = 0;
                    break;
                case SDLK_e:
                    keyboard[0xE] = 0;
                    break;
                case SDLK_f:
                    keyboard[0xF] = 0;
                    break;
            }
            break;
    }
}

void interpret_chip8() {
    int x, y, keyloop, n;
    short pixel;
    int py, px;
    uint16_t opcode = (cpu->memory[cpu->PC] << 8) | cpu->memory[cpu->PC + 1];
    switch(opcode >> 12) {
        case 0x0:
            switch(NIBBLE(opcode)) {
                //00E0: CLS
                case 0x0:
                    memset(display->pixels, 0, sizeof(display->pixels));
                    cpu->PC +=2;
                break;
                //00EE: RET
                case 0xE:
                    cpu->PC = pop() + 2;
                break;
                default:
                    printf("Unknown opcode: %x\n", opcode);
                break;
            }
        break;
        //1NNN: JMP
        case 0x1:
            cpu->PC = ADDR(opcode);
        break;
        //2NNN: CALL (aka push pc; jmp NNN)
        case 0x2:
            push(cpu->PC);
            cpu->PC = ADDR(opcode);
        break;
        //3xKK: SE Vx, byte
        case 0x3:
            if (cpu->V[GET_X(opcode)] == BYTE(opcode))
                cpu->PC += 4;
            else
                cpu->PC += 2;
        break;
        //4xkk - SNE Vx, byte
        case 0x4:
            if (cpu->V[GET_X(opcode)] != BYTE(opcode))
                cpu->PC += 4;
            else
                cpu->PC += 2;
        break;
        //5xy0 - SE Vx, Vy
        case 0x5:
            if (cpu->V[GET_X(opcode)] == cpu->V[GET_Y(opcode)])
                cpu->PC += 4;
            else
                cpu->PC += 2;
        break;
        //6xkk - LD Vx, byte
        case 0x6:
            cpu->V[GET_X(opcode)] = BYTE(opcode);
            cpu->PC += 2;
        break;
        //7xkk - ADD Vx, byte
        case 0x7:
            cpu->V[GET_X(opcode)] += BYTE(opcode);
            cpu->PC += 2;
        break;
        case 0x8:
            switch(NIBBLE(opcode)) {
                //8xy0 - LD Vx, Vy
                case 0x0:
                    cpu->V[GET_X(opcode)] = cpu->V[GET_Y(opcode)];
                    cpu->PC += 2;
                break;
                case 0x1:
                    cpu->V[GET_X(opcode)] = cpu->V[GET_X(opcode)] | cpu->V[GET_Y(opcode)];
                    cpu->PC += 2;
                break;
                case 0x2:
                    cpu->V[GET_X(opcode)] = cpu->V[GET_X(opcode)] & cpu->V[GET_Y(opcode)];
                    cpu->PC += 2;
                break;
                case 0x3:
                    cpu->V[GET_X(opcode)] = cpu->V[GET_X(opcode)] ^ cpu->V[GET_Y(opcode)];
                    cpu->PC += 2;

                break;
                case 0x4:
                    if ((((int)cpu->V[GET_X(opcode)]) + ((int)cpu->V[GET_Y(opcode)])) < 256 ) {
                        cpu->V[0xF] = 0;
                    } else {
                        cpu->V[0xF] = 1;
                    }
                    cpu->V[GET_X(opcode)] += cpu->V[GET_Y(opcode)];
                    cpu->PC += 2;
                break;
                case 0x5:
                    if ((((int)cpu->V[GET_X(opcode)]) - ((int)cpu->V[GET_Y(opcode)])) >= 0) {
                        cpu->V[0xF] = 1;
                    } else {
                        cpu->V[0xF] = 0;
                    }
                    cpu->V[GET_X(opcode)] -= cpu->V[GET_Y(opcode)];
                    cpu->PC += 2;

                break;
                case 0x6:
                    cpu->V[0xF] = cpu->V[GET_X(opcode)] & 7;
                    cpu->V[GET_X(opcode)] = cpu->V[GET_X(opcode)] >> 1;
                    cpu->PC +=2;
                break;
                /*case 0x7:
                    
                break;*/
                case 0xE:
                    cpu->V[0xF] = cpu->V[GET_X(opcode)] >> 7;
                    cpu->V[GET_X(opcode)] = cpu->V[GET_X(opcode)] << 1;
                    cpu->PC +=2;
                break;
                default:
                    printf("Unknown opcode: %x\n", opcode);
                break;
            }
        break;
        //9xy0 - SNE Vx, Vy
        case 0x9:
            if (cpu->V[GET_X(opcode)] != cpu->V[GET_Y(opcode)])
                cpu->PC += 4;
            else
                cpu->PC += 2;
        break;
        //Annn - LD I, addr
        case 0xA:
            cpu->I = ADDR(opcode);
            cpu->PC += 2;
        break;
        //Bnnn - JP V0, addr
        case 0xB:
            cpu->PC = ADDR(opcode) + cpu->V[0];
        break;

        case 0xC:
            srand(time(NULL));
            cpu->V[GET_X(opcode)] = rand() & BYTE(opcode);
            cpu->PC += 2;
        break;

        //Dxyn - DRW Vx, Vy, nibble
        case 0xD:
            cpu->V[0xF] = 0;
            for (y = 0; y < (NIBBLE(opcode)); y++) {
                py = (y + cpu->V[GET_Y(opcode)]) % 32;
                pixel = cpu->memory[cpu->I + y];
                for (x = 0; x<8; x++) {
                    if ((pixel & (0x80 >> x)) != 0) {
                        px = (cpu->V[GET_X(opcode)] + x) % 64;
                        if (display->pixels[px][py] == 1) {
                            cpu->V[0xF] = 1;
                        }
                        display->pixels[px][py] ^= 1;
                    }
                }
            }
            display->should_render = 1;
            cpu->PC += 2;
        break;

        //ExA1 - SKNP Vx
        case 0xE:
            switch(BYTE(opcode)) {
                //Ex9E - SKP Vx
                case 0x9E:
                    if (keyboard[cpu->V[GET_X(opcode)]] == 1)
                        cpu->PC += 4;
                    else
                        cpu->PC += 2;
                break;
                //ExA1 - SKNP Vx
                case 0xa1:
                    if (keyboard[cpu->V[GET_X(opcode)]] == 0)
                        cpu->PC += 4;
                    else
                        cpu->PC += 2;

                break;
                default:
                    printf("Unknown opcode: %x\n", opcode);
                break;
            }
        break;
        case 0xF:
            switch(BYTE(opcode)) {
                //Fx07 - LD Vx, DT
                case 0x07:
                    cpu->V[GET_X(opcode)] = cpu->sys_delay;
                    cpu->PC += 2;
                break;
                //Fx0A - LD Vx, K
                case 0x0A:
                    for(keyloop = 0; keyloop < 0xF; keyloop++) {
                        if (keyboard[keyloop] == 1) {
                            cpu->V[GET_X(opcode)] = keyloop;
                            cpu->PC +=2;
                        }
                    }
                break;
                //Fx15 - LD DT, Vx
                case 0x15:
                    cpu->sys_delay = cpu->V[GET_X(opcode)];
                    cpu->PC += 2;
                break;
                //Fx18 - LD ST, Vx
                case 0x18:
                    cpu->audio_delay = cpu->V[GET_X(opcode)];
                    cpu->PC += 2;
                break;
                //Fx1E - ADD I, Vx
                case 0x1E:
                    cpu->I += cpu->V[GET_X(opcode)];
                    cpu->PC += 2;
                break;
                //Fx29 - LD F, Vx
                case 0x29:
                    cpu->I = cpu->V[GET_X(opcode)] * 5;
                    cpu->PC += 2;
                break;
                //Fx33 - LD B, Vx
                case 0x33:
                    cpu->memory[cpu->I] = (cpu->V[GET_X(opcode)] / 100);
                    cpu->memory[cpu->I+1] = (cpu->V[GET_X(opcode)] / 10) % 10;
                    cpu->memory[cpu->I+2] = (cpu->V[GET_X(opcode)]) % 10;
                    cpu->PC += 2;
                break;
                //Fx55 - LD [I], Vx
                case 0x55:
                    for(n=0; n < GET_X(opcode); n++) {
                        cpu->memory[cpu->I + n] = cpu->V[n];
                    }
                    cpu->PC += 2;

                break;
                case 0x65:
                    for(n=0; n < GET_X(opcode); n++) {
                        cpu->V[n] = cpu->memory[cpu->I + n];
                    }
                    cpu->PC += 2;
                break;

                default:
                    printf("Unknown opcode: %x\n", opcode);
                break;

            }
        break;
        default:
            printf("Unknown opcode: %x\n", opcode);
        break;
    }

    if (cpu->sys_delay > 0)
        cpu->sys_delay--;

    if (cpu->audio_delay > 0) {
        if (cpu->audio_delay == 1) {
            //beep boop
        }
        cpu->audio_delay--;
    }
}

void run() {
    while (!quit) {
        check_keys();
        interpret_chip8();
        render();
    }
}

int read_rom(char *path_to_rom) {
    FILE *fp;
    if ((fp = fopen(path_to_rom, "r")) == NULL) {
        return 0;
    }
    
    fseek(fp, 0, SEEK_END);
    size_t rom_size = ftell(fp);
    rewind(fp);
    fread(cpu->memory+0x200, sizeof(unsigned char), rom_size, fp);
    fclose(fp);
    return 1;
}

void cleanup() {
    //cleanup SDL + free CPU and Display structs
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    free(display);
    free(cpu);
    SDL_Quit();
}

int main(int argc, char **argv) {
    init_cpu();
    if (argc <= 1) {
        printf("Usage: chip8 filename\n");
        return 0;
    }
    if (!read_rom(argv[1])) {
        printf("Rom not found\n");
        free(cpu);
        return 0;
    }
    init_display();
    run();
    cleanup();
    return 1;
}
