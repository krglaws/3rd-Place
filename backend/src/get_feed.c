#include <stdlib.h>

#include <log_manager.h>
#include <util.h>
#include <sql_wrapper.h>
#include <http_get.h>
#include <get_feed.h>


char* get_feed(const char* home_or_popular, const struct token_entry* client_info)
{
  if (home_or_popular == NULL)
  {
    return NULL;
  }

  // load feed html
  char* feed_html;
  if ((feed_html = load_file(HTML_FEED)) == NULL)
  {
    // set internal error flag
    set_error(INTERNAL);
    return NULL;
  }

  // fill in feed html with correct feed type.
  if (strcmp(home_or_popular, "home") == 0)
  {
    // client is not signed in, redirect to /login
    if (client_info == NULL)
    {
      free(feed_html);
      set_error(REDIRECT);
      return NULL;
    }

    if ((feed_html = fill_home_feed(feed_html, client_info)) == NULL)
    {
      return NULL;
    }
  }
  else if (strcmp(home_or_popular, "popular") == 0)
  {
    if ((feed_html = fill_popular_feed(feed_html, client_info)) == NULL)
    {
      return NULL;
    }
  }
  else {
    free(feed_html);
    return NULL;
  }

  // load main html
  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    // set error flag
    set_error(INTERNAL);
    free(feed_html);
    return NULL;
  }

  // fill in main template
  if ((main_html = replace(main_html, "{STYLE}", CSS_FEED)) == NULL ||
      (main_html = replace(main_html, "{SCRIPT}", "")) == NULL ||
      (main_html = fill_nav_login(main_html, client_info)) == NULL ||
      (main_html = replace(main_html, "{PAGE_BODY}", feed_html)) == NULL)
  {
    free(user_html);
    return NULL;
  }

  // cleanup and return
  free(feed_html);

  /* Can't return anything here yet, need to get
      the stored procedure that retreives the feed
      posts working first. */
  free(main_html);
  return NULL;
}
