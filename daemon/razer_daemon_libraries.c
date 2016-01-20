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
 #include "razer_daemon.h"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

//TODO check if library is already loaded

struct daemon_lib *daemon_load_fx_lib(struct razer_daemon *daemon,char *fx_pathname)
{
	#ifdef USE_DEBUGGING
		printf("loading fx lib: %s\n",fx_pathname);
	#endif
	void *lib = dlopen(fx_pathname,RTLD_LAZY);
	if(lib==NULL)
	{
		#ifdef USE_DEBUGGING
			printf("fx lib not loaded:%s (%s)\n",fx_pathname,dlerror());
		#endif
		return(NULL);
	} 
	else
	{
		void *sym = dlsym(lib,"fx_init");
		if(sym==NULL)
		{
			#ifdef USE_DEBUGGING
				printf("no init function found\n");
			#endif
		    dlclose(lib);
		    return(NULL);
		}
		else
		{
			razer_effect_init init = (razer_effect_init)sym;
			init(daemon);
		}
	}
	struct daemon_lib *dlib = daemon_create_lib(fx_pathname,lib);
	return(dlib);
}

#pragma GCC diagnostic pop

//int daemon_unload_fx_lib(struct razer_daemon *daemon,)


struct daemon_lib *daemon_create_lib(char *pathname,void *lib)
{
	struct daemon_lib *dlib = (struct daemon_lib*)malloc(sizeof(struct daemon_lib));
	dlib->id = 0;
	dlib->pathname = str_Copy(pathname);
	dlib->lib = lib;
	return(dlib);
}

int daemon_register_lib(struct razer_daemon *daemon,struct daemon_lib *lib)
{
	list_Push(daemon->libs,lib);
	lib->id = daemon->libs_uid++;
	return(lib->id);
}
