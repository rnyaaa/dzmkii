#ifndef _INPUT_H
#define _INPUT_H

#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <SDL.h>
#include "common.h"
#include "geometry.h"
#include "logger.h"

namespace DZKey {
    enum DZKey {
        ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
        Q, W, E, R, T, Y, U, I, O, P,
         A, S, D, F, G, H, J, K, L,
          Z, X, C, V, B, N, M,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
        ESC,
        BACKSPACE,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        DZKEY_MAX
    };
};

struct InputState
{
    bool key_prev[DZKey::DZKEY_MAX];
    bool key[DZKey::DZKEY_MAX];

    struct {
        bool shift;
        bool alt;
        bool ctrl;
    } modifier;
    
    struct {
        v2i pos;
        v2i delta;
        u8 timer;
        s32 wheel_delta = 0;
        bool left_button_down = false;
        bool middle_button_down = false;
        bool right_button_down = false;
        bool clicked = false;
        bool double_clicked = false;
    } mouse, mouse_prev;

    bool quit;

    u8 swap_timer;
    u8 swap_mousedown;
    void update()
    {
        const u8 *key_state = SDL_GetKeyboardState(nullptr);
        SDL_Event e;

        this->mouse_prev = this->mouse;
        memcpy(this->key_prev, this->key, sizeof(bool) * DZKey::DZKEY_MAX);
        memset(this->key, 0, sizeof(bool) * DZKey::DZKEY_MAX);
        memset(&this->modifier, 0, sizeof(this->modifier));
        this->swap_timer = this->mouse.timer;
        this->swap_mousedown = this->mouse.left_button_down;
        memset(&this->mouse, 0, sizeof(this->mouse));
        this->mouse.timer = this->swap_timer;
        this->mouse.left_button_down = this->swap_mousedown;
        this->mouse.clicked = false;
        this->mouse.double_clicked = false;

        if(this->mouse.timer > 0)
        {
            this->mouse.timer -= 1;
            //Log::verbose("Mouse timer: %d", this->mouse.timer);
        }
        this->quit = false;

        SDL_GetMouseState(&this->mouse.pos.x, &this->mouse.pos.y);
        this->mouse.delta = this->mouse_prev.pos - this->mouse.pos;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT || key_state[SDL_SCANCODE_ESCAPE])
            {
                Log::verbose("Quit!");
                this->quit = true;
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                this->mouse.wheel_delta = e.wheel.y;
                Log::verbose("2. wheel_delta: %d\n", this->mouse.wheel_delta);
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                this->mouse.left_button_down = true;
                if(this->mouse.timer > 0)
                {
                    this->mouse.double_clicked = true;
                }
                this->mouse.timer = 15;
            }
            else if (e.type == SDL_MOUSEBUTTONUP) 
            {
                this->mouse.left_button_down = false;
                if(this->mouse.timer > 0 && !this->mouse.double_clicked)
                {
                    this->mouse.clicked = true;
                }
            }
        }

#       define SET_KEY_DOWN(SDL_SCANCODE, DZKEY) \
            if (key_state[SDL_SCANCODE]) \
            { \
                this->key[DZKEY] = true; \
            }

        SET_KEY_DOWN(SDL_SCANCODE_0, DZKey::ZERO);
        SET_KEY_DOWN(SDL_SCANCODE_1, DZKey::ONE);
        SET_KEY_DOWN(SDL_SCANCODE_2, DZKey::TWO);
        SET_KEY_DOWN(SDL_SCANCODE_3, DZKey::THREE);
        SET_KEY_DOWN(SDL_SCANCODE_4, DZKey::FOUR);
        SET_KEY_DOWN(SDL_SCANCODE_5, DZKey::FIVE);
        SET_KEY_DOWN(SDL_SCANCODE_6, DZKey::SIX);
        SET_KEY_DOWN(SDL_SCANCODE_7, DZKey::SEVEN);
        SET_KEY_DOWN(SDL_SCANCODE_8, DZKey::EIGHT);
        SET_KEY_DOWN(SDL_SCANCODE_9, DZKey::NINE);
        SET_KEY_DOWN(SDL_SCANCODE_Q, DZKey::Q);
        SET_KEY_DOWN(SDL_SCANCODE_W, DZKey::W);
        SET_KEY_DOWN(SDL_SCANCODE_E, DZKey::E);
        SET_KEY_DOWN(SDL_SCANCODE_R, DZKey::R);
        SET_KEY_DOWN(SDL_SCANCODE_T, DZKey::T);
        SET_KEY_DOWN(SDL_SCANCODE_Y, DZKey::Y);
        SET_KEY_DOWN(SDL_SCANCODE_U, DZKey::U);
        SET_KEY_DOWN(SDL_SCANCODE_I, DZKey::I);
        SET_KEY_DOWN(SDL_SCANCODE_O, DZKey::O);
        SET_KEY_DOWN(SDL_SCANCODE_P, DZKey::P);
        SET_KEY_DOWN(SDL_SCANCODE_A, DZKey::A);
        SET_KEY_DOWN(SDL_SCANCODE_S, DZKey::S);
        SET_KEY_DOWN(SDL_SCANCODE_D, DZKey::D);
        SET_KEY_DOWN(SDL_SCANCODE_F, DZKey::F);
        SET_KEY_DOWN(SDL_SCANCODE_G, DZKey::G);
        SET_KEY_DOWN(SDL_SCANCODE_H, DZKey::H);
        SET_KEY_DOWN(SDL_SCANCODE_J, DZKey::J);
        SET_KEY_DOWN(SDL_SCANCODE_K, DZKey::K);
        SET_KEY_DOWN(SDL_SCANCODE_L, DZKey::L);
        SET_KEY_DOWN(SDL_SCANCODE_Z, DZKey::Z);
        SET_KEY_DOWN(SDL_SCANCODE_X, DZKey::X);
        SET_KEY_DOWN(SDL_SCANCODE_C, DZKey::C);
        SET_KEY_DOWN(SDL_SCANCODE_V, DZKey::V);
        SET_KEY_DOWN(SDL_SCANCODE_B, DZKey::B);
        SET_KEY_DOWN(SDL_SCANCODE_N, DZKey::N);
        SET_KEY_DOWN(SDL_SCANCODE_M, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F1, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F2, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F3, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F4, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F5, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F6, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F7, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F8, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F9, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_F10, DZKey::M);
        SET_KEY_DOWN(SDL_SCANCODE_ESCAPE, DZKey::ESC);
        SET_KEY_DOWN(SDL_SCANCODE_BACKSPACE, DZKey::BACKSPACE);
        SET_KEY_DOWN(SDL_SCANCODE_UP, DZKey::UP);
        SET_KEY_DOWN(SDL_SCANCODE_DOWN, DZKey::DOWN);
        SET_KEY_DOWN(SDL_SCANCODE_LEFT, DZKey::LEFT);
        SET_KEY_DOWN(SDL_SCANCODE_RIGHT, DZKey::RIGHT);
    }
};

#endif // _INPUT_H
