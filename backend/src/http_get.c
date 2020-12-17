#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <common.h>
#include <util.h>
#include <senderr.h>
#include <token_manager.h>
#include <sql_wrapper.h>
#include <get_user.h>
#include <get_post.h>
#include <get_community.h>
#include <http_get.h>


// used to signal a 500 error
static enum get_err gerr = 0;


struct response* http_get(struct request* req)
{
  if (req == NULL) return NULL;

  /* what are we getting? */
  char* content_type = NULL;
  char* content = NULL;
  int content_length = 0;
  if (strstr(req->uri, ".css"))
  {
    content_type = TEXTCSS;
    if (content = load_file(req->uri))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, ".js"))
  {
    content_type = APPJS;
    if (content = load_file(req->uri))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, ".ico"))
  {
    struct file_str* ico;
    content_type = IMGICO;
    if (ico = load_file_str(req->uri))
    {
      content = ico->contents;
      content_length = ico->len;
      free(ico);
    }
  }
  else if (strstr(req->uri, "./u/"))
  {
    content_type = TEXTHTML;
    if (content = get_user(req->uri + 4, req->client_info))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, "./c/"))
  {
    content_type = TEXTHTML;
    if (content = get_community(req->uri + 4, req->client_info))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, "./p/"))
  {
    content_type = TEXTHTML;
    if (content = get_post(req->uri + 4, req->client_info))
    {
      content_length = strlen(content);
    }
  }

  // do we have it?
  if (content_type == NULL || content == NULL)
  {
    if (content != NULL)
    {
      free(content);
    }

    // get error type
    enum get_err err = get_error();

    // redirect
    if (err == REDIRECT)
    {
      return signup_redirect();
    }

    // check for 500 error
    if (get_error() == 1)
    {
      return senderr(ERR_INTERNAL);
    }

    // default to 404
    return senderr(ERR_NOT_FOUND);
  }

  // prepare response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT200, CHARP, strlen(STAT200)));
  list_add(resp->header, datacont_new(content_type, CHARP, strlen(content_type)));
 
  char contlenline[80];
  int len = sprintf(contlenline, "Content-Length: %d\n", content_length);

  list_add(resp->header, datacont_new(contlenline, CHARP, len));
  resp->content = content;
  resp->content_length = content_length;

  return resp;
} // end http_get()


char* replace(char* template, const char* this, const char* withthat)
{
  char* location;

  // check if 'this' is in template
  if ((location = strstr(template, this)) == NULL)
  {
    // set error flag
    set_error(INTERNAL);
    log_err("replace(): no match for '%s' found in template", this);
    free(template);
    return NULL;
  }

  // figure out sizes
  int templen = strlen(template);
  int thislen = strlen(this);
  int withlen = strlen(withthat);
  int outputsize = (templen - thislen) + withlen + 1;

  // create output buffer
  char* output = malloc(outputsize);
  output[outputsize - 1] = '\0';

  // do the replacement
  int dist = (location - template); 
  memcpy(output, template, dist);
  memcpy(output + dist, withthat, withlen);
  memcpy(output + dist + withlen, template + dist + thislen, strlen(template + dist + thislen));

  free(template);

  return output;
} // end replace()


char* fill_nav_login(char* template, const struct token_entry* client_info)
{
  char user_link[64];

  if (client_info != NULL)
  {
    sprintf(user_link, "/u/%s", client_info->user_name);
    if ((template = replace(template, "{USER_LINK}", user_link)) == NULL ||
        (template = replace(template, "{USER_NAME}", client_info->user_name)) == NULL)
    {
      return NULL;
    }
  }

  else
  {
    if ((template = replace(template, "{USER_LINK}", "/signup")) == NULL ||
        (template = replace(template, "{USER_NAME}", "Sign Up")) == NULL)
    {
      return NULL;
    }
  }

  return template;
} // end fill_nav_login()


char* load_vote_wrapper(const char* type, const char* inner_html_path)
{
  // load vote wrapper file
  char* vote_wrapper;
  if ((vote_wrapper = load_file(HTML_VOTE_WRAPPER)) == NULL)
  {
    return NULL;
  }

  // load template file
  char* inner_html;
  if ((inner_html = load_file(inner_html_path)) == NULL)
  {
    free(vote_wrapper);
    return NULL;
  }

  if ((vote_wrapper = replace(vote_wrapper, "{CURRENT_ITEM}", inner_html)) == NULL ||
      (vote_wrapper = replace(vote_wrapper, "{ITEM_TYPE}", type)) == NULL ||
      (vote_wrapper = replace(vote_wrapper, "{ITEM_TYPE}", type)) == NULL)
  {
    free(inner_html);
    return NULL;
  }

  free(inner_html);
  return vote_wrapper;
} // end load_vote_wrapper()


enum vote_type check_for_vote(const enum vote_item_type item_type, const char* item_id, const char* user_id)
{
  char *upvote_query_fmt, *downvote_query_fmt;

  // are we looking for post votes or comment votes?
  if (item_type == POST_VOTE)
  {
    upvote_query_fmt = QUERY_POST_UPVOTE_BY_POSTID_USERID;
    downvote_query_fmt = QUERY_POST_DOWNVOTE_BY_POSTID_USERID;
  }
  else if (item_type == COMMENT_VOTE)
  {
    upvote_query_fmt = QUERY_COMMENT_UPVOTE_BY_COMMENTID_USERID;
    downvote_query_fmt = QUERY_COMMENT_DOWNVOTE_BY_COMMENTID_USERID;
  }
  else {
    log_crit("check_for_vote(): invalid item_type argument");
  }

  // query database for up votes
  char upvote_query[strlen(upvote_query_fmt) + 64];
  sprintf(upvote_query, upvote_query_fmt, item_id, user_id, item_id, user_id);
  list** upvote_query_result = query_database_ls(upvote_query);

  // query database for down votes
  char downvote_query[strlen(downvote_query_fmt) + 64];
  sprintf(downvote_query, downvote_query_fmt, item_id, user_id, item_id, user_id);
  list** downvote_query_result = query_database_ls(downvote_query);

  // NOTE:
  // might remove these check
  if (upvote_query_result[0] && downvote_query_result[0])
  {
    log_crit("check_for_vote(): downvote and upvote present for same %s_id=%s user_id=%s", item_type==POST_VOTE?"post":"comment", item_id, user_id);
  }
  if (upvote_query_result[0] && upvote_query_result[1] != NULL)
  {
    log_crit("check_for_vote(): multiple upvotes present for same %s_id=%s user_id=%s", item_type==POST_VOTE?"post":"comment", item_id, user_id);
  }
  if (downvote_query_result[0] && downvote_query_result[1] != NULL)
  {
    log_crit("check_for_vote(): multiple downvotes present for same %s_id=%s user_id=%s", item_type==POST_VOTE?"post":"comment", item_id, user_id);
  }

  // determine vote type
  enum vote_type result = upvote_query_result[0] ? UPVOTE : (downvote_query_result[0] ? DOWNVOTE : NOVOTE);

  // cleanup query results
  free(upvote_query_result[0]);
  free(downvote_query_result[0]);
  free(upvote_query_result);
  free(downvote_query_result);

  return result;
} // end check_for_vote()


void set__error(enum get_err err)
{
  gerr = err;
}


static int get_error()
{
  enum get_err err = gerr;
  gerr = NO_GET_ERR;
  return err;
}
