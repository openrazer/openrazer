#include "razer_daemon.h"

int dbus_error_check(char*message,DBusError *error)
{
	if(dbus_error_is_set(error))
	{
		#ifdef USE_DEBUGGING
		printf("dbus (%s) error:%s\n",message,error->message);
		#endif
		dbus_error_free(error);
		return(0);
	}
	return(1);
}

int dbus_open(struct razer_daemon *daemon)
{
	DBusError error;
	dbus_error_init(&error);
	daemon->dbus = dbus_bus_get(DBUS_BUS_SESSION,&error);
	if(!dbus_error_check("open",&error))
		return(0);
	if(!daemon->dbus)
		return(0);
	return(1);
}

int dbus_announce(struct razer_daemon *daemon)
{
	DBusError error;
	dbus_error_init(&error);
	dbus_bus_request_name(daemon->dbus,"org.voyagerproject.razer.chroma.daemon",DBUS_NAME_FLAG_REPLACE_EXISTING,&error);
	if(!dbus_error_check("request_name",&error))
		return(0);
	dbus_bus_add_match(daemon->dbus,"type='method_call',interface='org.voyagerproject.razer.chroma.daemon.fx.set'",&error);
	if(!dbus_error_check("add_match",&error))
		return(0);
	dbus_bus_add_match(daemon->dbus,"type='method_call',interface='org.voyagerproject.razer.chroma.daemon.set_fps'",&error);
	if(!dbus_error_check("add_match",&error))
		return(0);

	return(1);
}

int dbus_handle_messages(struct razer_daemon *daemon)
{
	DBusMessage *msg;
	DBusMessage *reply;
	DBusMessageIter parameters;
	dbus_connection_read_write(daemon->dbus,0);
	msg = dbus_connection_pop_message(daemon->dbus);
	if(!msg)
		return(0);
	printf("dbus: received message:type:%d ,path:%s ,interface:%s ,member:%s\n",dbus_message_get_type(msg),dbus_message_get_path(msg),dbus_message_get_interface(msg),dbus_message_get_member(msg));
    if (dbus_message_is_method_call(msg, "org.voyagerproject.razer.chroma.daemon.fx", "set")) 
	{
		#ifdef USE_DEBUGGING
			printf("dbus: method set fx called\n");
		#endif
		if(!dbus_message_iter_init(msg, &parameters))
		{
			#ifdef USE_DEBUGGING
				printf("dbus: signal set fx received without parameter\n");
			#endif
		}
		if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
		{
			char *fxname;
			dbus_message_iter_get_basic(&parameters,&fxname);
			printf("parameter:%s\n",fxname);
		}

		struct razer_rgb col={.r=5,.g=0,.b=0};
		for(int i=0;i<250;i++)
		{
			col.r = 5+(i*1);
			set_all(daemon->chroma->keys,&col);
			razer_update_keys(daemon->chroma,daemon->chroma->keys);
			//usleep((1000*1000)/255);
		}
		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
 		{
			printf(stderr, "Out Of Memory!\n");
			exit(1);
		}
		dbus_connection_flush(daemon->dbus);
	}
    if (dbus_message_is_method_call(msg, "org.voyagerproject.razer.chroma.daemon", "set_fps")) 
	{
		#ifdef USE_DEBUGGING
			printf("dbus: method set fps called\n");
		#endif
		if(!dbus_message_iter_init(msg, &parameters))
		{
			#ifdef USE_DEBUGGING
				printf("dbus: signal set fps received without parameter\n");
			#endif
		}
		if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
		{
			dbus_message_iter_get_basic(&parameters,&daemon->fps);
			printf("parameter:%d\n",daemon->fps);
		}
		//daemon->fps = daemon->fps - 1;
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&daemon->fps)) 
		{
			printf("dbus: Out Of Memory!\n");
			exit(1);
		} 
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
 		{
			printf(stderr, "Out Of Memory!\n");
			exit(1);
		}
		dbus_connection_flush(daemon->dbus);
	}

    if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect")) 
	{
		#ifdef USE_DEBUGGING
			printf("dbus: method Introspect called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *xml_data = 
		"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n\
		<node>\n\
		<interface name=\"org.freedesktop.DBus.Introspectable\">\n\
		<method name=\"Introspect\">\n\
		<arg direction=\"out\" name=\"data\" type=\"s\">\n\
		</arg></method>\n\
		</interface>\n\
		<interface name=\"org.voyagerproject.razer.chroma.daemon.fx\">\n\
		<method name=\"set\">\n\
		<arg direction=\"in\" name=\"fxname\" type=\"s\">\n\
		</arg></method></interface>\n\
		<interface name=\"org.freedesktop.DBus.Properties\">\n\
		<method name=\"Get\">\n\
		<arg direction=\"in\" name=\"interface\" type=\"s\">\n\
		<arg direction=\"in\" name=\"propname\" type=\"s\">\n\
		<arg direction=\"out\" name=\"value\" type=\"v\">\n\
		</arg></arg></arg></method>\n\
		<method name=\"Set\">\n\
		<arg direction=\"in\" name=\"interface\" type=\"s\">\n\
		<arg direction=\"in\" name=\"propname\" type=\"s\">\n\
		<arg direction=\"in\" name=\"value\" type=\"v\">\n\
		</arg></arg></arg></method>\n\
		<method name=\"GetAll\">\n\
		<arg direction=\"in\" name=\"interface\" type=\"s\">\n\
		<arg direction=\"out\" name=\"props\" type=\"a{sv}\">\n\
		</arg></arg></method>\n\
		</interface>\n\
		</node>\n";
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&xml_data)) 
		{
			printf("dbus: Out Of Memory!\n");
			exit(1);
		} 
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
 		{
			printf(stderr, "Out Of Memory!\n");
			exit(1);
		}
		dbus_connection_flush(daemon->dbus);
	}


	if(dbus_message_is_signal(msg,"org.voyagerproject.razer.chroma.daemon.fx.set.Type","set"))
	{
		#ifdef USE_DEBUGGING
			printf("dbus: signal set fx received\n");
		#endif
		if(!dbus_message_iter_init(msg, &parameters))
		{
			#ifdef USE_DEBUGGING
				printf("dbus: signal set fx received without parameter\n");
			#endif
		}
		if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
		{
			char *fxname;
			dbus_message_iter_get_basic(&parameters,&fxname);
			printf("parameter:%s\n",fxname);
		}
	}
	dbus_message_unref(msg);
}


