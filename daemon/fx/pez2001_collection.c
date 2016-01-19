/* 
 * razer_chroma_drivers - a driver/tools collection for razer chroma devices
 * (c) 2015 by Tim Theede aka Pez2001 <pez2001@voyagerproject.de> / vp
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * THIS SOFTWARE IS SUPPLIED AS IT IS WITHOUT ANY WARRANTY!
 *
 */
#include "pez2001_collection.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

struct razer_effect *effect_mix1 = NULL;

int effectmix1_update(struct razer_fx_render_node *render)
{
	int x,y;
	//if(render->second_input_frame && render->input_frame)
	//{
		for(x=0;x<render->device->columns_num;x++)
			for(y=0;y<render->device->rows_num;y++)
			{
				rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&render->second_input_frame->rows[y]->column[x],render->opacity);
				render->output_frame->update_mask |= 1<<y;
			}
	//}
	return(1);
}



struct razer_effect *effect1 = NULL;

int effect1_update(struct razer_fx_render_node *render)
{
	struct razer_rgb color;
	//struct razer_chroma *chroma = render->daemon->chroma;
	int x,y;
	int count = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	int dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	for(x=0;x<render->device->columns_num;x++)
		for(y=0;y<render->device->rows_num;y++)
		{
			color.r = (count+x)*(255/22);
			color.g = (count-x)*(255/22);
			color.b = (count+y)*(255/22);

			/*chroma->keys->rows[y]->column[x].r = (unsigned char)r;
			chroma->keys->rows[y]->column[x].g = (unsigned char)g;
			chroma->keys->rows[y]->column[x].b = (unsigned char)b;
			chroma->keys->update_mask |= 1<<y;*/
			/*render->output_frame->rows[y]->column[x].r = (unsigned char)r;
			render->output_frame->rows[y]->column[x].g = (unsigned char)g;
			render->output_frame->rows[y]->column[x].b = (unsigned char)b;*/
			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&color,render->opacity);
			render->output_frame->update_mask |= 1<<y;
		}
	//razer_update_keys(chroma,chroma->keys);
	count+=dir;
	#ifdef USE_DEBUGGING
		printf(" (1st One ## end counter:%d,count:%d,dir:%d)",daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0)),count,dir);
	#endif
	if(count<=0 || count>=daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0)))
		dir=-dir;
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),count);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),dir);	
	return(1);
}

struct razer_effect *effect2 = NULL;
struct razer_rgb effect2_base_rgb = {.r=255,.g=0,.b=0};

int effect2_update(struct razer_fx_render_node *render)
{
	struct razer_rgb color;
	int x,y;
	struct razer_rgb *base_color = daemon_get_parameter_rgb(daemon_effect_get_parameter_by_index(render->effect,0));
	int count = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	int dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	#ifdef USE_DEBUGGING
		printf(" (breath ## breathing color: %d,%d,%d,count:%d,dir:%d)",base_color->r,base_color->g,base_color->b,count,dir);
	#endif
	for(x=0;x<render->device->columns_num;x++)
		for(y=0;y<render->device->rows_num;y++)
		{
			color.r = (int)((float)count*((float)base_color->r/100.0f));
			color.g = (int)((float)count*((float)base_color->g/100.0f));
			color.b = (int)((float)count*((float)base_color->b/100.0f));

			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&color,render->opacity);
			render->output_frame->update_mask |= 1<<y;
		}
	count+=dir;
	if(count<1 || count>99)
		dir=-dir;
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),count);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),dir);	
	return(1);
}

struct razer_effect *effect3 = NULL;

