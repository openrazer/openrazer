#ifndef _RAZER_DAEMON_LIBRARIES_H_
#define _RAZER_DAEMON_LIBRARIES_H_


struct daemon_lib
{
	int id;
	char *pathname;
	void *lib;
};


struct daemon_lib *daemon_load_fx_lib(struct razer_daemon *daemon,char *fx_pathname);
struct daemon_lib *daemon_create_lib(char *pathname,void *lib);
int daemon_register_lib(struct razer_daemon *daemon,struct daemon_lib *lib);



#endif