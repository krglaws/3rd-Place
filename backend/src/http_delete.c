#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <server.h>
#include <response.h>
#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <string_map.h>
#include <http_get.h>
#include <http_delete.h>


static struct response* delete_user(const char* user_id, const struct auth_token* client_info);

static struct response* delete_post(const char* post_id, const struct auth_token* client_info);

static struct response* delete_comment(const char* comment_id, const struct auth_token* client_info);

static struct response* delete_community(const char* community_id, const struct auth_token* client_info);


struct response* http_delete(struct request* req)
{
  if (req == NULL)
  {
    log_crit("http_get(): NULL request object");
  }

  if (strcmp(req->uri, "./delete_user") == 0)
  {
    const char* user_name = get_map_value_str(req->query, "user_name");
    return delete_user(user_name, req->client_info);
  }

  if (strcmp(req->uri, "./delete_post") == 0)
  {
    const char* post_id = get_map_value_str(req->query, "post_id");
    return delete_post(post_id, req->client_info);
  }

  if (strcmp(req->uri, "./delete_comment") == 0)
  {
    const char* comment_id = get_map_value_str(req->query, "comment_id");
    return delete_comment(comment_id, req->client_info);
  }

  if (strcmp(req->uri, "./delete_community") == 0)
  {
    const char* community_id = get_map_value_str(req->query, "community_id");
    return delete_community(community_id, req->client_info);
  }

  return response_error(STAT404);
}


static struct response* delete_user(const char* user_name, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  ks_hashmap* user_info;
  if (user_name == NULL || (user_info = query_user_by_name(user_name)) == NULL)
  {
    // not found
    return response_error(STAT404);
  }

  // check if client user name == target user name
  if (strcmp(user_name, client_info->user_name) != 0)
  {
    ks_hashmap* admin_info;
    if ((admin_info = query_administrator_by_user_id(client_info->user_id)) == NULL)
    {
      // permission denied
      ks_hashmap_delete(user_info);
      return response_error(STAT403);
    }
    else ks_hashmap_delete(admin_info);
  }

  // logout
  remove_token(client_info->token);

  // remove user from DB
  const char* user_id = get_map_value_str(user_info, FIELD_USER_ID);
  if (sql_delete_user(user_id) != 0)
  {
    log_err("delete_user(): failed on call to sql_delete_user(): user_id=%s", user_id);
    ks_hashmap_delete(user_info);
    return response_error(STAT500);
  }

  ks_hashmap_delete(user_info);

  return response_redirect("/home");
}


static struct response* delete_post(const char* post_id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get post info
  ks_hashmap* post_info;
  if (post_id == NULL || (post_info = query_post_by_id(post_id)) == NULL)
  {
    // not found
    return response_error(STAT404);
  }
  const char* author_id = get_map_value_str(post_info, FIELD_POST_AUTHOR_ID);

  // check if client user id == author id 
  if (strcmp(author_id, client_info->user_id) != 0)
  {
    // check if client is a moderator
    ks_hashmap* mod_info;
    const char* community_id = get_map_value_str(post_info, FIELD_POST_COMMUNITY_ID);
    if ((mod_info = query_moderator_by_community_id_user_id(community_id, client_info->user_id)) == NULL)
    {
      // check if client is an administrator
      ks_hashmap* admin_info;
      if ((admin_info = query_administrator_by_user_id(client_info->user_id)) == NULL)
      {
        // check if client owns this community
        ks_hashmap* community_info = query_community_by_id(community_id);
        const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
        if (strcmp(owner_id, client_info->user_id) != 0)
        {
          // permission denied
          ks_hashmap_delete(post_info);
          ks_hashmap_delete(community_info);
          return response_error(STAT403);    
        }
        ks_hashmap_delete(community_info);
      }
      else ks_hashmap_delete(admin_info);
    }
    else ks_hashmap_delete(mod_info);
  }

  // remove post from DB
  if (sql_delete_post(post_id) != 0)
  {
    log_err("delete_post(): failed on call to sql_delete_post(): post_id=%s", post_id);
    ks_hashmap_delete(post_info);
    return response_error(STAT500);
  }

  // build redirect URI
  const char* community_name = get_map_value_str(post_info, FIELD_POST_COMMUNITY_NAME);
  char uri[64];
  sprintf(uri, "/c/%s", community_name);

  ks_hashmap_delete(post_info);

  // change this to redirect to community page
  return response_redirect("/home");
}


static struct response* delete_comment(const char* comment_id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get comment info
  ks_hashmap* comment_info;
  if (comment_id == NULL || (comment_info = query_comment_by_id(comment_id)) == NULL)
  {
    // not found
    return response_error(STAT400);
  }
  const char* author_id = get_map_value_str(comment_info, FIELD_COMMENT_AUTHOR_ID);

  // check if client user id == author id 
  if (strcmp(author_id, client_info->user_id) != 0)
  {
    // check if client is a moderator
    ks_hashmap* mod_info;
    const char* community_id = get_map_value_str(comment_info, FIELD_COMMENT_COMMUNITY_ID);
    if ((mod_info = query_moderator_by_community_id_user_id(community_id, client_info->user_id)) == NULL)
    {
      // check if client is an administrator
      ks_hashmap* admin_info;
      if ((admin_info = query_administrator_by_user_id(client_info->user_id)) == NULL)
      {
        // check if client owns this community
        ks_hashmap* community_info = query_community_by_id(community_id);
        const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
        if (strcmp(owner_id, client_info->user_id) != 0)
        {
          // permission denied
          ks_hashmap_delete(comment_info);
          ks_hashmap_delete(community_info);
          return response_error(STAT403);    
        }
        ks_hashmap_delete(community_info);
      }
      else ks_hashmap_delete(admin_info);
    }
    else ks_hashmap_delete(mod_info);
  }

  // remove comment from DB
  if (sql_delete_comment(comment_id) != 0)
  {
    log_err("delete_comment(): failed on call to sql_delete_comment(): comment_id=%s", comment_id);
    ks_hashmap_delete(comment_info);
    return response_error(STAT500);
  }

  // build redirect URI
  const char* post_id = get_map_value_str(comment_info, FIELD_COMMENT_POST_ID);
  char uri[32];
  sprintf(uri, "/p/%s", post_id);

  ks_hashmap_delete(comment_info);

  // redirect to post page
  return response_redirect(uri);
}


static struct response* delete_community(const char* community_id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get community info
  ks_hashmap* community_info;
  if (community_id == NULL || (community_info = query_community_by_id(community_id)) == NULL)
  {
    // not found
    return response_error(STAT404);
  }
  const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);

  // verify client has permission
  // check if client user id == author id 
  if (strcmp(owner_id, client_info->user_id) != 0)
  {
    // check if client is an administrator
    ks_hashmap* admin_info;
    if ((admin_info = query_administrator_by_user_id(client_info->user_id)) == NULL)
    {
      // permission denied
      ks_hashmap_delete(community_info);
      return response_error(STAT403);
    }
    else ks_hashmap_delete(admin_info);
  }

  // remove community from DB
  if (sql_delete_community(community_id) != 0)
  {
    log_err("delete_community(): failed on call to sql_delete_community(): community_id=%s", community_id);
    ks_hashmap_delete(community_info);
    return response_error(STAT500);
  }
  ks_hashmap_delete(community_info);

  // redirect to post page
  return response_redirect("/communities");
}
