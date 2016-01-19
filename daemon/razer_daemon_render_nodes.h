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
#ifndef _RAZER_DAEMON_RENDER_NODES_H_
#define _RAZER_DAEMON_RENDER_NODES_H_

struct razer_fx_render_node 
{
	int id;
	char *name;
	char *description;
	int compose_mode;
	struct razer_rgb_frame *input_frame;
	struct razer_rgb_frame *second_input_frame;
	struct razer_rgb_frame *output_frame;
	int input_frame_linked_uid;//-1==empty input frame buffer 
	int second_input_frame_linked_uid;//-1==empty input frame buffer 
	int output_frame_linked_uid;//-1==internal frame buffer used ; 0 == daemon output buffer is used
	struct razer_daemon *daemon;
	struct razer_chroma_device *device;
	struct razer_effect *effect;//create a copy of the effect struct for every render node
	float opacity;
	//float second_opacity;
	struct razer_fx_render_node *parent;
	list *subs;////struct razer_rx_render_node
	unsigned long start_ticks;
	int limit_render_time_ms;
	int running;//set to zero to stop node before next update
	int move_frame_buffer_linkage_to_next;//should the frame buffer linkage moved over if next node is getting activated
	//int continue_chain;
	int loop_count;// -1 == no looping
	struct razer_fx_render_node *prev;
	struct razer_fx_render_node *next;
};

char *daemon_render_node_to_json(struct razer_fx_render_node *render_node);

struct razer_fx_render_node *daemon_create_render_node(struct razer_chroma_device *device,struct razer_effect *effect,int input_render_node_uid,int second_input_render_node_uid,int output_render_node_uid,char *name,char *description);

int daemon_register_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node);
struct razer_fx_render_node *daemon_get_render_node(struct razer_daemon *daemon,int uid);


//int daemon_reset_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node);
int daemon_reset_render_node(struct razer_fx_render_node *render_node);
int daemon_render_node_fire_parameter_changed(struct razer_fx_render_node *render_node,struct razer_parameter *parameter);

void daemon_connect_frame_buffer(struct razer_chroma_device *device,struct razer_fx_render_node *render_node);
void daemon_disconnect_frame_buffer(struct razer_chroma_device *device);
void daemon_connect_input(struct razer_fx_render_node *render_node,struct razer_fx_render_node *input_node);
void daemon_connect_second_input(struct razer_fx_render_node *render_node,struct razer_fx_render_node *input_node);

void daemon_render_node_add_sub(struct razer_fx_render_node *render_node,struct razer_fx_render_node *sub_node);

int daemon_has_render_node_reached_render_limit(struct razer_daemon *daemon,struct razer_fx_render_node *render_node);

int daemon_update_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node);
int daemon_handle_event_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node,struct razer_chroma_event *event);

void daemon_set_default_render_node(struct razer_chroma_device *device,struct razer_fx_render_node *render_node);
#endif