int effect3_update(struct razer_fx_render_node *render)
{
	struct razer_rgb color;
	int count = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0));
	int dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	int rnd1 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	int rnd2 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,3));
	int rnd3 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,4));
	int x,y;
	float s = 0.1f;
	#ifdef USE_DEBUGGING
		printf(" (Wave ## count:%d,dir:%d)",count,dir);
	#endif
	for(x=0;x<render->device->columns_num;x++)
	{
		for(y=0;y<render->device->rows_num;y++)
		{
			color.r = (cos((count+((rnd1%4)*90)+y)*s)+sin(count+x)*s)*255;
			color.g = (cos((count+((rnd2%4)*90)+y)*s)+sin(count+x)*s)*255;
			color.b = (cos((count+((rnd3%4)*90)+y)*s)+sin(count+x)*s)*255;
			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&color,render->opacity);
			render->output_frame->update_mask |= 1<<y;
		}
	}
	count+=dir;
	if(count<=0 || count>=30)
	{
		dir=-dir;
		rnd1 = random();
		rnd2 = random();
		rnd3 = random();
	}
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0),count);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),dir);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),rnd1);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,3),rnd2);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,4),rnd3);	
	return(1);
}

struct razer_effect *effect4 = NULL;

int effect4_update(struct razer_fx_render_node *render)
{
	struct razer_rgb color;
	int v1 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0));
	int v1_dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	int v2 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	int v2_dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,3));
	int v3 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,4));
	int v3_dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,5));
	int rnd1 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,6));
	int rnd2 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,7));
	int rnd3 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,8));
	int x,y;
	#ifdef USE_DEBUGGING
		printf(" (Random ## v1:%d,v1dir:%d,v2:%d,v2dir:%d,v3:%d,v3dir:%d)",v1,v1_dir,v2,v2_dir,v3,v3_dir);
	#endif
	for(x=0;x<render->device->columns_num;x++)
		for(y=0;y<render->device->rows_num;y++)
		{
			color.r = (unsigned char)v1-v1*sin(y)+cos(x);
			color.g = (unsigned char)v2-v2*cos(y)-x;
			color.b = (unsigned char)v3-v3*sin(x);
			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&color,render->opacity);
			render->output_frame->update_mask |= 1<<y;
		}
	if((v1+v1_dir*rnd1)>=255 || (v1+v1_dir*rnd1)<=0)
	{
		v1_dir=-v1_dir;
		rnd1=(random()%3)+1;
	}
	if((v2+v2_dir*rnd2)>=255 || (v2+v2_dir*rnd2)<=0)
	{
		v2_dir=-v2_dir;
		rnd2=(random()%3)+1;
	}
	if((v3+v3_dir*rnd3)>=255 || (v3+v3_dir*rnd3)<=0)
	{
		v3_dir=-v3_dir;
		rnd3=(random()%3)+1;
	}

	v1=v1+v1_dir*rnd1;
	v2=v2+v2_dir*rnd2;
	v3=v3+v3_dir*rnd3;
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0),v1);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),v1_dir);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),v2);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,3),v2_dir);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,4),v3);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,5),v3_dir);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,6),rnd1);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,7),rnd2);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,8),rnd3);	
	return(1);
}

struct razer_effect *effect5 = NULL;

int effect5_update(struct razer_fx_render_node *render)
{
	struct razer_rgb color;
	int count = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0));
	int dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	int rnd1 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	int rnd2 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,3));
	int rnd3 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,4));
	float s = 0.1f;
	int x,y;
	for(x=0;x<render->device->columns_num;x++)
		for(y=0;y<render->device->rows_num;y++)
		{
			color.r = (cos((count+((rnd1%4)*90)+y)*s))*255;
			color.g = (cos((count+((rnd2%4)*90)+y)*s))*255;
			color.b = (cos((count+((rnd3%4)*90)+y)*s))*255;
			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&color,render->opacity);
			render->output_frame->update_mask |= 1<<y;
		}
	count+=dir;
	if(count<=0 || count>=30)
	{
		dir=-dir;
		rnd1= random();
		rnd2 = random();
		rnd3 = random();
	}
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0),count);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),dir);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),rnd1);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,3),rnd2);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,4),rnd3);	
	return(1);
}

struct razer_effect *effect6 = NULL;

