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
