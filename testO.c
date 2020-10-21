#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
// ---------------------------------------------------------------------------
// Header of DATA module
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
typedef struct //struct node
{
  ALLEGRO_MUTEX *mutex;
  ALLEGRO_COND *cond;
  float posiX;
  float posiY;

  int data;
  int key;

  char modifyWhich; // which variable to modify, 'X' or 'Y'
  bool ready;
  struct node *next;
} node; // };

typedef struct
{
  ALLEGRO_MUTEX *mutex;
  ALLEGRO_COND *cond;
  float posiX;
  float posiY;

  int data;
  int key;

  char modifyWhich; // which variable to modify, 'X' or 'Y'
  bool ready;
  struct node *next;

} DATA;

#define DATA_NEWINIT (       \
    (DATA){                  \
        .mutex = NULL,       \
        .cond = NULL,        \
        .posiX = 0,          \
        .posiY = 0,          \
        .modifyWhich = '\0', \
        .key = 0,            \
        .data = 0,           \
        .next = NULL,        \
        .ready = false})

DATA *DATA_new(void);
void DATA_delete(DATA *self);
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Header of MAIN module
// ---------------------------------------------------------------------------
const float FPS = 30;
const int SCREEN_W = 608;
const int SCREEN_H = 608;
const int BOUNCER_SIZE = 32;
ALLEGRO_BITMAP *bouncer;
ALLEGRO_DISPLAY *display;
#define BLACK (al_map_rgb(0, 0, 0))
#define MAGENTA (al_map_rgb(255, 0, 255))
static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg);

// ---------------------------------------------------------------------------
// Header of EVENT HANDLER Module
// ---------------------------------------------------------------------------
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CODE_CONTINUE (MIN(EXIT_SUCCESS, EXIT_FAILURE) - 1)
#define CODE_SUCCESS EXIT_SUCCESS
#define CODE_FAILURE EXIT_FAILURE
typedef int code;
code HandleEvent(ALLEGRO_EVENT ev);
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Header of REDRAW HANDLER Module
// ---------------------------------------------------------------------------
bool REDRAW_IS_READY;
void RedrawSetReady(void);
void RedrawClearReady(void);
bool RedrawIsReady(void);
void RedrawDo(); // to be defined in Implementation of EVENT HANDLER Module
// ---------------------------------------------------------------------------

struct node *head = NULL;
struct node *current = NULL;
// ---------------------------------------------------------------------------

//display the list
void printList()
{
  node *ptr = head;
  printf("\n[ ");
  while (ptr != NULL) //start from the beginning
  {
    printf("(%d,%d) ", ptr->key, ptr->data);
    ptr = ptr->next;
  }
  printf(" ]\n");
}

//render list
void renderList()
{
  node *ptr = head;
  while (ptr != NULL) //start from the beginning
  {
    printf("<%d , %d> \n", ptr->key, ptr->data);
    RedrawDo(ptr, bouncer);
    ptr = ptr->next;
  }
}

//insert link at the first location
void insertFirst(int key, int data)
{
  node *link = (struct node *)malloc(sizeof(node)); //create a link

  // ** lo meti yo
  assert(link);
  //*link = DATA_NEWINIT;
  link->mutex = al_create_mutex();
  link->cond = al_create_cond();
  assert(link->mutex);
  assert(link->cond);
  // **
  link->key = key;
  link->data = data;
  link->next = head; //point it to old first node
  head = link;       //point first to new first node
  printf("\nInsert it\n");
}

//is list empty
bool isEmpty()
{
  return head == NULL;
}

int length()
{
  int length = 0;
  node *current;
  for (current = head; current != NULL; current = current->next)
    length++;
  return length;
}

//find a link with given key
struct node *find(int key)
{
  node *current = head; //start from the first link
  if (head == NULL)     //if list is empty
    return NULL;

  while (current->key != key) //navigate through list
  {
    if (current->next == NULL)
      return NULL; //if it is last node
    else
      current = current->next; //go to next link
  }
  return current; //if data found, return the current Link
}

