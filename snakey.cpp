#include <vector>
#include <map>
#include <chrono>
#include <string>

#include <termbox.h>

using namespace std;
using namespace chrono;

const float    BASE_SPEED  = 30.0f;
const float    BOOST_MOD   = 4.0f;
const float    FRUIT_VALUE = 0.1f;
const int      NUM_FRUITS  = 2;
const float    MIN_LENGTH  = 0.25;

const uint16_t FRUIT_COLOR = TB_RED;
const uint16_t BACKGROUND  = TB_BLACK;

string win_message;
uint16_t message_color;

enum direction {
    UP,
    LEFT,
    DOWN,
    RIGHT,
    BOOST,      // not really a direction ¯\_(ツ)_/¯
};

struct point {
    float x;
    float y;
};

struct body_part {
    point pos;
    time_point<steady_clock> time;
};

struct fruit {
    point pos;
};
vector<fruit> fruits;

struct snake {
    point pos;

    direction d;
    bool boost;

    map<direction, char> controls;

    uint16_t head_color;
    uint16_t body_color;

    float length;

    vector<body_part> body;
};

snake player1;
snake player2;

time_point<steady_clock> last_update;
void reset() {
    tb_clear();

    win_message = "Player x Wins!";
    message_color = TB_DEFAULT;

    last_update = steady_clock::now();

    float width  = (float)tb_width();
    float height = (float)tb_height();

    point p_p1 = { width / 3,     height / 2};
    point p_p2 = { 3 * width / 4, height / 2 };

    player1.pos = p_p1;
    player2.pos = p_p2;

    player1.d = RIGHT;
    player2.d = LEFT;

    player1.body.push_back({p_p1, last_update});
    player2.body.push_back({p_p2, last_update});

    player1.length = 0.5f;
    player2.length = 0.5f;    

    fruits.clear();
}

void init() {
    srand(time(NULL));

    player1.controls[UP]    = 'w';
    player1.controls[LEFT]  = 'a';
    player1.controls[DOWN]  = 's';
    player1.controls[RIGHT] = 'd';
    player1.controls[BOOST] = 'r';

    player2.controls[UP]    = 'i';
    player2.controls[LEFT]  = 'j';
    player2.controls[DOWN]  = 'k';
    player2.controls[RIGHT] = 'l';
    player2.controls[BOOST] = 'u';

    player1.head_color = TB_YELLOW;
    player1.body_color = TB_GREEN;

    player2.head_color = TB_CYAN;
    player2.body_color = TB_MAGENTA;

    reset();
}

//  0 - ok
// -1 - quit
//  1 - reset
int process_input() {
    tb_event event;

    int val = tb_peek_event(&event, 10);

    if(val < 0) {
        return -1;
    }

    if(val > 0) {
        if(event.type == TB_EVENT_KEY) {
            char c = event.ch;
            uint16_t key = event.key;

            if(key == TB_KEY_ESC || key == TB_KEY_CTRL_C) {
                return -1;
            }

            if(key == TB_KEY_END) {
                reset();
                return 1;
            }

            for(snake * s : {&player1, &player2}) {
                for(direction d : {UP, DOWN, LEFT, RIGHT}) {
                    if(s->controls[d] == c)
                        s->d = d;
                }
                if(s->controls[BOOST] == c)
                    s->boost = !s->boost;
            }

        } else if(event.type == TB_EVENT_RESIZE) {
            return -1;
        }
    }

    return 0;
}

bool update() {
    time_point<steady_clock> this_update = steady_clock::now();
    float dt = duration_cast<milliseconds>(this_update - last_update).count() / 1000.0f;
    last_update = this_update;

    int id = 1;
    for(snake * s : {&player1, &player2}) {
        point new_pos = {s->pos.x, s->pos.y};
        float speed = dt * BASE_SPEED;
        if(s->boost) {
            if(s->length > MIN_LENGTH) {
                speed *= BOOST_MOD;
                s->length -= 2 * dt;
            } else {
                s->boost = false;
            }
            
        }

        switch(s->d) {
            case UP:    new_pos.y -= 1 * speed; break;
            case DOWN:  new_pos.y += 1 * speed; break;
            case LEFT:  new_pos.x -= 2 * speed; break;
            case RIGHT: new_pos.x += 2 * speed; break;
        }
        if(new_pos.x < 0) new_pos.x = tb_width() - 1;
        if(new_pos.y < 0) new_pos.y = tb_height() - 1;
        if(new_pos.x >= tb_width())  new_pos.x = 0;
        if(new_pos.y >= tb_height()) new_pos.y = 0;

        point p = {s->pos.x, s->pos.y};
        while( (int)p.x != (int)new_pos.x || (int)p.y != (int)new_pos.y ) {
            switch(s->d) {
                case UP:    p.y -= 1; break;
                case DOWN:  p.y += 1; break;
                case LEFT:  p.x -= 1; break;
                case RIGHT: p.x += 1; break;
            }

            if(p.x < 0) p.x = tb_width() - 1;
            if(p.y < 0) p.y = tb_height() - 1;
            if(p.x >= tb_width())  p.x = 0;
            if(p.y >= tb_height()) p.y = 0;

            s->body.push_back( {p, this_update} );
        }
        s->pos=new_pos;

        for(int i = 0; i < fruits.size(); i++) {
            fruit f = fruits[i];
            if(int(s->pos.x) == int(f.pos.x) && int(s->pos.y) == int(f.pos.y)) {
                s->length += FRUIT_VALUE;
                fruits.erase(fruits.begin() + i--);
            }
        }
        snake * other = (s == &player1) ? &player2 : &player1;

        for(int i = s->body.size() - 1; i >= 0; i--) {
            body_part bp = s->body[i];
            if(duration_cast<milliseconds>(this_update - bp.time).count() > 1000.0f * s->length) {
                s->body.erase(s->body.begin() + i--);
                tb_change_cell(bp.pos.x, bp.pos.y, ' ', BACKGROUND, BACKGROUND);
            } else {
                tb_change_cell(bp.pos.x, bp.pos.y, '#', s->body_color, BACKGROUND);

                if(int(other->pos.x) == int(bp.pos.x) && int(other->pos.y) == int(bp.pos.y)) {
                    tb_change_cell(player1.pos.x, player1.pos.y, '#', player1.head_color, BACKGROUND);
                    tb_change_cell(player2.pos.x, player2.pos.y, '#', player2.head_color, BACKGROUND);

                    win_message = "Player " + to_string(id) + " Wins!";
                    message_color = s->head_color;

                    return false;
                }
            }
        }
           
        tb_change_cell(s->pos.x, s->pos.y, '#', s->head_color, BACKGROUND);

        id++;
    }

    while(fruits.size() < NUM_FRUITS) {
        fruit f = { { float(rand() % tb_width()), float(rand() % tb_height()) } };
        fruits.push_back(f);
    }
    for(fruit f : fruits) {
        tb_change_cell(f.pos.x, f.pos.y, '#', FRUIT_COLOR, BACKGROUND);
    }

    return true;
}

int main() {
    tb_init();
    init();
    int input = 0;
    while(input >= 0) {
        input = process_input();
                
        if(!update()) {
            int xpos = tb_width() / 2 - win_message.length() / 2;

            for(int x = 0; x < win_message.length(); x++) {
                tb_change_cell(xpos + x, 3, win_message[x], message_color, BACKGROUND);
            }

            tb_present();
            while(input == 0) {
                input = process_input();
                
            }
        } else {
            tb_present();
        }
    }
        
    tb_shutdown();

    return 0;
}