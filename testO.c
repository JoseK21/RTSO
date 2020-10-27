#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
// ---------------------------------------------------------------------------
// Header of MARTIAN module
// ---------------------------------------------------------------------------
typedef struct node //struct node
{
  ALLEGRO_MUTEX *mutex;
  ALLEGRO_COND *cond;
  float posiX;
  float posiY;
  int period;
  int energy;
  char modifyWhich; // which variable to modify, 'X' or 'Y'
  bool ready;
  ALLEGRO_THREAD *thread_id;
  struct node *next;
} node; // };

#define DATA_NEWINIT (       \
    (node){                  \
        .mutex = NULL,       \
        .cond = NULL,        \
        .posiX = 0,          \
        .posiY = 0,          \
        .modifyWhich = '\0', \
        .energy = 0,         \
        .period = 0,         \
        .next = NULL,        \
        .thread_id = NULL,   \
        .ready = false})

// ---------------------------------------------------------------------------
// Header of MAIN module
// ---------------------------------------------------------------------------
const float FPS = 30;
const int SCREEN_W = 608;
const int SCREEN_H = 608;
const int BOUNCER_SIZE = 32;
int new_martian = 0;
int energy = 0;
int period = 0;
ALLEGRO_BITMAP *martian_img;
ALLEGRO_DISPLAY *display;
#define BLACK (al_map_rgb(0, 0, 0))
#define POINT 0, 0, 0
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
int length();
bool REDRAW_IS_READY;
void RedrawSetReady(void);
void RedrawClearReady(void);
bool RedrawIsReady(void);
void RedrawDo(); // to be defined in Implementation of EVENT HANDLER Module
// ---------------------------------------------------------------------------

struct node *head = NULL;
struct node *current = NULL;
// ---------------------------------------------------------------------------

// display the list of martian - console
void printListMartians()
{
  node *ptr = NULL;
  ptr = head;
  printf("\n[ ");
  while (ptr != NULL) //start from the beginning
  {
    printf("(%d,%d) ", ptr->energy, ptr->period);
    ptr = ptr->next;
  }
  printf(" ]\n");
}

void freeMartians()
{
  node *ptr = NULL;
  ptr = head;
  while (ptr != NULL) //start from the beginning
  {
    al_destroy_thread(ptr->thread_id);
    ptr = ptr->next;
  }
}

//render list
void renderListMartians()
{
  node *martianTemp = head;   // Header List
  while (martianTemp != NULL) //start from the beginning
  {
    RedrawDo(martianTemp, martian_img); // Pain image martian on screen
    martianTemp = martianTemp->next;    // Follow martian
  }
}

void clearListMartians()
{
  head = NULL;
}

//insert link at the first location
void addMartian(int energy, int period)
{
  node *link = (node *)malloc(sizeof(node)); //create a link

  assert(link);
  *link = DATA_NEWINIT;            // Set default values
  link->mutex = al_create_mutex(); // Mutex
  link->cond = al_create_cond();   // Condition
  assert(link->mutex);
  assert(link->cond);
  link->energy = energy;
  link->period = period;
  link->next = head; //point it to old first node
  head = link;       //point first to new first node
  printf("\nInsert it\n");
  length();
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
  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  Length: %d\n", length);
  return length;
}

//findMartian a link with given energy
struct node *findMartian(int energy)
{
  node *current = head; //start from the first link
  if (head == NULL)     //if list is empty
    return NULL;

  while (current->energy != energy) //navigate through list
  {
    if (current->next == NULL)
      return NULL; //if it is last node
    else
      current = current->next; //go to next link
  }
  return current; //if energy found, return the current Link
}

struct node *findLessEnergyMartian()
{
  if (head == NULL) //if list is empty
    return NULL;

  node *current = head; //start from the first link
  int energy_1 = 1000;
  node *martianTemp = head;   // Header List
  while (martianTemp != NULL) //start from the beginning
  {
    if (martianTemp->energy <= energy_1)
    {
      energy_1 = martianTemp->energy;
      current = martianTemp;
    }
    martianTemp = martianTemp->next; // Follow martian
  }
  free(martianTemp);
  return current; //if energy found, return the current Link
}