// ---------------------------------------------------------------------------
// Implementation of MAIN Module
// ---------------------------------------------------------------------------
int main()
{
  // Initialize Allegro, mouse, timer, display and bitmap
  // XXX - skimping on Allegro error-checking
  al_init();
  if (!al_install_keyboard())
  {
    printf("couldn't initialize keyboard\n");
    return 1;
  }
  if (!al_init_image_addon())
  {
    printf("couldn't initialize image addon\n");
    return 1;
  }
  ALLEGRO_FONT *font = al_create_builtin_font();
  if (!font)
  {
    printf("couldn't initialize font\n");
    return 1;
  }

  ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
  display = al_create_display(SCREEN_W, SCREEN_H);
  bouncer = al_create_bitmap(BOUNCER_SIZE, BOUNCER_SIZE);
  ALLEGRO_BITMAP *maze = al_load_bitmap("../src/maze.png");

  al_set_target_bitmap(bouncer);
  al_clear_to_color(MAGENTA);
  al_set_target_bitmap(al_get_backbuffer(display));
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  al_register_event_source(event_queue, al_get_display_event_source(display));
  al_register_event_source(event_queue, al_get_timer_event_source(timer));
  al_register_event_source(event_queue, al_get_keyboard_event_source());
  al_clear_to_color(BLACK);
  al_flip_display();
  // DATA *data = DATA_new();

  insertFirst(50, 10); // 50 key
                       // Meter en lista ---------------------------------------------------------------------------------------------------------------------------------
  node *foundLink = find(50);

  if (foundLink != NULL)
  {
    printf("Element found: ");
    printf("(%d,%d) ", foundLink->key, foundLink->data);
    printf("\n");
  }
  else
  {
    printf("Element not found.");
  }

  // Obtener data de lista ---------------------------------------------------------------------------------------------------------------------------------

  // Start timer
  al_start_timer(timer);

  // Set shared DATA
  al_lock_mutex(foundLink->mutex);
  foundLink->modifyWhich = 'X';
  foundLink->ready = false;
  al_unlock_mutex(foundLink->mutex);

  // Initialize and start thread_1
  ALLEGRO_THREAD *thread_1 = al_create_thread(Func_Thread, foundLink);
  al_start_thread(thread_1);
  al_lock_mutex(foundLink->mutex);
  while (!foundLink->ready)
  {
    al_wait_cond(foundLink->cond, foundLink->mutex);
  }
  al_unlock_mutex(foundLink->mutex);

  // Set shared DATA
  al_lock_mutex(foundLink->mutex);
  foundLink->modifyWhich = 'Y';
  foundLink->ready = false;
  al_unlock_mutex(foundLink->mutex);

  // Initialize and start thread_2
  ALLEGRO_THREAD *thread_2 = al_create_thread(Func_Thread, foundLink);
  al_start_thread(thread_2);
  al_lock_mutex(foundLink->mutex);
  while (!foundLink->ready)
  {
    al_wait_cond(foundLink->cond, foundLink->mutex);
  }
  al_unlock_mutex(foundLink->mutex);

  // Event loop
  int code = CODE_CONTINUE;
  while (code == CODE_CONTINUE)
  {
    al_clear_to_color(al_map_rgb(255, 255, 255));
    al_draw_bitmap(maze, 0, 0, 0);
    if (RedrawIsReady() && al_is_event_queue_empty(event_queue))
    {

      //RedrawDo(data, bouncer);
      renderList();
      sleep(1);
    }
    ALLEGRO_EVENT ev;
    al_wait_for_event(event_queue, &ev);
    code = HandleEvent(ev);
  }

  // Clean up resources and exit with appropriate code
  al_destroy_thread(thread_1);
  /*  al_destroy_thread(thread_2); */
  al_destroy_bitmap(bouncer);
  al_destroy_timer(timer);
  al_destroy_display(display);
  al_destroy_event_queue(event_queue);
  return code;
}

#define INCVAL (0.1f)
#define RESTVAL (0.01f)
#define UNKNOWN_ERROR (0)
static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg)
{
  DATA *data = arg;
  al_lock_mutex(data->mutex);
  char mw = data->modifyWhich;
  data->ready = true;
  al_broadcast_cond(data->cond);
  al_unlock_mutex(data->mutex);
  while (!al_get_thread_should_stop(thr))
  {
    al_lock_mutex(data->mutex);
    if (mw == 'X')
      data->posiX += INCVAL;
    else if (mw == 'Y')
      data->posiY += INCVAL;
    else
      assert(UNKNOWN_ERROR);
    al_unlock_mutex(data->mutex);
    al_rest(RESTVAL);
  }
  return NULL;
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of EVENT HANDLER Module
// ---------------------------------------------------------------------------
code HandleEvent(ALLEGRO_EVENT ev)
{
  switch (ev.type)
  {
  case ALLEGRO_EVENT_TIMER:
    RedrawSetReady();
    break;
  case ALLEGRO_EVENT_DISPLAY_CLOSE:
    return EXIT_SUCCESS;
  case ALLEGRO_EVENT_KEY_DOWN:
    if (ev.keyboard.keycode == ALLEGRO_KEY_X)
    {
      printf("FIN\n");
      return EXIT_SUCCESS;
    }
    else if (ev.keyboard.keycode == ALLEGRO_KEY_L)
    {
      printList();
    }
    else if (ev.keyboard.keycode == ALLEGRO_KEY_I)
    {
      insertFirst(50, 10);
    }
    break;
  default:
    break;
  }
  return CODE_CONTINUE;
}

void RedrawDo(DATA *data, ALLEGRO_BITMAP *bouncer)
{
  al_lock_mutex(data->mutex);
  float X = data->posiX;
  float Y = data->posiY;
  al_unlock_mutex(data->mutex);
  al_draw_bitmap(bouncer, X, Y, 0);
  al_flip_display();
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of DATA module
// ---------------------------------------------------------------------------
DATA *DATA_new(void)
{
  DATA *self = NULL;
  self = malloc(sizeof(*self));
  assert(self);
  *self = DATA_NEWINIT;
  self->mutex = al_create_mutex();
  self->cond = al_create_cond();
  assert(self->mutex);
  assert(self->cond);
  return self;
}

void DATA_delete(DATA *self)
{
  al_destroy_mutex(self->mutex);
  al_destroy_cond(self->cond);
  free(self);
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of REDRAW HANDLER Module
// ---------------------------------------------------------------------------
bool REDRAW_IS_READY = false;
void RedrawSetReady(void) { REDRAW_IS_READY = true; }
void RedrawClearReady(void) { REDRAW_IS_READY = false; }
bool RedrawIsReady(void)
{
  switch (REDRAW_IS_READY)
  {
  case true:
    RedrawClearReady();
    return true;
  default:
    return false;
  }
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------