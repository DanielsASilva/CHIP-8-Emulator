#include <chip8.h>
#include <cmath>
#include <vector>
#include <iostream>
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 512;

// Maps SDL key states to the chip 8 control scheme
void getInput(bool keys[]){
    const bool *key_states = SDL_GetKeyboardState(NULL);
    keys[0x0] = key_states[SDL_SCANCODE_X];
    keys[0x1] = key_states[SDL_SCANCODE_1];
    keys[0x2] = key_states[SDL_SCANCODE_2];
    keys[0x3] = key_states[SDL_SCANCODE_3];
    keys[0x4] = key_states[SDL_SCANCODE_Q];
    keys[0x5] = key_states[SDL_SCANCODE_W];
    keys[0x6] = key_states[SDL_SCANCODE_E];
    keys[0x7] = key_states[SDL_SCANCODE_A];
    keys[0x8] = key_states[SDL_SCANCODE_S];
    keys[0x9] = key_states[SDL_SCANCODE_D];
    keys[0xA] = key_states[SDL_SCANCODE_Z];
    keys[0xB] = key_states[SDL_SCANCODE_C];
    keys[0xC] = key_states[SDL_SCANCODE_4];
    keys[0xD] = key_states[SDL_SCANCODE_R];
    keys[0xE] = key_states[SDL_SCANCODE_F];
    keys[0xF] = key_states[SDL_SCANCODE_V];
}


void beep(SDL_AudioStream* stream){
    int sample_rate = 44100;
    int samples_to_push = 1024;
    float frequency = 440;
    float amplitude = 0.5;
    
    if (SDL_GetAudioStreamQueued(stream) > (samples_to_push * sizeof(float) * 2)) {
        return;
    }

    static float phase;
    std::vector<float> audio_buffer(samples_to_push);

    for(int i = 0; i < samples_to_push; ++i) {
        audio_buffer[i] = (phase < 0.5) ? amplitude : -amplitude;
        phase += frequency / sample_rate;

        if(phase >= 1)
            phase -= 1;
    }

    SDL_PutAudioStreamData(stream, audio_buffer.data(), audio_buffer.size() * sizeof(float));

}

int main(int argc, char* argv[]){

    // Initializing Chip-8
    chip8 c8; 
    
    if(!c8.loadROM(argv[1]))
        return EXIT_FAILURE;

    //c8.readRAM();
    //c8.disassemble();

    // Initializing SDL
     if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)){
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* gSDLWindow = SDL_CreateWindow("chip-8 emulator", WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if(gSDLWindow == nullptr)
        std::cout << "Couldn't create SDL window";

    SDL_Renderer* gSDLRenderer = SDL_CreateRenderer(gSDLWindow, NULL);
    SDL_Texture* gSDLTexture = SDL_CreateTexture(gSDLRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    SDL_SetTextureScaleMode(gSDLTexture, SDL_SCALEMODE_NEAREST);
    
    // Initializing SDL audio
    SDL_AudioSpec spec;
    spec.channels = 1;           
    spec.format = SDL_AUDIO_F32;
    spec.freq = 44100;         

    SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    
    if (!stream) {
        std::cerr << "Stream open failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(stream));
 
    // Emulator loop
    Uint64 currentTick;
    Uint64 executionTick = 0;
    Uint64 timerTick = 0;
    SDL_Event e;

    bool keys[16]{};
    while(true){
        currentTick = SDL_GetTicks();

        if(SDL_PollEvent(&e)){
            if(e.type == SDL_EVENT_QUIT)
                return false;
            if(e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
                return false;
        }

        if(c8.isBeeping())
            beep(stream);

        getInput(keys);
        if(currentTick - executionTick >= 1) {
            c8.fetch();
            c8.decode();
            c8.execute(1, keys);
            
            // Updates texture with the Video Buffer data
            SDL_UpdateTexture(gSDLTexture, NULL, c8.VBUF, 64 * sizeof(uint32_t)); 

            // Clears rendering target (screen)
            SDL_RenderClear(gSDLRenderer);

            // Copies texture to rendering target
            SDL_RenderTexture(gSDLRenderer, gSDLTexture, NULL, NULL);

            // Updates screen with backbuffer content
            SDL_RenderPresent(gSDLRenderer);        

            executionTick = SDL_GetTicks();
        }

        if(currentTick - timerTick >= 10) {
            c8.decreaseTimers();
            timerTick = SDL_GetTicks();
        }
        
    }

    SDL_DestroyTexture(gSDLTexture);
    SDL_DestroyRenderer(gSDLRenderer);
    SDL_DestroyWindow(gSDLWindow);
    SDL_Quit();

    return EXIT_SUCCESS;
}
