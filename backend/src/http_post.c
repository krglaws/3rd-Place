#include <stdlib.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <server.h>
#include <response.h>
#include <string_map.h>
#include <post_new.h>
#include <post_update.h>
#include <post_delete.h>
#include <http_post.h>


static void terminate_http_post();

static ks_hashmap* endpoints = NULL;


void init_http_post()
{
  endpoints = ks_hashmap_new(KS_CHARP, 32);

  // from post_new.c
  add_map_value_vp(endpoints, "./login", &login);
  add_map_value_vp(endpoints, "./logout", &logout);
  add_map_value_vp(endpoints, "./signup", &signup);
  add_map_value_vp(endpoints, "./vote", &vote);
  add_map_value_vp(endpoints, "./subscribe", &subscribe);
  add_map_value_vp(endpoints, "./new_comment", &new_comment);
  add_map_value_vp(endpoints, "./new_post", &new_post);
  add_map_value_vp(endpoints, "./new_community", &new_community);
  add_map_value_vp(endpoints, "./new_moderator", &new_moderator);

  // from post_update.c
  add_map_value_vp(endpoints, "./update_user_about", &update_user_about);
  add_map_value_vp(endpoints, "./update_user_password", &update_user_password);
  add_map_value_vp(endpoints, "./update_comment", &update_comment);
  add_map_value_vp(endpoints, "./update_post", &update_post);
  add_map_value_vp(endpoints, "./update_community_about", &update_community_about);

  // from post_delete.c
  add_map_value_vp(endpoints, "./delete_user", &delete_user);
  add_map_value_vp(endpoints, "./delete_post", &delete_post);
  add_map_value_vp(endpoints, "./delete_comment", &delete_comment);
  add_map_value_vp(endpoints, "./delete_community", &delete_community);
  add_map_value_vp(endpoints, "./delete_moderator", &delete_moderator);

  atexit(&terminate_http_post);
}


struct response* http_post(const struct request* req)
{
  struct response* (*func) (const struct request*);
  if ((func = get_map_value_vp(endpoints, req->uri)) != NULL)
  {
    return func(req);
  }

  return response_error(STAT404);
}


static void terminate_http_post()
{
  ks_hashmap_delete(endpoints);
}