// ---------------------------------------------------------------------------
// Implementation of MAIN Module
// ---------------------------------------------------------------------------
int main()
{
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
  ALLEGRO_FONT *font = al_create_builtin_font(); // Font Styles
  if (!font)
  {
    printf("couldn't initialize font\n");
    return 1;
  }

  ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);            // Clock : Timer - Allegro display screen
  display = al_create_display(SCREEN_W, SCREEN_H);              // Screen
  martian_img = al_load_bitmap("../src/martian.png");           // Martian Image
  ALLEGRO_BITMAP *maze_img = al_load_bitmap("../src/maze.png"); // Maze Image

  al_set_target_bitmap(martian_img);
  al_set_target_bitmap(al_get_backbuffer(display));
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  al_register_event_source(event_queue, al_get_display_event_source(display));
  al_register_event_source(event_queue, al_get_timer_event_source(timer));
  al_register_event_source(event_queue, al_get_keyboard_event_source());
  al_clear_to_color(BLACK);
  al_flip_display();

  addMartian(5, 10); // 5 energy
  // Meter en lista ---------------------------------------------------------------------------------------------------------------------------------
  node *foundLink = findMartian(5);

  if (foundLink != NULL)
    printf("Martian found:  (%d,%d) \n", foundLink->energy, foundLink->period);
  else
    printf("Element not found.\n");

  // Start timer
  al_start_timer(timer);

  // Set shared MARTIAN
  al_lock_mutex(foundLink->mutex);   // Lock Mutex
  foundLink->modifyWhich = 'X';      // Martian Movement
  foundLink->ready = false;          // Martian Condition
  al_unlock_mutex(foundLink->mutex); // UnLock Mutex

  /*   al_lock_mutex(foundLink->mutex);
  foundLink->thread_id = al_create_thread(Func_Thread, foundLink); // Add ID Thread to martian struct
  al_unlock_mutex(foundLink->mutex); */

  //al_start_thread(foundLink->thread_id); // Start Martian Thread

  // Event loop
  int code = CODE_CONTINUE;
  char snum[5];
  char energyLine[10] = "";
  int s30 = 0;
  char ch = '>';
  int current_seconds = 0;
  ALLEGRO_THREAD *current_thread = NULL;

  while (code == CODE_CONTINUE)
  {
    al_draw_bitmap(maze_img, POINT);

    if (RedrawIsReady() && al_is_event_queue_empty(event_queue))
    {
      s30++;
      if (s30 >= 30)
      {
        node *foundLessEM;
        strncat(energyLine, &ch, 1);
        s30 = 0;

        if (strlen(energyLine) > 11)
          strncpy(energyLine, "", 10);

        if (current_seconds == 0) // no hay marciano en ejecucion
        {
          if (current_thread != NULL) // valida si existe un hilo anterior (para detenerlo)
          {
            //foundLessEM->ready = true;
            al_set_thread_should_stop(current_thread);
            printf(">>> >>> Thread Stopped\n");
            current_thread = NULL;
          }
          else
          {
            foundLessEM = findLessEnergyMartian();
            if (foundLessEM != NULL)
            {
              printf("\nMartian found (LESS):  <%d, %d> \n", foundLessEM->energy, foundLessEM->period);
              current_seconds = foundLessEM->energy;
              //current_thread = foundLessEM->thread_id;

              // Set shared MARTIAN
              al_lock_mutex(foundLessEM->mutex);   // Lock Mutex
              foundLessEM->modifyWhich = 'X';      // Martian Movement
              foundLessEM->ready = false;          // Martian Condition
              al_unlock_mutex(foundLessEM->mutex); // UnLock Mutex

              ALLEGRO_THREAD *thread_1 = al_create_thread(Func_Thread, foundLessEM);
              current_thread = thread_1;

              //al_create_thread(Func_Thread, foundLessEM);
              //al_start_thread(foundLessEM->thread_id); // Start Martian Thread
               al_start_thread(current_thread);
            }
            else
            {
              current_seconds = 0;
            }
          }
        }
        else if (current_seconds >= 0)
        {
          current_seconds--;
        }
        else
        {
        }
      }

      /* GAME LOGIC HERE */

      renderListMartians();
      al_draw_text(font, al_map_rgb(178, 178, 178), 460, 10, 0, "ENERGY: ");
      al_draw_text(font, al_map_rgb(195, 145, 220), 520, 10, 0, energyLine);

      if (new_martian)
      {
        sprintf(snum, "%d", energy);
        al_draw_text(font, al_map_rgb(195, 145, 220), 10, 10, 0, "Energy: ");
        al_draw_text(font, al_map_rgb(255, 255, 255), 70, 10, 0, snum);

        sprintf(snum, "%d", period);
        al_draw_text(font, al_map_rgb(195, 145, 220), 108, 10, 0, "Period: ");
        al_draw_text(font, al_map_rgb(255, 255, 255), 170, 10, 0, snum);
      }

      al_flip_display();
    }
    ALLEGRO_EVENT ev;
    al_wait_for_event(event_queue, &ev);
    code = HandleEvent(ev);
  }

  // Clean up resources and exit with appropriate code

  freeMartians();      // Free source thread (Martian List)
  clearListMartians(); // Delete Martian List

  al_destroy_bitmap(martian_img);
  al_destroy_bitmap(maze_img);
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
  node *_martianData = arg;
  al_lock_mutex(_martianData->mutex);
  char mw = _martianData->modifyWhich;
  _martianData->ready = true;
  al_broadcast_cond(_martianData->cond);
  al_unlock_mutex(_martianData->mutex);
  while (!al_get_thread_should_stop(thr))
  {
    al_lock_mutex(_martianData->mutex);
    if (mw == 'X')
      _martianData->posiX += INCVAL;
    else if (mw == 'Y')
      _martianData->posiY += INCVAL;
    else
      assert(UNKNOWN_ERROR);
    al_unlock_mutex(_martianData->mutex);
    al_rest(RESTVAL);
  }
  return NULL;
}

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
      printListMartians();
    else if (ev.keyboard.keycode == ALLEGRO_KEY_T)
      length();
    else if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
    {
      new_martian = 0;
      energy = 0;
      period = 0;
    }
    else if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
    {
      if (new_martian == 1) // Deshabilita la creacion de marcianos
      {
        if (energy != 0 && period != 0)
          addMartian(energy, period);
        new_martian = 0;
        energy = 0;
        period = 0;
      }
      else // Habilita la creacion de marcianos
        new_martian = 1;
    }
    else if (new_martian == 1)
    {
      if (ev.keyboard.keycode == ALLEGRO_KEY_E)
        energy++;
      else if (ev.keyboard.keycode == ALLEGRO_KEY_P)
        period++;
    }
    break;
  default:
    break;
  }
  return CODE_CONTINUE;
}

void RedrawDo(node *martianData, ALLEGRO_BITMAP *martian_img)
{
  al_lock_mutex(martianData->mutex);
  float X = martianData->posiX;
  float Y = martianData->posiY;
  al_unlock_mutex(martianData->mutex);
  al_draw_bitmap(martian_img, X, Y, 0);
}

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