void dbus_close(struct razer_daemon *daemon)
{
	if(daemon->dbus)
		dbus_connection_unref(daemon->dbus);
}

int daemon_open(struct razer_daemon **daemon)
{
 	//signal(SIGINT,stop);
 	//signal(SIGKILL,stop);
    //signal(SIGTERM,stop);	

  	*daemon = (struct razer_daemon*)malloc(sizeof(struct razer_daemon));
 	(*daemon)->chroma = (struct razer_chroma*)malloc(sizeof(struct razer_chroma));
 	(*daemon)->dbus = NULL;
 	(*daemon)->running = 1;
 	(*daemon)->fps = 12;
 	if(!razer_open((*daemon)->chroma))
		return(0);
 	if(!dbus_open((*daemon)))
 		return(0);
 	if(!dbus_announce((*daemon)))
 		return(0);

    razer_set_custom_mode((*daemon)->chroma);
	clear_all((*daemon)->chroma->keys);
	razer_update_keys((*daemon)->chroma,(*daemon)->chroma->keys);

 	return(1);
}

void daemon_close(struct razer_daemon **daemon)
{
	dbus_close((*daemon));
 	razer_close((*daemon)->chroma);
 	free((*daemon)->chroma);
 	free(*daemon);
}

int daemon_run(struct razer_daemon *daemon)
{
    while(daemon->running)
	{	
		unsigned long ticks = razer_get_ticks();
		effect3(daemon);
		dbus_handle_messages(daemon);
		//usleep(6000);
		razer_update(daemon->chroma);
		razer_frame_limiter(daemon->chroma,daemon->fps);
		unsigned long end_ticks = razer_get_ticks();
		//printf("frame time:%ums,actual fps:%f\n",end_ticks-ticks,1000/(end_ticks-ticks));
	}
}

	int r,g,b;
	int rnd=0;
	int rnd2=0;
	int rnd3=0;
	int count = 0;
	int count_dir = 1;
	float s = 0.1f;

