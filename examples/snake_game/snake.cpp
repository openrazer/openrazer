#include "snake.h"
#include <list>
#include <stdlib.h>
#include <iostream>

using namespace std;

#define keys_max 1
int keys_history_index = 0;
int keys_history[keys_max];//ring buffer

#define MAX_X  21
#define MAX_Y  5

struct razer_chroma *chroma;
struct razer_pos last_pos,pos;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

typedef enum{
	LEFT,
	RIGHT,
	UP,
	DOWN
}direction_t;

class Cord{
	public:
		unsigned int x;
		unsigned int y;
};

class Snake {
	public:
		std::list<Cord> cords;
		direction_t direction;
		Snake(){
			Cord c;
			c.x = 5;
			c.y = 3;
			cords.push_front(c);
			direction = RIGHT;
			grow(); grow();
		}
		std::list<Cord> getCords(void){
			return cords;
		}
		Cord getFront(void){
			return cords.front();
		}
		void move(direction_t d){
			direction = d;
			grow();
			cords.pop_back();
		}

		void grow(void){
			Cord c;
			c.x  = cords.front().x;
			c.y  = cords.front().y;
			switch (direction) {
				case LEFT:
					if (c.x == 0) {
						c.x = MAX_X;
						if (c.y == 0) c.y = MAX_Y;
						c.y-=1;
					}
					c.x-=1;
					break;
				case RIGHT:
					if (c.x == MAX_X) {
						c.x = 0;
						if (c.y == MAX_Y) c.y = 0;
						c.y+=1;
					}
					c.x+=1;
					break;
				case UP:
					if (c.y == 0) c.y = MAX_Y;
					c.y-=1;
					break;
				case DOWN:
					if (c.y == MAX_Y) c.y = 0;
					c.y+=1;
					break;
			}
			cords.push_front(c);
		}

		void reset(void){
			Snake s = Snake();
			cords = s.cords;
			direction = s.direction;
		}
		void update(void){
			for (std::list<Cord>::iterator it=cords.begin(); it != cords.end(); ++it){
				chroma->keys->rows[(*it).y].column[(*it).x].r = 0x00;
				chroma->keys->rows[(*it).y].column[(*it).x].g = 0x00;
				chroma->keys->rows[(*it).y].column[(*it).x].b = 0xFF;
			}
			chroma->keys->update_mask = 0xff;
		}
};

void stop(int sig)
{
	//printf("Stopping input example\n");
	razer_close(chroma);
	exit(1);
}

int input_handler(struct razer_chroma *chroma, struct razer_chroma_event *event)
{
	//printf("input_handler called\n");
	if(event->type != RAZER_CHROMA_EVENT_TYPE_KEYBOARD || !event->sub_type)
		return(1);//depressed
	keys_history[0] = (long)event->value;
	return(1);
}

class Field {
	public:
	Field(void){
		reset();
	}
	void reset(void){
		unsigned char r=0x00;
		unsigned char g=128;
		unsigned char b=0x00;
		
		for(int x=0;x<22;x++) {
			for(int y=0;y<6;y++) {
				chroma->keys->rows[y].column[x].r = r;
				chroma->keys->rows[y].column[x].g = g;
				chroma->keys->rows[y].column[x].b = b;
				chroma->keys->update_mask |= 1<<y;
			}
		}
	}
	void update(void){
		reset();
	}
	
	void endGame(void){
	}
};

class Food {
	public:
		Food(){
			c.x=rand() % 23;
			c.y=rand() % 6;
			int illegal = 
				      (c.x == 0 && c.y == 0)||
				      (c.x == 3 && c.y == 0)||
				      (c.x == 15 && c.y == 4)||
				      (c.x == 17 && c.y == 4)||
				      (c.y == 3 && (c.x>14 && c.x<18)) ||
				      (c.y == 4 && (c.x>3 && c.x<9))   ||
				      (c.y == 0 && c.x>17);
			while (illegal) {
				c.x=rand() % 23;
				c.y=rand() % 6;
			        illegal = 
				      (c.x == 0 && c.y == 0)||
				      (c.x == 3 && c.y == 0)||
				      (c.x == 15 && c.y == 4)||
				      (c.x == 17 && c.y == 4)||
				      (c.y == 3 && (c.x>14 && c.x<18)) ||
				      (c.y == 4 && (c.x>3 && c.x<9))   ||
				      (c.y == 0 && c.x>17);
			}
		}
		void update(void){
			chroma->keys->rows[c.y].column[c.x].r = 0xFF;
			chroma->keys->rows[c.y].column[c.x].g = 0x00;
			chroma->keys->rows[c.y].column[c.x].b = 0x00;
			chroma->keys->update_mask = 0xff;
		}
		Cord getPosition(){
			return c;
		}
	private:
		Cord c;
};

class Game {
	public:
		Snake s;
		Field f;
		Food  g;
		Game(){
			s = Snake();
			f = Field();
			g = Food();
		}
		void update(void){
			f.update();
			razer_convert_keycode_to_pos(keys_history[0],&pos);
			if (last_pos.x!=pos.x || last_pos.y!=pos.y){
				last_pos.x=pos.x; last_pos.y=pos.y;
				//cout <<"key pos = "<<(pos).x<<","<<(pos).y<<endl;
			}
			direction_t d=LEFT;
			if(pos.x == 16 && pos.y == 4)
				d=UP;
			else if(pos.x == 16 && pos.y == 5)
				d=DOWN;
			else if(pos.x == 15 && pos.y == 5)
				d=LEFT;
			else if(pos.x == 17 && pos.y == 5)
				d=RIGHT;
			s.move(d);
			g.update();
			s.update();
			razer_update_keys(chroma,chroma->keys);
			usleep(400000);
			Cord sc = s.getFront();
			Cord fc = g.getPosition();
			if(sc.x==fc.x && sc.y==fc.y){
				Food tmp = Food();
				s.grow();
				s.grow();
				g = tmp;
			}
			razer_update(chroma);
			razer_frame_limiter(chroma,13);
		}
};


#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	uid_t uid = getuid();
	if(uid != 0)
		printf("input example needs root to work correctly.\n");	
	chroma  = razer_open();
	if(!chroma)
		exit(1);
	razer_set_input_handler(chroma,input_handler);
	razer_set_custom_mode(chroma);
	razer_clear_all(chroma->keys);
	razer_update_keys(chroma,chroma->keys);
	for(int i=0;i<10;i++)
		keys_history[i] = -1;
	signal(SIGINT,stop);
	signal(SIGKILL,stop);
	signal(SIGTERM,stop);	

	Game g = Game();


	while(true) g.update();
}

#pragma GCC diagnostic pop