int effect6_update(struct razer_fx_render_node *render)
{
	struct razer_rgb color;
	int count = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0));
	int dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	int rnd1 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	int rnd2 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,3));
	int rnd3 = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,4));
	int x,y;
	for(x=0;x<render->device->columns_num;x++)
		for(y=0;y<render->device->rows_num;y++)
		{
			color.r = (count+x)*(255/22);
			color.g = (count-x)*(255/22);
			color.b = (count-y)*(255/6);
			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&color,render->opacity);
			render->output_frame->update_mask |= 1<<y;
		}
		count+=dir;
		if(count<=0 || count>=440)
		{
		dir=-dir;
		rnd1 = random();
		rnd2 = random();
		rnd3 = random();
	}
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0),count);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),dir);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),rnd1);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,3),rnd2);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,4),rnd3);	
	return(1);
}

struct razer_effect *effect7 = NULL;

int effect7_update(struct razer_fx_render_node *render)
{
	int count = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0));
	int dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	struct razer_rgb color1={.r=50,.g=0,.b=0};
	struct razer_rgb color2={.r=120,.g=0,.b=0};
	struct razer_rgb color3={.r=255,.g=0,.b=0};
	struct razer_rgb color4={.r=120,.g=0,.b=0};
	struct razer_rgb color5={.r=50,.g=0,.b=0};
	struct razer_rgb color_black={.r=0,.g=0,.b=0};
	razer_set_frame_column(render->output_frame,count,&color1);
	razer_set_frame_column(render->output_frame,count-(dir*2),&color2);
	razer_set_frame_column(render->output_frame,count-(dir*3),&color3);
	razer_set_frame_column(render->output_frame,count-(dir*4),&color4);
	razer_set_frame_column(render->output_frame,count-(dir*5),&color5);
	razer_set_frame_column(render->output_frame,count-(dir*6),&color_black);
	//razer_mix_frames(render->output_frame,render->input_frame,render->opacity);
	razer_mix_frames(render->output_frame,render->input_frame,render->opacity);
	#ifdef USE_DEBUGGING
		printf(" (Copper ## count:%d,dir:%d)",count,dir);
	#endif
	count=count+dir;
	if(count>27 || count<-7)
		dir=-dir;
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0),count);	
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),dir);	
	return(1);
}

struct razer_effect *effect8 = NULL;
struct razer_int_array *effect8_v = NULL;
struct razer_int_array *effect8_vdir = NULL;
struct razer_rgb_array *effect8_vcols = NULL;
struct razer_rgb *effect8_vcol1 = NULL;
struct razer_rgb *effect8_vcol2 = NULL;
struct razer_rgb *effect8_vcol3 = NULL;

int effect8_update(struct razer_fx_render_node *render)
{
	struct razer_int_array *v=daemon_get_parameter_int_array(daemon_effect_get_parameter_by_index(render->effect,0));
	struct razer_int_array *vdir = daemon_get_parameter_int_array(daemon_effect_get_parameter_by_index(render->effect,1));
	struct razer_rgb_array *vcols = daemon_get_parameter_rgb_array(daemon_effect_get_parameter_by_index(render->effect,2));

	//int v[3]={1,12,22};
	//int vdir[3] = {1,1,1};
	//int vcols[3][3] = {{1,0,0},{0,1,0},{0,0,1}};

	int i;
	for(i=0;i<3;i++)
	{
		struct razer_rgb color1={.r=50*vcols->values[i]->r,.g=50*vcols->values[i]->g,.b=50*vcols->values[i]->b};
		struct razer_rgb color2={.r=120*vcols->values[i]->r,.g=120*vcols->values[i]->g,.b=120*vcols->values[i]->b};
		struct razer_rgb color3={.r=255*vcols->values[i]->r,.g=255*vcols->values[i]->g,.b=255*vcols->values[i]->b};
		struct razer_rgb color4={.r=120*vcols->values[i]->r,.g=120*vcols->values[i]->g,.b=120*vcols->values[i]->b};
		struct razer_rgb color5={.r=50*vcols->values[i]->r,.g=50*vcols->values[i]->g,.b=50*vcols->values[i]->b};
		//struct razer_rgb color_black={.r=0,.g=0,.b=0};
		razer_set_frame_column(render->output_frame,v->values[i],&color1);
		razer_set_frame_column(render->output_frame,v->values[i]-(vdir->values[i]*2),&color2);
		razer_set_frame_column(render->output_frame,v->values[i]-(vdir->values[i]*3),&color3);
		razer_set_frame_column(render->output_frame,v->values[i]-(vdir->values[i]*4),&color4);
		razer_set_frame_column(render->output_frame,v->values[i]-(vdir->values[i]*5),&color5);
		v->values[i]=v->values[i]+vdir->values[i];
		if(v->values[i]>27 || v->values[i]<-7)
			vdir->values[i]=-vdir->values[i];
	}
	razer_mix_frames(render->output_frame,render->input_frame,render->opacity);
	return(1);
}

