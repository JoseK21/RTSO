#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
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
  int id;
  char modifyWhich; // which variable to modify, 'X' or 'Y'
  bool isUsed;
  bool isActived;
  ALLEGRO_THREAD *thread_id;
  struct node *next;
} node; // };

struct report
{
  int data;
  struct report *next;
};

struct report *head_report = NULL;

#define DATA_NEWINIT (       \
    (node){                  \
        .mutex = NULL,       \
        .cond = NULL,        \
        .posiX = 0,          \
        .posiY = 0,          \
        .modifyWhich = '\0', \
        .energy = 0,         \
        .period = 0,         \
        .id = 0,             \
        .next = NULL,        \
        .isActived = false,  \
        .thread_id = NULL,   \
        .isUsed = false})

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
int __algorithm;
int __mode;
int __start_auto = 0; // 0 : false

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
/* REPORT */
void printReport()
{
  struct report *ptrReport = head_report;
  printf("\n< ");

  //start from the beginning
  while (ptrReport != NULL)
  {
    printf("[%d]", ptrReport->data);
    ptrReport = ptrReport->next;
  }
  printf(" >\n");
}

void addLast(struct report **head, int val)
{
  //create a new node
  struct report *newNode = malloc(sizeof(struct report));
  newNode->data = val;
  newNode->next = NULL;

  //if head is NULL, it is an empty list
  if (*head == NULL)
    *head = newNode;
  //Otherwise, find the last report and add the newNode
  else
  {
    struct report *lastNode = *head;

    //last report's next address will be NULL.
    while (lastNode->next != NULL)
    {
      lastNode = lastNode->next;
    }

    //add the newNode at the end of the linked list
    lastNode->next = newNode;
  }
}

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
  link->id = length() + 1;
  link->isActived = false;
  link->isUsed = false;
  link->next = head; //point it to old first node
  head = link;       //point first to new first node
  ALLEGRO_THREAD *martianThread_ = al_create_thread(Func_Thread, link);
  link->thread_id = martianThread_; // Add ID Thread to martian struct
  printf("\nMartian inserted...\n");
  al_start_thread(martianThread_); // Start Martian Thread
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

//findMartianID a link with given energy
struct node *findMartianID(int id)
{
  node *current = head; //start from the first link
  if (head == NULL)     //if list is empty
    return NULL;

  while (current->id != id) //navigate through list
  {
    if (current->next == NULL)
      return NULL; //if it is last node
    else
      current = current->next; //go to next link
  }
  return current; //if id found, return the current Link
}

struct node *findLessEnergyMartian()
{
  if (head == NULL) //if list is empty
    return NULL;

  node *current = head;       //start from the first link
  int energy_1 = 1000;        // Defaul Value
  node *martianTemp = head;   // Header List
  while (martianTemp != NULL) //start from the beginning
  {
    if (martianTemp->energy <= energy_1 && martianTemp->isUsed == false)
    {
      energy_1 = martianTemp->energy;
      current = martianTemp;
    }
    martianTemp = martianTemp->next; // Follow martian
  }

  if (current->isUsed == true)
    return NULL;  //if energy found, return the current Link
  return current; //if energy found, return the current Link
}

void resetPeriodTime(int sGame)
{
  if (head != NULL)
  {
    node *martianTemp = head; // Header List
    int residuo = 0;
    while (martianTemp != NULL) //start from the beginning
    {
      residuo = sGame % martianTemp->period;
      if (residuo == 0)
      {
        martianTemp->isUsed = false;
        printf("sGame : %d", sGame);
        printf("\tMartian Reset Calendar : %d\n", martianTemp->id);
      }
      martianTemp = martianTemp->next; // Follow martian
    }
  }
}

void endGame()
{
  al_show_native_message_box(display, "End of the game", ">>>>> Thanks <<<<<", "See you later..", NULL, ALLEGRO_MESSAGEBOX_QUESTION);
}
// ---------------------------------------------------------------------------
// Implementation of MAIN Module
// ---------------------------------------------------------------------------
float formuleRM()
{
  int Ci = 0;
  int Pi = 0;

  node *current;
  for (current = head; current != NULL; current = current->next)
  {
    Ci = Ci + current->energy;
    Pi = Pi + current->period;
  }

  printf("\tCi : %d\n", Ci);
  printf("\tPi : %d\n", Pi);
  float ui = (float)Ci / (float)Pi;
  printf("\ttCi/Pi : %f\n", ui);

  if (Pi == 0)
    return -1;
  else
    return ui;
}

int parameterAlgorithm(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Atention!, Please enter two input values (Algorithm and Mode)\n");
    return 0;
  }

  if (!strcmp(argv[1], "RM"))
  {
    __algorithm = 0;
    if (!strcmp(argv[2], "Manual"))
    {
      __mode = 0;
      return 1;
    }
    else if (!strcmp(argv[2], "Auto"))
    {
      __mode = 1;
      return 1;
    }
    else
    {
      printf("Mode error: <Manual or Auto>\n");
      return 0;
    }
  }
  else if (!strcmp(argv[1], "EDF"))
  {
    __algorithm = 1;
    if (!strcmp(argv[2], "Manual"))
    {
      __mode = 0;
      return 1;
    }
    else if (!strcmp(argv[2], "Auto"))
    {
      __mode = 1;
      return 1;
    }
    else
    {
      printf("Mode error: <Manual or Auto>\n");
      return 0;
    }
  }
  else
  {
    printf("Algorithm error: <RM or EDF>\n");
    return 0;
  }
}

