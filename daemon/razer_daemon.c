#include "razer_daemon.h"

int daemon_open(struct razer_daemon **daemon)
{
 
 	*daemon = (struct razer_daemon*)malloc(sizeof(struct razer_daemon));
 	(*daemon)->chroma = (struct razer_chroma*)malloc(sizeof(struct razer_chroma));
 	razer_open((*daemon)->chroma);
}

int daemon_close(struct razer_daemon **daemon)
{
 	razer_close((*daemon)->chroma);
 	free((*daemon)->chroma);
 	free(*daemon);
}

int daemon_loop(struct razer_daemon *daemon)
{

}

void test_effect4(struct razer_chroma *chroma,struct razer_keys *keys)
{
	int r,g,b;
	float fr,fg,fb;
	int rnd=random();
	int rnd2=random();
	int rnd3=random();
	int count = 0;
	int count_dir = 1;
	float s = 0.1f;
	while(1)
	{	
		int x,y;
		for(x=0;x<22;x++)
			for(y=0;y<6;y++)
		{
			r = (count+x)*(255/22);
			g = (count-x)*(255/22);
			b = (count+y)*(255/22);

			keys->rows[y].column[x].r = (unsigned char)r;
			keys->rows[y].column[x].g = (unsigned char)g;
			keys->rows[y].column[x].b = (unsigned char)b;
			keys->update_mask |= 1<<y;
		}
		razer_update_keys(chroma,keys);
		count+=count_dir;
		if(count<=0 || count>=44)
		{
			count_dir=-count_dir;
			rnd = random();
			rnd2 = random();
			rnd3 = random();
		}
		usleep(6000);
	}
}