struct razer_effect *effect9 = NULL;
struct razer_int_array *effect9_v = NULL;
struct razer_int_array *effect9_vdir = NULL;
struct razer_rgb_array *effect9_vcols = NULL;
struct razer_rgb *effect9_vcol1 = NULL;
struct razer_rgb *effect9_vcol2 = NULL;
struct razer_rgb *effect9_vcol3 = NULL;

int effect9_update(struct razer_fx_render_node *render)
{
	struct razer_int_array *v=daemon_get_parameter_int_array(daemon_effect_get_parameter_by_index(render->effect,0));
	struct razer_int_array *vdir = daemon_get_parameter_int_array(daemon_effect_get_parameter_by_index(render->effect,1));
	struct razer_rgb_array *vcols = daemon_get_parameter_rgb_array(daemon_effect_get_parameter_by_index(render->effect,2));
	//int v[3]={1,12,16};
	//int vdir[3] = {1,1,-1};
	//int vcols[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	int i,r;
	for(i=0;i<3;i++)
	{
		for(r=1;r<2;r++)
		{
			//struct razer_rgb color_black={.r=0,.g=0,.b=0};
			struct razer_rgb color={.r=50*vcols->values[i]->r,.g=50*vcols->values[i]->g,.b=50*vcols->values[i]->b};
			struct razer_rgb color2={.r=25*vcols->values[i]->r,.g=25*vcols->values[i]->g,.b=25*vcols->values[i]->b};
			razer_add_frame_column(render->output_frame,v->values[i],&color);
			razer_add_frame_column(render->output_frame,v->values[i]-1,&color2);
			razer_sub_frame_column(render->output_frame,v->values[i]-(vdir->values[i]*17),&color);
			razer_sub_frame_column(render->output_frame,v->values[i]-(vdir->values[i]*18),&color);
			razer_sub_frame_column(render->output_frame,v->values[i]-(vdir->values[i]*19),&color2);
			razer_sub_frame_column(render->output_frame,v->values[i]-(vdir->values[i]*20),&color2);
		}
		v->values[i]=v->values[i]+vdir->values[i];
		if(v->values[i]>27 || v->values[i]<-7)
			vdir->values[i]=-vdir->values[i];

	}
	razer_mix_frames(render->output_frame,render->input_frame,render->opacity);
	return(1);
}


struct razer_effect *effect_profile = NULL;
struct razer_rgb_frame *effect_profile_frame = NULL;


int effect_profile_update(struct razer_fx_render_node *render)
{
	/*struct razer_rgb color;
	//struct razer_chroma *chroma = render->daemon->chroma;
	int x,y;
	for(x=0;x<22;x++)
		for(y=0;y<6;y++)
		{
			color.r = (count+x)*(255/22);
			color.g = (count-x)*(255/22);
			color.b = (count+y)*(255/22);
			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&color,render->opacity);
			render->output_frame->update_mask |= 1<<y;
		}*/
	razer_mix_frames(render->output_frame,effect_profile_frame,1.0f-render->opacity);
	return(1);
}