int main(int argc, char *argv[])
{
  if (!parameterAlgorithm(argc, argv))
    return 0;
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

  // Start timer
  al_start_timer(timer);

  // Event loop
  int code = CODE_CONTINUE;
  char snum[5];
  char senergy[5];
  char speriod[5];
  char smode[10];
  char smmove[10];
  char energyLine[10] = "";
  int s30 = 0;
  int sGame = 0;
  int martianMovement = -1;
  char ch = '>';
  int current_seconds = 0;
  int preview_energy = 0;
  float u = 0;

  ALLEGRO_THREAD *current_thread = NULL;
  node *foundLessEM;

  while (code == CODE_CONTINUE)
  {
    al_draw_bitmap(maze_img, POINT);

    if (RedrawIsReady() && al_is_event_queue_empty(event_queue))
    {

      if (__mode == 0 || __start_auto == 1)
      {
        /* code */

        s30++;
        if (s30 >= 30) // un secundo
        {
          s30 = 0;
          sGame++;

          if (strlen(energyLine) > 9)
            strncpy(energyLine, "", 10);
          /*         u = formuleRM();
        if (u != -1 && u <= 0.69314718056)
        {
          printf("Si calendarizable u: %f\n", u);
        }
        else
        {
          printf("No (Error RM) u: %f\n", u);
        } */

          if (current_seconds == 0) // no hay marciano en ejecucion
          {
            if (foundLessEM != NULL) // valida si existe un hilo anterior (para detenerlo)
            {
              strncpy(energyLine, "", 10);
              al_lock_mutex(foundLessEM->mutex); // Lock Mutex

              foundLessEM->isActived = false;
              foundLessEM->isUsed = true;
              foundLessEM->energy = preview_energy;
              al_unlock_mutex(foundLessEM->mutex); // UnLock Mutex
              printf(">>> >>> Thread Stopped\n");
              foundLessEM = NULL;
            }
            else
            {
              // Reset energy - period
              resetPeriodTime(sGame);
              foundLessEM = findLessEnergyMartian();
              if (foundLessEM != NULL)
              {
                printf("\n%d.Martian found (LESS):  <%d, %d> isUsed(%d)\n", foundLessEM->id, foundLessEM->energy, foundLessEM->period, foundLessEM->isUsed);
                current_seconds = foundLessEM->energy;
                preview_energy = foundLessEM->energy;
                martianMovement = foundLessEM->id;
                addLast(&head_report, martianMovement);

                // Set shared MARTIAN - LOGIC MAZE HERE - EDIT modifyWhich
                al_lock_mutex(foundLessEM->mutex);   // Lock Mutex
                foundLessEM->modifyWhich = 'X';      // Martian Movement
                foundLessEM->isActived = true;       // Martian Thread
                al_unlock_mutex(foundLessEM->mutex); // UnLock Mutex
              }
              else
              {
                current_seconds = 0;
              }
            }
          }
          else if (current_seconds >= 0)
          {
            al_lock_mutex(foundLessEM->mutex);   // Lock Mutex
            foundLessEM->energy--;               // Martian Movement
            al_unlock_mutex(foundLessEM->mutex); // UnLock Mutex
            current_seconds--;
            strncat(energyLine, &ch, 1);
          }
          else
          {
            printf("Error : Negative current_seconds\n");
          }
        }
        renderListMartians();
      }

      if (martianMovement > 0)
      {
        sprintf(smode, "%d", martianMovement);
        al_draw_text(font, al_map_rgb(195, 145, 220), 440, 10, 0, smode);
      }

      al_draw_text(font, al_map_rgb(70, 70, 70), 10, 590, 0, "Mode: ");
      if (__mode == 0)
        al_draw_text(font, al_map_rgb(70, 70, 70), 55, 590, 0, "Manual");
      else
        al_draw_text(font, al_map_rgb(70, 70, 70), 55, 590, 0, "Auto");

      if (new_martian)
      {
        sprintf(senergy, "%d", energy);
        al_draw_text(font, al_map_rgb(195, 145, 220), 10, 10, 0, "Energy: ");
        al_draw_text(font, al_map_rgb(255, 255, 255), 70, 10, 0, senergy);

        sprintf(speriod, "%d", period);
        al_draw_text(font, al_map_rgb(195, 145, 220), 108, 10, 0, "Period: ");
        al_draw_text(font, al_map_rgb(255, 255, 255), 170, 10, 0, speriod);
      }

      al_draw_text(font, al_map_rgb(178, 178, 178), 460, 10, 0, "ENERGY: ");
      al_draw_text(font, al_map_rgb(195, 145, 220), 520, 10, 0, energyLine);

      al_flip_display();
    }
    ALLEGRO_EVENT ev;
    al_wait_for_event(event_queue, &ev);
    code = HandleEvent(ev);
  }

  // Clean up resources and exit with appropriate code
  endGame();
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
  //_martianData->isUsed = false;
  al_broadcast_cond(_martianData->cond);
  al_unlock_mutex(_martianData->mutex);
  while (!al_get_thread_should_stop(thr))
  {
    if (_martianData->isActived == true && _martianData->isUsed == false)
    {
      al_lock_mutex(_martianData->mutex);
      if (mw == 'X')
        _martianData->posiX += INCVAL;
      else
        _martianData->posiY += INCVAL;
      /* else
        assert(UNKNOWN_ERROR); */
      al_unlock_mutex(_martianData->mutex);
      al_rest(RESTVAL);
    }
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
    else if (ev.keyboard.keycode == ALLEGRO_KEY_S)
    {
      if (__mode == 1)
        __start_auto = 1;
    }
    else if (ev.keyboard.keycode == ALLEGRO_KEY_R)
    {
      printReport();
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
        if (energy != 0 && period != 0 && energy < period)
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