#include <stdio.h>
#include <string.h>
#include <kylestructs.h>

#include <server.h>
#include <response.h>
#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <string_map.h>
#include <post_delete.h>


struct response* delete_user(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* user_id = get_map_value_str(req->query, "id");

  ks_hashmap* user_info;
  if (user_id == NULL || (user_info = query_user_by_id(user_id)) == NULL)
  {
    // not found
    return response_error(STAT404);
  }
  ks_hashmap_delete(user_info);

  // check if client user name == target user name
  if (strcmp(user_id, req->client_info->user_id) != 0)
  {
    ks_hashmap* admin_info;
    if ((admin_info = query_administrator_by_user_id(req->client_info->user_id)) == NULL)
    {
      // permission denied
      ks_hashmap_delete(user_info);
      return response_error(STAT403);
    }
    else ks_hashmap_delete(admin_info);
  }

  // logout
  remove_token(req->client_info->token);

  // remove user from DB
  if (sql_delete_user(user_id) != 0)
  {
    log_err("delete_user(): failed on call to sql_delete_user(): user id=%s", user_id);
    ks_hashmap_delete(user_info);
    return response_error(STAT500);
  }

  return response_redirect("/home");
}


struct response* delete_post(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* post_id = get_map_value_str(req->query, "id");

  // get post info
  ks_hashmap* post_info;
  if (post_id == NULL || (post_info = query_post_by_id(post_id)) == NULL)
  {
    // not found
    return response_error(STAT404);
  }
  const char* author_id = get_map_value_str(post_info, FIELD_POST_AUTHOR_ID);
  const char* community_id = get_map_value_str(post_info, FIELD_POST_COMMUNITY_ID);

  // check if client user id == author id 
  if (strcmp(author_id, req->client_info->user_id) != 0)
  {
    // check if client is a moderator
    ks_hashmap* mod_info;
    if ((mod_info = query_moderator_by_user_id_community_id(req->client_info->user_id, community_id)) == NULL)
    {
      // check if client is an administrator
      ks_hashmap* admin_info;
      if ((admin_info = query_administrator_by_user_id(req->client_info->user_id)) == NULL)
      {
        // check if client owns this community
        ks_hashmap* community_info = query_community_by_id(community_id);
        const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
        if (strcmp(owner_id, req->client_info->user_id) != 0)
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
    log_err("delete_post(): failed on call to sql_delete_post(): post id=%s", post_id);
    ks_hashmap_delete(post_info);
    return response_error(STAT500);
  }

  // build redirect URI
  char uri[64];
  sprintf(uri, "/community?id=%s", community_id);
  ks_hashmap_delete(post_info);

  // change this to redirect to community page
  return response_redirect("/home");
}


struct response* delete_comment(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* comment_id = get_map_value_str(req->query, "id");

  // get comment info
  ks_hashmap* comment_info;
  if (comment_id == NULL || (comment_info = query_comment_by_id(comment_id)) == NULL)
  {
    // not found
    return response_error(STAT404);
  }
  const char* author_id = get_map_value_str(comment_info, FIELD_COMMENT_AUTHOR_ID);

  // check if client user id == author id 
  if (strcmp(author_id, req->client_info->user_id) != 0)
  {
    // check if client is a moderator
    ks_hashmap* mod_info;
    const char* community_id = get_map_value_str(comment_info, FIELD_COMMENT_COMMUNITY_ID);
    if ((mod_info = query_moderator_by_user_id_community_id(req->client_info->user_id, community_id)) == NULL)
    {
      // check if client is an administrator
      ks_hashmap* admin_info;
      if ((admin_info = query_administrator_by_user_id(req->client_info->user_id)) == NULL)
      {
        // check if client owns this community
        ks_hashmap* community_info = query_community_by_id(community_id);
        const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
        if (strcmp(owner_id, req->client_info->user_id) != 0)
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
    log_err("delete_comment(): failed on call to sql_delete_comment(): comment id=%s", comment_id);
    ks_hashmap_delete(comment_info);
    return response_error(STAT500);
  }

  // build redirect URI
  const char* post_id = get_map_value_str(comment_info, FIELD_COMMENT_POST_ID);
  char uri[64];
  sprintf(uri, "/post?id=%s", post_id);

  ks_hashmap_delete(comment_info);

  // redirect to post page
  return response_redirect(uri);
}


struct response* delete_community(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* community_id = get_map_value_str(req->query, "id");

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
  if (strcmp(owner_id, req->client_info->user_id) != 0)
  {
    // check if client is an administrator
    ks_hashmap* admin_info;
    if ((admin_info = query_administrator_by_user_id(req->client_info->user_id)) == NULL)
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
    log_err("delete_community(): failed on call to sql_delete_community(): community id=%s", community_id);
    ks_hashmap_delete(community_info);
    return response_error(STAT500);
  }
  ks_hashmap_delete(community_info);

  // redirect to post page
  return response_redirect("/communities");
}


struct response* delete_moderator(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* community_id = get_map_value_str(req->query, "cid");
  const char* user_id = get_map_value_str(req->query, "uid");

  // make sure community exists
  ks_hashmap* community_info;
  if ((community_info = query_community_by_id(community_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // make sure client is owner
  const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
  if (strcmp(req->client_info->user_id, owner_id) != 0)
  {
    ks_hashmap_delete(community_info);
    return response_error(STAT403);
  }
  ks_hashmap_delete(community_info);

  // check if moderator exists
  ks_hashmap* mod_info;
  if ((mod_info = query_moderator_by_user_id_community_id(user_id, community_id)) == NULL)
  {
    return response_error(STAT404);
  }
  ks_hashmap_delete(mod_info);

  // delete mod
  if (sql_delete_moderator(user_id, community_id) == -1)
  {
    log_err("delete_moderator(): failed on call to sql_delete_moderator(): user_id=%s, community_id=%s", user_id, community_id);
    return response_error(STAT500);
  }

  char uri[64];
  sprintf(uri, "/community?id=%s", community_id);
  return response_redirect(uri);
}