int effect_profile_reset(struct razer_fx_render_node *render)
{
	FILE *effect_profile_file = NULL;
	//reopen input file
	char *filename = daemon_get_parameter_string(daemon_effect_get_parameter_by_index(render->effect,0));
	effect_profile_file = fopen(filename,"rb");
	if(!strlen(filename))
	{
		#ifdef USE_DEBUGGING
			printf("(profile) warning: no profile used\n");
		#endif		
		return(1);
	}

	if(!effect_profile_file)
	{
		#ifdef USE_DEBUGGING
			printf("(profile) error: opening profile:%s\n",filename);
		#endif		
		return(0);
	}
	fseek(effect_profile_file,0,SEEK_END);
	int fsize = ftell(effect_profile_file);
	rewind(effect_profile_file);
	printf("file size:%d,struct size:%d,struct size-int size:%d\n",fsize,sizeof(struct razer_rgb_frame),sizeof(struct razer_rgb_frame)-sizeof(int));
	effect_profile_frame->update_mask = 255;
	//if(!fread(effect_profile_frame,sizeof(struct razer_rgb_frame)-sizeof(int),1,effect_profile_file))
	if(!fread(effect_profile_frame,402,1,effect_profile_file))
	{
		#ifdef USE_DEBUGGING
			printf("(profile) error: reading profile:%s\n",filename);
		#endif		
		return(0);
	}
		

	fclose(effect_profile_file);
	#ifdef USE_DEBUGGING
		printf("(profile) opened profile:%s\n",filename);
	#endif		
	return(1);
}



/*
void test_effect6(struct razer_keys *keys)
{
	int count = 50;
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
		r = keys->rows[y[i]]->column[x[i]].r;
		g = keys->rows[y[i]]->column[x[i]].g;
		b = keys->rows[y[i]]->column[x[i]].r;
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

		keys->rows[y[i]]->column[x[i]].r = (unsigned char)r;
		keys->rows[y[i]]->column[x[i]].g = (unsigned char)g;
		keys->rows[y[i]]->column[x[i]].b = (unsigned char)b;
		keys->update_mask |= 1<<y[i];
	}
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
		}
		for(i=mx/2;i<mx;i++)
		{
			x[i]=(random()%21)+1;
			y[i]=(random()%5)+1;
			dir[i]=0;
			vr[i]=random()%vmax;
			vg[i]=random()%vmax;
			vb[i]=random()%vmax;
		}
	}
}

void test_effect3_frame(struct razer_keys *keys)
{
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
		keys->rows[y]->column[x].r = (unsigned char)r;
		keys->rows[y]->column[x].g = (unsigned char)g;
		keys->rows[y]->column[x].b = (unsigned char)b;
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
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
			if(keys->heatmap[y][x]>max_clicks)
				max_clicks = keys->heatmap[y][x];
		}
	}
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
		rgb_from_hue((float)keys->heatmap[y][x]/(float)max_clicks,0.3f,0.0f,&color);
		rgb_mix(&keys->rows[y]->column[x],&color,0.7f);
		}
	}
	keys->update_mask = 63;//update all rows
}
*/