/*
void test_effect1(struct razer_keys *keys)
{
	int r=random();
	int r2=random();
	int r3=random();

	int v=1;
	int v2=254;
	int v3=127;
	int vdir = 1;
	int v2dir = -1;
	int v3dir = -1;
	while(1)
	{	
		int x,y;
		for(x=0;x<22;x++)
			for(y=0;y<6;y++)
		{
			keys->rows[y].column[x].r = (unsigned char)v-v*sin(y)+cos(x);
			keys->rows[y].column[x].g = (unsigned char)v2-v2*cos(y)-x;
			keys->rows[y].column[x].b = (unsigned char)v3-v3*sin(x);
			keys->update_mask |= 1<<y;
		}
		if(v+vdir*r>=255 || v+vdir*r<=0)
		{
			vdir=-vdir;
			r=(random()%3)+1;
		}
		if(v2+v2dir*r2>=255 || v2+v2dir*r2<=0)
		{
			v2dir=-v2dir;
			r2=(random()%3)+1;
		}
		if(v3+v3dir*r3>=255 || v3+v3dir*r3<=0)
		{
			v3dir=-v3dir;
			r3=(random()%3)+1;
		}

		v=v+vdir*r;
		v2=v2+v2dir*r2;
		v3=v3+v3dir*r3;
		update_keys(keys);
	}
}

void test_effect2(struct razer_keys *keys)
{
	int r,g,b;
	float fr,fg,fb;
	int rnd=random();
	int rnd2=random();
	int rnd3=random();
	int count = 0;
	int count_dir = 1;
	float s = 0.1f;
	while(1)
	{	
		int x,y;
		for(x=0;x<22;x++)
			for(y=0;y<6;y++)
		{
			r = (cos((count+((rnd%4)*90)+y)*s))*255;
			g = (cos((count+((rnd2%4)*90)+y)*s))*255;
			b = (cos((count+((rnd3%4)*90)+y)*s))*255;

			keys->rows[y].column[x].r = (unsigned char)r;
			keys->rows[y].column[x].g = (unsigned char)g;
			keys->rows[y].column[x].b = (unsigned char)b;
			keys->update_mask |= 1<<y;
		}
		update_keys(keys);
		count+=count_dir;
		if(count<=0 || count>=30)
		{
			count_dir=-count_dir;
			rnd = random();
			rnd2 = random();
			rnd3 = random();
		}
		usleep(60000);
	}
}

void test_effect3(struct razer_keys *keys)
{
	int r,g,b;
	float fr,fg,fb;
	int rnd=random();
	int rnd2=random();
	int rnd3=random();
	int count = 0;
	int count_dir = 1;
	float s = 0.1f;
	while(1)
	{	
		int x,y;
		for(x=0;x<22;x++)
			for(y=0;y<6;y++)
		{
			r = (cos((count+((rnd%4)*90)+y)*s)+sin(count+x)*s)*255;
			g = (cos((count+((rnd2%4)*90)+y)*s)+sin(count+x)*s)*255;
			b = (cos((count+((rnd3%4)*90)+y)*s)+sin(count+x)*s)*255;

			keys->rows[y].column[x].r = (unsigned char)r;
			keys->rows[y].column[x].g = (unsigned char)g;
			keys->rows[y].column[x].b = (unsigned char)b;
			keys->update_mask |= 1<<y;
		}
		update_keys(keys);
		count+=count_dir;
		if(count<=0 || count>=30)
		{
			count_dir=-count_dir;
			rnd = random();
			rnd2 = random();
			rnd3 = random();
		}
		usleep(6000);
	}
}


void test_effect5(struct razer_keys *keys)
{
	int r,g,b;
	float fr,fg,fb;
	int rnd=random();
	int rnd2=random();
	int rnd3=random();
	int count = 0;
	int count_dir = 1;
	float s = 0.1f;
	while(1)
	{	
		int x,y;
		for(x=0;x<22;x++)
			for(y=0;y<6;y++)
		{
			r = (count+x)*(255/22);
			g = (count-x)*(255/22);
			b = (count-y)*(255/6);

			keys->rows[y].column[x].r = (unsigned char)r;
			keys->rows[y].column[x].g = (unsigned char)g;
			keys->rows[y].column[x].b = (unsigned char)b;
			keys->update_mask |= 1<<y;
		}
		update_keys(keys);
		count+=count_dir;
		if(count<=0 || count>=440)
		{
			count_dir=-count_dir;
			rnd = random();
			rnd2 = random();
			rnd3 = random();
		}
		usleep(120000);
	}
}

void test_effect6(struct razer_keys *keys)
{
	int r,g,b;
	float fr,fg,fb;
	int rnd=random();
	int rnd2=random();
	int rnd3=random();
	int count = 50;
	int count_dir = 1;
	float s = 0.1f;
	int mx=5*2;
	int vmax=15;
	int x[mx],y[mx],dir[mx],vr[mx],vg[mx],vb[mx];
	int i;
	for(i=0;i<mx/2;i++)
	{
		x[i]=(random()%21)+1;
		y[i]=(random()%5)+1;
		dir[i]=1;
		vr[i]=random()%vmax;
		vg[i]=random()%vmax;
		vb[i]=random()%vmax;
		//vr[i]=10;
		//vg[i]=10;
		//vb[i]=10;
	}
	for(i=mx/2;i<mx;i++)
	{
		x[i]=(random()%21)+1;
		y[i]=(random()%5)+1;
		dir[i]=0;
		vr[i]=random()%vmax;
		vg[i]=random()%vmax;
		vb[i]=random()%vmax;
		vr[i]=10;
		vg[i]=10;
		vb[i]=10;
	}
	

	while(1)
	{	
		for(i=0;i<mx;i++)
		{
			if(random()%4)
			{
			int d=random()%4;
			switch(d)
			{
			case 0:
				if(x[i]<21)
					x[i]++;
			break;
			case 1:
				if(x[i]>0)
					x[i]--;
			break;
			case 2:
				if(y[i]<5)
					y[i]++;
			break;
			case 3:
				if(y[i]>0)
					y[i]--;
			break;
			}
			}
			r = keys->rows[y[i]].column[x[i]].r;
			g = keys->rows[y[i]].column[x[i]].g;
			b = keys->rows[y[i]].column[x[i]].r;
			if(dir[i])
			{
				if(r<255-vr[i])
					r+=vr[i];
				else
					r=255;
				if(g<255-vg[i])
					g+=vg[i];
				else
					g=255;
				if(b<255-vb[i])
					b+=vb[i];
				else
					b=255;
			}
			else
			{
				if(r>0+vr[i])
					r-=vr[i];
				else
					r=0;
				if(g>0+vg[i])
					g-=vg[i];
				else
					g=0;
				if(b>0+vb[i])
					b-=vb[i];
				else
					b=0;
			}

			keys->rows[y[i]].column[x[i]].r = (unsigned char)r;
			keys->rows[y[i]].column[x[i]].g = (unsigned char)g;
			keys->rows[y[i]].column[x[i]].b = (unsigned char)b;

			keys->update_mask |= 1<<y[i];
		}

		update_keys(keys);
		count--;
		if(count<0)
		{
			count=50;

			for(i=0;i<mx/2;i++)
			{
				x[i]=(random()%21)+1;
				y[i]=(random()%5)+1;
				dir[i]=1;
				vr[i]=random()%vmax;
				vg[i]=random()%vmax;
				vb[i]=random()%vmax;
				//vr[i]=10;
				//vg[i]=10;
				//vb[i]=10;
			}
			for(i=mx/2;i<mx;i++)
			{
				x[i]=(random()%21)+1;
				y[i]=(random()%5)+1;
				dir[i]=0;
				vr[i]=random()%vmax;
				vg[i]=random()%vmax;
				vb[i]=random()%vmax;
				//vr[i]=10;
				//vg[i]=10;
				//vb[i]=10;
			}
		}
		usleep(10000);
	}
}

void test_effect7(struct razer_keys *keys)
{
	int v=1;
	int vdir = 1;
	while(1)
	{	
		int x,y;
		struct razer_rgb color1={.r=50,.g=0,.b=0};
		struct razer_rgb color2={.r=120,.g=0,.b=0};
		struct razer_rgb color3={.r=255,.g=0,.b=0};
		struct razer_rgb color4={.r=120,.g=0,.b=0};
		struct razer_rgb color5={.r=50,.g=0,.b=0};
		struct razer_rgb color_black={.r=0,.g=0,.b=0};

		//for(x=-5;x<5;x++)
		keys->update_mask=63;
		set_keys_column(keys,v,&color1);
		set_keys_column(keys,v-(vdir*2),&color2);
		set_keys_column(keys,v-(vdir*3),&color3);
		set_keys_column(keys,v-(vdir*4),&color4);
		set_keys_column(keys,v-(vdir*5),&color5);
		set_keys_column(keys,v-(vdir*6),&color_black);
		v=v+vdir;
		if(v>27 || v<-7)
			vdir=-vdir;
		update_keys(keys);
		usleep(1000);
	}
}

void test_effect8(struct razer_keys *keys)
{
	int v[3]={1,12,22};
	int vdir[3] = {1,1,1};
	int vcols[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	int i;
	while(1)
	{	
		for(i=0;i<3;i++)
		{
		struct razer_rgb color1={.r=50*vcols[i][0],.g=50*vcols[i][1],.b=50*vcols[i][2]};
		struct razer_rgb color2={.r=120*vcols[i][0],.g=120*vcols[i][1],.b=120*vcols[i][2]};
		struct razer_rgb color3={.r=255*vcols[i][0],.g=255*vcols[i][1],.b=255*vcols[i][2]};
		struct razer_rgb color4={.r=120*vcols[i][0],.g=120*vcols[i][1],.b=120*vcols[i][2]};
		struct razer_rgb color5={.r=50*vcols[i][0],.g=50*vcols[i][1],.b=50*vcols[i][2]};
		struct razer_rgb color_black={.r=0,.g=0,.b=0};
		keys->update_mask=63;
		set_keys_column(keys,v[i],&color1);
		set_keys_column(keys,v[i]-(vdir[i]*2),&color2);
		set_keys_column(keys,v[i]-(vdir[i]*3),&color3);
		set_keys_column(keys,v[i]-(vdir[i]*4),&color4);
		set_keys_column(keys,v[i]-(vdir[i]*5),&color5);
		//set_keys_column(keys,v[i]-(vdir[i]*6),&color_black);
		v[i]=v[i]+vdir[i];
		if(v[i]>27 || v[i]<-7)
			vdir[i]=-vdir[i];
		}
		update_keys(keys);
		usleep(1000);
	}
}

void test_effect9(struct razer_keys *keys)
{
	int v[3]={1,12,16};
	int vdir[3] = {1,1,-1};
	int vcols[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	int i,r;
	while(1)
	{	
		keys->update_mask=63;
		for(i=0;i<3;i++)
		{
			for(r=1;r<2;r++)
			{
		struct razer_rgb color_black={.r=0,.g=0,.b=0};
		struct razer_rgb color={.r=50*vcols[i][0],.g=50*vcols[i][1],.b=50*vcols[i][2]};
		struct razer_rgb color2={.r=25*vcols[i][0],.g=25*vcols[i][1],.b=25*vcols[i][2]};
		//struct razer_rgb color={.r=vcols[i][0]*70*r,.g=vcols[i][1]*70*r,.b=vcols[i][2]*70*r};
		
		add_keys_column(keys,v[i],&color);
		add_keys_column(keys,v[i]-1,&color2);
		//struct razer_rgb color2={.r=vcols[i][0]*70*(3-r),.g=vcols[i][1]*70*(3-r),.b=vcols[i][2]*70*(3-r)};
		sub_keys_column(keys,v[i]-(vdir[i]*17),&color);
		sub_keys_column(keys,v[i]-(vdir[i]*18),&color);
		sub_keys_column(keys,v[i]-(vdir[i]*19),&color2);
		sub_keys_column(keys,v[i]-(vdir[i]*20),&color2);
		}
		//set_keys_column(keys,v[i]-(vdir[i]*6),&color_black);
		v[i]=v[i]+vdir[i];
		if(v[i]>27 || v[i]<-7)
			vdir[i]=-vdir[i];

		}
		update_keys(keys);
		usleep(100);
	}
}


int e1_rnd=20;
int e1_rnd2=60;
int e1_rnd3=90;
int e1_count = 0;
int e1_count_dir = 1;
float e1_s = 0.1f;

void test_effect_frame(struct razer_keys *keys)
{
	int r,g,b;
	float fr,fg,fb;
	int x,y;
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
			r = (cos((e1_count+((e1_rnd%4)*90)+y)*e1_s)+sin(e1_count+x)*e1_s)*255;
			g = (cos((e1_count+((e1_rnd2%4)*90)+y)*e1_s)+sin(e1_count+x)*e1_s)*255;
			b = (cos((e1_count+((e1_rnd3%4)*90)+y)*e1_s)+sin(e1_count+x)*e1_s)*255;

			keys->rows[y].column[x].r = (unsigned char)r;
			keys->rows[y].column[x].g = (unsigned char)g;
			keys->rows[y].column[x].b = (unsigned char)b;
			keys->update_mask |= 1<<y;
		}
	}
	e1_count+=e1_count_dir;
	if(e1_count<=0 || e1_count>=30)
	{
		e1_count_dir=-e1_count_dir;
		e1_rnd = random();
		e1_rnd2 = random();
		e1_rnd3 = random();
	}
}

int e2_rnd=50;
int e2_rnd2=75;
int e2_rnd3=100;
int e2_count = 0;
int e2_count_dir = 1;
float e2_s = 0.1f;

void test_effect2_frame(struct razer_keys *keys)
{
	int r,g,b;
	float fr,fg,fb;
	int x,y;
	for(x=0;x<22;x++)
		for(y=0;y<6;y++)
		{
			r = (e2_count+x)*(255/22);
			g = (e2_count-x)*(255/22);
			b = (e2_count+y)*(255/22);

			keys->rows[y].column[x].r = (unsigned char)r;
			keys->rows[y].column[x].g = (unsigned char)g;
			keys->rows[y].column[x].b = (unsigned char)b;
			keys->update_mask |= 1<<y;
		}
	e2_count+=e2_count_dir;
	if(e2_count<=0 || e2_count>=44)
	{
		e2_count_dir=-e2_count_dir;
		e2_rnd = random();
		e2_rnd2 = random();
		e2_rnd3 = random();
	}
}
int e3_rnd=50;
int e3_rnd2=75;
int e3_rnd3=100;
int e3_count = 0;
int e3_count_dir = 1;
float e3_s = 0.1f;
void test_effect3_frame(struct razer_keys *keys)
{
	int r,g,b;
	float fr,fg,fb;
	int x,y;
	clear_all(keys);
	for(x=0;x<22;x++)
	{
		r = (e3_count+x)*(255/22);
		g = (e3_count-x)*(255/22);
		b = (e3_count+x)*(255/22);
		y = (int)((sin(((x*30)+e3_count)*0.2f)*3.0f)+3.0f);
		if(y>5)
			y=5;
		if(y<0)
			y=0;
		keys->rows[y].column[x].r = (unsigned char)r;
		keys->rows[y].column[x].g = (unsigned char)g;
		keys->rows[y].column[x].b = (unsigned char)b;
		keys->update_mask |= 1<<y;
	}
	e3_count+=e3_count_dir;
	if(e3_count<=0 || e3_count>=440)
	{
		e3_count_dir=-e3_count_dir;
		e3_rnd = random();
		e3_rnd2 = random();
		e3_rnd3 = random();
	}
}
void test_effect_heatmap_frame(struct razer_keys *keys)
{
	int x,y;
	struct razer_rgb color;
	long i,max_clicks;
	max_clicks = 1;
	//clear_all(keys);
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
			if(keys->heatmap[y][x]>max_clicks)
				max_clicks = keys->heatmap[y][x];
		}
	}
	//printf("max clicks:%d\n",max_clicks);
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
		rgb_from_hue((float)keys->heatmap[y][x]/(float)max_clicks,0.3f,0.0f,&color);
		rgb_mix(&keys->rows[y].column[x],&color,0.7f);
	
		//keys->rows[y].column[x].g = color.g;
		//keys->update_mask |= 1<<y;
		}
	}
	keys->update_mask = 63;//update all rows
}
*/
/*float scroll_x,scroll_y;
int scroll_width,scroll_height;
double scroll_dir_x,scroll_dir_y;
unsigned char *scroll_buf=NULL;
void test_effect_scroll_frame(struct razer_keys *keys)
{
	int x,y;
	struct razer_rgb color;
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
			color.r = scroll_buf[((int)scroll_x+x+((int)scroll_y+y)*scroll_width)*3+0];
			color.g = scroll_buf[((int)scroll_x+x+((int)scroll_y+y)*scroll_width)*3+1];
			color.b = scroll_buf[((int)scroll_x+x+((int)scroll_y+y)*scroll_width)*3+2];
			rgb_mix(&keys->rows[y].column[x],&color,0.9f);
			//keys->rows[y].column[x].g = color.g;
			//keys->update_mask |= 1<<y;
		}
	}
	//usleep(50000);
	keys->update_mask = 63;//update all rows
	scroll_x += scroll_dir_x;
	scroll_y += scroll_dir_y;
	if(scroll_x<0)
	{
		scroll_x = 0;
		scroll_dir_x = -scroll_dir_x;
	}
	if(scroll_y<0)
	{
		scroll_y = 0;
		scroll_dir_y = -scroll_dir_y;
	}
	if(scroll_x>scroll_width-22)
	{
		scroll_x = scroll_width-22;
		scroll_dir_x = -scroll_dir_x;
	}
	if(scroll_y>scroll_height-6)
	{
		scroll_y = scroll_height-6;
		scroll_dir_y = -scroll_dir_y;
	}
}

SDL_Texture *sdl_icons[32];
void update_sdl(struct razer_keys *keys,SDL_Renderer *sdl,SDL_Window *window,SDL_Texture *tex)
{
	int x,y;
	unsigned char *pixels = (unsigned char*)malloc(4*22*6);
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
			pixels[((x+(y*22))*4)+3] = 255;
			pixels[((x+(y*22))*4)+0] = keys->rows[y].column[x].b;
			pixels[((x+(y*22))*4)+1] = keys->rows[y].column[x].g;
			pixels[((x+(y*22))*4)+2] = keys->rows[y].column[x].r;
		}
	}
	SDL_Rect rect;

	SDL_UpdateTexture(tex, NULL, pixels, 22*4);
	SDL_RenderClear(sdl);
	rect.x=0;
	rect.y=0;
	int w,h;
	SDL_GetWindowSize(window,&w,&h);
	rect.w=w;
	rect.h=h-32;
	SDL_RenderCopy(sdl,tex,NULL,&rect);

	int i;
	for(i=0;i<4;i++)
	{
		rect.x=10+i*26;
		rect.y=h-24;
		rect.w=16;
		rect.h=16;
		SDL_RenderCopy(sdl,sdl_icons[i],NULL,&rect);
	}


	SDL_RenderPresent(sdl);
}
void load_icons(SDL_Renderer *renderer,char *path,SDL_Texture **icons)
{

	//sdl_icons[0] = SDL_CreateTexture(sdl_renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,22,6);
    sdl_icons[0] = IMG_LoadTexture(renderer,"icons/IPencil.png");
    sdl_icons[1] = IMG_LoadTexture(renderer,"icons/ILink.png");
    sdl_icons[2] = IMG_LoadTexture(renderer,"icons/IWrench.png");
    sdl_icons[3] = IMG_LoadTexture(renderer,"icons/ITrash.png");
}
//list of last keystrokes
//time since hit /hitstamps

void capture_keys(struct razer_keys *keys,SDL_Renderer *renderer,SDL_Window *window,SDL_Texture *tex)
{
	struct timeval start,tv,select_tv;
	gettimeofday(&start, NULL);
	int ev_count = 0;
	int finput=open("/dev/input/event3",O_RDONLY | O_NONBLOCK | O_NOCTTY | O_NDELAY);
	fcntl(finput,F_SETFL,0);
	int last_keycode=0;
	fd_set rs;
	select_tv.tv_sec = 0;
	select_tv.tv_usec = 0;
	int r;
	int actual_mode = 0;
	int last_event_count=0;
	int done = 0;
	long actual_ms = start.tv_usec;
	long last_ms = start.tv_usec;
	long diff_ms = 0;
	while(!done)
	{

		gettimeofday(&tv, NULL);
		last_ms = actual_ms;
		actual_ms = tv.tv_usec;
		diff_ms = actual_ms-last_ms;
		SDL_Event event;
	    while(SDL_PollEvent(&event)) 
    	{
		    if(event.type == SDL_KEYUP)
    		{
		    	if(event.key.keysym.sym == SDLK_ESCAPE)
			    	done=1;
	      	}
		    if(event.type == SDL_MOUSEBUTTONUP)
    		{
		    	//if(event.key.keysym.sym == SDLK_ESCAPE)
				int w,h;
				SDL_GetWindowSize(window,&w,&h);
				int kw=w/22;
				int kh=(h-32)/6;
		    	if(event.button.y<h-32)
		    	{
		    		int kx = (event.button.x)/kw;
		    		int ky = event.button.y/kh;
		    		//printf("button pressed in:%d,%d\n",kx,ky);
					struct razer_rgb cr = {.r=128,.g=0,.b=0};
					set_key(keys,kx,ky,&cr);
		    	}
			    //	done=1;

	      	}
		    if(event.type == SDL_QUIT)
    		{
		    	done=1;
	      	}

 		}

		struct razer_pos pos;
		FD_ZERO(&rs);
		FD_SET(finput,&rs);
		r = select(finput+1,&rs,0,0,&select_tv);
		//r = select(finput+1,&rs,0,0,0);
		//clear_all(keys);
		if(actual_mode==0)
			test_effect_frame(keys);
		else
			if(actual_mode==1)
				test_effect2_frame(keys);
			else
				if(actual_mode==3)
					//test_effect3_frame(keys);
					test_effect_heatmap_frame(keys);
				else
					if(actual_mode==4)
					{
		//				test_effect_heatmap_frame(keys);
						//clear_all(keys);
						test_effect_scroll_frame(keys);
					}

		//clear_all(keys);
		test_effect_scroll_frame(keys);
		//test_effect_heatmap_frame(keys);
		struct razer_rgb lcr = {.r=128,.g=0,.b=0};
		if(actual_mode!=3 && actual_mode != 4)
		{
			set_keys_row(keys,pos.y,&lcr);
			set_keys_column(keys,pos.x,&lcr);
		}
		if(actual_mode==1 || actual_mode == 2)
		{
			int x,y;
			for(x=0;x<22;x++)
			{
				for(y=0;y<6;y++)
				{
					struct razer_pos hpos = {.x=x,.y=y};
					//struct razer_rgb hcol = {.r=keys->heatmap[y][x],.g=random()%4*3,.b=0};
					struct razer_rgb hcol = {.r=keys->heatmap[y][x],.g=0,.b=0};
					if(keys->heatmap[y][x]||actual_mode==2)
						set_key_pos(keys,&hpos,&hcol);
					//if(keys->heatmap[y][x]>0)
					//	keys->heatmap[y][x]-=1;
				}
			}
		}

		

		if(!r)
		{
			update_keys(keys);
			update_sdl(keys,renderer,window,tex);
			continue;
		}
		char buf[2048];
		if(FD_ISSET(finput,&rs))
		{

			int n=2048;
			n = read(finput,buf,2048);
			if(n<0)
			{
				//if(errno == EAGAIN)
				//	printf("waiting for data\n");
			}
			else if(n==0)
			{
				close(finput);
				return;
			}
			else				
			{
				int i;
				for(i=0;i<n/sizeof(struct input_event);i++)
				{
					struct input_event *event = buf+(i*sizeof(struct input_event));
					if(event->type==EV_KEY)
					{
						if(event->value==1)
						{
							int keycode = event->code;
							//if(keycode!=last_keycode)
							{
								if(keycode==183)
									actual_mode=0;
								if(keycode==184)
									actual_mode=1;
								if(keycode==185)
									actual_mode=2;
								if(keycode==186)
									actual_mode=3;
								if(keycode==187)
									actual_mode=4;
								//printf("ev_code:%d\n",event->code);
								struct razer_rgb cr = {.r=128,.g=0,.b=0};
								struct razer_rgb cg = {.r=0,.g=128,.b=0};
								struct razer_rgb cb = {.r=0,.g=0,.b=128};
								struct razer_rgb cw = {.r=255,.g=255,.b=255};
								struct razer_pos old;
								old.x=pos.x;
								old.y=pos.y;
								convert_keycode_to_pos(event->code,&pos);							

								//double pangle=rad2deg(pos_angle_radians(&old,&pos));
								double pangle=pos_angle_radians(&old,&pos);
								//printf("pangle:%f\n",pangle);
								scroll_dir_y=-cos(pangle);
								scroll_dir_x=-sin(pangle);
								//printf("dir:%f,%f\n",scroll_dir_x,scroll_dir_y);

								keys->heatmap[pos.y][pos.x]+=1;//80;
								//if(keys->heatmap[pos.y][pos.x]>255)
								//	keys->heatmap[pos.y][pos.x]=255;

								//set_key_pos(keys,&pos,&color);
								//if(!(ev_count%6))
								//	clear_all(keys);
								if(actual_mode!=4)
								{
									if(ev_count%4==0)
									{
										set_keys_row(keys,pos.y,&cr);
										set_keys_column(keys,pos.x,&cr);
									}
									if(ev_count%4==1)
									{
										set_keys_row(keys,pos.y,&cg);
										set_keys_column(keys,pos.x,&cg);
									}
									if(ev_count%4==2)
									{
										set_keys_row(keys,pos.y,&cb);
										set_keys_column(keys,pos.x,&cb);
									}
									//if(ev_count%4==3)
									//{
										//set_keys_row(keys,pos.y,&cb);
										//set_keys_column(keys,pos.x,&cb);
										//draw_circle(keys,&pos,1,&cw);
										//draw_circle(keys,&pos,2,&cr);
										//draw_circle(keys,&pos,3,&cw);
										draw_ring(keys,&pos,3,&cw);
									//}
								}
								//update_keys(keys);
								ev_count++;
							}
							last_keycode = keycode;
						}
					}
				}
				//update_keys(keys);
			}
		}	
		update_keys(keys);
		update_sdl(keys,renderer,window,tex);
		last_event_count = ev_count;
	}
}
*/

int daemonize()
{
	pid_t pid = 0;
	pid_t sid = 0;
	pid = fork();
	if(pid<0)
	{
		printf("razer_bcd: fork failed\n");
		exit(1);
	}
	if(pid)
	{
		#ifdef USE_DEBUGGING
			printf("killing razer_bcd parent process\n");
		#endif
		exit(0);
	}
	umask(0);
	sid = setsid();
	if(sid < 0)
		exit(1);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return(1);
}


int main(int argc,char *argv[])
{

	struct razer_daemon *daemon=NULL;
	daemon_open(&daemon);
    razer_set_custom_mode(daemon->chroma);
	clear_all(daemon->chroma->keys);
	razer_update_keys(daemon->chroma,daemon->chroma->keys);
	//capture_keys(&keys,sdl_renderer,sdl_window,sdl_texture);
	daemonize();
	test_effect4(daemon->chroma,daemon->chroma->keys);
    daemon_close(&daemon);
}