void effect(struct razer_daemon *daemon)
{
	int x,y;
	for(x=0;x<22;x++)
		for(y=0;y<6;y++)
		{
			r = (count+x)*(255/22);
			g = (count-x)*(255/22);
			b = (count+y)*(255/22);

			daemon->chroma->keys->rows[y].column[x].r = (unsigned char)r;
			daemon->chroma->keys->rows[y].column[x].g = (unsigned char)g;
			daemon->chroma->keys->rows[y].column[x].b = (unsigned char)b;
			daemon->chroma->keys->update_mask |= 1<<y;
		}
	razer_update_keys(daemon->chroma,daemon->chroma->keys);
	count+=count_dir;
	if(count<=0 || count>=44)
	{
		count_dir=-count_dir;
		rnd = random();
		rnd2 = random();
		rnd3 = random();
	}
}

void effect2(struct razer_daemon *daemon)
{
	int x,y;
	for(x=0;x<22;x++)
		for(y=0;y<6;y++)
		{
			r = (count+x)*(255/22);
			g = (count-x)*(255/22);
			b = (count-y)*(255/6);

			daemon->chroma->keys->rows[y].column[x].r = (unsigned char)r;
			daemon->chroma->keys->rows[y].column[x].g = (unsigned char)g;
			daemon->chroma->keys->rows[y].column[x].b = (unsigned char)b;
			daemon->chroma->keys->update_mask |= 1<<y;
		}
	razer_update_keys(daemon->chroma,daemon->chroma->keys);
	count+=count_dir;
	if(count<=0 || count>=440)
	{
		count_dir=-count_dir;
		rnd = random();
		rnd2 = random();
		rnd3 = random();
	}
}

int e1_rnd=20;
int e1_rnd2=60;
int e1_rnd3=90;
int e1_count = 0;
int e1_count_dir = 1;
float e1_s = 0.1f;

void effect3(struct razer_daemon *daemon)
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

			daemon->chroma->keys->rows[y].column[x].r = (unsigned char)r;
			daemon->chroma->keys->rows[y].column[x].g = (unsigned char)g;
			daemon->chroma->keys->rows[y].column[x].b = (unsigned char)b;
			daemon->chroma->keys->update_mask |= 1<<y;
		}
	}
	razer_update_keys(daemon->chroma,daemon->chroma->keys);
	e1_count+=e1_count_dir;
	if(e1_count<=0 || e1_count>=30)
	{
		e1_count_dir=-e1_count_dir;
		e1_rnd = random();
		e1_rnd2 = random();
		e1_rnd3 = random();
	}
}


/*

void sdl_update()
{
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
		update_sdl(keys,renderer,window,tex);
}

*/

/*
void create_sdl_window()
{
   	SDL_Init(SDL_INIT_VIDEO);      
	SDL_Window *sdl_window;
	SDL_Renderer *sdl_renderer;
	SDL_CreateWindowAndRenderer(22*32, 6*32, SDL_WINDOW_RESIZABLE, &sdl_window, &sdl_renderer);
	SDL_SetWindowTitle(sdl_window,"Razer Chroma Setup/Debug");
	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
	SDL_RenderClear(sdl_renderer);
	SDL_RenderPresent(sdl_renderer);
	SDL_Texture *sdl_texture = SDL_CreateTexture(sdl_renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,22,6);
	load_icons(sdl_renderer,"icons",sdl_icons);
}


void close_sdl_window()
{
  	SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}
*/
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
*/
//list of last keystrokes
//time since hit /hitstamps


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
	{
		printf("razer_bcd: setsid failed\n");
		exit(1);
	}
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return(1);
}


int main(int argc,char *argv[])
{
	printf("Starting razer blackwidow chroma daemon\n");
	#ifndef USE_DEBUGGING
		daemonize();
	#endif

	struct razer_daemon *daemon=NULL;
	if(!daemon_open(&daemon))
	{
		printf("razer_bcd: error initializing daemon\n");
		return(1);
	}
	daemon_run(daemon);
    daemon_close(&daemon);
}