/*
float scroll_x,scroll_y;
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
			rgb_mix(&keys->rows[y]->column[x],&color,0.9f);
			//keys->rows[y]->column[x].g = color.g;
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


#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

void fx_init(struct razer_daemon *daemon)
{
	struct razer_parameter *parameter = NULL;
	effect1 = daemon_create_effect();
	effect1->update = effect1_update;
	effect1->name = "First One";
	effect1->description = "First effect converted to the new render architecture";
	effect1->fps = 12;
	effect1->effect_class = 1;
	effect1->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;
	parameter = daemon_create_parameter_int("End Counter","End of animation (Integer)",44);//TODO refactor to daemon_add_effect_parameter_int(effect,key,desc,value)
	daemon_effect_add_parameter(effect1,parameter);	
	parameter = daemon_create_parameter_int("Effect Counter","Counter value(INT)",0);
	daemon_effect_add_parameter(effect1,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction","Effect direction value(INT)",1);
	daemon_effect_add_parameter(effect1,parameter);	

	int effect1_uid = daemon_register_effect(daemon,effect1);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect1->name,effect1->id);
	#endif

	effect2 = daemon_create_effect();
	effect2->update = effect2_update;
	effect2->name = "Breathing Color";
	effect2->description = "Color sweep from totally dimmed to full brightness";
	effect2->fps = 15;
	effect2->effect_class = 1;
	effect2->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;
	
	parameter = daemon_create_parameter_rgb("Base Color","Flashing Color (RGB)",&effect2_base_rgb);
	daemon_effect_add_parameter(effect2,parameter);	
	parameter = daemon_create_parameter_int("Effect Counter","Counter value(INT)",0);
	daemon_effect_add_parameter(effect2,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction","Effect direction value(INT)",1);
	daemon_effect_add_parameter(effect2,parameter);	
	int effect2_uid = daemon_register_effect(daemon,effect2);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect2->name,effect2->id);
	#endif

	effect3 = daemon_create_effect();
	effect3->update = effect3_update;
	effect3->name = "Random Color Waves";
	effect3->description = "Floating color waves";
	effect3->fps = 13;
	effect3->effect_class = 1;
	effect3->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;

	parameter = daemon_create_parameter_int("Effect Counter","Counter value(INT)",0);
	daemon_effect_add_parameter(effect3,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction","Effect direction value(INT)",1);
	daemon_effect_add_parameter(effect3,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization R","Red randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect3,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization G","Green randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect3,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization B","Blue randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect3,parameter);	

	int effect3_uid = daemon_register_effect(daemon,effect3);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect3->name,effect3->id);
	#endif

	effect4 = daemon_create_effect();
	effect4->update = effect4_update;
	effect4->name = "Random Color Waves #2";
	effect4->description = "Floating color waves";
	effect4->fps = 12;
	effect4->effect_class = 1;
	effect4->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;

	parameter = daemon_create_parameter_int("Effect Counter R","Red counter value(INT)",1);
	daemon_effect_add_parameter(effect4,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction R","Red effect direction value(INT)",1);
	daemon_effect_add_parameter(effect4,parameter);	
	parameter = daemon_create_parameter_int("Effect Counter G","Green counter value(INT)",254);
	daemon_effect_add_parameter(effect4,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction G","Green effect direction value(INT)",-1);
	daemon_effect_add_parameter(effect4,parameter);	
	parameter = daemon_create_parameter_int("Effect Counter B","Blue counter value(INT)",127);
	daemon_effect_add_parameter(effect4,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction B","Blue effect direction value(INT)",-1);
	daemon_effect_add_parameter(effect4,parameter);	


	parameter = daemon_create_parameter_int("Effect Randomization R","Red randomized parameter value(INT)",(random()%3)+1);
	daemon_effect_add_parameter(effect4,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization G","Green randomized parameter value(INT)",(random()%3)+1);
	daemon_effect_add_parameter(effect4,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization B","Blue randomized parameter value(INT)",(random()%3)+1);
	daemon_effect_add_parameter(effect4,parameter);	

	int effect4_uid = daemon_register_effect(daemon,effect4);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect4->name,effect4->id);
	#endif

	effect5 = daemon_create_effect();
	effect5->update = effect5_update;
	effect5->name = "Random Color Waves #3";
	effect5->description = "Floating color waves";
	effect5->fps = 15;
	effect5->effect_class = 1;
	effect5->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;

	parameter = daemon_create_parameter_int("Effect Counter","Counter value(INT)",0);
	daemon_effect_add_parameter(effect5,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction ","Effect direction value(INT)",1);
	daemon_effect_add_parameter(effect5,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization R","Red randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect5,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization G","Green randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect5,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization B","Blue randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect5,parameter);	

	int effect5_uid = daemon_register_effect(daemon,effect5);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect5->name,effect5->id);
	#endif

	effect6 = daemon_create_effect();
	effect6->update = effect6_update;
	effect6->name = "Random Color Waves #4";
	effect6->description = "Floating color waves";
	effect6->fps = 12;
	effect6->effect_class = 1;
	effect6->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;

	parameter = daemon_create_parameter_int("Effect Counter","Counter value(INT)",0);
	daemon_effect_add_parameter(effect6,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction ","Effect direction value(INT)",1);
	daemon_effect_add_parameter(effect6,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization R","Red randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect6,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization G","Green randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect6,parameter);	
	parameter = daemon_create_parameter_int("Effect Randomization B","Blue randomized parameter value(INT)",random());
	daemon_effect_add_parameter(effect6,parameter);	

	int effect6_uid = daemon_register_effect(daemon,effect6);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect6->name,effect6->id);
	#endif

	effect7 = daemon_create_effect();
	effect7->update = effect7_update;
	effect7->name = "Copper Bars #4";
	effect7->description = "Moving copper bars";
	effect7->fps = 14;
	effect7->effect_class = 1;
	effect7->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;

	parameter = daemon_create_parameter_int("Effect Counter","Counter value(INT)",1);
	daemon_effect_add_parameter(effect7,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction ","Effect direction value(INT)",1);
	daemon_effect_add_parameter(effect7,parameter);	

	int effect7_uid = daemon_register_effect(daemon,effect7);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect7->name,effect7->id);
	#endif

	effect8 = daemon_create_effect();
	effect8->update = effect8_update;
	effect8->name = "Arrayi Bars";
	effect8->description = "Test Array like usage";
	effect8->fps = 30;
	effect8->effect_class = 1;
	effect8->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;

	//int v[3]={1,12,22};
	//int vdir[3] = {1,1,1};
	//int vcols[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	effect8_v = daemon_create_int_array(3,1);
	effect8_vdir = daemon_create_int_array(3,1);
	effect8_vcols = daemon_create_rgb_array(3,1);
	effect8_v->values[0] = 1;
	effect8_v->values[1] = 12;
	effect8_v->values[2] = 22;
	effect8_vdir->values[0] = 1;
	effect8_vdir->values[1] = 1;
	effect8_vdir->values[2] = 1;
	effect8_vcol1 = rgb_create(255,0,0);
	effect8_vcol2 = rgb_create(0,255,0);
	effect8_vcol3 = rgb_create(0,0,255);
	effect8_vcols->values[0] = effect8_vcol1;
	effect8_vcols->values[1] = effect8_vcol2;
	effect8_vcols->values[2] = effect8_vcol3;
	parameter = daemon_create_parameter_int_array("Effect Counter Array","Counter values(int array)",effect8_v);
	daemon_effect_add_parameter(effect8,parameter);	
	parameter = daemon_create_parameter_int_array("Effect Direction Array","Direction values(int array)",effect8_vdir);
	daemon_effect_add_parameter(effect8,parameter);	
	parameter = daemon_create_parameter_rgb_array("Effect Colors Array","Base colors(rgb array)",effect8_vcols);
	daemon_effect_add_parameter(effect8,parameter);	

	int effect8_uid = daemon_register_effect(daemon,effect8);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect8->name,effect8->id);
	#endif



	effect9 = daemon_create_effect();
	effect9->update = effect9_update;
	effect9->name = "Arrayi Bars 2";
	effect9->description = "Test Array like usage";
	effect9->fps = 35;
	effect9->effect_class = 1;
	effect9->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;

	//int v[3]={1,12,16};
	//int vdir[3] = {1,1,-1};
	//int vcols[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	effect9_v = daemon_create_int_array(3,1);
	effect9_vdir = daemon_create_int_array(3,1);
	effect9_vcols = daemon_create_rgb_array(3,1);
	effect9_v->values[0] = 1;
	effect9_v->values[1] = 12;
	effect9_v->values[2] = 16;
	effect9_vdir->values[0] = 1;
	effect9_vdir->values[1] = 1;
	effect9_vdir->values[2] = -1;
	effect9_vcol1 = rgb_create(255,0,0);
	effect9_vcol2 = rgb_create(0,255,0);
	effect9_vcol3 = rgb_create(0,0,255);
	effect9_vcols->values[0] = effect9_vcol1;
	effect9_vcols->values[1] = effect9_vcol2;
	effect9_vcols->values[2] = effect9_vcol3;
	parameter = daemon_create_parameter_int_array("Effect Counter Array","Counter values(int array)",effect8_v);
	daemon_effect_add_parameter(effect9,parameter);	
	parameter = daemon_create_parameter_int_array("Effect Direction Array","Direction values(int array)",effect8_vdir);
	daemon_effect_add_parameter(effect9,parameter);	
	parameter = daemon_create_parameter_rgb_array("Effect Counter Array","Base colors(rgb array)",effect8_vcols);
	daemon_effect_add_parameter(effect9,parameter);	

	int effect9_uid = daemon_register_effect(daemon,effect9);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect9->name,effect9->id);
	#endif


	effect_mix1 = daemon_create_effect();
	effect_mix1->update = effect1_update;
	effect_mix1->name = "Default Mixer";
	effect_mix1->description = "Standard effect mixer";
	effect_mix1->fps = 12;
	effect_mix1->effect_class = 2;
	effect_mix1->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED | RAZER_EFFECT_SECOND_INPUT_USED;
	int effect_mix1_uid = daemon_register_effect(daemon,effect_mix1);
	#ifdef USE_DEBUGGING
		printf("registered mix effect: %s (uid:%d)\n",effect_mix1->name,effect_mix1->id);
	#endif


	effect_profile_frame = razer_create_rgb_frame(22,6);//TODO
	effect_profile = daemon_create_effect();
	effect_profile->update = effect_profile_update;
	effect_profile->reset = effect_profile_reset;
	effect_profile->name = "Profile display";
	effect_profile->description = "loads and displays profiles on the keyboard";
	effect_profile->fps = 15;
	effect_profile->effect_class = 1;
	effect_profile->input_usage_mask = RAZER_EFFECT_NO_INPUT_USED;
	parameter = daemon_create_parameter_string("Effect Profile","Filepath pointing to the profile (STRING)","");
	daemon_effect_add_parameter(effect_profile,parameter);	
	int effect_uid = daemon_register_effect(daemon,effect_profile);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect_profile->name,effect_profile->id);
	#endif



}

#pragma GCC diagnostic pop


void fx_shutdown(struct razer_daemon *daemon)
{
	daemon_unregister_effect(daemon,effect1);
	daemon_free_parameters(effect1->parameters);
	daemon_free_effect(effect1);

	daemon_unregister_effect(daemon,effect2);
	daemon_free_parameters(effect2->parameters);
	daemon_free_effect(effect2);

	daemon_unregister_effect(daemon,effect3);
	daemon_free_parameters(effect3->parameters);
	daemon_free_effect(effect3);

	daemon_unregister_effect(daemon,effect4);
	daemon_free_parameters(effect4->parameters);
	daemon_free_effect(effect4);

	daemon_unregister_effect(daemon,effect5);
	daemon_free_parameters(effect5->parameters);
	daemon_free_effect(effect5);

	daemon_unregister_effect(daemon,effect6);
	daemon_free_parameters(effect6->parameters);
	daemon_free_effect(effect6);

	daemon_unregister_effect(daemon,effect7);
	daemon_free_parameters(effect7->parameters);
	daemon_free_effect(effect7);

	daemon_unregister_effect(daemon,effect8);
	daemon_free_parameters(effect8->parameters);
	daemon_free_effect(effect8);

	daemon_unregister_effect(daemon,effect9);
	daemon_free_parameters(effect9->parameters);
	daemon_free_effect(effect9);

	daemon_unregister_effect(daemon,effect_mix1);
	daemon_free_parameters(effect_mix1->parameters);
	daemon_free_effect(effect_mix1);

	daemon_unregister_effect(daemon,effect_profile);
	daemon_free_parameters(effect_profile->parameters);
	daemon_free_effect(effect_profile);

}
