#include <vector>
#include <map>
#include <chrono>

#include <termbox.h>

using namespace std;

const float    BASE_SPEED  = 5.0f;
const float    BOOST_SPEED = 15.0f;
const int      FOOD_VALUE  = 10;

const uint16_t BACKGROUND  = TB_DEFAULT;

enum direction {
    UP,
    LEFT,
    DOWN,
    RIGHT,
};

struct point {
    float x;
    float y;
};

struct snake {
    point pos;

    direction d;

    map<direction, char> controls;

    uint16_t head_color;
    uint16_t body_color;

    vector<point> body;
};

snake player1;
snake player2;

bool process_input() {
    tb_event event;

    int val = tb_peek_event(&event, 25);

    if(val < 0) {
        return false;
    }

    if(val > 0) {
        if(event.type == TB_EVENT_KEY) {
            char c = event.ch;
            uint16_t key = event.key;

            if(key == TB_KEY_ESC || key == TB_KEY_CTRL_C) {
                return false;
            }

            for(direction d : {UP, DOWN, LEFT, RIGHT}) {
                for(snake * s : {&player1, &player2})
                    if(s->controls[d] == c) {
                        s->d = d;
                    }
            }

        } else if(event.type == TB_EVENT_RESIZE) {
            return false;
        }
    }

    return true;
}


bool update() {
    tb_clear();

    for(snake * s : {&player1, &player2}) {
        switch(s->d) {
            case UP:    s->pos.y -= 1; break;
            case DOWN:  s->pos.y += 1; break;
            case LEFT:  s->pos.x -= 2; break;
            case RIGHT: s->pos.x += 2; break;
        }

        tb_change_cell(s->pos.x, s->pos.y, '#', s->head_color, BACKGROUND);
    }

    tb_present();

    return true;
}

int main() {
    player1.controls[UP]    = 'w';
    player1.controls[LEFT]  = 'a';
    player1.controls[DOWN]  = 's';
    player1.controls[RIGHT] = 'd';

    player1.d = RIGHT;

    player2.controls[UP]    = 'i';
    player2.controls[LEFT]  = 'j';
    player2.controls[DOWN]  = 'k';
    player2.controls[RIGHT] = 'l';

    player2.d = LEFT;

    tb_init();

    float width  = (float)tb_width();
    float height = (float)tb_height();

    point p_p1 = { width / 3,     height / 2};
    point p_p2 = { 3 * width / 4, height / 2 };

    player1.pos = p_p1;
    player2.pos = p_p2;

    player1.head_color = TB_RED;
    player2.head_color = TB_BLUE;

    player1.body_color = TB_YELLOW;
    player2.head_color = TB_MAGENTA;

    while(process_input() && update()) {
        // keep running until error or exit
    }

    tb_shutdown();

    return 0;
}