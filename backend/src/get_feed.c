#include <stdlib.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <string_map.h>
#include <templating.h>
#include <http_get.h>
#include <get_feed.h>


static time_t get_post_date(ks_hashmap* post)
{
  const char* date_str = get_map_value(post, FIELD_POST_DATE_POSTED)->cp;
  return (time_t) strtol(date_str, NULL, 10);
}


static ks_list* bubble_sort(ks_list* posts)
{
  if (posts == NULL)
  {
    return NULL;
  }

  int count = ks_list_length(posts);

  // reverse post list
  // (odds are the posts at the beginning of
  // list are older)
  ks_list* rev_list = ks_list_new();
  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(posts, KS_LIST);
  while ((curr = ks_iterator_get(iter)) != NULL)
  {
    ks_list_insert(rev_list, ks_datacont_copy(curr), 0);
  }
  ks_iterator_delete(iter);
  ks_list_delete(posts);

  // bubble sort
  posts = rev_list;
  for (int i = 0; i < count; i++)
  {
    bool done = true;
    for (int j = 0; j < count-1-i; j++)
    {
      ks_datacont* postA = ks_list_get(posts, j);
      time_t tA = get_post_date(postA->hm);

      ks_datacont* postB = ks_list_get(posts, j+1);
      time_t tB = get_post_date(postB->hm);

      if (tA < tB)
      {
        done = false;

        // swap positions
        ks_datacont* temp = ks_datacont_copy(postA);
        ks_list_remove_at(posts, j);
        ks_list_insert(posts, temp, j + 1);
      }
    }
    if (done == true)
    {
      break;
    }
  }

  return posts;
}


static ks_list* merge_posts(ks_list* lsA, ks_list* lsB)
{
  if (lsA == NULL || lsB == NULL)
  {
    return (lsA ? lsA : (lsB ? lsB : NULL));
  }

  ks_list* merged = ks_list_new();

  ks_iterator* iterA = ks_iterator_new(lsA, KS_HASHMAP);
  ks_iterator* iterB = ks_iterator_new(lsB, KS_HASHMAP);
  ks_datacont* postA, *postB;

  do
  {
    postA = ks_datacont_copy(ks_iterator_get(iterA));
    time_t tA = postA ? get_post_date(postA->hm) : 0;

    postB = ks_datacont_copy(ks_iterator_get(iterB));
    time_t tB = postB ? get_post_date(postB->hm) : 0;

    ks_datacont* first = tA < tB ? postB : postA;
    ks_datacont* second = tA < tB ? postA : postB;

    if (first != NULL)
    {
      ks_list_add(merged, first);
    }

    if (second != NULL)
    {
      ks_list_add(merged, second);
    }
  } while (postA != NULL || postB != NULL);

  ks_iterator_delete(iterA);
  ks_iterator_delete(iterB);
  ks_list_delete(lsA);
  ks_list_delete(lsB);

  return merged;
}


static ks_list* wrap_posts(ks_list* posts, const struct auth_token* client_info)
{
  if (posts == NULL)
  {
    return NULL;
  }

  // list of vote wrappers
  ks_list* post_wrappers = ks_list_new();

  // iterate through each post
  ks_datacont* post_info;
  ks_iterator* iter = ks_iterator_new(posts, KS_LIST);
  while ((post_info = (ks_datacont*) ks_iterator_get(iter)) != NULL)
  {
    // add user post template path
    add_map_value_str(post_info->hm, TEMPLATE_PATH_KEY, HTML_FEED_POST);

    // move post points and ID into vote wrapper map
    const char* post_id = get_map_value(post_info->hm, FIELD_POST_ID)->cp;
    const char* points = get_map_value(post_info->hm, FIELD_POST_POINTS)->cp;

    ks_hashmap* wrapper = ks_hashmap_new(KS_CHARP, 8);
    add_map_value_str(wrapper, TEMPLATE_PATH_KEY, HTML_POST_VOTE_WRAPPER);
    add_map_value_str(wrapper, POST_ID_KEY, post_id);
    add_map_value_str(wrapper, POST_POINTS_KEY, points);

    // check if client user voted on this post
    char* upvote_class = UPVOTE_NOTCLICKED_STATE;
    char* downvote_class = DOWNVOTE_NOTCLICKED_STATE;
    if (client_info != NULL)
    {
      enum vote_type vt = check_for_vote(POST_VOTE, post_id, client_info->user_id);
      upvote_class = vt == UPVOTE ? UPVOTE_CLICKED_STATE : upvote_class;
      downvote_class = vt == DOWNVOTE ? DOWNVOTE_CLICKED_STATE : downvote_class;
    }
    add_map_value_str(wrapper, UPVOTE_CLICKED_KEY, upvote_class);
    add_map_value_str(wrapper, DOWNVOTE_CLICKED_KEY, downvote_class);

    // add post info map to vote wrapper map
    add_map_value_hm(wrapper, POST_KEY, post_info->hm);
    post_info->hm = NULL;
    ks_list_add(post_wrappers, ks_datacont_new(wrapper, KS_HASHMAP, 1));
  }
  ks_iterator_delete(iter);
  ks_list_delete(posts);

  return post_wrappers;
}


char* get_feed(const enum feed_type ft, const struct auth_token* client_info)
{
  if (ft == HOME_FEED && client_info == NULL)
  {
    set_error(REDIRECT);
    return NULL;
  }

  ks_hashmap* feed_info = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_str(feed_info, TEMPLATE_PATH_KEY, HTML_FEED);

  // get list
  ks_list* posts = NULL;
  if (ft == HOME_FEED)
  {
    add_map_value_str(feed_info, FEED_TITLE_KEY, "Home");

    // get list of subscriptions
    ks_list* subs;
    if ((subs = query_subscriptions_by_user_id(client_info->user_id)) != NULL)
    {
      posts = ks_list_new();

      // for each sub, get list of posts
      const ks_datacont* curr;
      ks_iterator* iter = ks_iterator_new(subs, KS_LIST);
      while ((curr = ks_iterator_get(iter)) != NULL)
      {
        const char* community_name = get_map_value(curr->hm, FIELD_SUB_COMMUNITY_NAME)->cp;
        ks_list* temp = query_posts_by_community_name(community_name);
        temp = bubble_sort(temp);
        posts = merge_posts(posts, temp);
      }
      ks_iterator_delete(iter);
      ks_list_delete(subs);
    }
  }
  else
  {
    add_map_value_str(feed_info, FEED_TITLE_KEY, "Popular");

    posts = query_all_posts();

    posts = bubble_sort(posts);
  }
  posts = wrap_posts(posts, client_info);

  // add feed list to feed_info map
  if (posts == NULL)
  {
    add_map_value_str(feed_info, FEED_POST_LIST_KEY, " ");
  }
  else
  {
    add_map_value_ls(feed_info, FEED_POST_LIST_KEY, posts);
  }

  // put page data together
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, feed_info);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_FEED);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, " ");
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_MAIN);
  add_nav_info(page_data, client_info);

  // build template
  char* html = build_template(page_data);
  ks_hashmap_delete(page_data);

  return html;
}
